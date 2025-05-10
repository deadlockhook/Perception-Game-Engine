#include "frustum.h"
#include "aabb.h"
#include "matrix4x4.h"
frustum_t frustum_t::from_matrix(const matrix4x4& m) {
    frustum_t f;

    f.planes[_left]   = plane_t::from_vector4(vector4(
        m[0][3] + m[0][0],
        m[1][3] + m[1][0],
        m[2][3] + m[2][0],
        m[3][3] + m[3][0])).normalized();

    f.planes[_right]  = plane_t::from_vector4(vector4(
        m[0][3] - m[0][0],
        m[1][3] - m[1][0],
        m[2][3] - m[2][0],
        m[3][3] - m[3][0])).normalized();

    f.planes[_bottom] = plane_t::from_vector4(vector4(
        m[0][3] + m[0][1],
        m[1][3] + m[1][1],
        m[2][3] + m[2][1],
        m[3][3] + m[3][1])).normalized();

    f.planes[_top]    = plane_t::from_vector4(vector4(
        m[0][3] - m[0][1],
        m[1][3] - m[1][1],
        m[2][3] - m[2][1],
        m[3][3] - m[3][1])).normalized();

    f.planes[_near]   = plane_t::from_vector4(vector4(
        m[0][3] + m[0][2],
        m[1][3] + m[1][2],
        m[2][3] + m[2][2],
        m[3][3] + m[3][2])).normalized();

    f.planes[_far]    = plane_t::from_vector4(vector4(
        m[0][3] - m[0][2],
        m[1][3] - m[1][2],
        m[2][3] - m[2][2],
        m[3][3] - m[3][2])).normalized();

    return f;
}

bool frustum_t::contains_point(const vector3& p, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        double dist = planes[i].distance_to_point(p);
        if (dist < -epsilon) {
            printf("❌ Point (%.3f, %.3f, %.3f) failed against plane %d: distance = %.6f\n",
                p.x, p.y, p.z, i, dist);
            printf("   Plane normal = (%.3f, %.3f, %.3f), d = %.3f\n",
                planes[i].normal.x, planes[i].normal.y, planes[i].normal.z, planes[i].d);
            return false;
        }
    }
    return true;
}


bool frustum_t::intersects_sphere(const vector3& center, double radius, double epsilon ) const {
    for (int i = 0; i < 6; ++i) {
        if (planes[i].distance_to_point(center) < -radius - epsilon)
            return false;
    }
    return true;
}

bool frustum_t::intersects_aabb(const aabb_t& box, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        const plane_t& plane = planes[i];

        vector3 p(
            plane.normal.x >= 0 ? box.max.x : box.min.x,
            plane.normal.y >= 0 ? box.max.y : box.min.y,
            plane.normal.z >= 0 ? box.max.z : box.min.z
        );

        if (plane.distance_to_point(p) < -epsilon)
            return false;
    }
    return true;
}

frustum_t frustum_t::split_world(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view) {
    frustum_t local = split(near_frac, far_frac, proj, view);
    return local.transform(view.inverse_affine());
}
frustum_t frustum_t::split(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view) {
    
    near_frac = std::clamp(near_frac, 0.0f, 1.0f);
    far_frac = std::clamp(far_frac, 0.0f, 1.0f);

    matrix4x4 inv_view_proj = (view * proj).inverse_affine();
  
    vector3 test = inv_view_proj.transform_vec4({ 0, 0, 0, 1 }).to_cartesian(); // center near plane
    printf("Reconstructed near center: %.3f %.3f %.3f\n", test.x, test.y, test.z);

    vector4 ndc_corners[8] = {
        { -1, -1, 0, 1 }, { 1, -1, 0, 1 },
        { -1,  1, 0, 1 }, { 1,  1, 0, 1 },
        { -1, -1, 1, 1 }, { 1, -1, 1, 1 },
        { -1,  1, 1, 1 }, { 1,  1, 1, 1 },
    };

    vector3 new_corners[8];
    for (int i = 0; i < 4; ++i) {
        vector4 n = ndc_corners[i];
        vector4 f = ndc_corners[i + 4];

        vector4 near_clip = n + (f - n) * near_frac;
        vector4 far_clip = n + (f - n) * far_frac;

        new_corners[i] = inv_view_proj.transform_vec4(near_clip).to_cartesian();
        new_corners[i + 4] = inv_view_proj.transform_vec4(far_clip).to_cartesian();
    }

    for (int i = 0; i < 8; ++i)
        printf("corner[%d]: %.3f %.3f %.3f\n", i, new_corners[i].x, new_corners[i].y, new_corners[i].z);

    frustum_t split_f = frustum_t::from_corners(new_corners);

    for (int i = 0; i < 6; ++i)
        split_f.planes[i] = split_f.planes[i].normalized();

    return split_f;
}


frustum_t frustum_t::transform(const matrix4x4& m) const {
    frustum_t result;
    for (int i = 0; i < 6; ++i)
        result.planes[i] = planes[i].transform(m);
    return result;
}


void frustum_t::get_corners(vector3 out[8], const matrix4x4& inv_view_proj) const {
    static const vector4 ndc[] = {
        { -1, -1,  0, 1 }, { 1, -1,  0, 1 },
        { -1,  1,  0, 1 }, { 1,  1,  0, 1 },
        { -1, -1,  1, 1 }, { 1, -1,  1, 1 },
        { -1,  1,  1, 1 }, { 1,  1,  1, 1 },
    };

    for (int i = 0; i < 8; ++i) {
        vector4 p = inv_view_proj.transform_vec4(ndc[i]);
        out[i] = p.to_cartesian();
    }
}

frustum_t frustum_t::from_corners(const vector3 corners[8]) {
    frustum_t result;
    result.planes[_near] = plane_t::from_points(corners[1], corners[0], corners[2]); 
    result.planes[_far] = plane_t::from_points(corners[5], corners[7], corners[6]); 
    result.planes[_left] = plane_t::from_points(corners[0], corners[4], corners[6]); 
    result.planes[_right] = plane_t::from_points(corners[5], corners[1], corners[3]);
    result.planes[_top] = plane_t::from_points(corners[2], corners[6], corners[7]); 
    result.planes[_bottom] = plane_t::from_points(corners[4], corners[0], corners[1]); 

    for (int i = 0; i < 6; ++i)
        result.planes[i] = result.planes[i].normalized();

    return result;
}




bool frustum_t::contains_aabb_completely(const aabb_t& box, double epsilon) const {
    for (int i = 0; i < 6; ++i) {
        const plane_t& plane = planes[i];

        vector3 n(
            plane.normal.x >= 0 ? box.min.x : box.max.x,
            plane.normal.y >= 0 ? box.min.y : box.max.y,
            plane.normal.z >= 0 ? box.min.z : box.max.z
        );
        if (plane.distance_to_point(n) < -epsilon)
            return false;
    }
    return true;
}
void frustum_t::print() const {
    printf("[frustum_t]\n");
    for (int i = 0; i < 6; ++i) {
        printf("  Plane %d: normal = (%.3f, %.3f, %.3f), d = %.3f\n",
            i, planes[i].normal.x, planes[i].normal.y, planes[i].normal.z, planes[i].d);
    }
}