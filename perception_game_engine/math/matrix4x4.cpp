#include "matrix4x4.h"
#include <stdio.h>
#include "vector4.h"

matrix4x4 matrix4x4::identity() {
    matrix4x4 result;
    result.m[0][0] = 1.0f;
    result.m[1][1] = 1.0f;
    result.m[2][2] = 1.0f;
    result.m[3][3] = 1.0f;
    return result;
}

matrix4x4 matrix4x4::zero() {
    return matrix4x4{};
}

matrix4x4 matrix4x4::operator*(const matrix4x4& rhs) const {
    matrix4x4 result;

    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                result.m[row][col] += m[row][k] * rhs.m[k][col];

    return result;
}

matrix4x4& matrix4x4::operator*=(const matrix4x4& rhs) {
    *this = (*this) * rhs;
    return *this;
}

matrix4x4 matrix4x4::translate(const vector3& v) {
    matrix4x4 result = identity();
    result.m[0][3] = (float)v.x;
    result.m[1][3] = (float)v.y;
    result.m[2][3] = (float)v.z;
    return result;
}



matrix4x4 matrix4x4::scale(const vector3& v) {
    matrix4x4 result = identity();
    result.m[0][0] = static_cast<float>(v.x);
    result.m[1][1] = static_cast<float>(v.y);
    result.m[2][2] = static_cast<float>(v.z);
    return result;
}

matrix4x4 matrix4x4::rotate(const quat& q) {
    matrix4x4 result = identity();

    double x = q.x, y = q.y, z = q.z, w = q.w;
    double xx = x * x, yy = y * y, zz = z * z;
    double xy = x * y, xz = x * z, yz = y * z;
    double wx = w * x, wy = w * y, wz = w * z;

    result.m[0][0] = static_cast<float>(1.0 - 2.0 * (yy + zz));
    result.m[0][1] = static_cast<float>(2.0 * (xy - wz));
    result.m[0][2] = static_cast<float>(2.0 * (xz + wy));

    result.m[1][0] = static_cast<float>(2.0 * (xy + wz));
    result.m[1][1] = static_cast<float>(1.0 - 2.0 * (xx + zz));
    result.m[1][2] = static_cast<float>(2.0 * (yz - wx));

    result.m[2][0] = static_cast<float>(2.0 * (xz - wy));
    result.m[2][1] = static_cast<float>(2.0 * (yz + wx));
    result.m[2][2] = static_cast<float>(1.0 - 2.0 * (xx + yy));

    return result;
}
matrix4x4 matrix4x4::trs(const vector3& t, const quat& r, const vector3& s) {
    return translate(t) * rotate(r) * scale(s);
}

vector3 matrix4x4::transform_point(const vector3& v) const {
    vector4 r = transform_vec4(vector4(v, 1.0));
    return r.to_cartesian();
}


vector3 matrix4x4::transform_direction(const vector3& v) const {
    float x_ = static_cast<float>(v.x);
    float y_ = static_cast<float>(v.y);
    float z_ = static_cast<float>(v.z);

    float x = x_ * m[0][0] + y_ * m[1][0] + z_ * m[2][0];
    float y = x_ * m[0][1] + y_ * m[1][1] + z_ * m[2][1];
    float z = x_ * m[0][2] + y_ * m[1][2] + z_ * m[2][2];

    return { x, y, z };
}

vector4 matrix4x4::transform_vec4(const vector4& v) const {
    return vector4(
        m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
        m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
        m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
        m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
    );
}


matrix4x4 matrix4x4::transposed() const {
    matrix4x4 r;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            r.m[row][col] = m[col][row];
    return r;
}
 
void  matrix4x4::to_float_array(float out[16]) const {
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            out[row * 4 + col] = m[row][col];
}
bool matrix4x4::equals(const matrix4x4& other, float epsilon ) const {
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            if (std::abs(m[i][j] - other.m[i][j]) > epsilon)
                return false;
    return true;
}

