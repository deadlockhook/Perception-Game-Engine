#pragma once
#include <cmath>
#include <cfloat>
#include <cstdint>
#include <algorithm>

constexpr double M_PI = 3.14159265358979323846;
constexpr double M_PI_2 = M_PI / 2.0;
constexpr double M_PI_4 = M_PI / 4.0;
constexpr double M_E = 2.71828182845904523536;
constexpr double M_TAU = 6.28318530717958647692;
constexpr double M_HALF_PI = 1.57079632679489661923;
constexpr double M_QUARTER_PI = 0.78539816339744830961;

constexpr double DEG2RAD = M_PI / 180.0;
constexpr double RAD2DEG = 180.0 / M_PI;

constexpr float  FLT_EPSILON2 = FLT_EPSILON * 2.0f;
constexpr double DBL_EPSILON2 = DBL_EPSILON * 2.0;

inline bool is_near_zero(double x, double epsilon = DBL_EPSILON2) {
    return std::abs(x) <= epsilon;
}

inline bool is_equal(double a, double b, double epsilon = DBL_EPSILON2) {
    return std::abs(a - b) <= epsilon;
}

template <typename T>
inline T clamp(T v, T min_val, T max_val) {
    return (v < min_val) ? min_val : (v > max_val ? max_val : v);
}

template <typename T>
inline T saturate(T v) {
    return clamp(v, static_cast<T>(0), static_cast<T>(1));
}

template <typename T>
inline T lerp(const T& a, const T& b, double t) {
    return static_cast<T>(a + (b - a) * t);
}

template <typename T>
inline T square(T v) {
    return v * v;
}

template <typename T>
inline int sign(T x) {
    return (x > 0) - (x < 0);
}

inline double radians(double deg) {
    return deg * DEG2RAD;
}

inline double degrees(double rad) {
    return rad * RAD2DEG;
}

inline double wrap_angle(double angle) {
    angle = std::fmod(angle + M_PI, M_TAU);
    if (angle < 0) angle += M_TAU;
    return angle - M_PI;
}

inline double wrap_angle_positive(double angle) {
    angle = std::fmod(angle, M_TAU);
    return (angle < 0) ? angle + M_TAU : angle;
}

template <typename T>
inline T map_range(T value, T in_min, T in_max, T out_min, T out_max) {
    return out_min + (value - in_min) * (out_max - out_min) / (in_max - in_min);
}

inline bool is_power_of_two(uint32_t x) {
    return x != 0 && (x & (x - 1)) == 0;
}
