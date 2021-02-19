#include "render.h"

#include <array>
#include <atomic>
#include <chrono>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <thread>
#include <vector>

#include <Windows.h>

#include <glad/glad_wgl.h>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "camera.h"
#include "config.h"
#include "constants.h"
#include "glext.h"
#include "input.h"
#include "model_cube.h"
#include "model_milkyway.h"
#include "model_orbit.h"
#include "model_planet.h"
#include "model_renderbuffer.h"
#include "model_saturn_ring.h"
#include "orbital.h"
#include "planet.h"
#include "ui.h"
#include "utils.h"
#include "v.h"

namespace render
{
    constexpr int PLANET_INDEX_START = -1; // draw#### 함수들의 시작임을 표시하는 상수

    /**************************************************************************************************************/
    // 본 파일 내에서 사용하는 데이터 형식들

    // 바라보는 행성을 중심에 둘 수 있도록 좌표 재 계산한 것.
    struct RelativeLocation
    {
        glm::vec4 location;    // 행성 위치 (w 는 현재 진근점이각)
        glm::mat4 ModelMatrix; // 모델 매트릭스
        bool      updated;     // 계산끝남.
    };

    /**************************************************************************************************************/

    std::mutex lock;
    #define LOCK_RENDER std::lock_guard<std::mutex> __lock_render(render::lock);

    // FPS
    std::array<char, 32> fpsString;   // 마지막 fps
    clock_point          fpsLastTime; // fps 마지막 계산 시간
    int                  fpsFrames;   // delta-Time 동안 수행한 프레임 수

    float                speedIs;     // 마지막 속도 값
    std::array<char, 32> speedString; // 마지막 속도 string

    std::array<char, 32> dateString; // 날자 저장

    // openGL 모델
    model::RenderBuffer modelHelp;        // 도움말
    model::RenderBuffer modelWatermark;   // 워터마크
    model::RenderBuffer modelPause;       // 일시 정지

    model::MilkyWay            modelBackground;  // 배경
    std::vector<model::Orbit>  modelOrbit;       // 궤도
    std::vector<model::Planet> modelPlanets;     // 행성
    model::SaturnRing          modelSaturnRing;  // 토성 고리

    // 텍스쳐 로딩중
    constexpr int modelTotalCounts = // 백그라운드에서 로딩할 모델 갯수
        1 +     // 배경
        1 +     // 토성 고리
        10 * 2; // 행성별 텍스쳐 및 궤도
    std::atomic_bool loadedNecessary  = false; // true 가 되야 로딩 텍스트 띄움
    std::atomic_bool loadingCompleted = false; // 로딩중...
    std::atomic_int  loadingProgress = 0;

    // 현재 날자
    double today;
    
    /**************************************************************************************************************/

