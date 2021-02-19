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
    class Planet
    {
    private:
        // 셰이더
        GLuint shaderId = 0;

        // 텍스쳐
        GLuint textureId = 0;
        GLuint textureIdSpecular = 0;
        GLuint textureIdNight = 0;
        GLuint textureIdCloud = 0;

        // 버텍스 갯수
        int indicesCount = 0;

        // 셰이더 데이터
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;

        bool isEarth = false;

        // 셰이더에 넘겨줄 값들
        // 셰이더 참조.
        GLuint uniformProjectionMatrix = 0;
        GLuint uniformViewMatrix = 0;
        GLuint uniformModelMatrix = 0;

        GLuint uniformCameraPos = 0;
        GLuint uniformLightPos = 0;

    public:
        // model 초기화
        void init(const planet::Planet& planet)
        {
            this->shaderId = glext::loadShader(PLANET_MODEL_SHADER_NAME);

            this->textureId = glext::loadTexture(planet._name);

            if (planet._index == planet::PlanetIndex::Earth)
            {
                this->textureIdSpecular = glext::loadTexture(PLANET_EARTH_SPECULAR_TEXTURE);
                this->textureIdNight    = glext::loadTexture(PLANET_EARTH_NIGHT_TEXTURE   );
                this->textureIdCloud    = glext::loadTexture(PLANET_EARTH_CLOUD_TEXTURE   );
            }

            // 버텍스 생성하기
            glext::dispatch([&]() {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;
                model::initSphere(
                    planet._radius,
                    PLANET_MODEL_SLICES_AND_STACKS,
                    PLANET_MODEL_SLICES_AND_STACKS,
                    vertices,
                    indices,
                    true);

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
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float))); glEnableVertexAttribArray(2);

                // 셰이더에 들어갈 변수 아이디 얻기
                this->uniformProjectionMatrix  = glGetUniformLocation(this->shaderId, "projectionMatrix");
                this->uniformViewMatrix        = glGetUniformLocation(this->shaderId, "viewMatrix"      );
                this->uniformModelMatrix       = glGetUniformLocation(this->shaderId, "modelMatrix"     );

                this->uniformLightPos          = glGetUniformLocation(this->shaderId, "light.pos"       );
                this->uniformCameraPos         = glGetUniformLocation(this->shaderId, "cameraPos"       );

                // 셰이더 설정
                glUseProgram(this->shaderId);
                defer(glUseProgram(0));

                glUniform1i(glGetUniformLocation(this->shaderId, "shaderTexture"), 0);

                // 고정 데이터 설정 -> 태양은 무시함
                if (planet._index != planet::PlanetIndex::Sun)
                {
                    glUniform3fv(glGetUniformLocation(this->shaderId, "light.color"    ), 1, LIGHT_COLOR    );
                    glUniform1f (glGetUniformLocation(this->shaderId, "light.ambient"  ),    LIGHT_AMBIENT  );
                    glUniform1f (glGetUniformLocation(this->shaderId, "light.diffuse"  ),    LIGHT_DIFFUSE  );
                    glUniform1f (glGetUniformLocation(this->shaderId, "light.specular" ),    LIGHT_SPECULAR );
                    glUniform1i (glGetUniformLocation(this->shaderId, "light.shininess"),    LIGHT_SHININESS);
                    glUniform1i (glGetUniformLocation(this->shaderId, "shaderTexture"  ),    0              );
                }

                // 지구는...
                if (planet._index == planet::PlanetIndex::Earth)
                {
                    isEarth = true;

                    glUniform1i(glGetUniformLocation(this->shaderId, "isEarth"), 1);

                    glUniform1f(glGetUniformLocation(this->shaderId, "light.ambient"), LIGHT_EARTH_AMBIENT);

                    glUniform1i(glGetUniformLocation(this->shaderId, "shaderTextureSpecular" ), 1);
                    glUniform1i(glGetUniformLocation(this->shaderId, "shaderTextureNight"    ), 2);
                    glUniform1i(glGetUniformLocation(this->shaderId, "shaderTextureCloud"    ), 3);
                }
            });
        }

        // model 그리기
        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix,
            const glm::mat4 modelMatrix,
            const glm::vec3 lightPos,
            const glm::vec3 cameraPos
        )
        {
            // 셰이더 설정
            glUseProgram(this->shaderId);
            defer(glUseProgram(0));

            // 텍스쳐 바인딩
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            if (this->isEarth)
            {
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, this->textureIdSpecular);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, this->textureIdNight);
                glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, this->textureIdCloud);
            }

            // uniform 입력 정보 업데이트
            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(viewMatrix      ));
            glUniformMatrix4fv(this->uniformModelMatrix,      1, GL_FALSE, glm::value_ptr(modelMatrix     ));

            glUniform3fv(this->uniformLightPos,  1, glm::value_ptr(lightPos ));
            glUniform3fv(this->uniformCameraPos, 1, glm::value_ptr(cameraPos));

            // 버텍스 어레이 설정
            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            // 렌더링
            glDrawElements(GL_TRIANGLES, this->indicesCount, GL_UNSIGNED_INT, nullptr);

            if (this->isEarth)
            {
                // defer 안먹어서...
                glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
                glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);

                glActiveTexture(GL_TEXTURE0);
            }
        }
    };
}
