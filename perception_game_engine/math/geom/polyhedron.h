#pragma once
#include "../vector3.h"
#include "../matrix4x4.h"
#include "../aabb.h"
#include <cstddef>
#include "kdop.h"
#include "../../crt/s_vector.h"
#include "geom.h"

struct frustum_t;
struct matrix4x4;
struct box_t;
struct obb_t;

struct polyhedron_face_t {
    int* indices = nullptr;
    size_t count = 0;
};

struct polyhedron_t {
    vector3* vertices = nullptr;
    size_t vertex_count = 0;

    polyhedron_face_t* faces = nullptr;
    size_t face_count = 0;

    polyhedron_t() = default;
    ~polyhedron_t();

    void clear();
    bool is_valid() const;

    void allocate(size_t vertex_count, size_t face_count);
    void set_face(size_t face_index, const int* indices, size_t count);

    void transform(const matrix4x4& m);
    aabb_t compute_aabb() const;
    double volume() const;
    vector3 center_of_mass() const;

    const polyhedron_face_t& get_face(size_t index) const;
    vector3 get_vertex(size_t index) const;

    polyhedron_t(const polyhedron_t&) = delete;
    polyhedron_t& operator=(const polyhedron_t&) = delete;

    void to_triangles(s_vector<int>& out_indices) const;
    void to_lines(s_vector<std::pair<int, int>>& out_edges) const;

    template <size_t N>
    bool from_kdop(const kdop_t<N>& kdop);
    bool from_frustum(const frustum_t& f, const matrix4x4& inv_view_proj);
    bool from_aabb(const aabb_t& box);
    bool from_box(const box_t& box);
    bool from_obb(const obb_t& obb);
    bool from_sphere(const vector3& center, double radius, int lat_div = 8, int long_div = 12);
    bool from_capsule(const vector3& center, double radius, double height, const vector3& axis, int lat_div = 6, int long_div = 12);
    bool from_cylinder(const vector3& center, double radius, double height, const vector3& axis, int segments = 12);
    bool from_cone(const cone_t& cone, int segments = 16);
    bool from_torus(const torus_t& torus, int major_segments = 24, int minor_segments = 12);
    bool from_disc(const disc_t& disc, int segments = 32);
    bool from_ellipsoid(const ellipsoid_t& e, int lat_div = 12, int long_div = 24);
    bool from_segment(const vector3& start, const vector3& end, double radius = 0.01, int segments = 12);
    bool from_line(const line_t& line, double extent = 1.0, double radius = 0.01, int segments = 12);
    bool from_triangle(const triangle_t& tri);
    
    bool has_duplicate_vertices(double epsilon) const;
    size_t merge_close_vertices(double epsilon);
    bool repair(double epsilon = 1e-6);
    bool is_manifold() const;

    bool is_closed() const;
    bool is_convex(double epsilon = 1e-6) const;
    vector3 get_support_point(const vector3& dir) const;
    bool export_to_obj(const char* path) const;
};