float matrix4x4::determinant() const {

    const float* a = &m[0][0];
    float det =
        a[0] * a[5] * a[10] * a[15] + a[0] * a[9] * a[14] * a[7] + a[0] * a[13] * a[6] * a[11]
        - a[0] * a[13] * a[10] * a[7] - a[0] * a[9] * a[6] * a[15] - a[0] * a[5] * a[14] * a[11]
        + a[4] * a[1] * a[14] * a[11] + a[4] * a[9] * a[2] * a[15] + a[4] * a[13] * a[10] * a[3]
        - a[4] * a[13] * a[2] * a[11] - a[4] * a[9] * a[10] * a[3] - a[4] * a[1] * a[10] * a[15]
        + a[8] * a[1] * a[6] * a[15] + a[8] * a[5] * a[14] * a[3] + a[8] * a[13] * a[2] * a[7]
        - a[8] * a[13] * a[6] * a[3] - a[8] * a[5] * a[2] * a[15] - a[8] * a[1] * a[14] * a[7]
        + a[12] * a[1] * a[10] * a[7] + a[12] * a[5] * a[2] * a[11] + a[12] * a[9] * a[6] * a[3]
        - a[12] * a[9] * a[2] * a[7] - a[12] * a[5] * a[10] * a[3] - a[12] * a[1] * a[6] * a[11];

    return det;
}

matrix4x4 matrix4x4::perspective(float fov_y_rad, float aspect, float z_near, float z_far) {
    float f = 1.0f / std::tan(fov_y_rad * 0.5f);
    float range_inv = 1.0f / (z_near - z_far);

    matrix4x4 result = {};
    result.m[0][0] = f / aspect;
    result.m[1][1] = f;
    result.m[2][2] = (z_far + z_near) * range_inv;
    result.m[2][3] = -1.0f;
    result.m[3][2] = (2.0f * z_far * z_near) * range_inv;
    result.m[3][3] = 0.0f;
    return result;
}

matrix4x4 matrix4x4::orthographic(float left, float right, float bottom, float top, float z_near, float z_far) {
    matrix4x4 result = identity();

    float width = right - left;
    float height = top - bottom;
    float depth = z_far - z_near;

    result.m[0][0] = 2.0f / width;
    result.m[1][1] = 2.0f / height;
    result.m[2][2] = -2.0f / depth;

    result.m[3][0] = -(right + left) / width;
    result.m[3][1] = -(top + bottom) / height;
    result.m[3][2] = -(z_far + z_near) / depth;

    return result;
}

matrix4x4 matrix4x4::look_at(const vector3& eye, const vector3& target, const vector3& up) {
    vector3 f = (target - eye).normalized();
    vector3 r = f.cross(up).normalized();
    vector3 u = r.cross(f);

    matrix4x4 result = identity();

    result[0][0] = static_cast<float>(r.x);
    result[0][1] = static_cast<float>(r.y);
    result[0][2] = static_cast<float>(r.z);
    result[0][3] = static_cast<float>(-r.dot(eye));

    result[1][0] = static_cast<float>(u.x);
    result[1][1] = static_cast<float>(u.y);
    result[1][2] = static_cast<float>(u.z);
    result[1][3] = static_cast<float>(-u.dot(eye));

    result[2][0] = static_cast<float>(-f.x);
    result[2][1] = static_cast<float>(-f.y);
    result[2][2] = static_cast<float>(-f.z);
    result[2][3] = static_cast<float>(f.dot(eye));

    result[3][0] = 0.0f;
    result[3][1] = 0.0f;
    result[3][2] = 0.0f;
    result[3][3] = 1.0f;

    return result;
}

