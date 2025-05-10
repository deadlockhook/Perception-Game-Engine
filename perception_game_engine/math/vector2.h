#pragma once
#include <cmath>
#include <cstdint>

struct vector2 {
    double x = 0.0;
    double y = 0.0;

    vector2() = default;
    vector2(double value) : x(value), y(value) {}
    vector2(double x, double y) : x(x), y(y) {}

    vector2 operator-() const { return { -x, -y }; }

    vector2 operator+(const vector2& rhs) const { return { x + rhs.x, y + rhs.y }; }
    vector2 operator-(const vector2& rhs) const { return { x - rhs.x, y - rhs.y }; }
    vector2 operator*(double scalar) const { return { x * scalar, y * scalar }; }
    vector2 operator/(double scalar) const { return { x / scalar, y / scalar }; }

    vector2& operator+=(const vector2& rhs) { x += rhs.x; y += rhs.y; return *this; }
    vector2& operator-=(const vector2& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
    vector2& operator*=(double scalar) { x *= scalar; y *= scalar; return *this; }
    vector2& operator/=(double scalar) { x /= scalar; y /= scalar; return *this; }

    bool operator==(const vector2& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const vector2& rhs) const { return !(*this == rhs); }

    double length_squared() const { return x * x + y * y; }
    double length() const { return std::sqrt(length_squared()); }

    vector2 normalized() const {
        double len = length();
        return (len != 0.0) ? (*this / len) : vector2::zero();
    }

    void normalize() {
        double len = length();
        if (len != 0.0) {
            x /= len;
            y /= len;
        }
    }

    double dot(const vector2& rhs) const {
        return x * rhs.x + y * rhs.y;
    }

    double cross(const vector2& rhs) const {
        return x * rhs.y - y * rhs.x;
    }

    vector2 perpendicular() const {
        return { -y, x };
    }

    vector2 rotated(double radians) const {
        double c = std::cos(radians);
        double s = std::sin(radians);
        return { x * c - y * s, x * s + y * c };
    }

    vector2 abs() const {
        return { std::abs(x), std::abs(y) };
    }

    double angle_between(const vector2& other) const {
        double len_product = length() * other.length();
        if (len_product == 0.0) return 0.0;
        double cos_angle = dot(other) / len_product;
        return std::acos(std::fmax(-1.0, std::fmin(1.0, cos_angle))); // Clamp for safety
    }

    double distance_to(const vector2& other) const {
        return (*this - other).length();
    }

    double distance_squared_to(const vector2& other) const {
        return (*this - other).length_squared();
    }

    vector2 safe_normalized(const vector2& fallback = vector2::zero()) const {
        double len = length();
        return (len != 0.0) ? (*this / len) : fallback;
    }

    vector2 project_onto(const vector2& normal) const {
        double denom = normal.length_squared();
        return denom != 0.0 ? normal * (dot(normal) / denom) : vector2::zero();
    }

    vector2 reflect(const vector2& normal) const {
        return *this - normal * (2.0 * dot(normal));
    }

    static vector2 clamp(const vector2& v, const vector2& min, const vector2& max) {
        return {
            (v.x < min.x ? min.x : (v.x > max.x ? max.x : v.x)),
            (v.y < min.y ? min.y : (v.y > max.y ? max.y : v.y))
        };
    }

    static vector2 lerp(const vector2& a, const vector2& b, double t) {
        return a + (b - a) * t;
    }

    static vector2 zero() { return { 0.0, 0.0 }; }
    static vector2 one() { return { 1.0, 1.0 }; }
    static vector2 up() { return { 0.0, 1.0 }; }
    static vector2 down() { return { 0.0, -1.0 }; }
    static vector2 left() { return { -1.0, 0.0 }; }
    static vector2 right() { return { 1.0, 0.0 }; }
};
