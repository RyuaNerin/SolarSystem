#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "config.h"
#include "constants.h"
#include "orbital.h"
#include "planet.h"

namespace model
{
    // 궤도 그리는 모델
    class Orbit
    {
    private:
        // 셰이더
        GLuint shader = 0;

        // 셰이더 데이터
        GLuint vaoSolid = 0;
        GLuint vboSolid = 0;

        GLuint vaoDotted = 0;
        GLuint vboDotted = 0;

        // 셰이더에 넘겨줄 값들
        // 셰이더 참조.
        GLuint uniformProjectionMatrix = 0;
        GLuint uniformViewMatrix       = 0;
        GLuint uniformModelMatrix      = 0;

        GLuint uniformAngleCurrent = 0;
        GLuint uniformColor = 0;

        v::Color planetColor = 0;

        // 현재 정보들...
        int verticesCountSolid = 0; // 버텍스 갯수
        int verticesCountDotted = 0; // 버텍스 갯수

        int generate(const planet::Planet& planet, float step, GLuint* vao, GLuint* vbo)
        {
            int verticesCount = 0;

            std::vector<GLfloat> vertices;
            {
                // 미리 어느정도 할당
                vertices.reserve(300 * 7);

                for (float day = 0; day <= planet._resolutionPeriod; day += step)
                {
                    verticesCount++;
                    const auto pos = orbital::getCurrentPosition(planet._index, day);

                    // 버텍스
                    vertices.push_back(pos.x);
                    vertices.push_back(pos.y);
                    vertices.push_back(pos.z);
                    vertices.push_back(pos.w);
                }
            }

            glext::dispatch([&]() {
                // 버텍스 생성하기
                glGenVertexArrays(1, vao);

                glBindVertexArray(*vao);
                defer(glBindVertexArray(0));

                glGenBuffers(1, vbo);
                glBindBuffer(GL_ARRAY_BUFFER, *vbo);
                defer(glBindBuffer(GL_ARRAY_BUFFER, 0));

                // 할당
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

                // attri 사용 설정
                glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0); glEnableVertexAttribArray(0);
            });

            return verticesCount;
        }

    public:
        // 초기화
        void init(const planet::Planet& planet)
        {
            // 태양은 없음
            if (planet._index == planet::PlanetIndex::Sun) return;

            this->planetColor = planet._color;

            this->shader = glext::loadShader(ORBIT_SHADER);

            // 버텍스 생성
            // 선이랑 점이랑 단위가 다르므로 따로 만들어두기.
            this->verticesCountDotted = this->generate(planet, ORBIT_DOTTED_DAY_PER_VERTEX, &this->vaoDotted, &this->vboDotted);
            this->verticesCountSolid  = this->generate(planet, ORBIT_SOLID_DAY_PER_VERTEX,  &this->vaoSolid,  &this->vboSolid );

            this->uniformProjectionMatrix = glGetUniformLocation(this->shader, "projectionMatrix");
            this->uniformViewMatrix       = glGetUniformLocation(this->shader, "viewMatrix"      );
            this->uniformModelMatrix      = glGetUniformLocation(this->shader, "modelMatrix"     );

            this->uniformAngleCurrent = glGetUniformLocation(this->shader, "angleCurrent");
            this->uniformColor        = glGetUniformLocation(this->shader, "color");

            // 셰이더 설정
            glUseProgram(this->shader);
            defer(glUseProgram(0));

            glUniform1f(glGetUniformLocation(this->shader, "angleVisible"), ORBIT_GRADIENT_START_ANGLE);
            glUniform1f(glGetUniformLocation(this->shader, "alphaGradient"), ORBIT_GRADIENT_ALPHA_MAX);
        }

        // 렌더링
        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix,
            const glm::mat4 modelMatrix,
            const config::Config cfg,
            const float angle
        )
        {
            // 셰이더 설정
            glUseProgram(this->shader);
            defer(glUseProgram(0));

            // uniform 입력 정보 업데이트
            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(viewMatrix      ));
            glUniformMatrix4fv(this->uniformModelMatrix,      1, GL_FALSE, glm::value_ptr(modelMatrix     ));

            glUniform1f(this->uniformAngleCurrent, cfg.showOrbitGradient ? angle : -1);
            
            glUniform4fv(this->uniformColor, 1, cfg.showOrbitColored ? this->planetColor : ORBIT_COLOR);

            // 렌더링
            switch (cfg.showOrbitType)
            {
            case config::OrbitType::Dotted:
            {
                // 버텍스 어레이 설정
                glBindVertexArray(this->vaoDotted);
                defer(glBindVertexArray(0));

                // 그리기
                glPointSize(ORBIT_DOTTED_SIZE);
                glDrawArrays(GL_POINTS, 0, this->verticesCountDotted);
            }
                break;

            case config::OrbitType::Solid:
            {
                // 버텍스 어레이 설정
                glBindVertexArray(this->vaoSolid);
                defer(glBindVertexArray(0));

                // 그리기
                glLineWidth(ORBIT_SOLID_WIDTH);
                glDrawArrays(GL_LINE_LOOP, 0, this->verticesCountSolid);
            }
                break;

            default:
                throw;
            }
        }
    };
}
