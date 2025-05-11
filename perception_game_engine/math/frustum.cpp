#include "frustum.h"
#include "aabb.h"
#include "matrix4x4.h"

frustum_t frustum_t::from_matrix(const matrix4x4& m) {
    frustum_t f;

    f.planes[_left] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 0));
    f.planes[_right] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 0, true));
    f.planes[_bottom] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 1));
    f.planes[_top] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 1, true));
    f.planes[_near] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 2));
    f.planes[_far] = plane_t::from_vector4(matrix4x4::row_sum(m, 3, 2, true));

    for (int i = 0; i < 6; ++i)
        f.planes[i] = f.planes[i].normalized();

    return f;
}


frustum_t frustum_t::transform(const matrix4x4& m) const {
    frustum_t out;
    for (int i = 0; i < 6; ++i)
        out.planes[i] = planes[i].transform(m);
    return out;
}

void frustum_t::get_corners(vector3 out[8], const matrix4x4& inv_view_proj) const {
    static const vector4 ndc[] = {
        {-1, -1, 0, 1}, {1, -1, 0, 1}, {-1, 1, 0, 1}, {1, 1, 0, 1},
        {-1, -1, 1, 1}, {1, -1, 1, 1}, {-1, 1, 1, 1}, {1, 1, 1, 1}
    };

    for (int i = 0; i < 8; ++i) {
        vector4 corner = inv_view_proj.transform_vec4(ndc[i]);
        out[i] = corner.to_cartesian();
    }
}

frustum_t frustum_t::from_corners(const vector3 corners[8]) {
    frustum_t f;

    // Use triangle winding to form planes
    f.planes[_left] = plane_t::from_points(corners[0], corners[4], corners[6]);
    f.planes[_right] = plane_t::from_points(corners[5], corners[1], corners[7]);
    f.planes[_bottom] = plane_t::from_points(corners[0], corners[1], corners[5]);
    f.planes[_top] = plane_t::from_points(corners[7], corners[3], corners[2]);
    f.planes[_near] = plane_t::from_points(corners[0], corners[2], corners[1]);
    f.planes[_far] = plane_t::from_points(corners[5], corners[7], corners[4]);

    return f;
}

frustum_t frustum_t::split(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view) {
    matrix4x4 proj_split = proj;
    float near = near_frac;
    float far = far_frac;
    proj_split[2][2] = far / (far - near);
    proj_split[3][2] = -near * far / (far - near);

    matrix4x4 view_proj = proj_split * view;
    return from_matrix(view_proj);
}

frustum_t frustum_t::split_world(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view) {
    matrix4x4 view_proj = proj * view;
    vector3 corners[8];

    frustum_t temp = from_matrix(view_proj);
    temp.get_corners(corners, view_proj.inverse());

    return from_corners(corners);
}


bool frustum_t::contains_point(const vector3& p, double epsilon) const {
    for (int i = 0; i < 6; ++i)
        if (!planes[i].is_in_front(p, epsilon))
            return false;
    return true;
}

bool frustum_t::intersects_sphere(const vector3& center, double radius, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        if (planes[i].distance_to_point(center) < -radius - epsilon)
            return false;
    }
    return true;
}

bool frustum_t::intersects_aabb(const aabb_t& box, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        const plane_t& plane = planes[i];

        vector3 positive = box.min;
        if (plane.normal.x >= 0) positive.x = box.max.x;
        if (plane.normal.y >= 0) positive.y = box.max.y;
        if (plane.normal.z >= 0) positive.z = box.max.z;

        if (!plane.is_in_front(positive, epsilon))
            return false;
    }
    return true;
}

bool frustum_t::contains_aabb_completely(const aabb_t& box, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        const plane_t& plane = planes[i];
        if (!plane.is_in_front(box.min, epsilon)) return false;
        if (!plane.is_in_front(box.max, epsilon)) return false;
    }
    return true;
}

void frustum_t::print() const {
    for (int i = 0; i < 6; ++i)
        planes[i].debug_distance_to(vector3::zero(), plane_name(i));
}