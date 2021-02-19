#pragma once

#include <mutex>
#include <vector>

namespace config
{
    enum OrbitType : int
    {
        Dotted, Solid
    };

    struct Config
    {
        float speed; // 재생 속도

        bool vsync; // 수직동기화

        bool playAnimation; // 일시정지됨

        bool showDate; // 날자 표시
        bool showSpeed; // 애니메이션 배속 표시
        bool showFPS; // fps 표시

        bool showHelp; // 도움말 표시

        bool showOrbit; // 궤도 표시

        bool showOrbitColored; // 궤도에 색상 넣기
        bool showOrbitGradient;
        int  showOrbitType; // 궤도를 선으로 표시

        bool showPlanetName; // 행성 이름 표시

        bool showBackground; // 배경 표시

        bool enableMSAA; // MSAA 켜기/끄기
    };

    Config reset(); // 설정 초기화
    Config update(const Config cfg); // 설정 업데이트
    
    Config get(); // 현재 설정 가져오기

    void speedDelta(float delta, bool isMouse); // 재생 속도 변경

    void togglePlay(); // 애니메이션 켜기/끄기
    void toggleHelp(); // 도움말 켜기/끄기

    void pause(); // 애니메이션 정지
    void play(); // 애니메이션 재생
};

