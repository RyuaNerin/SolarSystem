// 키보드 및 마우스 이벤트 처리

#pragma once

#include <string>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace input
{
    void handleKeyRepeat(GLFWwindow* window, float deltaSeconds); // 반복키 처리

    void cbKey(GLFWwindow* window, int key, int scancode, int action, int mods); // 키보드 처리

    void cbCursorPos(GLFWwindow* window, double xpos, double ypos); // 마우스 이동
    void cbMouseButton(GLFWwindow* window, int button, int action, int mods); // 마우스 버튼
    void cbScroll(GLFWwindow* window, double xoffset, double yoffset); // 마우스 휠

    void init();
    const std::wstring& getHelpString() noexcept;
}
