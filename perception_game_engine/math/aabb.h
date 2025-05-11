#pragma once
#include "vector3.h"
#include <utility>
#include <limits>
#include "matrix4x4.h"

struct ray_t;
struct aabb_t {
    vector3 min;
    vector3 max;

    aabb_t() : min(), max() {}
    aabb_t(const vector3& min, const vector3& max) : min(min), max(max) {}
   
    bool contains(const vector3& point) const {
        return point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y &&
            point.z >= min.z && point.z <= max.z;
    }

    bool contains_point(const vector3& point) const {
        return contains(point);
    }

    static aabb_t from_center_extents(const vector3& center, const vector3& extent) {
        return aabb_t(center - extent, center + extent);
    }

    bool operator==(const aabb_t& rhs) const {
        return min == rhs.min && max == rhs.max;
    }

    bool equals(const aabb_t& rhs, double epsilon = 1e-6) const {
        return min.equals(rhs.min, epsilon) && max.equals(rhs.max, epsilon);
    }

    void invalidate() {
        min = vector3(std::numeric_limits<double>::infinity());
        max = vector3(-std::numeric_limits<double>::infinity());
    }

    vector3 center() const {
        return (min + max) * 0.5;
    }

    bool contains(const aabb_t& other) const {
        return contains(other.min) && contains(other.max);
    }

    vector3 extent() const {
        return (max - min) * 0.5;
    }

    vector3 size() const {
        return max - min;
    }

    bool intersects(const aabb_t& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }

    void expand(double margin) {
        min -= vector3(margin);
        max += vector3(margin);
    }

    double volume() const {
        vector3 s = size();
        return s.x * s.y * s.z;
    }

    static aabb_t merge(const aabb_t& a, const aabb_t& b) {
        aabb_t result = a;
        result.expand(b);
        return result;
    }

    aabb_t transform(const matrix4x4& mat) const {

        vector3 corners[8] = {
            {min.x, min.y, min.z}, {max.x, min.y, min.z},
            {min.x, max.y, min.z}, {max.x, max.y, min.z},
            {min.x, min.y, max.z}, {max.x, min.y, max.z},
            {min.x, max.y, max.z}, {max.x, max.y, max.z},
        };
        aabb_t result;
        result.min = result.max = mat.transform_point(corners[0]);
        for (int i = 1; i < 8; ++i)
            result.expand(mat.transform_point(corners[i]));
        return result;
    }

    bool intersect_ray(const ray_t& ray, double& tmin, double& tmax) const;

    void expand(const vector3& point) {
        min.x = std::min(min.x, point.x);
        min.y = std::min(min.y, point.y);
        min.z = std::min(min.z, point.z);
        max.x = std::max(max.x, point.x);
        max.y = std::max(max.y, point.y);
        max.z = std::max(max.z, point.z);
    }

    void expand(const aabb_t& box) {
        expand(box.min);
        expand(box.max);
    }

    bool is_valid() const {
        return min.x <= max.x && min.y <= max.y && min.z <= max.z;
    }

    double surface_area() const {
        vector3 s = size();
        return 2.0 * (s.x * s.y + s.y * s.z + s.z * s.x);
    }
};
