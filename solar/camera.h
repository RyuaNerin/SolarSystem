// 카메라 관련 함수들

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/mat4x4.hpp>

#include "v.h"

namespace camera
{
    struct Camera
    {
        glm::mat4 matProjection;    // Projection Camera
        glm::mat4 matView;          // View Camera
        glm::mat4 matViewMilkyway;  // 배경 매트릭스
        glm::mat4 matRotateInverse; // 카메라를 회전시켜도 헹상 오른쪽 위에 행성명이 나올 수 있도록 미리 역행렬 계산해둠.

        glm::vec3 posCamera; // view 레벨에서 행성 대비 카메라의 위치

        v::Size2f screen; // 화면 크기
        float aspect; // 화면 비율

        int focusedPlanet; // 중심 행성

        // polar
        float radius;
        float elevation;
        float azimuth;
    };

    Camera reset(); // 카메라 초기화 함수
    void resetAngle(); // 카메라 각도 초기화
    void resetCenter(); // 카메라 위치 초기화

    Camera get();

    void update(const Camera cam);


    void cbFramebufferSize(GLFWwindow* window, int w, int h); // 화면 비율 설정
    void setFocus(int planetIndex); // 행성 바라보기 설정

    void zoomDelta(float delta, bool byMouse); // 줌 설정. 양수면 확대, 음수면 축소됨.
    void rotateDelta(float deltaAzimuth, float deltaElevation); // 화면 회전
    void moveDeltaStart(v::Point2f location); //화면 이동 시작
    void moveDelta(v::Point2f location); //화면 이동
    void moveDelta(float deltaX, float deltaY); //화면 이동
}
