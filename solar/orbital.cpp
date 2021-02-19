#include "orbital.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "constants.h"
#include "planet.h"
#include "v.h"

// 참고자료
// https://ssd.jpl.nasa.gov/?planet_pos
// 케플러 상수 : https://ssd.jpl.nasa.gov/txt/p_elem_t1.txt
// 게산 공식 : https://ssd.jpl.nasa.gov/txt/aprx_pos_planets.pdf

namespace orbital
{
    //constexpr double MOON_AU = au2unit(0.00257f);
    constexpr double MOON_AU = au2unit(0.0257f);

    constexpr double PI   = glm::pi<double>();
    constexpr double PI2  = glm::two_pi<double>();
    constexpr double PI_2 = glm::half_pi<double>();

    constexpr double KEPLER_EQUAION_MAX_TRY   = 20; // KeplerEquaion 구할 때 쓸 최대 반복수
    constexpr double KEPLER_EQUAION_TOLERANCE = 10e-6; // KeplerEquaion 허용 오차

    constexpr double daysPerCentry = 100 * 365.25;

    struct Elements
    {
        double a; // 장반경
        double e; // 이심률
        double I; // 기울기
        double L; // 평균 경도
        double Lp; // 근일점 경도
        double o; // 승교점 적경

        constexpr Elements() : a(0), e(0), I(0), L(0), Lp(0), o(0) { };
        constexpr Elements(
            double a,
            double e,
            double i,
            double l,
            double lp,
            double o
        ) : a(au2unit(a)),
            e(glm::radians(e)),
            I(glm::radians(i)),
            L(glm::radians(l)),
            Lp(glm::radians(lp)),
            o(glm::radians(o))
        {
            // 단위 변경...
            // degree 는 radian 으로, AU 단위도 내부단위로
        }
    };
    struct J2000
    {
        const Elements base;
        const Elements delta;

        constexpr J2000(Elements base, Elements delta) : base(base), delta(delta)
        {
        }
    };

    // https://ssd.jpl.nasa.gov/txt/p_elem_t1.txt
    // Keplerian elements and their rates, with respect to the mean ecliptic
    // and equinox of J2000, valid for the time - interval 1800 AD - 2050 AD.
    const std::vector<J2000> elements =
    {
        { {}, {} }, // 태양
        //        a              e               I                L            long.peri.      long.node.
        //    AU, AU/Cy     rad, rad/Cy     deg, deg/Cy      deg, deg/Cy      deg, deg/Cy     deg, deg/Cy
        { {  0.38709927,     0.20563593,     7.00497902,     252.25032350,    77.45779628,    48.33076593, },
          {  0.00000037,     0.00001906,    -0.00594749,  149472.67411175,     0.16047689,    -0.12534081, } }, // 수
        { {  0.72333566,     0.00677672,     3.39467605,     181.97909950,   131.60246718,    76.67984255, },
          {  0.00000390,    -0.00004107,    -0.00078890,   58517.81538729,     0.00268329,    -0.27769418, } }, // 금
        { {  1.00000261,     0.01671123,    -0.00001531,     100.46457166,   102.93768193,     0.00000000, },
          {  0.00000562,    -0.00004392,    -0.01294668,   35999.37244981,     0.32327364,     0.00000000, } }, // 지
        { {  1.52371034,     0.09339410,     1.84969142,      -4.55343205,   -23.94362959,    49.55953891, },
          {  0.00001847,    -0.00007882,    -0.00813131,   19140.30268499,     0.44441088,    -0.29257343, } }, // 화
        { {  5.20288700,     0.04838624,     1.30439695,      34.39644051,    14.72847983,   100.47390909, },
          { -0.00011607,    -0.00013253,    -0.00183714,    3034.74612775,     0.21252668,     0.20469106, } }, // 목
        { {  9.53667594,     0.05386179,     2.48599187,      49.95424423,    92.59887831,   113.66242448, },
          { -0.00125060,    -0.00050991,     0.00193609,    1222.49362201,    -0.41897216,    -0.28867794, } }, // 토
        { { 19.18916464,     0.04725744,     0.77263783,     313.23810451,   170.95427630,    74.01692503, },
          { -0.00196176,    -0.00004397,    -0.00242939,     428.48202785,     0.40805281,     0.04240589, } }, // 천
        { { 30.06992276,     0.00859048,     1.77004347,     -55.12002969,    44.96476227,   131.78422574, },
          {  0.00026291,     0.00005105,     0.00035372,     218.45945325,    -0.32241464,    -0.00508664, } }, // 해
        { { 39.48211675,     0.24882730,    17.14001206,     238.92903833,   224.06891629,   110.30393684, },
          { -0.00031596,     0.00005170,     0.00004818,     145.20780515,    -0.04062942,    -0.01183482, } }, // 명
    };

