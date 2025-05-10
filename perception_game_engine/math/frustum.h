#pragma once
#include "plane.h"

struct matrix4x4;
struct aabb_t;


struct frustum_t {
    enum { _left = 0, _right, _bottom, _top, _near, _far };
    plane_t planes[6];

    const plane_t& get_plane(int i) const { return planes[i]; }
    plane_t& get_plane(int i) { return planes[i]; }

    frustum_t transform(const matrix4x4& m) const; // In header


    void get_corners(vector3 out[8], const matrix4x4& inv_view_proj) const;
    static frustum_t from_matrix(const matrix4x4& m);
    static frustum_t split(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view);
    static frustum_t split_world(float near_frac, float far_frac, const matrix4x4& proj, const matrix4x4& view);

    bool contains_point(const vector3& p, double epsilon = 1e-6) const;
    bool intersects_aabb(const aabb_t& box, double epsilon = 1e-6) const;
    bool intersects_sphere(const vector3& center, double radius, double epsilon = 1e-6) const;
    bool contains_aabb_completely(const aabb_t& box, double epsilon = 1e-6) const;
    void print() const;

    static frustum_t from_corners(const vector3 corners[8]);
private:
    static const char* plane_name(int i) {
        static const char* names[] = { "Left", "Right", "Bottom", "Top", "Near", "Far" };
        return (i >= 0 && i < 6) ? names[i] : "Unknown";
    }

};
