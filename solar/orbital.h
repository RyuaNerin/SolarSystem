#pragma once

#include <glm/glm.hpp>

namespace orbital
{
    // 케플러의 행성운동법칙에 따른 행성의 위치 계산
    // 기준은 J2000, 2050 년까지 유효.
    // 마지막 값은 진근점이각
    glm::vec4 getCurrentPosition(int planetIndex, double days);
}
