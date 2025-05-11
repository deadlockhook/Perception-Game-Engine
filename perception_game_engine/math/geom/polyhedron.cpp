#include "polyhedron.h"
#include "../frustum.h" 
#include "../box.h" 
#include "../obb.h"
#include <unordered_map>

polyhedron_t::~polyhedron_t() {
    clear();
}

void polyhedron_t::clear() {
    if (faces) {
        for (size_t i = 0; i < face_count; ++i) {
            if (faces[i].indices) {
                free(faces[i].indices);
                faces[i].indices = nullptr;
            }
        }
        free(faces);
        faces = nullptr;
    }

    if (vertices) {
        free(vertices);
        vertices = nullptr;
    }

    vertex_count = 0;
    face_count = 0;
}

void polyhedron_t::allocate(size_t v_count, size_t f_count) {
    clear();
    if (v_count > 0)
        vertices = static_cast<vector3*>(malloc(sizeof(vector3) * v_count));
    if (f_count > 0)
        faces = static_cast<polyhedron_face_t*>(malloc(sizeof(polyhedron_face_t) * f_count));

    vertex_count = v_count;
    face_count = f_count;

    for (size_t i = 0; i < face_count; ++i) {
        faces[i].indices = nullptr;
        faces[i].count = 0;
    }
}

void polyhedron_t::set_face(size_t face_index, const int* indices, size_t count) {
    if (face_index >= face_count || !indices || count < 3)
        return;

    polyhedron_face_t& face = faces[face_index];
    face.count = count;
    face.indices = static_cast<int*>(malloc(sizeof(int) * count));
    for (size_t i = 0; i < count; ++i)
        face.indices[i] = indices[i];
}

bool polyhedron_t::is_valid() const {
    if (!vertices || vertex_count == 0 || !faces || face_count == 0)
        return false;

    for (size_t i = 0; i < face_count; ++i) {
        if (!faces[i].indices || faces[i].count < 3)
            return false;
        for (size_t j = 0; j < faces[i].count; ++j)
            if (faces[i].indices[j] < 0 || static_cast<size_t>(faces[i].indices[j]) >= vertex_count)
                return false;
    }
    return true;
}

void polyhedron_t::transform(const matrix4x4& m) {
    if (!vertices || vertex_count == 0)
        return;

    for (size_t i = 0; i < vertex_count; ++i)
        vertices[i] = m.transform_point(vertices[i]);
}

aabb_t polyhedron_t::compute_aabb() const {
    aabb_t box;
    box.invalidate();

    if (!vertices || vertex_count == 0)
        return box;

    for (size_t i = 0; i < vertex_count; ++i)
        box.expand(vertices[i]);

    return box;
}

double polyhedron_t::volume() const {
    if (!is_valid())
        return 0.0;

    double total = 0.0;

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 3) continue;

        const vector3& a = vertices[face.indices[0]];
        for (size_t i = 1; i + 1 < face.count; ++i) {
            const vector3& b = vertices[face.indices[i]];
            const vector3& c = vertices[face.indices[i + 1]];
            total += a.dot(b.cross(c)) / 6.0;
        }
    }

    return total;
}

vector3 polyhedron_t::center_of_mass() const {
    if (!is_valid()) return vector3(0.0, 0.0, 0.0);

    vector3 ref = compute_aabb().center(); // shift origin
    vector3 weighted_sum(0.0, 0.0, 0.0);
    double total_volume = 0.0;

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 3) continue;

        const vector3& a = vertices[face.indices[0]];
        for (size_t i = 1; i + 1 < face.count; ++i) {
            const vector3& b = vertices[face.indices[i]];
            const vector3& c = vertices[face.indices[i + 1]];

            vector3 ra = a - ref;
            vector3 rb = b - ref;
            vector3 rc = c - ref;

            double vol = ra.dot(rb.cross(rc)) / 6.0;
            vector3 local_centroid = (ra + rb + rc) / 4.0;
            weighted_sum += (ref + local_centroid) * vol;
            total_volume += vol;
        }
    }

    if (std::abs(total_volume) < 1e-8)
        return ref;

    return weighted_sum / total_volume;
}



const polyhedron_face_t& polyhedron_t::get_face(size_t index) const {
    static const polyhedron_face_t dummy = { nullptr, 0 };
    if (index >= face_count) return dummy;
    return faces[index];
}

vector3 polyhedron_t::get_vertex(size_t index) const {
    if (index >= vertex_count) return vector3(0.0, 0.0, 0.0);
    return vertices[index];
}

bool polyhedron_t::from_frustum(const frustum_t& f, const matrix4x4& inv_view_proj) {
    vector3 corners[8];
    f.get_corners(corners, inv_view_proj); 

    static const int face_indices[6][4] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7}, 
        {0, 4, 5, 1}, 
        {1, 5, 6, 2}, 
        {2, 6, 7, 3}, 
        {3, 7, 4, 0}  
    };

    allocate(8, 6);
    for (int i = 0; i < 8; ++i)
        vertices[i] = corners[i];

    for (int i = 0; i < 6; ++i)
        set_face(i, face_indices[i], 4);

    return true;
}


template <size_t N>
bool polyhedron_t::from_kdop(const kdop_t<N>& kdop) {
    if (!kdop.is_valid()) return false;

    const auto& axes = kdop.get_axes();

    allocate(N * 2, 0); 
    for (size_t i = 0; i < N; ++i) {
        vertices[i * 2 + 0] = axes[i] * kdop.mins[i];
        vertices[i * 2 + 1] = axes[i] * kdop.maxs[i];
    }

    face_count = 0;
    faces = nullptr;

    return true;
}

