#pragma once
#include "../vector3.h"
#include "../ray.h"
#include "../aabb.h"

//capsule_t
//sphere_t
//triangle_t

struct capsule_t {
    vector3 a;       
    vector3 b;      
    double radius;

    capsule_t();
    capsule_t(const vector3& a, const vector3& b, double radius);

    double length() const;
    double length_squared() const;
    vector3 center() const;
    vector3 direction() const;

    aabb_t to_aabb() const;
    bool contains_point(const vector3& p, double epsilon = 1e-6) const;
    double distance_to_point(const vector3& p) const;
    vector3 closest_point(const vector3& p) const;

    bool intersects_sphere(const vector3& center, double r) const;
    bool intersects_capsule(const capsule_t& other) const;
    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;

    bool equals(const capsule_t& other, double epsilon = 1e-6) const;

    double volume() const;
    double surface_area() const;
    bool is_valid() const;
    void transform(const matrix4x4& m);
    vector3 normal_at(const vector3& p) const;
    std::pair<vector3, vector3> get_segment() const;

    static capsule_t lerp(const capsule_t& a, const capsule_t& b, double t);

};

struct sphere_t {
    vector3 center;
    double radius;

    sphere_t();
    sphere_t(const vector3& center, double radius);

    double volume() const;
    double surface_area() const;
    bool is_valid() const;

    bool contains_point(const vector3& point, double epsilon = 1e-6) const;
    bool contains_sphere(const sphere_t& other, double epsilon = 1e-6) const;
    double distance_to_point(const vector3& point) const;
    vector3 closest_point(const vector3& point) const;

    bool intersects_sphere(const sphere_t& other) const;
    bool intersects_aabb(const aabb_t& box) const;
    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;

    aabb_t to_aabb() const;

    void transform(const matrix4x4& m);

    void expand(double delta);
    bool equals(const sphere_t& other, double epsilon = 1e-6) const;

    static sphere_t merge(const sphere_t& a, const sphere_t& b);
    static sphere_t from_aabb(const aabb_t& box);
};

struct triangle_t {
    vector3 a;
    vector3 b;
    vector3 c;

    triangle_t();
    triangle_t(const vector3& a, const vector3& b, const vector3& c);

    vector3 normal() const;
    vector3 centroid() const;
    double area() const;
    double perimeter() const;
    bool is_degenerate(double epsilon = 1e-8) const;

    bool contains_point(const vector3& p, double epsilon = 1e-6) const;
    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;

    void barycentric_coords(const vector3& p, double& u, double& v, double& w) const;
    vector3 from_barycentric(double u, double v, double w) const;

    bool intersects_ray(const ray_t& ray, double* out_t = nullptr, double epsilon = 1e-6) const;

    vector3 get_vertex(int i) const;
    vector3 get_edge(int i) const;
    triangle_t flipped() const;
    bool is_front_facing(const vector3& view_dir) const;
    bool equals(const triangle_t& other, double epsilon = 1e-6) const;
};