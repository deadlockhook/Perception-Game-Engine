#include "quat.h"
#include "math_defs.h"
#include <iostream>

quat quat::identity() {
    return quat(0.0, 0.0, 0.0, 1.0);
}

quat quat::from_axis_angle(const vector3& axis, double radians) {
    vector3 norm_axis = axis.normalized();
    double half_angle = radians * 0.5;
    double sin_half = std::sin(half_angle);
    double cos_half = std::cos(half_angle);

    quat result(
        norm_axis.x * sin_half,
        norm_axis.y * sin_half,
        norm_axis.z * sin_half,
        cos_half
    );

    return result;
}

vector3 quat::operator*(const vector3& v) const {
    vector3 qv(x, y, z);
    vector3 uv = qv.cross(v);
    vector3 uuv = qv.cross(uv);
    uv *= (2.0 * w);
    uuv *= 2.0;
    vector3 result = v + uv + uuv;
    return result;
}

quat quat::from_euler(const vector3& angles) {
    auto rad_angles = angles.to_radians();

    double cy = std::cos(rad_angles.y * 0.5);
    double sy = std::sin(rad_angles.y * 0.5);
    double cp = std::cos(rad_angles.x * 0.5);
    double sp = std::sin(rad_angles.x * 0.5);
    double cr = std::cos(rad_angles.z * 0.5);
    double sr = std::sin(rad_angles.z * 0.5);

    return quat(
        sr * cp * cy - cr * sp * sy,
        cr * sp * cy + sr * cp * sy,
        cr * cp * sy - sr * sp * cy,
        cr * cp * cy + sr * sp * sy
    );
}

quat quat::operator*(const quat& rhs) const {
    return quat(
        w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
        w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
        w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
        w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
    );
}




double quat::length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

quat quat::normalized() const {
    double len = length();
    return (len != 0.0) ? (*this / len) : identity();
}

void quat::normalize() {
    double len = length();
    if (len != 0.0) {
        x /= len; y /= len; z /= len; w /= len;
    }
}

quat quat::conjugate() const {
    return quat(-x, -y, -z, w);
}

quat quat::inverse() const {
    double len_sq = x * x + y * y + z * z + w * w;
    if (len_sq == 0.0) return identity();
    return conjugate() * (1.0 / len_sq);
}

quat quat::lerp(const quat& a, const quat& b, double t) {
    return quat(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t,
        a.w + (b.w - a.w) * t
    ).normalized();
}

quat quat::slerp(const quat& a, const quat& b, double t) {
    double dot = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;

    quat b2 = (dot < 0.0) ? quat(-b.x, -b.y, -b.z, -b.w) : b;
    if (std::abs(dot) > 0.9995)
        return lerp(a, b2, t);

    double theta = std::acos(dot);
    double sin_theta = std::sin(theta);

    double wa = std::sin((1.0 - t) * theta) / sin_theta;
    double wb = std::sin(t * theta) / sin_theta;

    return quat(
        a.x * wa + b2.x * wb,
        a.y * wa + b2.y * wb,
        a.z * wa + b2.z * wb,
        a.w * wa + b2.w * wb
    );
}

vector3 quat::to_euler() const {
    vector3 euler;

    double sinp = 2.0 * (w * x + y * z);
    double cosp = 1.0 - 2.0 * (x * x + y * y);
    euler.x = std::atan2(sinp, cosp);

    double siny = 2.0 * (w * y - z * x);
    if (std::abs(siny) >= 1.0)
        euler.y = std::copysign(M_PI / 2, siny); 
    else
        euler.y = std::asin(siny);

    double sinr = 2.0 * (w * z + x * y);
    double cosr = 1.0 - 2.0 * (y * y + z * z);
    euler.z = std::atan2(sinr, cosr);

    return euler;
}

bool quat::operator==(const quat& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool quat::operator!=(const quat& rhs) const {
    return !(*this == rhs);
}

quat quat::operator-() const {
    return quat(-x, -y, -z, -w);
}

quat quat::operator/(double scalar) const {
    return quat(x / scalar, y / scalar, z / scalar, w / scalar);
}

quat quat::operator*(double scalar) const {
    return quat(x * scalar, y * scalar, z * scalar, w * scalar);
}

double quat::dot(const quat& other) const {
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

double quat::angle_between(const quat& a, const quat& b) {
    double d = std::fmin(std::fmax(a.dot(b), -1.0), 1.0);
    return std::acos(d) * 2.0;
}

void quat::to_array(double out[4]) const {
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = w;
}

quat quat::from_array(const double in[4]) {
    return quat(in[0], in[1], in[2], in[3]);
}

vector3 quat::rotate(const vector3& v) const {
    vector3 u(x, y, z);
    double s = w;

    return u * (2.0 * u.dot(v)) +
        v * (s * s - u.dot(u)) +
        u.cross(v) * (2.0 * s);
}

bool quat::is_valid() const {
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z) &&
        std::isfinite(w) && std::abs(this->length() - 1.0) < 1e-6;
}

bool quat::equals(const quat& other, double epsilon) const {
    return this->normalized().equals_exact(other.normalized(), epsilon);
}

bool quat::equals_exact(const quat& other, double epsilon) const {
    return std::abs(x - other.x) <= epsilon &&
        std::abs(y - other.y) <= epsilon &&
        std::abs(z - other.z) <= epsilon &&
        std::abs(w - other.w) <= epsilon;
}

