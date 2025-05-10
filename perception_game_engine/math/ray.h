#pragma once
#include <utility>     // for std::pair
#include <limits>      // for infinity
#include "vector3.h"

struct aabb_t; // forward declaration only needed here

struct ray_t {
    vector3 origin;
    vector3 direction;

    ray_t() = default;

    ray_t(const vector3& origin, const vector3& direction);

    vector3 point_at(double t) const;

    ray_t reversed() const {
        return ray_t(origin, direction * -1.0);
    }

    ray_t offset(double epsilon) const {
        return ray_t(origin + direction * epsilon, direction);
    }

    bool is_valid() const {
        return direction.is_finite() && origin.is_finite() &&
            direction.length_squared() > 0.0;
    }

    std::pair<vector3, vector3> as_segment(double distance) const {
        return { origin, point_at(distance) };
    }

    double project_length(const vector3& point) const {
        return (point - origin).dot(direction);
    }

    vector3 closest_point(const vector3& point) const {
        return origin + direction * project_length(point);
    }

    double distance_to_point(const vector3& point) const {
        return (point - closest_point(point)).length();
    }

    bool intersects_aabb(const aabb_t& box, double& out_tmin, double& out_tmax) const;
};
