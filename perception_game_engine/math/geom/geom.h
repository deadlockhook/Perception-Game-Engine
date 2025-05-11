#pragma once
#include "../vector3.h"
#include "../ray.h"
#include "../aabb.h"

//capsule_t
//sphere_t
//ellipsoid_t
//triangle_t
//segment_t
//cylinder_t
//cone_t
//disc_t
//line_t
//torus_t

struct plane_t;

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

struct ellipsoid_t {
    vector3 center;
    vector3 radii;
    quat orientation;

    ellipsoid_t() = default;
    ellipsoid_t(const vector3& center, const vector3& radii, const quat& orientation);

    bool is_valid() const;
    bool is_sphere(double epsilon = 1e-6) const;

    ellipsoid_t transformed(const matrix4x4& m) const;
    ellipsoid_t normalized() const;

    double signed_distance(const vector3& pt) const;
    vector3 closest_point(const vector3& pt) const;
    double distance_to_point(const vector3& pt) const;
    bool contains_point(const vector3& pt) const;

    vector3 sample_point(double u, double v) const;
    aabb_t compute_aabb() const;

    void get_axes(vector3& out_x, vector3& out_y, vector3& out_z) const;

    bool intersects_ray(const ray_t& ray, double& out_t) const;

    bool operator==(const ellipsoid_t& rhs) const;
    bool operator!=(const ellipsoid_t& rhs) const;
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

struct segment_t {
    vector3 start;
    vector3 end;

    segment_t();
    segment_t(const vector3& start, const vector3& end);


    vector3 center() const;
    vector3 direction() const;
    double length() const;
    double length_squared() const;
    vector3 point_at(double t) const;
    double project_point_t(const vector3& p) const;

    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;

    bool intersects_sphere(const vector3& center, double radius) const;
    bool intersects_aabb(const aabb_t& box) const;

    ray_t to_ray() const;
    std::pair<vector3, vector3> as_pair() const;
    void transform(const matrix4x4& m);
    bool equals(const segment_t& other, double epsilon = 1e-6) const;
};

struct cylinder_t {
    vector3 a;  
    vector3 b;      
    double radius;

    cylinder_t();
    cylinder_t(const vector3& a, const vector3& b, double radius);

    vector3 center() const;
    vector3 axis() const;
    double height() const;
    double volume() const;
    double surface_area() const;
    vector3 point_at(double t) const;

    vector3 support(const vector3& dir) const;
    void project_onto_axis(const vector3& axis, double& out_min, double& out_max) const;

    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;
    bool contains_point(const vector3& p, double epsilon = 1e-6) const;

    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;
    bool intersects_sphere(const vector3& center, double r) const;
    bool intersects_cylinder(const cylinder_t& other) const;

    aabb_t to_aabb() const;

    void transform(const matrix4x4& m);
    std::pair<vector3, vector3> get_segment() const;
    bool equals(const cylinder_t& other, double epsilon = 1e-6) const;
    bool is_valid() const;
    static cylinder_t lerp(const cylinder_t& a, const cylinder_t& b, double t);
};

struct cone_t {
    vector3 apex;
    vector3 direction;
    double angle_rad;
    double height;

    cone_t();
    cone_t(const vector3& apex, const vector3& direction, double angle_rad, double height);

    vector3 base_center() const;
    double radius() const;
    double volume() const;
    double surface_area() const;

    bool contains_point(const vector3& p, double epsilon = 1e-6) const;
    double distance_to_point(const vector3& p) const;
    vector3 closest_point(const vector3& p) const;

    bool intersects_sphere(const vector3& center, double r) const;
    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;

    aabb_t to_aabb() const;

    void transform(const matrix4x4& m);
    bool equals(const cone_t& other, double epsilon = 1e-6) const;
    bool is_valid() const;
    static cone_t lerp(const cone_t& a, const cone_t& b, double t);
};
struct disc_t {
    vector3 center;
    vector3 normal;
    double radius;

    disc_t();
    disc_t(const vector3& center, const vector3& normal, double radius);

    double area() const;

    bool contains_point(const vector3& p, double epsilon = 1e-6) const;
    vector3 closest_point(const vector3& p) const;
    double distance_to_point(const vector3& p) const;

    bool intersects_sphere(const vector3& sphere_center, double r) const;
    bool intersects_ray(const ray_t& ray, double* out_t = nullptr) const;

    aabb_t to_aabb() const;

    vector3 support(const vector3& dir) const;
    void project_onto_axis(const vector3& axis, double& out_min, double& out_max) const;

    void transform(const matrix4x4& m);
    bool equals(const disc_t& other, double epsilon = 1e-6) const;
    bool is_valid() const;
    static disc_t lerp(const disc_t& a, const disc_t& b, double t);
};

struct line_t {
    vector3 point;     
    vector3 direction;  

    line_t() = default;
    line_t(const vector3& point, const vector3& direction); 

    static line_t from_points(const vector3& a, const vector3& b); 

    vector3 point_at(double t) const;

    bool is_valid() const;

    line_t reversed() const;

    double project_length(const vector3& pt) const;
    vector3 closest_point(const vector3& pt) const;
    double distance_to_point(const vector3& pt) const;
    double distance_to_line(const line_t& other) const;
    std::pair<vector3, vector3> closest_points_between(const line_t& other) const;

    bool intersects_plane(const plane_t& plane, vector3& out_point) const;

    bool operator==(const line_t& other) const;
    bool operator!=(const line_t& other) const;
};

struct torus_t {
    vector3 center;
    quat orientation;
    double major_radius;
    double minor_radius;

    torus_t() = default;
    torus_t(const vector3& center, const quat& orientation, double major_radius, double minor_radius);

    bool is_valid() const;

    torus_t transformed(const matrix4x4& m) const;

    vector3 closest_point(const vector3& pt) const;
    double distance_to_point(const vector3& pt) const;
    bool contains_point(const vector3& pt) const;

    double signed_distance(const vector3& pt) const;

    aabb_t compute_aabb() const;

    vector3 sample_point(double u, double v) const;
    void get_axes(vector3& out_x, vector3& out_y, vector3& out_z) const;

    bool operator==(const torus_t& other) const;
    bool operator!=(const torus_t& other) const;
};

