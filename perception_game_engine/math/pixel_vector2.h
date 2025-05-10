#pragma once
#include <cmath>
#include <cstdint>
#include <windef.h>

struct pixel_vector2 {
    int x = 0;
    int y = 0;

    pixel_vector2() = default;
    pixel_vector2(int value) : x(value), y(value) {}
    pixel_vector2(int x, int y) : x(x), y(y) {}

    pixel_vector2 operator-() const { return { -x, -y }; }

    pixel_vector2 operator+(const pixel_vector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
    pixel_vector2 operator-(const pixel_vector2& rhs) const { return { x - rhs.x, y - rhs.y }; }
    pixel_vector2 operator*(int scalar) const { return { x * scalar, y * scalar }; }
    pixel_vector2 operator/(int scalar) const { return { x / scalar, y / scalar }; }

    pixel_vector2& operator+=(const pixel_vector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    pixel_vector2& operator-=(const pixel_vector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    pixel_vector2& operator*=(int scalar) { x *= scalar; y *= scalar; return *this; }
    pixel_vector2& operator/=(int scalar) { x /= scalar; y /= scalar; return *this; }

    bool operator==(const pixel_vector2& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const pixel_vector2& rhs) const { return !(*this == rhs); }

    void set(int new_x, int new_y) { x = new_x; y = new_y; }

    int length_squared() const { return x * x + y * y; }
    double length() const { return std::sqrt(static_cast<double>(length_squared())); }
    int area() const { return x * y; }
    double aspect_ratio() const { return (y != 0) ? static_cast<double>(x) / y : 0.0; }

    int dot(const pixel_vector2& rhs) const { return x * rhs.x + y * rhs.y; }

    pixel_vector2 abs() const { return { std::abs(x), std::abs(y) }; }
    pixel_vector2 swap_xy() const { return { y, x }; }

    bool is_zero() const { return x == 0 && y == 0; }
    bool is_positive() const { return x >= 0 && y >= 0; }

    static pixel_vector2 _min(const pixel_vector2& a, const pixel_vector2& b) {
        return { (a.x < b.x ? a.x : b.x), (a.y < b.y ? a.y : b.y) };
    }

    static pixel_vector2 _max(const pixel_vector2& a, const pixel_vector2& b) {
        return { (a.x > b.x ? a.x : b.x), (a.y > b.y ? a.y : b.y) };
    }

    static pixel_vector2 clamp(const pixel_vector2& v, const pixel_vector2& min, const pixel_vector2& max) {
        return {
            (v.x < min.x ? min.x : (v.x > max.x ? max.x : v.x)),
            (v.y < min.y ? min.y : (v.y > max.y ? max.y : v.y))
        };
    }

    POINT to_point() const { return { x, y }; }
    SIZE to_size() const { return { x, y }; }

    static pixel_vector2 zero() { return { 0, 0 }; }
    static pixel_vector2 one() { return { 1, 1 }; }
};
