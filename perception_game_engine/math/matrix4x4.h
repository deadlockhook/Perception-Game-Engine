#pragma once
#pragma once
#include <cmath>
#include <cstdint>
#include "vector3.h"
#include "quat.h"

struct matrix4x4 {
    float m[4][4] = {}; 

    matrix4x4() = default;

    static matrix4x4 identity();
    static matrix4x4 zero();

    float& operator()(int row, int col) { return m[row][col]; }
    const float& operator()(int row, int col) const { return m[row][col]; }

    matrix4x4 operator*(const matrix4x4& rhs) const;
    matrix4x4& operator*=(const matrix4x4& rhs);

    vector3 transform_point(const vector3& v) const;
    vector3 transform_direction(const vector3& v) const;

    matrix4x4 transposed() const;
    matrix4x4 inverted() const;
    float determinant() const;

    bool equals(const matrix4x4& other, float epsilon = 1e-5f) const;
    void to_float_array(float out[16]) const;

    static matrix4x4 translate(const vector3& v);
    static matrix4x4 scale(const vector3& v);
    static matrix4x4 rotate(const quat& q);
    static matrix4x4 trs(const vector3& t, const quat& r, const vector3& s);
    static matrix4x4 perspective(float fov_y_rad, float aspect, float z_near, float z_far);
    static matrix4x4 orthographic(float left, float right, float bottom, float top, float z_near, float z_far);
    static matrix4x4 look_at(const vector3& eye, const vector3& target, const vector3& up);
    
    void print() const;

};
