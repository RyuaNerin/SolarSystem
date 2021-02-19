#include "config.h"

#include <algorithm>

#include "constants.h"

namespace config
{
    // 동기화용 변수
    std::mutex lock;
    #define LOCK_CONFIG std::lock_guard<std::mutex> __lock_camera(lock);

    Config cfg;

    Config reset()
    {
        Config cfgNew{};

        cfgNew.vsync = true;

        cfgNew.speed = SPEED_DEFAULT;

        cfgNew.playAnimation = true;

        cfgNew.showDate = true;
        cfgNew.showSpeed = true;
        cfgNew.showFPS = true;

        cfgNew.showHelp = true;

        cfgNew.showOrbit = true;
        cfgNew.showOrbitColored = true;
        cfgNew.showOrbitGradient = true;
        cfgNew.showOrbitType = OrbitType::Solid;

        cfgNew.showPlanetName = true;

        cfgNew.showBackground = true;

        cfgNew.enableMSAA = true;

        return update(cfgNew);
    }

    Config get()
    {
        LOCK_CONFIG;
        return cfg;
    }

    void speedDelta(float delta, bool byMouse)
    {
        LOCK_CONFIG;

        // 범위 자르기
        // 변화값은 𝑦=𝑘^𝑥 형태임
        cfg.speed = std::clamp(
            cfg.speed * std::pow(byMouse ? SPEED_DELTA_SPEED_WHEEL : SPEED_DELTA_SPEED_KEY, delta),
            SPEED_MIN,
            SPEED_MAX
        );

        std::cout << "speed :" << cfg.speed << std::endl;
    }

    Config update(const Config cfgNew)
    {
        LOCK_CONFIG;

        // 값 업데이트
        cfg.speed             = cfgNew.speed;
        cfg.playAnimation     = cfgNew.playAnimation;
        cfg.showDate          = cfgNew.showDate;
        cfg.showSpeed         = cfgNew.showSpeed;
        cfg.showFPS           = cfgNew.showFPS;
        cfg.showHelp          = cfgNew.showHelp;
        cfg.showOrbit         = cfgNew.showOrbit;
        cfg.showOrbitColored  = cfgNew.showOrbitColored;
        cfg.showOrbitGradient = cfgNew.showOrbitGradient;
        cfg.showOrbitType     = cfgNew.showOrbitType;
        cfg.showPlanetName    = cfgNew.showPlanetName;
        cfg.showBackground    = cfgNew.showBackground;

        // MSAA 값 변화
        if (cfg.enableMSAA != cfgNew.enableMSAA)
        {
            cfg.enableMSAA = cfgNew.enableMSAA;
            if (cfgNew.enableMSAA)
            {
                glEnable(GL_MULTISAMPLE);
            }
            else
            {
                glDisable(GL_MULTISAMPLE);
            }
        }

        // 수직동기화
        if (cfg.vsync != cfgNew.vsync)
        {
            cfg.vsync = cfgNew.vsync;
            if (cfgNew.vsync)
            {
                glfwSwapInterval(1);
            }
            else
            {
                glfwSwapInterval(0);
            }
        }

        return cfgNew;
    }

    void togglePlay()
    {
        LOCK_CONFIG;
        cfg.playAnimation = !cfg.playAnimation;
    }
    void toggleHelp()
    {
        LOCK_CONFIG;
        cfg.showHelp = !cfg.showHelp;
    }

    void pause()
    {
        LOCK_CONFIG;
        cfg.playAnimation = false;
    }

    void play()
    {
        LOCK_CONFIG;
        cfg.playAnimation = true;
    }
}
