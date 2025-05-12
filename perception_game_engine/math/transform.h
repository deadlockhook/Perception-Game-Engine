#pragma once
#include "vector3.h"
#include "quat.h"
#include "matrix4x4.h"
#include "aabb.h"

struct transform_t {

    vector3 position = vector3(0.0);
    quat rotation = quat::identity();
    vector3 scale = vector3(1.0);

    transform_t() = default;

    transform_t(const vector3& position, const quat& rotation, const vector3& scale)
        : position(position), rotation(rotation), scale(scale) {
    }

    static transform_t identity() {
        return transform_t(vector3(0.0), quat::identity(), vector3(1.0));
    }

    matrix4x4 to_matrix() const {
        return matrix4x4::trs(position, rotation, scale);
    }

    transform_t inverse() const {
        vector3 inv_scale = vector3(1.0 / scale.x, 1.0 / scale.y, 1.0 / scale.z);
        quat inv_rotation = rotation.inverse();
        vector3 inv_position = inv_rotation * (position * -inv_scale);
        return transform_t(inv_position, inv_rotation, inv_scale);
    }

    vector3 transform_point(const vector3& p) const {
        return rotation * (p * scale) + position;
    }

    vector3 transform_direction(const vector3& dir) const {
        return rotation * (dir * scale);
    }

    vector3 transform_normal(const vector3& normal) const {
        vector3 scaled_normal = vector3(normal.x / scale.x, normal.y / scale.y, normal.z / scale.z);
        return (rotation * scaled_normal).normalized();
    }

    transform_t combine(const transform_t& child) const {
        vector3 new_position = transform_point(child.position);
        quat new_rotation = rotation * child.rotation;
        vector3 new_scale = scale * child.scale;
        return transform_t(new_position, new_rotation, new_scale);
    }

    bool is_identity(double epsilon = 1e-6) const {
        return position.equals(vector3(0.0), epsilon) &&
            rotation == quat::identity() &&
            scale.equals(vector3(1.0), epsilon);
    }

    static transform_t from_matrix(const matrix4x4& m) {
        vector3 pos = vector3(m[0][3], m[1][3], m[2][3]);
        vector3 scl = m.get_scale();
        matrix4x4 rot_m = m;
        rot_m[0][0] /= scl.x; rot_m[0][1] /= scl.x; rot_m[0][2] /= scl.x;
        rot_m[1][0] /= scl.y; rot_m[1][1] /= scl.y; rot_m[1][2] /= scl.y;
        rot_m[2][0] /= scl.z; rot_m[2][1] /= scl.z; rot_m[2][2] /= scl.z;
        quat rot = rot_m.get_rotation(); 
        return transform_t(pos, rot, scl);
    }

    static transform_t look_at(const vector3& position, const vector3& target, const vector3& up = vector3(0, 1, 0)) {
        vector3 forward = (target - position).normalized();
        vector3 right = up.cross(forward).normalized();
        vector3 true_up = forward.cross(right); 

        matrix4x4 rot_basis = matrix4x4::identity();
        rot_basis[0][0] = static_cast<float>(right.x);
        rot_basis[1][0] = static_cast<float>(right.y);
        rot_basis[2][0] = static_cast<float>(right.z);

        rot_basis[0][1] = static_cast<float>(true_up.x);
        rot_basis[1][1] = static_cast<float>(true_up.y);
        rot_basis[2][1] = static_cast<float>(true_up.z);

        rot_basis[0][2] = static_cast<float>(forward.x);
        rot_basis[1][2] = static_cast<float>(forward.y);
        rot_basis[2][2] = static_cast<float>(forward.z);

        quat rotation = rot_basis.get_rotation(); 
        return transform_t(position, rotation, vector3(1.0));
    }

    static transform_t lerp(const transform_t& a, const transform_t& b, double t) {
        vector3 pos = vector3::lerp(a.position, b.position, t);
        quat rot = quat::slerp(a.rotation, b.rotation, t);
        vector3 scl = vector3::lerp(a.scale, b.scale, t);
        return transform_t(pos, rot, scl);
    }

    aabb_t transform_aabb(const aabb_t& box) const {
        vector3 corners[8];
        box.get_corners(corners);
        aabb_t result(transform_point(corners[0]), transform_point(corners[0]));
        for (int i = 1; i < 8; ++i)
            result.expand(transform_point(corners[i]));
        return result;
    }
};
