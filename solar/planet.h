// 행성 정보 선언부
#pragma once

#include <functional>

#include <glm/mat4x4.hpp>

#include "glext.h"
#include "v.h"

namespace planet
{
    class Planet;

    enum PlanetIndex
    {
        None = -1,
        Sun,     // 태양
        Mercury, // 수성
        Venus,   // 금성
        Earth,   // 지구
        Mars,    // 화성
        Jupiter, // 목성
        Saturn,  // 토성
        Uranus,  // 천왕성
        Neptune, // 해왕성
        Pluto,   // 명왕성
        Moon,    // 달
    };

    // 행성 목록. planet.cpp 에 선언되어 있음.
    extern std::vector<Planet> planetList;

    class Planet
    {
    public:
        /****************************************************************************************************/
        // 행성 기본 정보

        int const _index; // 행성 Index
        v::Color const _color; // 행성 궤도 고유 색

        const char* const _name; // 이름

        float const _radius; // 행성 반지름

        float const _resolutionPeriod; // 공전 주기
        float const _rotationPeriod;   // 자전 주기

        float const _angleRad; // 자전축 12시 0

        int const _parentIndex; // 부모 행성

        /****************************************************************************************************/
        // 행성 위치 정보

        double rotationSum = 0; // 자전 delta 누적값
        float rotationThetaRad = 0; // 자전 각도

        /****************************************************************************************************/

        // Planet 초기화용 생성자
        Planet(int index, v::Color color, const char* name, float diameter, float resolutionPeriod, float rotationPeriod, float angle, int parentIndex);

        // 현재 위치 계산하는 함수
        void addDays(double dayDelta);

        // 자전용 LocalMatrix 가져오는 함수. axisOnly 가 true 면 자전축만큼만 회전합니다.
        glm::mat4 getLocalMatrix(bool axisOnly) const;

        // 하위 행성 반복하는 iter
        // foreach<      >(std::function<int,       >        );
        // foreach<T1    >(std::function<int, T1    >, T1    );
        // foreach<T1, T2>(std::function<int, T1, T2>, T1, T2);
        template <
            typename... Targs,
            typename STD_FUNCTION_T = std::function<void(Targs..., int)>
        >
        constexpr void foreach(STD_FUNCTION_T const& f, Targs... Fargs) const
        {
            for (size_t subIndex = 0; subIndex < planet::planetList.size(); subIndex++)
            {
                // 같은 행성 처리 안함
                if (subIndex == this->_index) continue;

                // 자식 행성 처리
                if (planet::planetList.at(subIndex)._parentIndex == this->_index)
                {
                    f(Fargs..., static_cast<int>(subIndex));
                }
            }
        }
    };
}