    void textureLoadingAsync(HDC hdc, HGLRC ctx)
    {
        wglMakeCurrent(hdc, ctx);
        defer(wglMakeCurrent(hdc, nullptr); wglDeleteContext(ctx););

        /****************************************************************************************************/
        // 기본 데이터 읽고 텍스트 생성부분

        // 폰트 데이터 불러오기
        glext::dispatch([&]() {
            model::Text::instance().init();
        });

        v::Size2f size = 0;

        // 워터마크 생성
        glext::dispatch([&]() {
            size = model::Text::instance().measure(WINDOW_WATERMARK_TEXT_SIZE, WINDOW_WATERMARK_TEXT);
            size.w += 10;
            size.h += 10;

            modelWatermark.init(size);

            modelWatermark.drawStart();
            defer(modelWatermark.drawEnd());

            model::Text::instance().draw(
                size,
                v::Rect2f(0, size),
                WINDOW_WATERMARK_TEXT_ALIGN_HORIZ,
                model::Alignment::Near,
                WINDOW_WATERMARK_TEXT_SIZE,
                WINDOW_WATERMARK_TEXT_COLOR,
                WINDOW_WATERMARK_TEXT
            );
        });

        // 여기까지만 로드되면 로딩띄워도 됨.
        glFlush();
        loadedNecessary = true;

        // 도움말 텍스트 버퍼 생성
        glext::dispatch([&]() {
            size = model::Text::instance().measure(WINDOW_HELP_TEXT_SIZE, input::getHelpString());

            modelHelp.init(size);

            modelHelp.drawStart();
            defer(modelHelp.drawEnd());

            model::Text::instance().draw(
                size,
                v::Rect2f(0, size),
                WINDOW_HELP_TEXT_ALIGN_HORIZ,
                model::Alignment::Near,
                WINDOW_HELP_TEXT_SIZE,
                WINDOW_HELP_TEXT_COLOR,
                input::getHelpString()
            );
        });

        // 일시 정지 생성
        // TODO 아 애니메이션 넣으면 딱인디
        glext::dispatch([&]() {
            size = model::Text::instance().measure(WINDOW_PAUSED_TEXT_SIZE, WINDOW_PAUSED_TEXT);
            size.h += 10; // 글자 잘려서 마진.

            modelPause.init(size);

            modelPause.drawStart();
            defer(modelPause.drawEnd());

            model::Text::instance().draw(
                    size,
                    v::Rect2f(0, size),
                WINDOW_PAUSED_TEXT_ALIGN_HORIZ,
                model::Alignment::Far,
                WINDOW_PAUSED_TEXT_SIZE,
                WINDOW_PAUSED_TEXT_COLOR,
                WINDOW_PAUSED_TEXT
            );
        });

        /****************************************************************************************************/
        // 행성 로딩부분

        modelPlanets.resize(planet::planetList.size());
        modelOrbit.resize(planet::planetList.size());

        // 행성 텍스쳐
        for (size_t i = 0; i < planet::planetList.size(); i++)
        {
            // 태양은 별도작업함.

            const auto& planet = planet::planetList.at(i);

            // 태양은 궤도 없음.
            if (i != 0)
            {
                modelOrbit.at(i).init(planet);
                loadingProgress++;
            }

            // 행성
            modelPlanets.at(i).init(planet);
            loadingProgress++;
        }

        // 배경
        modelBackground.init();
        loadingProgress++;

        // 토성 고리
        modelSaturnRing.init(planet::planetList.at(planet::PlanetIndex::Saturn));
        loadingProgress++;

        glFlush();
        loadingCompleted = true;
    }

