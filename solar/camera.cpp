#include "camera.h"

#include <iostream>
#include <mutex>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

#include <array>

#include "constants.h"
#include "planet.h"
#include "utils.h"
#include "v.h"

namespace camera
{
    std::mutex lock;
    #define LOCK_CAMERA std::lock_guard<std::mutex> __lock_camera(lock);

    // 중심 좌표
    glm::vec3 center;

    Camera mat{};

    // 중심 이동할 때 쓰는거...
    glm::vec3 panningCameraPos;
    glm::mat4 panningMatrix;

    // 프로젝션 매트릭스 업데이트. facesLock 하면 안됨
    void updateProjection()
    {
        mat.matProjection = glm::perspective(CAMERA_PERSPECTIVE_FOV, mat.aspect, CAMERA_PERSPECTIVE_NEAR, CAMERA_PERSPECTIVE_FAR);
    }

    // 뷰 매트릭스 업데이트. facesLock 하면 안됨
    void updateView()
    {
        mat.matViewMilkyway = glm::mat4(1);
        mat.matViewMilkyway = glm::rotate(mat.matViewMilkyway, glm::radians( mat.elevation), glm::vec3(1, 0, 0));
        mat.matViewMilkyway = glm::rotate(mat.matViewMilkyway, glm::radians(-mat.azimuth  ), glm::vec3(0, 1, 0));

        mat.matRotateInverse = glm::inverse(mat.matViewMilkyway);

        // Polar-view
        mat.matView = glm::mat4(1);
        mat.matView = glm::translate(mat.matView, center);
        mat.matView = glm::rotate(mat.matView, glm::radians( mat.azimuth  ), glm::vec3(0, 1, 0));
        mat.matView = glm::rotate(mat.matView, glm::radians(-mat.elevation), glm::vec3(1, 0, 0));
        mat.matView = glm::translate(mat.matView, glm::vec3(0, 0, mat.radius));

        mat.posCamera = glm::vec3(mat.matView * glm::vec4(0, 0, 0, 1)) + center;

        mat.matView = glm::inverse(mat.matView);

        // 패닝용 매트릭스 계산
        // 비슷한데 중심점 이동 안하는 거만 다르다
        panningMatrix = glm::mat4(1);
        panningMatrix = glm::rotate(panningMatrix, glm::radians( mat.azimuth  ), glm::vec3(0, 1, 0));
        panningMatrix = glm::rotate(panningMatrix, glm::radians(-mat.elevation), glm::vec3(1, 0, 0));
        panningMatrix = glm::translate(panningMatrix, glm::vec3(0, 0, mat.radius));

        panningCameraPos = glm::vec3(panningMatrix * glm::vec4(0, 0, 0, 1));

        panningMatrix = glm::inverse(panningMatrix);

        /*
        if constexpr (IS_DEBUG)
        {
            std::cout
                << "Camera"
                << " Focus " << mat.focusedPlanet
                << " mat.radius:" << mat.radius
                << " mat.azimuth: " << mat.azimuth
                << " mat.elevation: " << mat.elevation
                << " Center " << glm::to_string(center)
                << " Camera " << glm::to_string(mat.posCamera) << std::endl;
        }
        */
    }

    Camera reset()
    {
        LOCK_CAMERA;

        center = glm::vec3(0);

        mat.radius = CAMERA_RADIUS_DEFAULT;

        mat.elevation = CAMERA_ELEVATION_DEFAULT;
        mat.azimuth = CAMERA_AZIMUTH_DEFAULT;

        mat.focusedPlanet = 0;

        updateView();

        return mat;
    }

    void resetAngle()
    {
        LOCK_CAMERA;

        mat.radius = CAMERA_RADIUS_DEFAULT;

        mat.elevation = CAMERA_ELEVATION_DEFAULT;
        mat.azimuth = CAMERA_AZIMUTH_DEFAULT;

        updateView();
    }

    void resetCenter()
    {
        LOCK_CAMERA;

        center = glm::vec3(0);

        updateView();
    }

    Camera get()
    {
        LOCK_CAMERA;

        return mat;
    }
    