matrix4x4 matrix4x4::inverted() const {
    matrix4x4 inv;
    const float* m = &this->m[0][0];
    float* invOut = &inv.m[0][0];

    invOut[0] = m[5] * m[10] * m[15] -
        m[5] * m[11] * m[14] -
        m[9] * m[6] * m[15] +
        m[9] * m[7] * m[14] +
        m[13] * m[6] * m[11] -
        m[13] * m[7] * m[10];

    invOut[4] = -m[4] * m[10] * m[15] +
        m[4] * m[11] * m[14] +
        m[8] * m[6] * m[15] -
        m[8] * m[7] * m[14] -
        m[12] * m[6] * m[11] +
        m[12] * m[7] * m[10];

    invOut[8] = m[4] * m[9] * m[15] -
        m[4] * m[11] * m[13] -
        m[8] * m[5] * m[15] +
        m[8] * m[7] * m[13] +
        m[12] * m[5] * m[11] -
        m[12] * m[7] * m[9];

    invOut[12] = -m[4] * m[9] * m[14] +
        m[4] * m[10] * m[13] +
        m[8] * m[5] * m[14] -
        m[8] * m[6] * m[13] -
        m[12] * m[5] * m[10] +
        m[12] * m[6] * m[9];

    invOut[1] = -m[1] * m[10] * m[15] +
        m[1] * m[11] * m[14] +
        m[9] * m[2] * m[15] -
        m[9] * m[3] * m[14] -
        m[13] * m[2] * m[11] +
        m[13] * m[3] * m[10];

    invOut[5] = m[0] * m[10] * m[15] -
        m[0] * m[11] * m[14] -
        m[8] * m[2] * m[15] +
        m[8] * m[3] * m[14] +
        m[12] * m[2] * m[11] -
        m[12] * m[3] * m[10];

    invOut[9] = -m[0] * m[9] * m[15] +
        m[0] * m[11] * m[13] +
        m[8] * m[1] * m[15] -
        m[8] * m[3] * m[13] -
        m[12] * m[1] * m[11] +
        m[12] * m[3] * m[9];

    invOut[13] = m[0] * m[9] * m[14] -
        m[0] * m[10] * m[13] -
        m[8] * m[1] * m[14] +
        m[8] * m[2] * m[13] +
        m[12] * m[1] * m[10] -
        m[12] * m[2] * m[9];

    invOut[2] = m[1] * m[6] * m[15] -
        m[1] * m[7] * m[14] -
        m[5] * m[2] * m[15] +
        m[5] * m[3] * m[14] +
        m[13] * m[2] * m[7] -
        m[13] * m[3] * m[6];

    invOut[6] = -m[0] * m[6] * m[15] +
        m[0] * m[7] * m[14] +
        m[4] * m[2] * m[15] -
        m[4] * m[3] * m[14] -
        m[12] * m[2] * m[7] +
        m[12] * m[3] * m[6];

    invOut[10] = m[0] * m[5] * m[15] -
        m[0] * m[7] * m[13] -
        m[4] * m[1] * m[15] +
        m[4] * m[3] * m[13] +
        m[12] * m[1] * m[7] -
        m[12] * m[3] * m[5];

    invOut[14] = -m[0] * m[5] * m[14] +
        m[0] * m[6] * m[13] +
        m[4] * m[1] * m[14] -
        m[4] * m[2] * m[13] -
        m[12] * m[1] * m[6] +
        m[12] * m[2] * m[5];

    invOut[3] = -m[1] * m[6] * m[11] +
        m[1] * m[7] * m[10] +
        m[5] * m[2] * m[11] -
        m[5] * m[3] * m[10] -
        m[9] * m[2] * m[7] +
        m[9] * m[3] * m[6];

    invOut[7] = m[0] * m[6] * m[11] -
        m[0] * m[7] * m[10] -
        m[4] * m[2] * m[11] +
        m[4] * m[3] * m[10] +
        m[8] * m[2] * m[7] -
        m[8] * m[3] * m[6];

    invOut[11] = -m[0] * m[5] * m[11] +
        m[0] * m[7] * m[9] +
        m[4] * m[1] * m[11] -
        m[4] * m[3] * m[9] -
        m[8] * m[1] * m[7] +
        m[8] * m[3] * m[5];

    invOut[15] = m[0] * m[5] * m[10] -
        m[0] * m[6] * m[9] -
        m[4] * m[1] * m[10] +
        m[4] * m[2] * m[9] +
        m[8] * m[1] * m[6] -
        m[8] * m[2] * m[5];

    float det = m[0] * invOut[0] + m[1] * invOut[4] + m[2] * invOut[8] + m[3] * invOut[12];

    if (det == 0.0f)
        return identity(); 

    float inv_det = 1.0f / det;
    for (int i = 0; i < 16; ++i)
        invOut[i] *= inv_det;

    return inv;
}


