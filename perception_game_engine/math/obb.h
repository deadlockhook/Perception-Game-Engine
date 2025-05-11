#pragma once
#include "vector3.h"
#include "matrix4x4.h"
#include "quat.h"
#include "aabb.h"
#include "ray.h"

struct obb_t {
    vector3 center;
    vector3 axes[3];         // Local axes (X, Y, Z)
    vector3 half_extents;

    obb_t();
    obb_t(const vector3& center, const vector3 axes[3], const vector3& half_extents);
    obb_t(const vector3& center, const quat& orientation, const vector3& half_extents);

    void transform(const matrix4x4& mat);
    bool contains_point(const vector3& p) const;
    bool intersects(const obb_t& other) const;
    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;
    aabb_t to_aabb() const;

    void get_corners(vector3 out[8]) const;
    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;
    double volume() const;
    double surface_area() const;
    vector3 support(const vector3& dir) const;
    bool equals(const obb_t& rhs, double epsilon = 1e-6) const;
    bool operator==(const obb_t& rhs) const;
    bool is_valid(double epsilon = 1e-6) const;

    void project_onto_axis(const vector3& axis, double& out_min, double& out_max) const;

    static obb_t from_points(const vector3* points, int count); 
};