#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"
#include "defer.h"
#include "glext.h"
#include "model_text.h"
#include "v.h"

namespace model
{
    // 렌더러로 만들어두고 바로바로 쓰는 모델
    class RenderBuffer
    {
    private:
        GLuint shader = 0;

        GLuint textureId = 0;

        GLuint fbo = 0;
        GLuint rbo = 0;

        GLuint vao = 0;
        GLuint vbo = 0;

        v::Size2i size = 0;

        // 셰이더 데이터
        GLuint uniformProjection = 0;

        // 셰이더에 ortho 매트릭스 넘겨야 하는데, 윈도우 크기 달라지면 한번만 업데이트
        std::mutex matrixOrthoLock;
        glm::mat4  matrixOrtho = glm::mat4(0);
        v::Size2f  matrixOrthoSize = -1;

    public:
        v::Size2i getSize()
        {
            return this->size;
        }

        void init(v::Size2i size)
        {
            this->size = size;

            // 셰이더 불러오고 기본 설정
            this->shader = glext::loadShader(GeoMetry2D_TEX_SHADER);

            {
                glUseProgram(this->shader);
                defer(glUseProgram(0));

                glUniform1i(glGetUniformLocation(this->shader, "shaderTexture"), 0);

                this->uniformProjection = glGetUniformLocation(this->shader, "projectionMatrix");

                // vbo vao 초기화
                glGenVertexArrays(1, &this->vao);

                glBindVertexArray(this->vao);
                defer(glBindVertexArray(0));

                // 어레이 버퍼
                glGenBuffers(1, &this->vbo);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
                defer(glBindBuffer(GL_ARRAY_BUFFER, 0));

                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

                // attribute 활성화
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); glEnableVertexAttribArray(1);
            }

            /**************************************************/

            // 프레임버퍼 생성 및 바인딩
            glGenFramebuffers(1, &this->fbo);

            glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
            defer(glBindFramebuffer(GL_FRAMEBUFFER, 0));

            /**************************************************/
            // 프레임버퍼랑 텍스쳐랑 묶는 부분

            // 텍스쳐 생성 및 바인딩
            glGenTextures(1, &this->textureId);

            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            // 텍스쳐 초기화하고
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->size.w, this->size.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

            // 텍스쳐를 프레이버퍼로 묶어버리기
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->textureId, 0);

            /**************************************************/
            // 렌더러 버퍼 생성 및 바인딩
            glGenRenderbuffers(1, &this->rbo);

            glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
            defer(glBindRenderbuffer(GL_RENDERBUFFER, 0));

            // 렌더러 버퍼 초기화. 높이버퍼 없이 색상만 쓰기.
            glRenderbufferStorage(GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, this->size.w, this->size.h);

            const auto frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (frameBufferStatus != GL_FRAMEBUFFER_COMPLETE)
                throw std::runtime_error(std::string("Framebuffer is not complete!") + std::to_string(frameBufferStatus));
        }

        // 버퍼에 그릴 수 있도록 프레임 버퍼 설정
        void drawStart()
        {
            // 프레임버퍼 생성 및 바인딩
            glBindFramebuffer(GL_FRAMEBUFFER, this->fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, this->rbo);
            glBindTexture(GL_TEXTURE_2D, this->textureId);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // 초기화
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, this->size.w, this->size.h);

            glDisable(GL_BLEND);
        }

        // 렌더링 끝.
        void drawEnd()
        {
            glEnable(GL_BLEND);
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        void draw(v::Size2f windowSize, v::Rect2f rect, Alignment horizontalAlignment, Alignment verticalAlignment)
        {
            // 텍스쳐 우선
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            defer(glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

            // 셰이더 활성화
            glUseProgram(this->shader);
            defer(glUseProgram(0));

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

            // 텍스쳐 바인딩
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

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

            // 그릴 좌표 계산
            float x = 0, y = 0;

            if (rect.w == -1) rect.w = static_cast<float>(this->size.w);
            switch (horizontalAlignment)
            {
            case Alignment::Near:   x = rect.x;                               break;
            case Alignment::Middle: x = rect.x + (rect.w - this->size.w) / 2; break;
            case Alignment::Far:    x = rect.x + (rect.w - this->size.w);     break;
            }

            if (rect.h == -1) rect.h = static_cast<float>(this->size.h);
            switch (verticalAlignment)
            {
            case Alignment::Near:   y = rect.y;                               break;
            case Alignment::Middle: y = rect.y + (rect.h - this->size.h) / 2; break;
            case Alignment::Far:    y = rect.y + (rect.h - this->size.h);     break;
            }
            float w = static_cast<float>(this->size.w);
            float h = static_cast<float>(this->size.h);

            std::array<float, 24> vertices = {
                x,     y + h,   0.0f, 0.0f,
                x + w, y,       1.0f, 1.0f,
                x,     y,       0.0f, 1.0f,

                x,     y + h,   0.0f, 0.0f,
                x + w, y + h,   1.0f, 0.0f,
                x + w, y,       1.0f, 1.0f,
            };

            // 버텍스 어레이 비인딩하고
            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            // vbo 바인딩한 다음에
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
            defer(glBindBuffer(GL_ARRAY_BUFFER, 0));

            // vbo 업데이트
            glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());

            // 렌더링
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
    };
}