matrix4x4 matrix4x4::inverse() const {
    matrix4x4 result;
    const float* mat = &this->m[0][0];
    float* inv = &result.m[0][0];

    inv[0] = mat[5] * mat[10] * mat[15] - mat[5] * mat[11] * mat[14] - mat[9] * mat[6] * mat[15] +
        mat[9] * mat[7] * mat[14] + mat[13] * mat[6] * mat[11] - mat[13] * mat[7] * mat[10];

    inv[4] = -mat[4] * mat[10] * mat[15] + mat[4] * mat[11] * mat[14] + mat[8] * mat[6] * mat[15] -
        mat[8] * mat[7] * mat[14] - mat[12] * mat[6] * mat[11] + mat[12] * mat[7] * mat[10];

    inv[8] = mat[4] * mat[9] * mat[15] - mat[4] * mat[11] * mat[13] - mat[8] * mat[5] * mat[15] +
        mat[8] * mat[7] * mat[13] + mat[12] * mat[5] * mat[11] - mat[12] * mat[7] * mat[9];

    inv[12] = -mat[4] * mat[9] * mat[14] + mat[4] * mat[10] * mat[13] + mat[8] * mat[5] * mat[14] -
        mat[8] * mat[6] * mat[13] - mat[12] * mat[5] * mat[10] + mat[12] * mat[6] * mat[9];

    inv[1] = -mat[1] * mat[10] * mat[15] + mat[1] * mat[11] * mat[14] + mat[9] * mat[2] * mat[15] -
        mat[9] * mat[3] * mat[14] - mat[13] * mat[2] * mat[11] + mat[13] * mat[3] * mat[10];

    inv[5] = mat[0] * mat[10] * mat[15] - mat[0] * mat[11] * mat[14] - mat[8] * mat[2] * mat[15] +
        mat[8] * mat[3] * mat[14] + mat[12] * mat[2] * mat[11] - mat[12] * mat[3] * mat[10];

    inv[9] = -mat[0] * mat[9] * mat[15] + mat[0] * mat[11] * mat[13] + mat[8] * mat[1] * mat[15] -
        mat[8] * mat[3] * mat[13] - mat[12] * mat[1] * mat[11] + mat[12] * mat[3] * mat[9];

    inv[13] = mat[0] * mat[9] * mat[14] - mat[0] * mat[10] * mat[13] - mat[8] * mat[1] * mat[14] +
        mat[8] * mat[2] * mat[13] + mat[12] * mat[1] * mat[10] - mat[12] * mat[2] * mat[9];

    inv[2] = mat[1] * mat[6] * mat[15] - mat[1] * mat[7] * mat[14] - mat[5] * mat[2] * mat[15] +
        mat[5] * mat[3] * mat[14] + mat[13] * mat[2] * mat[7] - mat[13] * mat[3] * mat[6];

    inv[6] = -mat[0] * mat[6] * mat[15] + mat[0] * mat[7] * mat[14] + mat[4] * mat[2] * mat[15] -
        mat[4] * mat[3] * mat[14] - mat[12] * mat[2] * mat[7] + mat[12] * mat[3] * mat[6];

    inv[10] = mat[0] * mat[5] * mat[15] - mat[0] * mat[7] * mat[13] - mat[4] * mat[1] * mat[15] +
        mat[4] * mat[3] * mat[13] + mat[12] * mat[1] * mat[7] - mat[12] * mat[3] * mat[5];

    inv[14] = -mat[0] * mat[5] * mat[14] + mat[0] * mat[6] * mat[13] + mat[4] * mat[1] * mat[14] -
        mat[4] * mat[2] * mat[13] - mat[12] * mat[1] * mat[6] + mat[12] * mat[2] * mat[5];

    inv[3] = -mat[1] * mat[6] * mat[11] + mat[1] * mat[7] * mat[10] + mat[5] * mat[2] * mat[11] -
        mat[5] * mat[3] * mat[10] - mat[9] * mat[2] * mat[7] + mat[9] * mat[3] * mat[6];

    inv[7] = mat[0] * mat[6] * mat[11] - mat[0] * mat[7] * mat[10] - mat[4] * mat[2] * mat[11] +
        mat[4] * mat[3] * mat[10] + mat[8] * mat[2] * mat[7] - mat[8] * mat[3] * mat[6];

    inv[11] = -mat[0] * mat[5] * mat[11] + mat[0] * mat[7] * mat[9] + mat[4] * mat[1] * mat[11] -
        mat[4] * mat[3] * mat[9] - mat[8] * mat[1] * mat[7] + mat[8] * mat[3] * mat[5];

    inv[15] = mat[0] * mat[5] * mat[10] - mat[0] * mat[6] * mat[9] - mat[4] * mat[1] * mat[10] +
        mat[4] * mat[2] * mat[9] + mat[8] * mat[1] * mat[6] - mat[8] * mat[2] * mat[5];

    double det = mat[0] * inv[0] + mat[1] * inv[4] + mat[2] * inv[8] + mat[3] * inv[12];
    if (std::abs(det) < 1e-8)
        return matrix4x4::identity(); 

    double inv_det = 1.0 / det;
    for (int i = 0; i < 16; ++i)
        inv[i] *= inv_det;

    return result;
}

