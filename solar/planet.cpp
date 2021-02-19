#include "planet.h"

#include <iostream>
#include <random>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils.h"

namespace planet
{
    std::vector<Planet> planetList =
    {
        //     행성이름              색상      이름       크기km      공전 day     자전 day    자전축    부모
        //Planet(PlanetIndex::Sun,     0x000000, "Sun",     1392684.0f,     0.000f,    27.050f,     7.25f, PlanetIndex::None), // 태양
        Planet(PlanetIndex::Sun,     0x000000, "Sun",      100000.0f,     0.000f,    27.050f,     7.25f, PlanetIndex::None ), // 태양
        Planet(PlanetIndex::Mercury, 0xcfcfc4, "Mercury",    4879.4f,    87.969f,    58.646f,     0.03f, PlanetIndex::Sun  ), // 수성
        Planet(PlanetIndex::Venus,   0xe9d1bf, "Venus",     12103.7f,   224.701f,   243.025f,   177.00f, PlanetIndex::Sun  ), // 금성
        Planet(PlanetIndex::Earth,   0x89cff0, "Earth",     12756.2f,   365.242f,     0.997f,    23.43f, PlanetIndex::Sun  ), // 지구
        Planet(PlanetIndex::Mars,    0xff9899, "Mars",       6792.4f,   686.971f,     1.025f,    25.19f, PlanetIndex::Sun  ), // 화성
        Planet(PlanetIndex::Jupiter, 0xe5d9d3, "Jupiter",  142984.0f,  4332.590f,     0.413f,     3.13f, PlanetIndex::Sun  ), // 목성
        Planet(PlanetIndex::Saturn,  0xfdfd96, "Saturn",   120536.0f, 10759.220f,     0.440f,    26.73f, PlanetIndex::Sun  ), // 토성
        Planet(PlanetIndex::Uranus,  0xd6fffe, "Uranus",    51118.0f, 30707.489f,    27.321f,    97.77f, PlanetIndex::Sun  ), // 천왕성
        Planet(PlanetIndex::Neptune, 0xc1c6fc, "Neptune",   49528.0f, 60182.000f,     0.671f,    28.32f, PlanetIndex::Sun  ), // 해왕성
        Planet(PlanetIndex::Pluto,   0xc6a4a4, "Pluto",      1188.3f, 90560.000f,     6.387f,   115.60f, PlanetIndex::Sun  ), // 명왕성
        Planet(PlanetIndex::Moon,    0xdfd8e1, "Moon",       3474.2f,    27.321f,    27.321f,     6.68f, PlanetIndex::Earth), // 달
    };

    Planet::Planet(
        int index,
        v::Color color,
        const char* name,
        float diameter,
        float resolutionPeriod,
        float rotationPeriod,
        float angle,
        int parentIndex
    ) : _index(index),
        _color(color),
        _name(name),
        _radius(km2unit(diameter) / 2 * PLANET_SIZE_MUL),
        _resolutionPeriod(resolutionPeriod),
        _rotationPeriod(rotationPeriod),
        _angleRad(glm::radians(angle)),
        _parentIndex(parentIndex),
        rotationSum(utils::rand() * rotationPeriod)
    {
    }

    // 현재 위치 계산하는 함수
    void Planet::addDays(double dayDelta)
    {
        // 자전
        this->rotationSum += dayDelta;
        while (this->rotationSum > this->_rotationPeriod)
            this->rotationSum -= this->_rotationPeriod;

        this->rotationThetaRad = static_cast<float>(PI2 * this->rotationSum / this->_rotationPeriod);
    }

    glm::mat4 Planet::getLocalMatrix(bool axisOnly) const
    {
        glm::mat4 mat(1);

        // 자전축 기울이기
        mat = glm::rotate(mat, -this->_angleRad, glm::vec3(0, 0, 1));

        if (!axisOnly)
        {
            // 자전
            mat = glm::rotate(mat, this->rotationThetaRad, glm::vec3(0, 1, 0));
        }

        return mat;
    }
}
