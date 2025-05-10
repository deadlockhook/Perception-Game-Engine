#pragma once
#include <cmath>
#include <cstdint>
#include "vector2.h"

struct vector3 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;

    vector3() = default;
    vector3(double value) : x(value), y(value), z(value) {}
    vector3(double x, double y, double z) : x(x), y(y), z(z) {}

    vector3 operator-() const { return { -x, -y, -z }; }

    vector3 operator+(const vector3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
    vector3 operator-(const vector3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
    vector3 operator*(double scalar) const { return { x * scalar, y * scalar, z * scalar }; }
    vector3 operator/(double scalar) const { return { x / scalar, y / scalar, z / scalar }; }

    vector3& operator+=(const vector3& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; return *this; }
    vector3& operator-=(const vector3& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this; }
    vector3& operator*=(double scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    vector3& operator/=(double scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    bool operator==(const vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const vector3& rhs) const { return !(*this == rhs); }
    double& operator[](int i) { return *((&x) + i); }

    const double& operator[](int i) const { return *((&x) + i); }

    double length_squared() const { return x * x + y * y + z * z; }
    double length() const { return std::sqrt(length_squared()); }
  
    bool equals(const vector3& rhs, double epsilon = 1e-6) const {
        return std::abs(x - rhs.x) < epsilon &&
            std::abs(y - rhs.y) < epsilon &&
            std::abs(z - rhs.z) < epsilon;
    }

    vector3 normalized() const {
        double len = length();
        return (len != 0.0) ? (*this / len) : vector3::zero();
    }

    void normalize() {
        double len = length();
        if (len != 0.0) {
            x /= len; y /= len; z /= len;
        }
    }

    vector3 normalized()
    {
		double len = length();
		return (len != 0.0) ? (*this / len) : vector3::zero();
    }

    double dot(const vector3& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }

    vector3 cross(const vector3& rhs) const {
        return {
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        };
    }

    vector3 abs() const {
        return { std::abs(x), std::abs(y), std::abs(z) };
    }

    double distance_to(const vector3& other) const {
        return (*this - other).length();
    }

    double distance_squared_to(const vector3& other) const {
        return (*this - other).length_squared();
    }

    double angle_between(const vector3& other) const {
        double dot_val = dot(other);
        double len_product = length() * other.length();
        return (len_product != 0.0) ? std::acos(std::fmax(-1.0, std::fmin(1.0, dot_val / len_product))) : 0.0;
    }

    vector3 project_onto(const vector3& normal) const {
        double denom = normal.length_squared();
        return denom != 0.0 ? normal * (dot(normal) / denom) : vector3::zero();
    }

    vector3 reflect(const vector3& normal) const {
        return *this - normal * (2.0 * dot(normal));
    }

    static vector3 lerp(const vector3& a, const vector3& b, double t) {
        return a + (b - a) * t;
    }

    static vector3 clamp(const vector3& v, const vector3& min, const vector3& max) {
        return {
            (v.x < min.x ? min.x : (v.x > max.x ? max.x : v.x)),
            (v.y < min.y ? min.y : (v.y > max.y ? max.y : v.y)),
            (v.z < min.z ? min.z : (v.z > max.z ? max.z : v.z))
        };
    }

    vector3 safe_normalized(const vector3& fallback = vector3::zero()) const {
        double len = length();
        return (len != 0.0) ? (*this / len) : fallback;
    }
    
    bool is_zero() const {
        return x == 0.0 && y == 0.0 && z == 0.0;
    }

    bool is_finite() const {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }

    bool is_normalized(double epsilon = 1e-8) const {
        return std::abs(length_squared() - 1.0) < epsilon;
    }

    vector2 xy() const { return { x, y }; }
    vector2 xz() const { return { x, z }; }
    vector2 yz() const { return { y, z }; }

    static vector3 unit_x() { return { 1.0, 0.0, 0.0 }; }
    static vector3 unit_y() { return { 0.0, 1.0, 0.0 }; }
    static vector3 unit_z() { return { 0.0, 0.0, 1.0 }; }

    static vector3 zero() { return { 0.0, 0.0, 0.0 }; }
    static vector3 one() { return { 1.0, 1.0, 1.0 }; }
    static vector3 up() { return { 0.0, 1.0, 0.0 }; }
    static vector3 down() { return { 0.0, -1.0, 0.0 }; }
    static vector3 left() { return { -1.0, 0.0, 0.0 }; }
    static vector3 right() { return { 1.0, 0.0, 0.0 }; }
    static vector3 forward() { return { 0.0, 0.0, 1.0 }; }
    static vector3 backward() { return { 0.0, 0.0, -1.0 }; }
};
