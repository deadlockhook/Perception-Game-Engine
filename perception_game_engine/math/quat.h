#pragma once
#include <cmath>
#include "vector3.h"

struct quat {
    double x = 0.0, y = 0.0, z = 0.0, w = 1.0;

    quat() = default;
    quat(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

    bool operator==(const quat& rhs) const;
    bool operator!=(const quat& rhs) const;
    quat operator-() const;

    quat operator/(double scalar) const;
    quat operator*(double scalar) const;

    quat operator*(const quat& rhs) const;
    vector3 operator*(const vector3& v) const;

    quat operator-(const quat& rhs) const {
        return quat(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
    }

    static quat identity();
    static quat from_axis_angle(const vector3& axis, double radians);
    static quat from_euler(double pitch_rad, double yaw_rad, double roll_rad);

    double length() const;
    quat normalized() const;
    void normalize();
    quat inverse() const;
    quat conjugate() const;

    static quat lerp(const quat& a, const quat& b, double t);
    static quat slerp(const quat& a, const quat& b, double t);

    vector3 to_euler() const;
    void to_array(double out[4]) const;
    static quat from_array(const double in[4]);

    double dot(const quat& other) const;

    static double angle_between(const quat& a, const quat& b);

    vector3 rotate(const vector3& v) const;
    bool is_valid() const;

};