    void update(const Camera cam)
    {
        LOCK_CAMERA;

        if (cam.focusedPlanet == mat.focusedPlanet &&
            cam.radius        == mat.radius        &&
            cam.azimuth       == mat.azimuth       &&
            cam.elevation     == mat.elevation)
            return;

        mat.focusedPlanet = cam.focusedPlanet;

        mat.radius    = cam.radius;
        mat.elevation = cam.elevation;
        mat.azimuth   = cam.azimuth;

        updateView();
    }

    void cbFramebufferSize(GLFWwindow* window, int w, int h)
    {
        std::cout << "cbFramebufferSize w: " << w << " h: " << h << std::endl;
        if (w == 0 || h == 0) return; // 최소화 무시

        LOCK_CAMERA;

        mat.screen.w = static_cast<float>(w);
        mat.screen.h = static_cast<float>(h);
        mat.aspect = static_cast<float>(w) / h;

        // 매트릭스 업데이트
        updateProjection();
    }

    void setFocus(int planetIndex)
    {
        LOCK_CAMERA;

        mat.focusedPlanet = planetIndex;
        center = glm::vec3(0);

        // 매트릭스 업데이트
        updateView();
    }

    void zoomDelta(float delta, bool byMouse) // 줌 설정. 양수면 확대, 음수면 축소됨.
    {
        LOCK_CAMERA;

        mat.radius = std::clamp(
            mat.radius / std::pow(byMouse ? CAMERA_RADIUS_SPEED_MOUSE : CAMERA_RADIUS_SPEED_KEY, delta),
            CAMERA_RADIUS_MIN,
            CAMERA_RADIUS_MAX
        );

        updateView();
    }
    void rotateDelta(float deltaAzimuth, float deltaElevation) // 화면 회전
    {
        LOCK_CAMERA;

        // mat.elevation 은 최대 최소가 정해져있음.
        mat.elevation = std::clamp(
            mat.elevation + deltaElevation,
            CAMERA_ELEVATION_MIN,
            CAMERA_ELEVATION_MAX
        );

        // mat.azimuth 조정. -180 ~ 180 에 있다가, 범위 초과 시 반대쪽에서 나타나도록.
        mat.azimuth += deltaAzimuth;
        while (mat.azimuth <   0) mat.azimuth += 360.0f;
        while (mat.azimuth > 360) mat.azimuth -= 360.0f;

        updateView();
    }

    v::Point2f moveMouseStart; // 마우스 이동 시작 위치
    void moveDeltaStart(v::Point2f location)
    {
        return;
        // Deprecated

        LOCK_CAMERA;

        moveMouseStart = location;
    }

    void moveDeltaInner(float deltaX, float deltaY);
    void moveDelta(v::Point2f location)
    {
        return;
        // Deprecated

        LOCK_CAMERA;

        auto deltaX = location.x - moveMouseStart.x;
        auto deltaY = location.y - moveMouseStart.y;
        moveMouseStart = location;

        moveDeltaInner(deltaX, deltaY);
    }
    void moveDelta(float deltaX, float deltaY)
    {
        return;
        // Deprecated

        LOCK_CAMERA;

        moveDeltaInner(
            mat.screen.w * deltaX * CAMERA_MOVEMENT_DELTA,
            mat.screen.h * deltaY * CAMERA_MOVEMENT_DELTA
        );
    }

    void moveDeltaInner(float deltaX, float deltaY)
    {
        auto matrix = mat.matProjection * panningMatrix;

        // half of the fov is center to top of screen
        auto targetDistance = glm::length(panningCameraPos) * glm::tan(glm::radians(CAMERA_PERSPECTIVE_FOV / 2));

        // we use only clientHeight here so aspect ratio does not distort speed
        {
            const float distance = 2.f * deltaX * targetDistance / mat.screen.h;
            glm::vec3 v = matrix[0];
            v = v * -distance;
            center += v;
        }

        {
            const float distance = 2.f * deltaY * targetDistance / mat.screen.h;
            glm::vec3 v = matrix[1];
            v = v * distance;
            center += v;
        }

        updateView();
    }
}
