#pragma once

#include <algorithm>
#include <array>
#include <iostream>
#include <ostream>
#include <vector>

#include <glm/glm.hpp>

namespace v
{
    // rgba 컬러를 정의합니다. 곰셈 연산자를 사용할 수 있습니다.
    class Color
    {
    public:
        union
        {
            struct { float r, g, b, a; };
            struct { float raw[4]; };
        };

        Color() = default;
        constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) { }
        constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) { };
        constexpr Color(int color) :
            r(((color >> 16) & 0xFF) / 255.0f),
            g(((color >>  8) & 0xFF) / 255.0f),
            b(((color >>  0) & 0xFF) / 255.0f),
            a(color > 0x00FFFFFF ? ((color >> 24) & 0xFF) / 255.0f : 1.0f)
        { }
        constexpr Color(int color, float a) :
            r(((color >> 16) & 0xFF) / 255.0f),
            g(((color >>  8) & 0xFF) / 255.0f),
            b(((color >>  0) & 0xFF) / 255.0f),
            a(a)
        { }

        constexpr operator       float* ()       { return &this->raw[0]; }
        constexpr operator const float* () const { return &this->raw[0]; }

        constexpr Color operator*(const Color v)       { return Color(this->r * v.r, this->g * v.g, this->b * v.b, this->a * v.a); }
        constexpr Color operator*(const Color v) const { return Color(this->r * v.r, this->g * v.g, this->b * v.b, this->a * v.a); }

        constexpr Color operator*(const float v)       { return Color(this->r * v, this->g * v, this->b * v, this->a * v); }
        constexpr Color operator*(const float v) const { return Color(this->r * v, this->g * v, this->b * v, this->a * v); }
    };

    /****************************************************************************************************/

    // 2차원상의 좌표를 정의합니다
    template<typename T>
    class Point2Base
    {
    public:
        union
        {
            struct { T x, y; };
            struct { T raw[2]; };
        };

        constexpr Point2Base() = default;
        constexpr Point2Base(T v) : x(v), y(v) { }
        constexpr Point2Base(T r, T g) : x(r), y(g) { }

        template <typename T2>
        constexpr Point2Base<T>& operator=(Point2Base<T2> arg) noexcept
        {
            this->x = static_cast<T>(arg.x);
            this->y = static_cast<T>(arg.y);
            return this;
        }

        template <typename T2>
        constexpr operator Point2Base<T2> () noexcept
        {
            Point2Base<T2> v;
            v.x = static_cast<T2>(this->x);
            v.y = static_cast<T2>(this->y);
            return v;
        }

        constexpr operator glm::vec2()
        {
            return glm::vec2(static_cast<float>(this.x), static_cast<float>(this.y));
        }
    };
    typedef Point2Base<double> Point2d; // double 형 point2
    typedef Point2Base<float>  Point2f; // float  형 point2
    typedef Point2Base<int>    Point2i; // int    형 point2

    /****************************************************************************************************/

    // 3차원상의 좌표를 정의합니다
    template<typename T>
    class Point3Base
    {
    public:
        union
        {
            struct { T x, y, z; };
            struct { T raw[3]; };
        };

        constexpr Point3Base() = default;
        constexpr Point3Base(T v) : x(v), y(v), z(v) { }
        constexpr Point3Base(T x, T y, T z) : x(x), y(y), z(z) { }

        constexpr operator       T* ()       { return &this->raw[0]; }
        constexpr operator const T* () const { return &this->raw[0]; }
    };
    typedef Point3Base<float>  Point3f; // int    형 point3

    /****************************************************************************************************/

    // 2차원상의 크기를 정의합니다.
    template<typename T>
    class Size2Base
    {
    public:
        union
        {
            struct { T w, h; };
            struct { T raw[2]; };
        };

        constexpr Size2Base() = default;
        constexpr Size2Base(T v) : w(v), h(v) { }
        constexpr Size2Base(T r, T g) : w(r), h(g) { }
        
        constexpr bool operator==(const Size2Base<T> arg) noexcept { return this->w == arg.w && this->h == arg.h; }
        constexpr bool operator!=(const Size2Base<T> arg) noexcept { return this->w != arg.w || this->h != arg.h; }

        template<typename T2>
        constexpr operator Size2Base<T2>()
        {
            return Size2Base<T2>(
                static_cast<T2>(this->w),
                static_cast<T2>(this->h)
            );
        }
        template<typename T2>
        constexpr operator Size2Base<T2>() const
        {
            return Size2Base<T2>(
                static_cast<T2>(this->w),
                static_cast<T2>(this->h)
            );
        }
    };
    typedef Size2Base<float> Size2f; // float 형 point
    typedef Size2Base<int>   Size2i; // int   형 point

    /****************************************************************************************************/

    // 2차원상의 좌표와 크기를 정의합니다.
    template<typename T>
    class Rect2Base
    {
    public:
        union
        {
            struct { T x, y, w, h; };
            struct { T raw[4]; };
        };

        constexpr Rect2Base() = default;
        constexpr Rect2Base(T v) : x(v), y(v), w(v), h(v) { }
        constexpr Rect2Base(T x, T y, T w, T h) : x(x), y(y), w(w), h(h) { }
        constexpr Rect2Base(Point2Base<T> loc, Size2Base<T> sz) : x(loc.x), y(loc.y), w(sz.w), h(sz.h) { }
    };
    typedef Rect2Base<float> Rect2f; // float 형 rect2
    typedef Rect2Base<int>   Rect2i; // int 형 rect2

    /****************************************************************************************************/

    // 2차원상의 시작 좌표와 끝 좌표를 정의합니다.
    template<typename T>
    class Bound2Base
    {
    public:
        union
        {
            struct { T l, t, r, b; };
            struct { T raw[4]; };
        };

        constexpr Bound2Base() = default;
        constexpr Bound2Base(T v) : l(v), t(v), r(v), b(v) { }
        constexpr Bound2Base(T l, T t, T r, T b) : l(l), t(t), r(r), b(b) { }
    };
    typedef Bound2Base<int> Bound2i; // int 형 bound2

    /****************************************************************************************************/

    // 범위를 정의합니다.
    template<typename T>
    class Range
    {
    public:
        union
        {
            struct { T valueMin, valueMax; };
            struct { T raw[2]; };
        };

        constexpr Range() = default;
        constexpr Range(T v) : valueMin(v), valueMax(v) { };
        constexpr Range(T min, T max) : valueMin(min), valueMax(max) {};

        constexpr T operator*(const T v)       { return this->valueMin + (this->valueMax - this->valueMin) * v; }
        constexpr T operator*(const T v) const { return this->valueMin + (this->valueMax - this->valueMin) * v; }
    };
    typedef Range<float> Rangef;
    typedef Range<int> Rangei;
}
