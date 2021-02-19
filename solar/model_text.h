// https://learnopengl.com/In-Practice/2D-Game/Render-text
// 문서를 바탕으로 재구성

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <tuple>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "v.h"

namespace model
{
    //  기준점으로부터 이동방향으로 크기를 제한 했을 때 상대적 위치
    enum class Alignment
    {
        Near,   // 시작 방향
        Middle, // 중앙
        Far,    // 끝 방향
    };

    class Text
    {
    private:
        struct Face
        {
            struct CharInfo
            {
                GLuint     TextureID; // ID handle of the glyph textureId
                v::Size2i  Size;      // size of glyph
                v::Point2i Bearing;   // offset from baseline to left/top of glyph
                FT_Pos     Advance;   // horizontal offset to advance to next glyph
            };

            FT_Face tfFace = nullptr; // FT_Face 객체

            std::mutex lock;
            std::map<wchar_t, CharInfo> info;

            // 해당하는 문자 정보 가져오기
            CharInfo getChar(wchar_t c);
        };

        struct MeasureDetail
        {
            v::Size2i              sizeTotal; // 크기
            std::vector<v::Size2i> size;      // 각 줄의 크기...
            int                    lines;     // 총 줄 수
        };

        FT_Library ftLibrary = nullptr;

        std::mutex facesLock;
        std::map<int, std::unique_ptr<Face>> faces; // size 별 폰트들 저장된 저장소

        GLint shader = 0; // 셰이더

        GLuint vao = 0;
        GLuint vbo = 0;

        int uniformText = 0;
        int uniformProjection = 0;
        int uniformTextColor = 0;

        // 셰이더에 ortho 매트릭스 넘겨야 하는데, 윈도우 크기 달라지면 한번만 업데이트
        std::mutex matrixOrthoLock;
        glm::mat4  matrixOrtho = glm::mat4(0);
        v::Size2f  matrixOrthoSize = -1;

        // Face 가져오는 함수
        Face* getFace(int size);

        // 문자열의 크기나 줄 수 등의 정보를 가져오는 함수
        template <typename T>
        Text::MeasureDetail measureDetail(Face* face, const std::basic_string<T>& str);

        // 실제로 렌더링하는 함수
        template <typename T>
        void drawLine(Face* face, float x, float y, float z, v::Color color, const std::basic_string<T>& str);

        // 실제로 렌더링하는 함수
        template <typename T>
        v::Rect2f drawAll(v::Size2f windowSize, v::Rect2f rect, float z, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::basic_string<T>& str);

        Text() = default;

    public:
        static Text& instance()
        {
            static Text game;
            return game;
        }

        // 셰이더 초기화
        void init();

        // 문자열 크기 계산하는 함수
        v::Size2i measure(int size, const std::string& str);
        v::Size2i measure(int size, const std::wstring& str);

        // 점을 기준으로 문자를 쓰는 함수.
        v::Rect2f drawAt(v::Size2f windowSize, v::Point2f location, Alignment horizontalAlignment, int size, v::Color color, const std::string& str,  float z = 0);
        v::Rect2f drawAt(v::Size2f windowSize, v::Point2f location, Alignment horizontalAlignment, int size, v::Color color, const std::wstring& str, float z = 0);

        // 사각형에 범위 내에 글씨를 쓰는 함수.
        v::Rect2f draw(v::Size2f windowSize, v::Rect2f location, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::string& str,  float z = 0);
        v::Rect2f draw(v::Size2f windowSize, v::Rect2f location, Alignment horizontalAlignment, Alignment verticalAlignment, int size, v::Color color, const std::wstring& str, float z = 0);
    };
}
