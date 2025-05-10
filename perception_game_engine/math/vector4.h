#pragma once
#define NOMINMAX
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>
#include "vector3.h"

struct vector4 {
    double x, y, z, w;

    vector4() : x(0), y(0), z(0), w(0) {}
    vector4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
    vector4(const vector3& v, double w) : x(v.x), y(v.y), z(v.z), w(w) {}
    vector4(const vector2& v, double z, double w) : x(v.x), y(v.y), z(z), w(w) {}

    static vector4 zero() { return vector4(0, 0, 0, 0); }
    static vector4 one() { return vector4(1, 1, 1, 1); }

    explicit operator vector3() const { return vector3(x, y, z); }
    explicit operator double* () { return &x; }

    vector4 operator+(const vector4& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w }; }
    vector4 operator-(const vector4& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w }; }
    vector4 operator*(double scalar) const { return { x * scalar, y * scalar, z * scalar, w * scalar }; }
    vector4 operator/(double scalar) const { return { x / scalar, y / scalar, z / scalar, w / scalar }; }

    vector4& operator+=(const vector4& rhs) { x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this; }
    vector4& operator-=(const vector4& rhs) { x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this; }
    vector4& operator*=(double scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    vector4& operator/=(double scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

    bool operator==(const vector4& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w; }
    bool operator!=(const vector4& rhs) const { return !(*this == rhs); }

    double& operator[](int i) {
        return *((&x) + i);
    }

    const double& operator[](int i) const {
        return *((&x) + i);
    }

    bool is_zero(double epsilon = 1e-8) const {
        return std::abs(x) < epsilon && std::abs(y) < epsilon &&
            std::abs(z) < epsilon && std::abs(w) < epsilon;
    }

    bool is_finite() const {
        return std::isfinite(x) && std::isfinite(y) &&
            std::isfinite(z) && std::isfinite(w);
    }

    vector4 select(const vector4& a, const vector4& b, const vector4& mask) const {
        return vector4(
            mask.x != 0 ? a.x : b.x,
            mask.y != 0 ? a.y : b.y,
            mask.z != 0 ? a.z : b.z,
            mask.w != 0 ? a.w : b.w
        );
    }

    bool is_nan() const {
        return std::isnan(x) || std::isnan(y) || std::isnan(z) || std::isnan(w);
    }
    double angle_between(const vector4& rhs) const {
        double denom = length() * rhs.length();
        return denom > 0 ? std::acos(std::clamp(dot(rhs) / denom, -1.0, 1.0)) : 0.0;
    }

    bool is_normalized(double epsilon = 1e-6) const {
        return std::abs(length() - 1.0) < epsilon;
    }

    void to_array(float out[4]) const {
        out[0] = static_cast<float>(x);
        out[1] = static_cast<float>(y);
        out[2] = static_cast<float>(z);
        out[3] = static_cast<float>(w);
    }

    static vector4 from_rgba(uint32_t rgba) {
        return vector4(
            ((rgba >> 24) & 0xFF) / 255.0,
            ((rgba >> 16) & 0xFF) / 255.0,
            ((rgba >> 8) & 0xFF) / 255.0,
            (rgba & 0xFF) / 255.0
        );
    }

    double length() const { return std::sqrt(x * x + y * y + z * z + w * w); }
    double length_squared() const { return x * x + y * y + z * z + w * w; }

    vector4 normalize() const {
        double len = length();
        return len > 0.0 ? *this / len : vector4::zero();
    }

    double dot(const vector4& rhs) const {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    bool equals(const vector4& rhs, double epsilon = 1e-6) const {
        return std::abs(x - rhs.x) < epsilon &&
            std::abs(y - rhs.y) < epsilon &&
            std::abs(z - rhs.z) < epsilon &&
            std::abs(w - rhs.w) < epsilon;
    }

    vector4 clone() const { return *this; }

    void set(double nx, double ny, double nz, double nw) {
        x = nx; y = ny; z = nz; w = nw;
    }

    vector3 xyz() const { return vector3(x, y, z); }
    void set_xyz(const vector3& v) { x = v.x; y = v.y; z = v.z; }

    vector3 to_cartesian() const {
        return w != 0.0 ? vector3(x, y, z) / w : vector3(x, y, z);
    }

    static vector4 lerp(const vector4& a, const vector4& b, double t) {
        return a + (b - a) * t;
    }

    static vector4 min(const vector4& a, const vector4& b) {
        return vector4(std::min(a.x, b.x), std::min(a.y, b.y),
            std::min(a.z, b.z), std::min(a.w, b.w));
    }

    static vector4 max(const vector4& a, const vector4& b) {
        return vector4(std::max(a.x, b.x), std::max(a.y, b.y),
            std::max(a.z, b.z), std::max(a.w, b.w));
    }

    static vector4 clamp(const vector4& v, const vector4& min_v, const vector4& max_v) {
        return vector4::max(min_v, vector4::min(v, max_v));
    }


    vector4 multiply(const vector4& rhs) const {
        return vector4(x * rhs.x, y * rhs.y, z * rhs.z, w * rhs.w);
    }

    vector4 divide(const vector4& rhs) const {
        return vector4(x / rhs.x, y / rhs.y, z / rhs.z, w / rhs.w);
    }

    size_t hash() const {
        size_t h = std::hash<double>()(x);
        h ^= std::hash<double>()(y) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<double>()(z) + 0x9e3779b9 + (h << 6) + (h >> 2);
        h ^= std::hash<double>()(w) + 0x9e3779b9 + (h << 6) + (h >> 2);
        return h;
    }

    void serialize(double* out) const {
        out[0] = x; out[1] = y; out[2] = z; out[3] = w;
    }

    static vector4 deserialize(const double* in) {
        return vector4(in[0], in[1], in[2], in[3]);
    }

    static vector4 slerp(const vector4& a, const vector4& b, double t) {
        double cos_theta = a.dot(b);
        if (cos_theta > 0.9995) {
            return vector4::lerp(a, b, t).normalize();
        }
        cos_theta = std::clamp(cos_theta, -1.0, 1.0);
        double theta = std::acos(cos_theta);
        double sin_theta = std::sin(theta);
        double w1 = std::sin((1 - t) * theta) / sin_theta;
        double w2 = std::sin(t * theta) / sin_theta;
        return a * w1 + b * w2;
    }

    static vector4 unit_x() { return vector4(1, 0, 0, 0); }
    static vector4 unit_y() { return vector4(0, 1, 0, 0); }
    static vector4 unit_z() { return vector4(0, 0, 1, 0); }
    static vector4 unit_w() { return vector4(0, 0, 0, 1); }
};