bool polyhedron_t::from_aabb(const aabb_t& box) {
    if (!box.is_valid()) return false;

    vector3 min = box.min;
    vector3 max = box.max;

    vector3 corners[8] = {
        {min.x, min.y, min.z}, {max.x, min.y, min.z},
        {max.x, max.y, min.z}, {min.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z},
        {max.x, max.y, max.z}, {min.x, max.y, max.z},
    };

    static const int face_indices[6][4] = {
        {0, 3, 2, 1}, 
        {4, 5, 6, 7}, 
        {0, 1, 5, 4}, 
        {1, 2, 6, 5}, 
        {2, 3, 7, 6}, 
        {3, 0, 4, 7}  
    };



    allocate(8, 6);
    for (int i = 0; i < 8; ++i)
        vertices[i] = corners[i];

    for (int i = 0; i < 6; ++i)
        set_face(i, face_indices[i], 4);

    return true;
}

bool polyhedron_t::from_box(const box_t& box) {
    if (!box.is_valid())
        return false;

    vector3 corners[8];
    box.get_corners(corners);

    static const int face_indices[6][4] = {
        {0, 1, 2, 3}, 
        {4, 5, 6, 7}, 
        {0, 4, 5, 1}, 
        {1, 5, 6, 2}, 
        {2, 6, 7, 3}, 
        {3, 7, 4, 0}  
    };

    allocate(8, 6);
    for (int i = 0; i < 8; ++i)
        vertices[i] = corners[i];

    for (int i = 0; i < 6; ++i)
        set_face(i, face_indices[i], 4);

    return true;
}


bool polyhedron_t::from_obb(const obb_t& obb) {
    vector3 scaled_axes[3] = {
        obb.axes[0] * obb.half_extents.x,
        obb.axes[1] * obb.half_extents.y,
        obb.axes[2] * obb.half_extents.z
    };


    static const int signs[8][3] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1},
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}
    };

    vector3 corners[8];
    for (int i = 0; i < 8; ++i) {
        corners[i] = obb.center
            + scaled_axes[0] * signs[i][0]
            + scaled_axes[1] * signs[i][1]
            + scaled_axes[2] * signs[i][2];
    }

    static const int face_indices[6][4] = {
        {0, 3, 2, 1}, 
        {4, 5, 6, 7}, 
        {0, 1, 5, 4},
        {1, 2, 6, 5}, 
        {2, 3, 7, 6}, 
        {3, 0, 4, 7}  
    };

    allocate(8, 6);
    for (int i = 0; i < 8; ++i)
        vertices[i] = corners[i];

    for (int i = 0; i < 6; ++i)
        set_face(i, face_indices[i], 4);

    return true;
}

void polyhedron_t::to_triangles(s_vector<int>& out_indices) const {
    out_indices.clear();

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 3) continue;

        int i0 = face.indices[0];
        for (size_t i = 1; i + 1 < face.count; ++i) {
            int i1 = face.indices[i];
            int i2 = face.indices[i + 1];

            out_indices.push_back(i0);
            out_indices.push_back(i1);
            out_indices.push_back(i2);
        }
    }
}

void polyhedron_t::to_lines(s_vector<std::pair<int, int>>& out_edges) const {
    out_edges.clear();

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 2) continue;

        for (size_t i = 0; i < face.count; ++i) {
            int a = face.indices[i];
            int b = face.indices[(i + 1) % face.count];
            if (a > b) std::swap(a, b);

            std::pair<int, int> edge(a, b);
            if (!out_edges.contains(edge))
                out_edges.push_back(edge);
        }
    }
}


bool polyhedron_t::is_closed() const {
    if (!is_valid()) return false;

    struct edge_key_t {
        int a, b;
        edge_key_t(int i, int j) : a(std::min(i, j)), b(std::max(i, j)) {}
        bool operator==(const edge_key_t& rhs) const { return a == rhs.a && b == rhs.b; }
    };

    struct edge_hash {
        size_t operator()(const edge_key_t& e) const {
            return (size_t(e.a) * 73856093) ^ (size_t(e.b) * 19349663);
        }
    };

    std::unordered_map<edge_key_t, int, edge_hash> edge_counts;

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 2) continue;

        for (size_t i = 0; i < face.count; ++i) {
            int a = face.indices[i];
            int b = face.indices[(i + 1) % face.count];
            edge_key_t key(a, b);
            edge_counts[key]++;
        }
    }

    for (const auto& kv : edge_counts) {
        if (kv.second != 2)
            return false; 
    }

    return true;
}

bool polyhedron_t::is_convex(double epsilon) const {
    if (!is_valid()) return false;

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 3) continue;

        const vector3& a = vertices[face.indices[0]];
        const vector3& b = vertices[face.indices[1]];
        const vector3& c = vertices[face.indices[2]];

        vector3 normal = (b - a).cross(c - a).normalized();

        double d = -normal.dot(a); 

        for (size_t vi = 0; vi < vertex_count; ++vi) {
            if (std::find(face.indices, face.indices + face.count, int(vi)) != face.indices + face.count)
                continue; 

            double dist = normal.dot(vertices[vi]) + d;
            if (dist > epsilon)
                return false; 
        }
    }

    return true;
}


template bool polyhedron_t::from_kdop<3>(const kdop_t<3>&);
template bool polyhedron_t::from_kdop<7>(const kdop_t<7>&);
template bool polyhedron_t::from_kdop<13>(const kdop_t<13>&);