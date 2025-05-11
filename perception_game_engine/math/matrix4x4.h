#pragma once
#pragma once
#include <cmath>
#include <cstdint>
#include "vector3.h"
#include "vector4.h"
#include "quat.h"

struct matrix4x4 {
    float m[4][4] = {}; 

    matrix4x4() = default;

    static matrix4x4 identity();
    static matrix4x4 zero();

    float& operator()(int row, int col) { return m[row][col]; }
    const float& operator()(int row, int col) const { return m[row][col]; }
    
    float* operator[](int row) { return m[row]; }
    const float* operator[](int row) const { return m[row]; }
 
    inline const float* data() const { return &m[0][0]; }
    inline float* data() { return &m[0][0]; }

    matrix4x4 operator*(const matrix4x4& rhs) const;
    matrix4x4& operator*=(const matrix4x4& rhs);

    vector3 transform_point(const vector3& v) const;
    vector3 transform_direction(const vector3& v) const;

    matrix4x4 transposed() const;
    matrix4x4 inverted() const;
    float determinant() const;

    bool equals(const matrix4x4& other, float epsilon = 1e-5f) const;
    void to_float_array(float out[16]) const;
    matrix4x4 inverse() const;
    matrix4x4 inverse_affine() const;
    vector4 transform_vec4(const vector4& v) const;

    static matrix4x4 translate(const vector3& v);
    static matrix4x4 scale(const vector3& v);
    static matrix4x4 rotate(const quat& q);
    static matrix4x4 trs(const vector3& t, const quat& r, const vector3& s);
    static matrix4x4 perspective(float fov_y_rad, float aspect, float z_near, float z_far);
    static matrix4x4 orthographic(float left, float right, float bottom, float top, float z_near, float z_far);
    static matrix4x4 look_at(const vector3& eye, const vector3& target, const vector3& up);
    
    vector4 row_sum( int c1, int c2, bool subtract = false) {
        return vector4(
            m[0][c1] + (subtract ? -m[0][c2] : m[0][c2]),
            m[1][c1] + (subtract ? -m[1][c2] : m[1][c2]),
            m[2][c1] + (subtract ? -m[2][c2] : m[2][c2]),
            m[3][c1] + (subtract ? -m[3][c2] : m[3][c2])
        );
    }

    static vector4 row_sum(const matrix4x4& m, int c1, int c2, bool subtract = false) {
        return vector4(
            m[0][c1] + (subtract ? -m[0][c2] : m[0][c2]),
            m[1][c1] + (subtract ? -m[1][c2] : m[1][c2]),
            m[2][c1] + (subtract ? -m[2][c2] : m[2][c2]),
            m[3][c1] + (subtract ? -m[3][c2] : m[3][c2])
        );
    }
    vector3 get_scale() const;
    void print() const;

};

