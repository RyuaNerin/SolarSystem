// 렌더링 및 윈도우 담당

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "model_text.h"

namespace render
{
    using clock = std::chrono::steady_clock;
    using clock_point = clock::time_point;

    void init(GLFWwindow* window); // openGL 렌더링 관련 부분 초기화

    void resetDate() noexcept;

    void render(GLFWwindow* window, clock_point renderStartClock, double deltaSeconds); // 화면 렌더링
}
