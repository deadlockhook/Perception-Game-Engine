#pragma once
#include "../vector3.h"
#include "../matrix4x4.h"
#include "../aabb.h"
#include <cstddef>
#include "kdop.h"
#include "../../crt/s_vector.h"

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

    bool from_frustum(const frustum_t& f, const matrix4x4& inv_view_proj);
    void to_triangles(s_vector<int>& out_indices) const;
    void to_lines(s_vector<std::pair<int, int>>& out_edges) const;

    template <size_t N>
    bool from_kdop(const kdop_t<N>& kdop);

    bool from_aabb(const aabb_t& box);
    bool from_box(const box_t& box);
    bool from_obb(const obb_t& obb);
    bool is_closed() const;
    bool is_convex(double epsilon = 1e-6) const;

};
