#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "config.h"
#include "v.h"

namespace ui
{
    void init(GLFWwindow* window); // UI 좌표 초기화 작업

    void render(v::Size2f winRect, const config::Config cfgc, const camera::Camera camc); // UI 렌더링

    void toggleVisible() noexcept; // UI 표시/숨기기
    void close() noexcept; // UI 닫기

    bool hitTest(v::Point2f pos) noexcept; // 현재 마우스가 UI 안에 있는지 확인

    // 클릭 색을 정의합니다.
    struct Color
    {
        v::Color colorDefault = 0; // 기본 색
        v::Color colorFocused = 0; // 포커스 색
        v::Color colorClicked = 0; // 클릭 색

        constexpr Color() = default;
        constexpr Color(const v::Color d, const v::Color f, const v::Color c) : colorDefault(d), colorFocused(f), colorClicked(c) { }
    };
}
