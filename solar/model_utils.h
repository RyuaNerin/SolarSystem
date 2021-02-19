#pragma once

#include <cmath>
#include <vector>

#include <glad/glad.h>

#include "constants.h"

namespace model
{
    // 3차원 구체 버텍스 찍는 함수.
    // indices 마지막에 GL_PRIMITIVE_RESTART = RESTART_INDEX 인거 주의!!!
    // VVV NNN UV | VVV NNN UV | VVV NNN UV | ...
    void initSphere(
        float radius,
        int stackCount,
        int sectorCount,
        std::vector<GLfloat>& vertices,
        std::vector<GLuint>& indices,
        bool withNormal,
        bool withTexCoords = true
    )
    {
        // 버텍스들 계산하기
        {
            // 1 버텍스당 데이터 수
            int v = 3;
            if (withNormal) v += 3;
            if (withTexCoords) v += 2;

            // 미리 할당
            vertices.resize((stackCount + 1) * (sectorCount + 1) * v);

            // 메모리로 접근할거임.
            auto virticesIt = vertices.begin();

            // 법선벡터 구하는거
            const float inverseRadius = 1.0f / radius;

            for (int stack = 0; stack <= stackCount; stack++)
            {
                // 스택 각도
                const auto stackAngle = PI_2 - PI * stack / stackCount;

                // 스택 좌표
                const auto y = radius * std::sinf(stackAngle);
                const auto xz = radius * std::cosf(stackAngle);

                for (int sector = 0; sector <= sectorCount; sector++)
                {
                    // 섹터 각도
                    const auto sectorAngle = PI2 * sector / sectorCount;

                    // 현재 섹터 좌표
                    const auto x = xz * std::sinf(sectorAngle);
                    const auto z = xz * std::cosf(sectorAngle);

                    // 버텍스 설정
                    *virticesIt++ = x;
                    *virticesIt++ = y;
                    *virticesIt++ = z;

                    if (withNormal)
                    {
                        // 노말 벡터
                        *virticesIt++ = x * inverseRadius;
                        *virticesIt++ = y * inverseRadius;
                        *virticesIt++ = z * inverseRadius;
                    }

                    // 텍스쳐 매핑
                    if (withTexCoords)
                    {
                        *virticesIt++ = static_cast<float>(sector) / stackCount;
                        *virticesIt++ = static_cast<float>(stack ) / sectorCount;
                    }
                }
            }
        }

        // index 계산하는 부분
        /*
        p11 - p12
        |   /   |
        p21 - p22

               반시계 (바깥쪽)
        위   : p11 > p21 > p12
        아래 : p12 > p21 > p22
        */
        {
            indices.resize((2 * stackCount - 2) * (sectorCount) * 3);
            auto indicesIt = indices.begin();

            for (int stack = 0; stack < stackCount; stack++)
            {

                for (int sector = 0; sector < sectorCount; sector++)
                {
                    auto p11 = (stack + 0) * (sectorCount + 1) + sector;
                    auto p12 = (stack + 0) * (sectorCount + 1) + sector + 1;

                    auto p21 = (stack + 1) * (sectorCount + 1) + sector;
                    auto p22 = (stack + 1) * (sectorCount + 1) + sector + 1;

                    // 위쪽
                    if (stack > 0)
                    {
                        *indicesIt++ = p11;
                        *indicesIt++ = p21;
                        *indicesIt++ = p12;
                    }

                    // 아래쪽
                    if (stack < (stackCount - 1))
                    {
                        *indicesIt++ = p12;
                        *indicesIt++ = p21;
                        *indicesIt++ = p22;
                    }
                }
            }
        }
    }
}
