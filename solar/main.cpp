#include <exception>

#include <Windows.h>

#include <glad/glad.h>
#include <glad/glad_wgl.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "camera.h"
#include "config.h"
#include "constants.h"
#include "glext.h"
#include "input.h"
#include "render.h"

// 생성된 윈도우
GLFWwindow* window;

// opengl 라이브러리 초기화
inline void initOpenGL()
{
    /**************************************************************************************************************/

    // glfw 초기화
    if (glfwInit() != GLFW_TRUE)
    {
        throw std::runtime_error(std::string("Failed to initialize GLFW. error-code :") + std::to_string(glfwGetError(nullptr)));
    }

    // 사용할 OpenGL 버전 명시.
    glfwWindowHint(GLFW_SAMPLES,               OPENGL_MSAA_SAMPLES     ); // 안티엘리어싱
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3                       ); // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3                       );
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE); // deprecated API 사용하지 않기
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE                 ); // deprecated API 제외하기

    // glfw 윈도우 생성
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    window = glfwCreateWindow(WINDOW_SIZE.w, WINDOW_SIZE.h, WINDOW_CAPTION, nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(window);

    // glad 초기화
    if (!gladLoadGL())
    {
        throw std::runtime_error("Failed to initialize Glad");
    }
    
    // glad-wsl 초기화
    if (!gladLoadWGL(GetDC(glfwGetWin32Window(window))))
    {
        throw std::runtime_error("Failed to initialize Glad-WGL");
    }

    /**************************************************************************************************************/
    // opengl 라이브러리 버전 출력
    std::cout << "----- VERSION -----" << std::endl;
    std::cout << "GLFW version : " << glfwGetVersionString() << std::endl;
    std::cout << "GLAD version : " << GLVersion.major << "." << GLVersion.minor << std::endl;

    std::cout << "OpenGL version  : " << glGetString(GL_VERSION)  << std::endl;
    std::cout << "OpenGL vendor   : " << glGetString(GL_VENDOR)   << std::endl;
    std::cout << "OpenGL venderer : " << glGetString(GL_RENDERER) << std::endl;

    std::cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    std::cout << std::endl;
}

// 콜백함수 등록
inline void setCallback()
{
    // 화면 크기 변경
    glfwSetFramebufferSizeCallback(window, camera::cbFramebufferSize);
    camera::cbFramebufferSize(window, WINDOW_SIZE.w, WINDOW_SIZE.h);

    // 키보드 마우스
    glfwSetInputMode          (window, GLFW_STICKY_KEYS, GL_TRUE); // 입력받음.
    glfwSetKeyCallback        (window, input::cbKey             ); // 키 입력
    glfwSetCursorPosCallback  (window, input::cbCursorPos       ); // 마우스 이동
    glfwSetMouseButtonCallback(window, input::cbMouseButton     ); // 마우스 버튼
    glfwSetScrollCallback     (window, input::cbScroll          ); // 마우스 휠
}

// glfw 메인 루프
inline void mainLoop()
{
    auto lastRenderClock = render::clock::now(); // 마지막 렌더링 시간

    // 화면이 닫히지 않는 한 계속 반복함.
    while (!glfwWindowShouldClose(window))
    {
        glext::dispatchInvoke();

        // 렌더링 시간 측정용
        const auto renderStartClock = render::clock::now();

        // 이전 갱신 후 지난 시간 계산
        const float deltaSeconds = std::chrono::duration<float>(renderStartClock - lastRenderClock).count();
        lastRenderClock = renderStartClock;

        // 키 입력 처리
        input::handleKeyRepeat(window, deltaSeconds);

        // 렌더링
        render::render(window, renderStartClock, deltaSeconds);

        // 화면 갱신
        glfwSwapBuffers(window);

        // 이벤트 처리
        glfwPollEvents();
    }

    // glfw 종료
    glfwTerminate();
}

inline void mainBody()
{
    initOpenGL();

    input::init();
    render::init(window);
    ui::init(window);

    // 리셋 및 초기화
    camera::reset();
    config::reset();

    setCallback();

    mainLoop();
}

int main(int argc, char** argv)
{
    // 디버그 모드면 오류 핸들링 하지 않고 다시 던지기 (디버거에 break 걸리게)
    if constexpr (IS_DEBUG)
    {
        mainBody();
    }
    else
    {
        try
        {
            mainBody();
        }
        catch (const std::exception& e)
        {
            MessageBoxA(NULL, e.what(), WINDOW_CAPTION, MB_OK | MB_ICONERROR);
            return 1;
        }
    }

    return 0;
}