matrix4x4 matrix4x4::inverse_affine() const {
    matrix4x4 result;


    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

    float det =
        a00 * (a11 * a22 - a12 * a21) -
        a01 * (a10 * a22 - a12 * a20) +
        a02 * (a10 * a21 - a11 * a20);

    if (std::abs(det) < 1e-8f)
        return matrix4x4::identity();

    float inv_det = 1.0f / det;

    result.m[0][0] = (a11 * a22 - a12 * a21) * inv_det;
    result.m[0][1] = -(a01 * a22 - a02 * a21) * inv_det;
    result.m[0][2] = (a01 * a12 - a02 * a11) * inv_det;

    result.m[1][0] = -(a10 * a22 - a12 * a20) * inv_det;
    result.m[1][1] = (a00 * a22 - a02 * a20) * inv_det;
    result.m[1][2] = -(a00 * a12 - a02 * a10) * inv_det;

    result.m[2][0] = (a10 * a21 - a11 * a20) * inv_det;
    result.m[2][1] = -(a00 * a21 - a01 * a20) * inv_det;
    result.m[2][2] = (a00 * a11 - a01 * a10) * inv_det;

    float tx = m[0][3], ty = m[1][3], tz = m[2][3];
    result.m[0][3] = -(result.m[0][0] * tx + result.m[0][1] * ty + result.m[0][2] * tz);
    result.m[1][3] = -(result.m[1][0] * tx + result.m[1][1] * ty + result.m[1][2] * tz);
    result.m[2][3] = -(result.m[2][0] * tx + result.m[2][1] * ty + result.m[2][2] * tz);

    result.m[3][0] = result.m[3][1] = result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;

    return result;
}

vector3 matrix4x4::get_scale() const {
    vector3 x(m[0][0], m[1][0], m[2][0]);
    vector3 y(m[0][1], m[1][1], m[2][1]);
    vector3 z(m[0][2], m[1][2], m[2][2]);
    return vector3(x.length(), y.length(), z.length());
}


void matrix4x4::print() const {
    printf("[matrix4x4]\n");
    for (int row = 0; row < 4; ++row) {
        printf("  [ ");
        for (int col = 0; col < 4; ++col) {
            printf("%8.3f ", m[row][col]);
        }
        printf("]\n");
    }
}


