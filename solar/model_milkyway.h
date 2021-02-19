#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "constants.h"
#include "defer.h"
#include "glext.h"
#include "model_utils.h"
#include "v.h"

namespace model
{
    class MilkyWay
    {
    private:
        // 셰이더
        GLuint shader = 0;

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
        GLuint uniformViewMatrix       = 0;
        GLuint uniformTexture          = 0;

    public:
        // model 초기화
        void init()
        {
            this->shader = glext::loadShader(MILKYWAY_MODEL_SHADER);

            this->textureId = glext::loadTexture(MILKYWAY_MODEL_TEXTURE);

            glext::dispatch([&]() {
                // 버텍스 생성하기
                std::vector<GLfloat> vertices;;
                std::vector<GLuint> indices;
                initSphere(
                    MILKYWAY_MODEL_RADIUS,
                    MILKYWAY_MODEL_STACKS,
                    MILKYWAY_MODEL_SLICES,
                    vertices,
                    indices,
                    false
                );

                this->indicesCount = indices.size();

                // VertexArray 생성
                glGenVertexArrays(1, &this->vao);
                glBindVertexArray(this->vao); defer(glBindVertexArray(0));

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

                this->uniformProjectionMatrix  = glGetUniformLocation(this->shader, "projectionMatrix");
                this->uniformViewMatrix        = glGetUniformLocation(this->shader, "viewMatrix"      );
                this->uniformTexture           = glGetUniformLocation(this->shader, "shaderTexture"   );
            });
        }

        // model 그리기
        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix
        )
        {
            // 셰이더 설정
            glUseProgram(this->shader);
            defer(glUseProgram(0));

            // 텍스쳐 바인딩
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            // 버텍스 어레이 설정
            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            // uniform 입력 정보 업데이트
            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(viewMatrix      ));

            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            // 렌더링
            glDrawElements(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, nullptr);
        }
    };
}

