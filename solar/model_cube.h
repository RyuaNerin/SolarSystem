#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "glext.h"

namespace model
{
    // https://learnopengl.com/Advanced-OpenGL/Face-culling

    class Cube
    {
    private:
        // 셰이더
        GLuint shader = 0;

        // 텍스쳐
        GLuint textureId = 0;

        // 셰이더 데이터
        GLuint vao = 0;
        GLuint vbo = 0;

        // 셰이더에 넘겨줄 값들
        // 셰이더 참조.
        GLuint uniformProjectionMatrix = 0;
        GLuint uniformViewMatrix       = 0;
        GLuint uniformModelMatrix      = 0;

    public:
        void init()
        {
            this->shader = glext::loadShader("cube");

            this->textureId = glext::loadTexture("cube");

            const float vertices[] = {
                // Back tfFace
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
                -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                // Front tfFace
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
                -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
                // Left tfFace
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
                // Right tfFace
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
                 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
                 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
                // Bottom tfFace
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
                -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
                -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
                // Top tfFace
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
                 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
                -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
                -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left      
            };
            glGenVertexArrays(1, &this->vao);
            glBindVertexArray(this->vao); defer(glBindVertexArray(0));

            glGenBuffers(1, &this->vbo);
            glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0 * sizeof(float))); glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); glEnableVertexAttribArray(1);

            // 셰이더 설정
            glUseProgram(this->shader);
            defer(glUseProgram(0));

            this->uniformProjectionMatrix = glGetUniformLocation(this->shader, "projectionMatrix");
            this->uniformViewMatrix       = glGetUniformLocation(this->shader, "viewMatrix"      );
            this->uniformModelMatrix      = glGetUniformLocation(this->shader, "modelMatrix"     );
            
            glUniform1i(glGetUniformLocation(this->shader, "shaderTexture"), 0);
        }

        void draw(
            const glm::mat4 projectionMatrix,
            const glm::mat4 viewMatrix,
            const glm::mat4 modelMatrix
        )
        {
            glUseProgram(this->shader);
            defer(glUseProgram(0));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, this->textureId);
            defer(glBindTexture(GL_TEXTURE_2D, 0));

            glBindVertexArray(this->vao);
            defer(glBindVertexArray(0));

            glUniformMatrix4fv(this->uniformProjectionMatrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
            glUniformMatrix4fv(this->uniformViewMatrix,       1, GL_FALSE, glm::value_ptr(viewMatrix      ));
            glUniformMatrix4fv(this->uniformModelMatrix,      1, GL_FALSE, glm::value_ptr(modelMatrix     ));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    };
}