    void init(GLFWwindow* window)
    {
        // 기본 색 지정
        glColor4f(1, 1, 1, 1);
        glClearColor(0, 0, 0, 1);

        glEnable(GL_DEPTH_TEST    ); // 깊이 정보 사용하기
        glEnable(GL_COLOR_MATERIAL); // 빛을 비추어도 원래 색상 유지

        glEnable(GL_POINT_SPRITE);
        glEnable(GL_PROGRAM_POINT_SIZE); // 픽셀 크기 변경 활성화
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // 픽셀 크기 변경 활성화

        // 후면 제거
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Alpha 채널 계산 활성화
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 멀티샘플링 활성화
        //glfwWindowHint(GLFW_SAMPLES, 4); <- main 에서 윈도우 생성할 때 함.
        glEnable(GL_MULTISAMPLE);

        // 외각선을 부드럽게 처리하는 부분
        glHint(GL_POINT_SMOOTH_HINT,          GL_NICEST); // 점
        glHint(GL_LINE_SMOOTH_HINT,           GL_NICEST); // 줄
        glHint(GL_POLYGON_SMOOTH_HINT,        GL_NICEST); // 폴리곤
        glHint(GL_TEXTURE_COMPRESSION_HINT,   GL_NICEST); // 텍스쳐

        // 텍스쳐 축소/확대 : 보간하기
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // linear, mipmap 도 linear...
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // 텍스쳐 매핑
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // 비등방성 필터링 적용하기
        {
            // 원하고자 하는 값보다 낮은 값을 지원할 수 있어서...
            GLfloat v = 0;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &v);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min<GLfloat>(v, OPENGL_ANISTROPY));
        }

        /**************************************************************************************************************/
        // 백그라운드에서 텍스쳐를 가져오기 위한 부분
        // https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows#wglShareLists
        // glext.h 참조.

        constexpr std::array<int, 9> attribs = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB, // Core profile 사용함.
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, // OpenGL 3 이상의 API 만을 사용함
            0,
        };

        const auto ctxCurrent = wglGetCurrentContext();
        const auto dc = wglGetCurrentDC();
        const auto ctxBackground = wglCreateContextAttribsARB(dc, ctxCurrent, attribs.data());

        std::thread thd(textureLoadingAsync, dc, ctxBackground);
        thd.detach();
    }

    void resetDate() noexcept
    {
        today = 0;
    }

    // 케플러 공식을 활용하여 행성 위치 구하는 함수
    void calcPlanetResolution(std::vector<RelativeLocation>& relLoc)
    {
        relLoc.at(0).location = glm::vec4(0, 0, 0, 0);

        for (size_t i = 1; i < planet::planetList.size(); i++)
        {
            relLoc.at(i).location = orbital::getCurrentPosition(static_cast<int>(i), today);
        }
    }


    // 현재 중심 행성을 기준으로 전체 상대 위치 계산하는 함수
    void calcRelativeLocation(std::vector<RelativeLocation>& relLoc, const camera::Camera cam, glm::mat4 modelMatrix = glm::mat4(1), int planetIndex = PLANET_INDEX_START)
    {
        if (planetIndex == PLANET_INDEX_START)
        {
            calcRelativeLocation(relLoc, cam, modelMatrix, cam.focusedPlanet);
            return;
        }

        RelativeLocation& rl = relLoc.at(planetIndex);
        rl.ModelMatrix = modelMatrix;

        const auto& planet = planet::planetList.at(planetIndex);

        // 부모 행성 확인
        if (planet._parentIndex != -1)
        {
            auto& rlParent = relLoc.at(planet._parentIndex);
            if (!rlParent.updated)
            {
                rlParent.updated = true;

                // 부모행성 위치 계산. 내 공전위치만큼 뺀 것.
                const glm::mat4 modelMatrixParent = glm::translate(
                    modelMatrix,
                    -glm::vec3(rl.location)
                );
                calcRelativeLocation(relLoc, cam, modelMatrixParent, planet._parentIndex);
            }
        }

        // 자식 행성 위치 계산
        for (auto& planetChild : planet::planetList)
        {
            if (planetChild._index == planetIndex) continue; // 나 스킵

            if (planetChild._parentIndex == -1) continue; // 부모 없는거 스킵
            if (planetChild._parentIndex != planetIndex) continue; // 내가 부모 아닌거 스킵

            auto& rlChild = relLoc.at(planetChild._index);
            if (rlChild.updated) continue; // 업데이트 된거 스킵
            rlChild.updated = true;

            // 매트릭스를 자식행성 공전위치로 이동시켜서 전송
            const glm::mat4 modelMatrixChild = glm::translate(
                modelMatrix,
                glm::vec3(rlChild.location)
            );
            calcRelativeLocation(relLoc, cam, modelMatrixChild, planetChild._index);
        }
    }

    // 배경 그리는 함수
    void drawBackground(const camera::Camera& cam)
    {
        // 앞 자르기
        glCullFace(GL_FRONT);
        defer(glCullFace(GL_BACK));

        // 깊이 검사 최적화. 배경은 항상 Z 가 1이기 때문에...
        glDepthFunc(GL_LEQUAL);
        defer(glDepthFunc(GL_LESS));

        // 텍스쳐 우선
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        defer(glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));

        // 카메라 회전만 설정한 상태로 배경을 그려야 확대, 축소를 해도 배경이 동일하게 그려진다.
        modelBackground.draw(cam.matProjection, cam.matViewMilkyway);
    }

    // 행성들 궤도 그리는 함수
    void drawOrbit(
        const std::vector<RelativeLocation> relPos,
        const config::Config cfg,
        const camera::Camera cam,
        const int planetIndex = PLANET_INDEX_START
    )
    {
        if (planetIndex == PLANET_INDEX_START)
        {
            drawOrbit(relPos, cfg, cam, 0);
            return;
        }

        const auto& planet = planet::planetList.at(planetIndex);

        // 태양은 안함
        if (planetIndex != planet::PlanetIndex::Sun)
        {
            const auto& rlParent = relPos.at(planet._parentIndex);
            const auto& rlCurernt = relPos.at(planetIndex);

            // 부모 행성의 위치가 기본위치다!
            const glm::mat4 modelMatrix = planet._parentIndex == -1 ? glm::mat4(1) : rlParent.ModelMatrix;

            // 궤도 렌더링
            modelOrbit.at(planetIndex).draw(
                cam.matProjection,
                cam.matView,
                modelMatrix,
                cfg,
                rlCurernt.location.w
            );
        }

        // 자식 행성 드로잉
        planet.foreach(drawOrbit, relPos, cfg, cam);
    }

    // 행성 그리는 함수
    void drawPlanet(
        const float deltaSecond,
        const std::vector<RelativeLocation> relPos,
        const config::Config cfg,
        const camera::Camera cam,
        const glm::vec3 lightPos,
        const int planetIndex = PLANET_INDEX_START
    )
    {
        if (planetIndex == PLANET_INDEX_START)
        {
            // 태양부터 시작
            drawPlanet(deltaSecond, relPos, cfg, cam, lightPos, 0);
            return;
        }

        const auto& planet = planet::planetList.at(planetIndex);

        // 현재 카메라가 보는 행성을 0, 0 으로 하는, 상대적인 위치를 계산하도록 하는 매트릭스
        const auto modelMatrix = relPos.at(planetIndex).ModelMatrix;

        // 자전축만큼 회전하고 그리기
        {
            const auto matRotation = modelMatrix * planet.getLocalMatrix(false);

            // 내 위치
            const glm::vec3 myCenter = cam.matView * modelMatrix * glm::vec4(0, 0, 0, 1);

            // 내 기준 태양의 상대 위치 얻어오기
            const glm::vec3 lightPosRelative = lightPos - myCenter;

            // 내 기준 카메라의 상대 위치 얻어오기
            const glm::vec3 camPosRelative = cam.posCamera - glm::vec3(modelMatrix * glm::vec4(myCenter, 1));

            modelPlanets.at(planetIndex).draw(
                cam.matProjection,
                cam.matView,
                matRotation,
                lightPosRelative,
                camPosRelative
            );

            // 렌더링
            if (planetIndex == planet::PlanetIndex::Saturn)
            {
                // 후면제거 끄기
                glDisable(GL_CULL_FACE);
                defer(glEnable(GL_CULL_FACE));

                // 고리 렌더링
                modelSaturnRing.draw(
                    cam.matProjection,
                    cam.matView,
                    matRotation
                );
            }
        }

        // 이름 쓰기
        if (cfg.showPlanetName)
        {
            // 글씨를 쓸 위치 구하기
            constexpr float rt = glm::root_two<float>();
            const glm::vec3 v = cam.matRotateInverse * glm::vec4(planet._radius / rt, planet._radius / rt, planet._radius * 3, 1);

            // 화면상의 좌표 구하기
            const auto c = glm::project(v, cam.matView * modelMatrix, cam.matProjection, glm::vec4(0, 0, cam.screen.w, cam.screen.h));

            // 화면에 보이는가
            if (0 <= c.z && c.z <= 1 &&
                0 <= c.x && c.x <= cam.screen.w &&
                0 <= c.y && c.y <= cam.screen.h)
            {
                // 행성과 카메라(0,0,0) 간의 거리 구하기
                const auto len = glm::length(glm::vec3(cam.matView * modelMatrix * glm::vec4(0, 0, 0, 1)));

                // 폰트 사이즈 계산
                int fontSize = 0;
                if (len <= PLANET_NAME_TEXT_SIZE_NEAR_D) // 가까울 때
                {
                    fontSize = PLANET_NAME_TEXT_SIZE_NEAR;
                }
                else if (len >= PLANET_NAME_TEXT_SIZE_FAR_D) // 멀 때
                {
                    fontSize = PLANET_NAME_TEXT_SIZE_FAR;
                }
                else // 적절한 사이즈로 재 계산
                {
                    fontSize =
                        std::clamp(
                            PLANET_NAME_TEXT_SIZE_NEAR
                            -
                            static_cast<int>(
                                std::floor(
                                    (PLANET_NAME_TEXT_SIZE_NEAR - PLANET_NAME_TEXT_SIZE_FAR)
                                    * (len - PLANET_NAME_TEXT_SIZE_NEAR_D)
                                    / (PLANET_NAME_TEXT_SIZE_FAR_D - PLANET_NAME_TEXT_SIZE_NEAR_D)
                                )
                            ),
                            PLANET_NAME_TEXT_SIZE_FAR,
                            PLANET_NAME_TEXT_SIZE_NEAR
                        );
                }

                // 행성 이름 쓰기
                model::Text::instance().drawAt(
                    cam.screen,
                    v::Point2f(c.x, cam.screen.h - c.y),
                    model::Alignment::Near,
                    fontSize,
                    PLANET_NAME_TEXT_COLOR,
                    planet._name,
                    c.z
                );
            }
        }

        // 자식 행성 드로잉
        planet.foreach(drawPlanet, deltaSecond, relPos, cfg, cam, lightPos);
    }

    // 도움말, FPS, watermark, ui 등 화면에 그리는 무언가를 그리는 함수
    void drawUserInterface(const config::Config cfg, const camera::Camera cam, const clock_point renderStartClock)
    {
        // 화면에 직접 텍스트 쓸 영역 재계산
        const v::Rect2f viewTextRect(
            WINDOW_TEXT_MARGIN,
            WINDOW_TEXT_MARGIN,
            cam.screen.w - WINDOW_TEXT_MARGIN * 2,
            cam.screen.h - WINDOW_TEXT_MARGIN * 2
        );

        glDepthFunc(GL_ALWAYS); // Depth 테스트 하지 않고 항상 화면을 갱신하도록한다.
        defer(glDepthFunc(GL_LESS)); // 복구

        // 로딩중 텍스트 띄우기
        if (!loadingCompleted)
        {
            std::array<wchar_t, 128> wcsBuff{};

            std::swprintf(
                wcsBuff.data(),
                wcsBuff.size(),
                WINDOW_LOADING_TEXT_FORMAT,
                loadingProgress.load(),
                modelTotalCounts
            );

            // LOADING 쓰기
            model::Text::instance().draw(
                cam.screen,
                viewTextRect,
                model::Alignment::Middle,
                model::Alignment::Middle,
                WINDOW_LOADING_TEXT_SIZE,
                WINDOW_LOADING_TEXT_COLOR,
                wcsBuff.data()
            );
        }
        else
        {
            // 도움말
            if (cfg.showHelp)
            {
                modelHelp.draw(
                    cam.screen,
                    viewTextRect,
                    WINDOW_HELP_TEXT_ALIGN_HORIZ,
                    WINDOW_HELP_TEXT_ALIGN_VERTI
                );
            }

            // paused
            if (!cfg.playAnimation)
            {
                modelPause.draw(
                    cam.screen,
                    viewTextRect,
                    WINDOW_PAUSED_TEXT_ALIGN_HORIZ,
                    WINDOW_PAUSED_TEXT_ALIGN_VERTI
                );
            }

            // 날자 표시
            if (cfg.showDate)
            {
                // J2000 기준 타임스탬프 계산
                const __time64_t tt = 946727935 + static_cast<time_t>(today * 24 * 60 * 60);

                // timestamp to tm.
                tm tm;
                _gmtime64_s(&tm, &tt);
                std::snprintf(
                    dateString.data(),
                    dateString.size(),
                    WINDOW_DATE_TEXT,
                    1900 + tm.tm_year,
                    1 + tm.tm_mon,
                    tm.tm_mday,
                    tm.tm_hour,
                    tm.tm_min,
                    tm.tm_sec
                );

                // formatting
                model::Text::instance().draw(
                    cam.screen,
                    viewTextRect,
                    WINDOW_DATE_TEXT_ALIGN_HORIZ,
                    WINDOW_DATE_TEXT_ALIGN_VERTI,
                    WINDOW_DATE_TEXT_SIZE,
                    WINDOW_DATE_TEXT_COLOR,
                    dateString.data()
                );
            }

            // UI 렌더링
            // 속도랑 fps 는 화면 최상위에 올라와야 하기 때문에 UI 부터 렌더링함.
            ui::render(cam.screen, cfg, cam);


            // 속도랑 fps 는 순서대로 쌓아야 하기 때문에...
            v::Rect2f rectRightTop(
                viewTextRect.x,
                viewTextRect.y,
                viewTextRect.w,
                viewTextRect.h
            );

            // 속도 표시
            if (cfg.showSpeed)
            {
                if (speedIs != cfg.speed)
                {
                    speedIs = cfg.speed;

                    // 포메팅
                    std::snprintf(speedString.data(), speedString.size(), WINDOW_SPEED_TEXT_FORMAT, cfg.speed);
                }

                const auto size = model::Text::instance().draw(
                    cam.screen,
                    rectRightTop,
                    WINDOW_SPEED_TEXT_ALIGN_HORIZ,
                    WINDOW_SPEED_TEXT_ALIGN_VERTI,
                    WINDOW_SPEED_TEXT_SIZE,
                    WINDOW_SPEED_TEXT_COLOR,
                    speedString.data()
                );

                rectRightTop.y += size.h;
            }

            // fps
            {
                fpsFrames++;

                // fps 갱신 주기가 지났으면 fps 계산
                const double ms = std::chrono::duration<double, std::milli>(renderStartClock - fpsLastTime).count();
                if (ms >= WINDOW_FPS_UPDATE_INTERVAL)
                {
                    // fps 계산
                    const auto fps = fpsFrames / ms * 1000;

                    // 초기화
                    fpsFrames = 0;
                    fpsLastTime = renderStartClock;

                    // 렌더링 시간 계산 및 포메팅
                    const auto renderTime = std::chrono::duration<double, std::milli>(clock::now() - renderStartClock).count();

                    std::snprintf(fpsString.data(), fpsString.size(), WINDOW_FPS_TEXT_FORMAT, fps, renderTime);
                }

                if (cfg.showFPS)
                {
                    // 위쪽에 속도 썼으면 그만큼 밑으로 내려야 하므로...

                    model::Text::instance().draw(
                        cam.screen,
                        rectRightTop,
                        WINDOW_FPS_TEXT_ALIGN_HORIZ,
                        WINDOW_FPS_TEXT_ALIGN_VERTI,
                        WINDOW_FPS_TEXT_SIZE,
                        WINDOW_FPS_TEXT_COLOR,
                        fpsString.data()
                    );
                }
            }
        }

        // 오예
        modelWatermark.draw(
            cam.screen,
            viewTextRect,
            WINDOW_WATERMARK_TEXT_ALIGN_HORIZ,
            WINDOW_WATERMARK_TEXT_ALIGN_VERTI
        );
    }

    void render(GLFWwindow* window, clock_point renderStartClock, double deltaSeconds)
    {
        LOCK_RENDER;

        // 준비 안됐으면 기다림
        if (!loadedNecessary) return;

        // 현재 카메라 및 설정 정보 가져오기.
        const auto cfg = config::get();
        const auto cam = camera::get();

        /****************************************************************************************************/

        // 영역 설정
        glViewport(
            0,
            0,
            static_cast<int>(cam.screen.w),
            static_cast<int>(cam.screen.h)
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 버퍼 초기화
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 로딩이 끝나야 행성 렌더링을 함.
        if (loadingCompleted)
        {
            /****************************************************************************************************/
            // 애니메이션 처리
            if (cfg.playAnimation)
            {
                // 마지막 렌더링으로부터 지난 시간 (부드럽고 정확한 애니메이션 처리)
                const auto delta = deltaSeconds * cfg.speed;
                today += deltaSeconds * cfg.speed;

                // 행성 위치 계산
                for (auto& p : planet::planetList)
                {
                    p.addDays(delta);
                }
            }

            /****************************************************************************************************/
            // 계산부
            std::vector<RelativeLocation> relPos(planet::planetList.size());

            // 행성 위치 계산
            calcPlanetResolution(relPos);

            // 상대위치 전환
            calcRelativeLocation(relPos, cam);

            /****************************************************************************************************/
            // 렌더링 영역

            if (cfg.showBackground)
            {
                drawBackground(cam);
            }

            // 궤도 그리기
            if (cfg.showOrbit)
            {
                drawOrbit(relPos, cfg, cam);
            }

            // 행성 그리기
            {
                // 광원의 위치는 태양의 위치랑 같다.
                const glm::vec3 solarPosition = cam.matView * relPos.at(0).ModelMatrix * glm::vec4(0, 0, 0, 1);

                drawPlanet(
                    static_cast<float>(deltaSeconds),
                    relPos,
                    cfg,
                    cam,
                    solarPosition
                );
            }
        }

        // 글씨쓰기
        drawUserInterface(cfg, cam, renderStartClock);
    }
}