    double getEccentricity(const double M, const double e) {
        double M1 = 0;
        double M2 = M;

        for (int i = 0; i < KEPLER_EQUAION_MAX_TRY; i++)
        {
            M1 = M2;
            M2 = M2 +   (M + e * glm::sin(M1) - M1)
                      / (1 - e * glm::cos(M1));
            if (glm::abs(M1 - M2) < KEPLER_EQUAION_TOLERANCE)
                break;
        }

        return M2;
    }

    glm::vec4 getCurrentPosition(int planetIndex, double days)
    {
        // 달은 비교적 정확하지 않은 궤도를 그리기 때문에 궤도를 표시하기 어려운 관계로 고정궤도로 이동시킴.
        if (planetIndex == planet::PlanetIndex::Moon)
        {
            // https://en.wikipedia.org/wiki/Orbit_of_the_Moon#/media/File:Lunar_Orbit_and_Orientation_with_respect_to_the_Ecliptic.tif
            // 5.14 도 기울게
            constexpr auto sy = 0.0895896429900152875272158894646;  // sin 5.14
            constexpr auto cy = 0.99597876275999058304925149124373; // cos 5.14

            const auto theta = PI2 * std::fmod(days, 27.321f) / 27.321f;

            const auto c = std::cos(theta);
            const auto s = std::sin(theta);

            return glm::vec4(
                MOON_AU * c * cy,
                MOON_AU * c * sy,
                MOON_AU * -s,
                glm::degrees(theta)
            );
        }

        const J2000 kec = elements.at(planetIndex);

        auto ke = kec.base;

        // 2.
        {
            const auto delta = days / daysPerCentry;

            ke.a  += kec.delta.a  * delta;
            ke.e  += kec.delta.e  * delta;
            ke.I  += kec.delta.I  * delta;
            ke.L  += kec.delta.L  * delta;
            ke.Lp += kec.delta.Lp * delta;
            ke.o  += kec.delta.o  * delta;
        }

        // 3.
        const auto w = ke.Lp - ke.o;
        const auto M = std::fmod(ke.L - ke.Lp, PI2) - PI;

        // 4.
        const auto E = getEccentricity(M, ke.e);

        const double x = ke.a * (glm::cos(E) - ke.e);
        const double y = ke.a * (glm::sqrt(1 - ke.e * ke.e) * glm::sin(E));

        const double cosw = glm::cos(w);
        const double sinw = glm::sin(w);
        const double cosO = glm::cos(ke.o);
        const double sinO = glm::sin(ke.o);
        const double cosI = glm::cos(ke.I);
        const double sinI = glm::sin(ke.I);

        const double Xecl = (cosw * cosO - sinw * sinO * cosI) * x + (-sinw * cosO - cosw * sinO * cosI) * y;
        const double Yecl = (cosw * sinO + sinw * cosO * cosI) * x + (-sinw * sinO + cosw * cosO * cosI) * y;
        const double Zecl =                      (sinw * sinI) * x +                       (cosw * sinI) * y;

        double v = glm::atan(y, x);
        if (v < 0) v = PI2 + v;

        //Result array
        return glm::vec4(
            Xecl,
            Zecl, // Y 축으로 회전시키기 위해서 Z 축과 Y 축 교환
            -Yecl, // opengl 의 y 축은 밑에서부터.
            glm::degrees(v)
        );
    }
}
