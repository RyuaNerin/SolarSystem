#include "input.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <mutex>
#include <sstream>
#include <vector>

#include "camera.h"
#include "render.h"
#include "utils.h"
#include "config.h"
#include "ui.h"
#include "planet.h"

namespace input
{
    using clock = std::chrono::steady_clock;
    using clock_point = clock::time_point;

    class Binding
    {
    public:
        int            key;             // key
        int            action;          // action
        const wchar_t* helpKeyName;     // 도움말에 표시할 키 이름
        const wchar_t* helpDescription; // 도움말에 쓸

        std::function<void(float delta)> func; // 동작 함수
    };

    constexpr int REM = -1; // key 가 이거면 주석용 혹은 라인 구분용이니 나중에 지울거임

    constexpr int LONG_PRESS = 0x1000; // 오래 누른 버튼 분리하기 위해서

    std::vector<Binding> keyboard =
    {
        { GLFW_KEY_P, GLFW_PRESS, L"P", L"애니메이션 재생 / 일시정지", [](float) { config::togglePlay(); } },
        { GLFW_KEY_H, GLFW_PRESS, L"H", L"도움말 표시",                [](float) { config::toggleHelp(); } },
        { REM, },

        // 설정창
        { GLFW_KEY_O,      GLFW_PRESS, L"O",    L"옵션 열기", [](float) { ui::toggleVisible(); } },
        { GLFW_KEY_ESCAPE, GLFW_PRESS, nullptr, nullptr,      [](float) { ui::close();         } },
        { REM, },

        // 중심 설정
        { GLFW_KEY_1, GLFW_PRESS, L"1 - 0", L"태양 ~ 명왕성으로 이동", [](float) { camera::setFocus(planet::PlanetIndex::Sun    ); } },
        { GLFW_KEY_2, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Mercury); } },
        { GLFW_KEY_3, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Venus  ); } },
        { GLFW_KEY_4, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Earth  ); } },
        { GLFW_KEY_5, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Mars   ); } },
        { GLFW_KEY_6, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Jupiter); } },
        { GLFW_KEY_7, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Saturn ); } },
        { GLFW_KEY_8, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Uranus ); } },
        { GLFW_KEY_9, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Neptune); } },
        { GLFW_KEY_0, GLFW_PRESS, nullptr, nullptr,                    [](float) { camera::setFocus(planet::PlanetIndex::Pluto  ); } },
        { REM, },

        // 카메라 초기화들
        //{ GLFW_KEY_INSERT, GLFW_RELEASE, L"INSERT", L"카메라 각도 및 위치 초기화", [](float) { camera::reset();       } },
        { GLFW_KEY_DELETE, GLFW_RELEASE, L"DELETE", L"카메라 각도 초기화",         [](float) { camera::resetAngle();  } },
        //{ GLFW_KEY_R,      GLFW_RELEASE, L"R",      L"카메라 위치 초기화",         [](float) { camera::resetCenter(); } },
        { REM, },

        // 카메라 회전 조작
        { GLFW_KEY_UP,    GLFW_REPEAT, L"UP. DOWN",    L"카메라 높이각도 조절", [](float delta) { camera::rotateDelta(0, -delta * CAMERA_ELEVATION_SPEED_KEY); } },
        { GLFW_KEY_DOWN,  GLFW_REPEAT, nullptr,       nullptr,                  [](float delta) { camera::rotateDelta(0, +delta * CAMERA_ELEVATION_SPEED_KEY); } },
        { GLFW_KEY_LEFT,  GLFW_REPEAT, L"LEFT, RIGHT", L"카메라 회전 조절",     [](float delta) { camera::rotateDelta(+delta * CAMERA_AZIMUTH_SPEED_KEY, 0); } },
        { GLFW_KEY_RIGHT, GLFW_REPEAT, nullptr,       nullptr,                  [](float delta) { camera::rotateDelta(-delta * CAMERA_AZIMUTH_SPEED_KEY, 0); } },
        { REM, },

        // 속도 조절
        { GLFW_KEY_MINUS,       GLFW_REPEAT, L"+, -", L"애니메이션 속도 조절", [](float delta) { config::speedDelta(-delta, false); } },
        { GLFW_KEY_KP_SUBTRACT, GLFW_REPEAT, nullptr, nullptr,                 [](float delta) { config::speedDelta(-delta, false); } },
        { GLFW_KEY_EQUAL,       GLFW_REPEAT, nullptr, nullptr,                 [](float delta) { config::speedDelta(+delta, false); } },
        { GLFW_KEY_KP_ADD,      GLFW_REPEAT, nullptr, nullptr,                 [](float delta) { config::speedDelta(+delta, false); } },
        { REM, },

        // 화면 이동
        /*
        { GLFW_KEY_W, GLFW_REPEAT,  L"W, A, S, D", L"카메라 이동", [](float delta) { camera::moveDelta(0,      +delta); } },
        { GLFW_KEY_S, GLFW_REPEAT,  nullptr,      nullptr,         [](float delta) { camera::moveDelta(0,      -delta); } },
        { GLFW_KEY_A, GLFW_REPEAT,  nullptr,      nullptr,         [](float delta) { camera::moveDelta(-delta, 0);      } },
        { GLFW_KEY_D, GLFW_REPEAT,  nullptr,      nullptr,         [](float delta) { camera::moveDelta(+delta, 0);      } },
        { REM, },
        */

        // Page UP / DOWN
        { GLFW_KEY_PAGE_UP ,  GLFW_REPEAT, L"PAGE UP",   L"확대", [](float delta) { camera::zoomDelta(+delta, false); } },
        { GLFW_KEY_PAGE_DOWN, GLFW_REPEAT, L"PAGE DOWN", L"축소", [](float delta) { camera::zoomDelta(-delta, false); } },
    };

    std::vector<Binding> mouse =
    {
        // 일시 정지 / 재생
        { GLFW_MOUSE_BUTTON_LEFT,  GLFW_RELEASE, L"L-CLICK", L"애니메이션 일시정지", [](float delta) { config::pause(); } },
        { GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE, L"R-CLICK", L"애니메이션 재생",     [](float delta) { config::play();  } },
        { REM, },

        // 도움말 처리
        { REM, 0, L"L-DRAG", L"카메라 각도 변경", nullptr },
        //{ REM, 0, L"R-DRAG", L"카메라 중심 이동", nullptr },
        { REM, },

        { REM, 0, L"WHEEL", L"확대 / 축소", nullptr },
        { REM, 0, L"Alt + WHEEL", L"애니메이션 속도 조절", nullptr },
        { REM, },

        { GLFW_MOUSE_BUTTON_RIGHT | LONG_PRESS, GLFW_RELEASE, L"LONG_PRESS R-CLICK", L"설정 열기/닫기", [](float) { ui::toggleVisible(); } },
    };

    bool        mouseDown;       // 마우스 누름 상태를 저장하는 변수
    int         mouseDownButton; // 누른 마우스 버튼
    bool        mouseDrag;       // 마우스 드래그 상태임
    v::Point2f  mousePosition;   // 마우스 버튼을 누른 위치
    clock_point mousePressed; // 마우스 버튼 누른 시간

    // 반복키 처리
    void handleKeyRepeat(GLFWwindow* window, float deltaSeconds)
    {
        for (auto& b : keyboard)
        {
            // 반복키로 설정되어 있고 현재 눌린 키는 처리
            if (b.action == GLFW_REPEAT && glfwGetKey(window, b.key) == GLFW_PRESS)
            {
                b.func(deltaSeconds);
            }
        }
    }

    // 한번 누르면 수행되는 것들을 수행한다.
    void handle(const std::vector<Binding>& lst, int key, int action)
    {
        for (auto &b : lst)
        {
            // 키 상태랑 액션이랑 맞는 것 처리.
            if (b.key == key && b.action == action)
            {
                b.func(0);
                return;
            }
        }
    }

    void cbKey(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        std::cout << "cbKey key: " << key << " scancode: " << scancode << " action : " << action << " mods : " << mods << std::endl;
        handle(keyboard, key, action);
    }

    // 마우스 이동 처리하는 함수.
    void onMouseMove(v::Point2f pos)
    {
        // 마우스 드래그 모드가 아니면 마우스가 이동한 거리 계산
        const auto dx = pos.x - mousePosition.x;
        const auto dy = pos.y - mousePosition.y;

        // 이동거리가 0 이면 처리하지 않음
        if (dx == 0 && dy == 0)
        {
            return;
        }

        if (!mouseDrag)
        {
            // 이동거리가 일정 범위보다 작으면 처리하지 않음.
            const auto deltaDistance = std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
            if (deltaDistance <= WINDOW_DRAG_MIN)
            {
                return;
            }
        }

        mousePosition = pos;

        // 드래그 모드로 전환하기
        mouseDrag = true;

        // 마우스 드래그 중이면 각도 변경 및 중심점 변경하기
        if (mouseDrag)
        {
            switch (mouseDownButton)
            {
            case GLFW_MOUSE_BUTTON_LEFT:
                camera::rotateDelta(-dx * CAMERA_AZIMUTH_SPEED_MOUSE, dy * CAMERA_ELEVATION_SPEED_MOUSE);
                break;

            case GLFW_MOUSE_BUTTON_RIGHT:
                camera::moveDelta(pos);
                break;

            default:
                break;
            }
        }
    }

    void cbCursorPos(GLFWwindow* window, double xpos, double ypos)
    {
        // 마우스를 누르고 있으면 내부 연산 처리
        if (mouseDown)
        {
            onMouseMove(v::Point2d(xpos, ypos));
        }
    }

    void cbMouseButton(GLFWwindow* window, int button, int action, int mods)
    {
        std::cout << "cbMouseButton button: " << button << " action: " << action << " mods: " << mods << std::endl;

        // 현재 마우스 커서 위치 가져오는 부분
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        const v::Point2f pos(
            static_cast<float>(x),
            static_cast<float>(y)
        );

        switch (action)
        {
        case GLFW_PRESS: // onMouseDown
            // UI 영역인지 확인
            if (ui::hitTest(pos)) return;

            // 왼쪽이나 오른쪽
            if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)
            {
                if (!mouseDown) // 이미 누른 중이면 작업 안함
                {
                    mousePosition = pos;
                    mouseDown = true; // 마우스 누른 상태로 변경
                    mouseDownButton = button; // 지금 누른 버튼이 왼쪽버튼

                    // 오른쪽이면 클릭점 통보하기.
                    if (button == GLFW_MOUSE_BUTTON_RIGHT)
                    {
                        camera::moveDeltaStart(pos);
                    }

                    // 누른 시간 기록하기
                    mousePressed = clock::now();

                    return;
                }
            }
            break;

        case GLFW_RELEASE: // onMouseUp
            // 처음 눌렀던 버튼과 일치하지 않으면 작업하지 않음
            if (mouseDown && button == mouseDownButton)
            {
                if (!mouseDrag) // 드래그 작업을 하지 않아야만 버튼 작업으로 처리
                {
                    // 누른 시간 확인하기
                    const float deltaSeconds = std::chrono::duration<float, std::milli>(clock::now() - mousePressed).count();
                    if (deltaSeconds > WINDOW_LONG_PRESSED_TIME)
                    {
                        // 오른쪽을 길게 누르는 작업 별도 분리
                        handle(mouse, button | LONG_PRESS, action);
                    }
                    else
                    {
                        handle(mouse, button, action);
                    }
                }
                mouseDown = false;
                mouseDrag = false;
            }
            break;

        default:
            break;
        }
    }

    void cbScroll(GLFWwindow* window, double xoffset, double yoffset)
    {
        std::cout << "cbScroll xoffset: " << xoffset << " yoffset: " << yoffset << std::endl;

        // Alt + Scroll
        if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
        {
            config::speedDelta(static_cast<float>(yoffset), true);
        }
        else
        {
            camera::zoomDelta(static_cast<float>(yoffset), true);
        }
    }

    std::wstring helpString;
    void init()
    {
        const auto append = [](std::wstringstream& ss, const std::vector<Binding>& lst) {
            // 키 이름 맞추기 위해서
            int keyNamePad = 0;
            for (auto& b : lst)
            {
                if (b.helpKeyName == nullptr) continue;
                keyNamePad = std::max(keyNamePad, static_cast<int>(std::wcslen(b.helpKeyName)));
            }

            // 텍스트 만들기
            for (auto& b : lst)
            {
                // 키 할당 안되어있음 = 여백
                if (b.key == REM && b.helpKeyName == nullptr)
                {
                    ss << std::endl;
                    continue;
                }

                // 이름 없으면 스킵
                if (b.helpKeyName == nullptr) continue;
                ss << b.helpKeyName << std::wstring(keyNamePad - std::wcslen(b.helpKeyName), ' ') << " : " << b.helpDescription << std::endl;
            }
        };

        std::wstringstream ss;

        ss << "-- KEYBOARD --" << std::endl;
        append(ss, keyboard);
        ss << std::endl;
        ss << std::endl;

        ss << "-- MOUSE --" << std::endl;
        append(ss, mouse);

        helpString = ss.str();

        // 공백용으로 남겨놨던 key=0 빼버리기
        const auto remove = [](std::vector<Binding>& lst) {
            auto it = lst.begin();
            while (it != lst.end())
            {
                if (it->key == REM)
                {
                    it = lst.erase(it);
                }
                else
                {
                    it++;
                }
            }
        };

        remove(keyboard);
        remove(mouse);
    }

    const std::wstring& getHelpString() noexcept
    {
        return helpString;
    }
}
