#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "defer.h"
#include "model_utils.h"
#include "planet.h"

namespace model
{
    constexpr double SUN_TIME_DELTA = 0.1;

    // 태양 렌더링
    class Sun
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

        GLuint uniformAspectRatio = 0;
        GLuint uniformSunSize = 0;
        GLuint uniformRandAngle = 0;
        GLuint uniformCamAngle = 0;
        GLuint uniformSunScreenPos = 0;

        double randAngle = 0;

        float radius = 0;

    public:
        // model 초기화
        void init(const planet::Planet& planet)
        {
            this->shaderId = glext::loadShader(SUN_SHADER);

            this->textureId = glext::loadTexture(planet._name);

            this->radius = planet._radius / 4;

            // 버텍스 생성하기
            std::vector<GLfloat> vertices;
            std::vector<GLuint> indices;
            model::initSphere(
                planet._radius * 2,
                PLANET_MODEL_SLICES_AND_STACKS,
                PLANET_MODEL_SLICES_AND_STACKS,
                vertices,
                indices,
                false,
                false);

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
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);

            // 셰이더에 들어갈 변수 아이디 얻기
            this->uniformProjectionMatrix = glGetUniformLocation(this->shaderId, "projectionMatrix");
            this->uniformViewMatrix       = glGetUniformLocation(this->shaderId, "viewMatrix"      );
            this->uniformModelMatrix      = glGetUniformLocation(this->shaderId, "modelMatrix"     );

            this->uniformAspectRatio  = glGetUniformLocation(this->shaderId, "aspectRatio" );
            this->uniformSunSize      = glGetUniformLocation(this->shaderId, "sunSize"     );
            this->uniformRandAngle    = glGetUniformLocation(this->shaderId, "randAngle"   );
            this->uniformCamAngle     = glGetUniformLocation(this->shaderId, "camAngle"    );
            this->uniformSunScreenPos = glGetUniformLocation(this->shaderId, "sunScreenPos");

            // 셰이더 설정
            glUseProgram(this->shaderId);
            defer(glUseProgram(0));

            glUniform1i(glGetUniformLocation(this->shaderId, "shaderTexture"), 0);
        }

        // model 그리기
        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix,
            const glm::mat4 modelMatrix,
            const camera::Camera cam,
            const double deltaSecond
        )
        {
            randAngle += SUN_TIME_DELTA * deltaSecond;

            // 셰이더 설정
            glUseProgram(this->shaderId);
            defer(glUseProgram(0));

            // 텍스쳐 바인딩
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            // uniform 입력 정보 업데이트
            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));
            glUniformMatrix4fv(this->uniformModelMatrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            glm::vec4 sunPos = viewMatrix * modelMatrix * glm::vec4(0, 0, 0, 1);            
            const auto sunScreenRatio = this->radius / glm::length(glm::vec3(sunPos));
            sunPos = projectionMatrix * sunPos;

            glUniform1f(this->uniformAspectRatio, cam.aspect);
            glUniform1f(this->uniformSunSize,     sunScreenRatio);
            glUniform1f(this->uniformRandAngle,   randAngle);
            glUniform1f(this->uniformCamAngle,    glm::radians(cam.elevation) + glm::radians(cam.azimuth));

            glUniform2fv(this->uniformSunScreenPos, 1, glm::value_ptr(sunPos));

            // 버텍스 어레이 설정
            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            // 렌더링
            glDrawElements(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, nullptr);
        }
    };
}
