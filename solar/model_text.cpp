#include "model_text.h"

#include <exception>
#include <sstream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"
#include "defer.h"
#include "glext.h"

namespace model
{
    void Text::init()
    {
        // 폰트 초기화
        if (FT_Init_FreeType(&this->ftLibrary))
            throw std::runtime_error("Could not init FreeType Library.");
        //defer(FT_Done_FreeType(this->ftLibrary));

        // 셰이더 초기화
        this->shader = glext::loadShader(FONT_SHADER);

        glUniform1i(glGetUniformLocation(this->shader, "text"), 0);

        // vbo vao 초기화
        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);
        defer(glBindVertexArray(0));

        glGenBuffers(1, &this->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        defer(glBindBuffer(GL_ARRAY_BUFFER, 0));

        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 5, NULL, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);

        this->uniformProjection = glGetUniformLocation(this->shader, "projection");
        this->uniformText       = glGetUniformLocation(this->shader, "str"      );
        this->uniformTextColor  = glGetUniformLocation(this->shader, "textColor" );
    }

    Text::Face* Text::getFace(int size)
    {
        // 중복실행 방지
        std::lock_guard<std::mutex> _lock(this->facesLock);

        // 일단 해당 사이즈에 대해서 열려있으면 그거 반환
        auto f = this->faces.find(size);
        if (f != this->faces.end()) return f->second.get();

        // 생성
        auto fc = std::make_unique<Face>();

        // 해당 폰트 사이즈 열기
        if (FT_New_Face(ftLibrary, FONT_PATH, 0, &fc->tfFace))
            throw std::runtime_error("Could not load font.");
        //defer(FT_Done_Face(ftFace));

        // 크기 선택
        FT_Set_Pixel_Sizes(fc->tfFace, 0, size);

        // 추가
        this->faces.insert(std::make_pair(size, std::move(fc)));

        return this->faces.find(size)->second.get();
    }

    Text::Face::CharInfo Text::Face::getChar(wchar_t c)
    {
        // 중복실행 방지
        std::lock_guard<std::mutex> _lock(this->lock);

        // 있으면 반환
        auto f = this->info.find(c);
        if (f != this->info.end()) return f->second;

        // 캐릭터 글리프 가져오기
        if (FT_Load_Char(this->tfFace, c, FT_LOAD_RENDER))
            throw std::runtime_error("Could not load char.");

        // disable byte-alignment restriction
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        GLuint textureId = 0;

        // generate textureId
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            this->tfFace->glyph->bitmap.width,
            this->tfFace->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            this->tfFace->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        auto chInfo = CharInfo
        {
            textureId,
            v::Size2i(this->tfFace->glyph->bitmap.width, this->tfFace->glyph->bitmap.rows),
            v::Point2i(this->tfFace->glyph->bitmap_left, this->tfFace->glyph->bitmap_top),
            tfFace->glyph->advance.x
        };
        this->info.insert(std::make_pair(c, chInfo));

        return chInfo;
    }

    template <typename T>
    Text::MeasureDetail Text::measureDetail(Face* face, const std::basic_string<T>& str)
    {
        MeasureDetail r{};
        
        {
            std::basic_stringstream<T> ss(str);
            std::basic_string<T> line;

            const auto chH = face->getChar('H');

            while (std::getline(ss, line))
            {
                int width = 0;
                int height = 0;

                // 빈 줄이면...??
                // 높이를 H 문자 기준으로 계산하기
                if (line.size() == 0)
                {
                    //height = glm::max(height, chH.Size.h);
                    height = (static_cast<int>(face->tfFace->size->metrics.height >> 6) + 1) * 2 / 3;
                }
                else
                {
                    for (auto c : line)
                    {
                        const auto ch = face->getChar(c);

                        width += (ch.Advance >> 6);

                        //height = glm::max(height, ch.Size.h + chH.Bearing.y - ch.Bearing.y);
                        height = glm::max(height, static_cast<int>(face->tfFace->size->metrics.height >> 6) + 1);
                    }
                }

                // 전체 사이즈 계산
                r.sizeTotal.w  = glm::max(r.sizeTotal.w, width );
                r.sizeTotal.h += height;

                // 사이즈 벡터에 밀어넣고
                r.size.push_back({ width, height });

                // 줄 수 기록
                r.lines++;
            }
        }

        return r;
    }

    template <typename T>
    void Text::drawLine(Face* face, float x, float y, float z, v::Color color, const std::basic_string<T>& str)
    {
        glBindVertexArray(this->vao);
        defer(glBindVertexArray(0));
            
        // vbo 바인딩
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
        defer(glBindBuffer(GL_ARRAY_BUFFER, 0));

        const auto chH = face->getChar('H');

        // iterate through all characters
        for (auto c = str.begin(); c != str.end(); c++)
        {
            const auto ch = face->getChar(*c);

            const auto xpos = x + static_cast<float>(ch.Bearing.x);
            const auto ypos = y + static_cast<float>(chH.Bearing.y - ch.Bearing.y);

            const auto w = static_cast<float>(ch.Size.w);
            const auto h = static_cast<float>(ch.Size.h);

            // update vbo for each character
            std::array<float, 30> vertices = {
                xpos,     ypos + h, z,   0.0f, 1.0f,
                xpos + w, ypos,     z,   1.0f, 0.0f,
                xpos,     ypos,     z,   0.0f, 0.0f,

                xpos,     ypos + h, z,   0.0f, 1.0f,
                xpos + w, ypos + h, z,   1.0f, 1.0f,
                xpos + w, ypos,     z,   1.0f, 0.0f,
            };
            
            // 캐릭터 텍스쳐 바인딩
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);

            // 업데이트
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

            // 렌더링
            glDrawArrays(GL_TRIANGLES, 0, 6);

            // bitshift by 6 to get value in pixels (1/64th times 2^6 = 64)
            x += (ch.Advance >> 6);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    template <typename T>
    v::Rect2f Text::drawAll(v::Size2f windowSize, v::Rect2f rect, float z, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::basic_string<T>& str)
    {
        glUseProgram(this->shader);
        defer(glUseProgram(0));

        glUniform4fv(this->uniformTextColor, 1, color);

        auto face = this->getFace(size);

        // 매트릭스 업데이트
        {
            std::lock_guard<std::mutex> _lock(this->matrixOrthoLock);
            if (this->matrixOrthoSize != windowSize)
            {
                this->matrixOrthoSize = windowSize;

                this->matrixOrtho = glm::ortho(0.0f, windowSize.w, windowSize.h, 0.0f);
            }

            glUniformMatrix4fv(this->uniformProjection, 1, GL_FALSE, glm::value_ptr(this->matrixOrtho));
        }

        // 영역 제한
        v::Rect2f rectOut{};
        auto md = this->measureDetail(face, str);

        {
            // 그리는 영역 제한
            if (rect.w >= 0 && rect.h >= 0)
            {
                glEnable(GL_SCISSOR_TEST);
                glScissor(
                    static_cast<GLint>(rect.x),
                    static_cast<GLint>(rect.y),
                    static_cast<GLsizei>(rect.w),
                    static_cast<GLsizei>(rect.h)
                );
            }
            defer(glDisable(GL_SCISSOR_TEST));

            // 수직 좌표 계산
            if (rect.h == -1) rect.h = static_cast<float>(md.sizeTotal.h);

            switch (verticalAlignment)
            {
            case Alignment::Near: // 위쪽
                rect.y += 0;
                break;
            case Alignment::Middle: // 다운데
                rect.y += (rect.h - md.sizeTotal.h) / 2;
                break;
            case Alignment::Far: // 아래쪽
                rect.y += (rect.h - md.sizeTotal.h);
                break;
            }
            rectOut.y = rect.y;

            // 한 줄씩 쓰기
            std::basic_stringstream<T> ss(str);
            std::basic_string<T> line;

            float xx = 0;

            int lineIndex = 0;
            while (std::getline(ss, line))
            {
                auto& sizeThisLine = md.size.at(lineIndex++);

                switch (horizontalAlignment)
                {
                case Alignment::Near:
                    xx = rect.x;
                    break;
                case Alignment::Middle:
                    xx = rect.x + (rect.w - sizeThisLine.w) / 2;
                    break;
                case Alignment::Far:
                    xx = rect.x + (rect.w - sizeThisLine.w);
                    break;
                }

                if (rectOut.x == 0 || xx < rectOut.x)
                    rectOut.x = xx;

                // 문자 수가 1개 이상임
                if (line.size() > 0)
                {
                    drawLine(face, xx, rect.y, z, color, line);
                }

                rect.y += sizeThisLine.h;
            }
        }

        if (rect.w == -1)
        {
            rectOut.w = static_cast<float>(md.sizeTotal.w);
        }
        else
        {
            rectOut.w = std::min(rect.w, static_cast<float>(md.sizeTotal.w));
        }
        rectOut.h = static_cast<float>(md.sizeTotal.h);

        return rectOut;
    }

    // 문자열 크기 계산하는 함수
    v::Size2i Text::measure(int size, const std::string& str)
    {
        // 정보 가져오고
        auto face = this->getFace(size);
        return this->measureDetail<char>(face, str).sizeTotal;
    }
    v::Size2i Text::measure(int size, const std::wstring& str)
    {
        // 정보 가져오고
        auto face = this->getFace(size);
        return this->measureDetail<wchar_t>(face, str).sizeTotal;
    }

    // 점을 기준으로 문자를 쓰는 함수.
    v::Rect2f Text::drawAt(v::Size2f windowSize, v::Point2f location, Alignment horizontalAlignment, int size, v::Color color, const std::string& str, float z)
    {
        return this->drawAll<char>(windowSize, v::Rect2f(location, -1), z, horizontalAlignment, Alignment::Near, size, color, str);
    }
    v::Rect2f Text::drawAt(v::Size2f windowSize, v::Point2f location, Alignment horizontalAlignment, int size, v::Color color, const std::wstring& str, float z)
    {
        return this->drawAll<wchar_t>(windowSize, v::Rect2f(location, -1), z, horizontalAlignment, Alignment::Near, size, color, str);
    }

    // 사각형에 범위 내에 글씨를 쓰는 함수.
    v::Rect2f Text::draw(v::Size2f windowSize, v::Rect2f location, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::string& str, float z)
    {
        return this->drawAll<char>(windowSize, location, z, horizontalAlignment, verticalAlignment, size, color, str);
    }
    v::Rect2f Text::draw(v::Size2f windowSize, v::Rect2f location, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::wstring& str, float z)
    {
        return this->drawAll<wchar_t>(windowSize, location, z, horizontalAlignment, verticalAlignment, size, color, str);
    }
}
