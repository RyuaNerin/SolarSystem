#pragma once

#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glext.h"
#include "constants.h"

namespace model
{
    class SaturnRing
    {
    private:
        // 셰이더
        GLuint shaderId = 0;

        // 텍스쳐
        GLuint textureId = 0;

        // 버텍스 갯수
        int indicesCount = 0;

        // 셰이더 데이터
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        // 셰이더에 넘겨줄 값들
        // 셰이더 참조.
        GLuint uniformProjectionMatrix = 0;
        GLuint uniformViewMatrix = 0;
        GLuint uniformModelMatrix = 0;

        GLuint uniformCameraPos = 0;
        GLuint uniformLightPos = 0;

    public:
        void init(const planet::Planet& planet)
        {
            this->shaderId = glext::loadShader(PLANET_SATURN_RING_SHADER);

            this->textureId = glext::loadTexture(PLANET_SATURN_RING_TEXTURE);

            std::vector<GLfloat> vertices;
            std::vector<GLuint> indices;

            // 벡터 생성하기
            {
                vertices.resize((PLANET_SATURN_RING_PARTICLES + 1) * 10);

                auto verticesIt = vertices.begin();
                for (int i = 0; i <= PLANET_SATURN_RING_PARTICLES; i++)
                {
                    const auto theta = -PI_2 + PI2 * i / PLANET_SATURN_RING_PARTICLES;
                    const auto c = std::cosf(theta);
                    const auto s = std::sinf(theta);

                    const float xout = planet._radius * PLANET_STAURN_RING_RAIDUS_MUL_OUT * c;
                    const float yout = planet._radius * PLANET_STAURN_RING_RAIDUS_MUL_OUT * s;

                    float xin = planet._radius * PLANET_STAURN_RING_RAIDUS_MUL_IN * c;
                    float yin = planet._radius * PLANET_STAURN_RING_RAIDUS_MUL_IN * s;

                    //////////////////////////////////////////////////

                    // 바깥 위치
                    *verticesIt++ = xout;
                    *verticesIt++ = 0;
                    *verticesIt++ = yout;

                    // 텍스쳐
                    *verticesIt++ = 1;
                    *verticesIt++ = 0;

                    //////////////////////////////////////////////////

                    // 안쪽 위치
                    *verticesIt++ = xin;
                    *verticesIt++ = 0;
                    *verticesIt++ = yin;

                    // 텍스쳐
                    *verticesIt++ = 0;
                    *verticesIt++ = 0;
                }

                indices.resize(PLANET_SATURN_RING_PARTICLES * 3 * 2);
                auto indicesIt = indices.begin();
                for (int i = 0; i < PLANET_SATURN_RING_PARTICLES; i++)
                {
                    auto po0 = i * 2 + 0; auto po1 = i * 2 + 2;
                    auto pi0 = i * 2 + 1; auto pi1 = i * 2 + 3;

                    *indicesIt++ = po0;
                    *indicesIt++ = pi0;
                    *indicesIt++ = po1;

                    *indicesIt++ = po1;
                    *indicesIt++ = pi0;
                    *indicesIt++ = pi1;
                }
                this->indicesCount = indices.size();
            }

            glext::dispatch([&]() {
                // VertexArray 생성
                glGenVertexArrays(1, &this->vao);
                glBindVertexArray(this->vao);
                defer(glBindVertexArray(0));

                // 버텍스 저장
                glGenBuffers(1, &this->vbo);
                glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

                // 인덱스 저장
                glGenBuffers(1, &this->ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

                // attri 사용 설정
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
            
                this->uniformProjectionMatrix  = glGetUniformLocation(this->shaderId, "projectionMatrix");
                this->uniformViewMatrix        = glGetUniformLocation(this->shaderId, "viewMatrix"      );
                this->uniformModelMatrix       = glGetUniformLocation(this->shaderId, "modelMatrix"     );

                // 셰이더 설정
                glUseProgram(this->shaderId);
                defer(glUseProgram(0));

                glUniform1i(glGetUniformLocation(this->shaderId, "shaderTexture"), 0);
                glUniform1f(glGetUniformLocation(this->shaderId, "alpha"        ), PLANET_SATURN_RING_ALPHA);
            });
        }

        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix,
            const glm::mat4 modelMatrix
        )
        {
            // 셰이더 설정
            glUseProgram(this->shaderId);
            defer(glUseProgram(0));

            // 텍스쳐 바인딩
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            // uniform 입력 정보 업데이트
            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(viewMatrix      ));
            glUniformMatrix4fv(this->uniformModelMatrix,      1, GL_FALSE, glm::value_ptr(modelMatrix     ));

            // 버텍스 어레이 설정
            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            // 렌더링
            glDrawElements(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, nullptr);
        }
    };
}
