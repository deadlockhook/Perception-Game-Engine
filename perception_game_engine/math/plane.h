#pragma once
#include "vector3.h"
#include "vector4.h"

struct matrix4x4;
struct ray_t;
struct plane_t {
    vector3 normal;
    double d;

    plane_t() : normal(), d(0.0) {}
    plane_t(const vector3& normal, double d) : normal(normal), d(d) {}
    
    bool operator==(const plane_t& rhs) const {
        return normal == rhs.normal && d == rhs.d;
    }

    bool operator!=(const plane_t& rhs) const {
        return !(*this == rhs);
    }

    bool is_valid() const {
        return normal.is_finite() && normal.length_squared() > 1e-8;
    }

    double dot4(const vector4& v) const {
        return normal.x * v.x + normal.y * v.y + normal.z * v.z + d * v.w;
    }

    plane_t transform(const matrix4x4& m) const;

    static plane_t from_point_normal(const vector3& point, const vector3& normal) {
        vector3 n = normal.normalized();
        double d = -n.dot(point);
        return plane_t(n, d);
    }

    static plane_t from_points(const vector3& a, const vector3& b, const vector3& c) {
        vector3 n = (b - a).cross(c - a).normalized();
        double d = -n.dot(a);
        return plane_t(n, d);
    }
    static plane_t from_vector4(const vector4& v) {
        return plane_t(vector3(v.x, v.y, v.z), v.w);
    }

    double distance_to_point(const vector3& point) const {
        return normal.dot(point) + d;
    }

    bool is_in_front(const vector3& point, double epsilon = 1e-6) const {
        return distance_to_point(point) >= -epsilon;
    }

    bool is_behind(const vector3& point, double epsilon = 1e-6) const {
        return distance_to_point(point) < -epsilon;
    }

    plane_t flipped() const {
        return plane_t(-normal, -d);
    }

    plane_t normalized() const {
        double len = normal.length();
        if (len > 0.0)
            return plane_t(normal / len, d / len);
        return *this;
    }

    bool equals(const plane_t& rhs, double epsilon = 1e-6) const {
        return normal.equals(rhs.normal, epsilon) && std::abs(d - rhs.d) < epsilon;
    }

    vector3 project_point(const vector3& p) const {
        return p - normal * distance_to_point(p);
    }

    vector4 to_vector4() const {
        return vector4(normal.x, normal.y, normal.z, d);
    }

    bool intersect_line(const vector3& p0, const vector3& p1, vector3& out_point) const {
        vector3 dir = p1 - p0;
        double denom = normal.dot(dir);
        if (std::abs(denom) < 1e-8)
            return false; 

        double t = -(normal.dot(p0) + d) / denom;
        if (t < 0.0 || t > 1.0)
            return false; 

        out_point = p0 + dir * t;
        return true;
    }

    void debug_distance_to(const vector3& p, const char* label = "") const {
        double dist = distance_to_point(p);
        printf("  %s plane: normal = (%.3f, %.3f, %.3f), d = %.3f, distance to point = %.6f\n",
            label, normal.x, normal.y, normal.z, d, dist);
    }

    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;
    bool intersects_segment(const vector3& p0, const vector3& p1, vector3* out_point = nullptr) const;
    int clip_triangle(const vector3 tri[3], vector3 out[4]) const;

    enum class side_t { front, back, intersecting };
    side_t classify_point(const vector3& p, double epsilon = 1e-6) const;
    side_t classify_triangle(const vector3 tri[3], double epsilon = 1e-6) const;

};

struct aabb_t;
struct halfspace_t : public plane_t {
    halfspace_t() = default;
    explicit halfspace_t(const plane_t& p) : plane_t(p) {}

    bool contains(const vector3& p, double epsilon = 1e-6) const;
    bool intersects_sphere(const vector3& center, double radius, double epsilon = 1e-6) const;
    bool intersects_aabb(const aabb_t& box, double epsilon = 1e-6) const;

    double signed_distance(const vector3& p) const;
    vector3 closest_point(const vector3& p) const;
    void flip();
};


