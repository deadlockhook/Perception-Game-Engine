#pragma once
#include "aabb.h"
#include "matrix4x4.h"
#include "vector3.h"
#include "ray.h"

struct box_t {
    aabb_t local_bounds;
    matrix4x4 transform;

    box_t();
    box_t(const aabb_t& bounds, const matrix4x4& transform);

    static box_t from_center_extent(const vector3& center, const vector3& extent);

    vector3 center() const;
    vector3 extents() const;
    vector3 size() const;
    double volume() const;
    double surface_area() const;

    aabb_t to_world_aabb() const;
    void get_corners(vector3 out[8]) const;

    bool contains_point(const vector3& p, double epsilon = 0.0, bool inclusive = true) const;
    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;

    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;

    vector3 transform_point_to_local(const vector3& world_point) const;
    bool equals(const box_t& other, double epsilon = 1e-6) const;
    bool is_valid() const;
};
