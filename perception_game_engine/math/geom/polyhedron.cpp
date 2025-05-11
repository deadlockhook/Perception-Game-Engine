#include "polyhedron.h"
#include "../frustum.h" 
#include "../box.h" 
#include "../obb.h"
#include <unordered_map>
#include "../math_defs.h"
#include "../../crt/s_string.h"

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

    vector3 ref = compute_aabb().center(); 
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

vector3 polyhedron_t::get_support_point(const vector3& dir) const {
    if (!is_valid() || dir.length_squared() < 1e-12)
        return vector3(0.0, 0.0, 0.0);

    double max_dot = -std::numeric_limits<double>::infinity();
    vector3 best(0.0, 0.0, 0.0);

    for (size_t i = 0; i < vertex_count; ++i) {
        double dot = vertices[i].dot(dir);
        if (dot > max_dot) {
            max_dot = dot;
            best = vertices[i];
        }
    }

    return best;
}

bool polyhedron_t::from_sphere(const vector3& center, double radius, int lat_div, int long_div) {
    if (lat_div < 2 || long_div < 3 || radius <= 0.0)
        return false;

    const int vertex_lat_count = lat_div - 1;
    const int vertex_count = 2 + vertex_lat_count * long_div;
    const int cap_count = long_div;
    const int band_face_count = (lat_div - 2) * long_div;
    const int total_faces = 2 * cap_count + band_face_count;

    allocate(vertex_count, total_faces);

    int v = 0;
    vertices[v++] = center + vector3(0, radius, 0); 

    for (int i = 1; i < lat_div; ++i) {
        double theta = M_PI * i / lat_div;
        double y = std::cos(theta);
        double r = std::sin(theta);

        for (int j = 0; j < long_div; ++j) {
            double phi = 2.0 * M_PI * j / long_div;
            double x = r * std::cos(phi);
            double z = r * std::sin(phi);

            vertices[v++] = center + radius * vector3(x, y, z);
        }
    }

    vertices[v++] = center + vector3(0, -radius, 0); 

    int top_index = 0;
    int bottom_index = vertex_count - 1;

    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int i1 = 1 + j;
        int i2 = 1 + next;
        int face[3] = { top_index, i2, i1 };
        set_face(j, face, 3);
    }

    int face_index = long_div;
    for (int i = 0; i < vertex_lat_count - 1; ++i) {
        for (int j = 0; j < long_div; ++j) {
            int next = (j + 1) % long_div;
            int row1 = 1 + i * long_div;
            int row2 = 1 + (i + 1) * long_div;

            int a = row1 + j;
            int b = row1 + next;
            int c = row2 + next;
            int d = row2 + j;

            int quad[4] = { a, b, c, d };
            set_face(face_index++, quad, 4);
        }
    }

    int base = 1 + (lat_div - 2) * long_div;
    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int i1 = base + j;
        int i2 = base + next;
        int face[3] = { bottom_index, i1, i2 };
        set_face(face_index++, face, 3);
    }

    return true;
}

bool polyhedron_t::from_cylinder(const vector3& center, double radius, double height, const vector3& axis, int segments) {
    if (segments < 3 || radius <= 0.0 || height <= 0.0 || !axis.is_finite())
        return false;

    vector3 up = axis.normalized();
    vector3 right = up.perpendicular();
    vector3 forward = up.cross(right);

    vector3 top_center = center + up * (height * 0.5);
    vector3 bottom_center = center - up * (height * 0.5);

    int v_count = 2 * segments + 2;
    int f_count = segments * 3;

    allocate(v_count, f_count);

    int v = 0;

    for (int i = 0; i < segments; ++i) {
        double angle = (2.0 * M_PI * i) / segments;
        vector3 dir = std::cos(angle) * right + std::sin(angle) * forward;
        vertices[v++] = bottom_center + dir * radius;
    }
    for (int i = 0; i < segments; ++i) {
        double angle = (2.0 * M_PI * i) / segments;
        vector3 dir = std::cos(angle) * right + std::sin(angle) * forward;
        vertices[v++] = top_center + dir * radius;
    }

    int bottom_index = v++;
    int top_index = v++;

    vertices[bottom_index] = bottom_center;
    vertices[top_index] = top_center;

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int a = i;
        int b = next;
        int c = segments + next;
        int d = segments + i;

        int quad[4] = { a, b, c, d };
        set_face(i, quad, 4);
    }

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int face[3] = { bottom_index, next, i };
        set_face(segments + i, face, 3);
    }

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int a = segments + i;
        int b = segments + next;
        int face[3] = { top_index, a, b };
        set_face(2 * segments + i, face, 3);
    }

    return true;
}

bool polyhedron_t::from_capsule(const vector3& center, double radius, double height, const vector3& axis, int lat_div, int long_div) {
    if (lat_div < 2 || long_div < 3 || radius <= 0.0 || height <= 0.0 || !axis.is_finite())
        return false;

    const int hemi_lat = lat_div;
    const int ring_count = hemi_lat * 2 + 1; 
    const int vertex_count = 2 + (ring_count - 1) * long_div; 
    const int face_count = 2 * long_div + (ring_count - 2) * long_div;

    allocate(vertex_count, face_count);

    vector3 up = axis.normalized();
    vector3 right = up.perpendicular().normalized();
    vector3 forward = up.cross(right).normalized();

    vector3 top_center = center + up * (height * 0.5);
    vector3 bottom_center = center - up * (height * 0.5);

    vertices[0] = top_center + up * radius;
    vertices[vertex_count - 1] = bottom_center - up * radius;

    int v = 1;
    for (int i = 1; i < ring_count; ++i) {
        double t = double(i) / (ring_count);
        double theta = t * M_PI;

        double y = std::cos(theta);
        double r = std::sin(theta);

        vector3 ring_center;
        if (i < hemi_lat + 1)
            ring_center = top_center;
        else if (i == hemi_lat + 1)
            ring_center = center;
        else
            ring_center = bottom_center;

        for (int j = 0; j < long_div; ++j) {
            double phi = 2.0 * M_PI * j / long_div;
            vector3 dir = std::cos(phi) * right + std::sin(phi) * forward;
            vector3 pos = ring_center + up * (y * radius) + dir * (r * radius);
            vertices[v++] = pos;
        }
    }

    int f = 0;
    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int face[3] = { 0, 1 + next, 1 + j };
        set_face(f++, face, 3);
    }

    for (int i = 0; i < ring_count - 2; ++i) {
        int row1 = 1 + i * long_div;
        int row2 = row1 + long_div;
        for (int j = 0; j < long_div; ++j) {
            int next = (j + 1) % long_div;
            int quad[4] = { row1 + j, row1 + next, row2 + next, row2 + j };
            set_face(f++, quad, 4);
        }
    }

    int base = 1 + (ring_count - 2) * long_div;
    int bottom = vertex_count - 1;
    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int face[3] = { bottom, base + j, base + next };
        set_face(f++, face, 3);
    }

    return true;
}

bool polyhedron_t::from_cone(const cone_t& cone, int segments) {
    if (segments < 3 || !cone.is_valid())
        return false;

    vector3 apex = cone.apex;
    vector3 dir = cone.direction.normalized();
    double height = cone.height;
    double radius = std::tan(cone.angle_rad) * height;
    vector3 base_center = apex + dir * height;

    vector3 up = dir;
    vector3 right = up.perpendicular().normalized();
    vector3 forward = up.cross(right).normalized();

    int vertex_count = 1 + segments + 1; 
    int face_count = segments * 2;     

    allocate(vertex_count, face_count);

    int v = 0;
    vertices[v++] = apex;

    for (int i = 0; i < segments; ++i) {
        double angle = (2.0 * M_PI * i) / segments;
        vector3 dir = std::cos(angle) * right + std::sin(angle) * forward;
        vertices[v++] = base_center + dir * radius;
    }

    vertices[v++] = base_center;
    int base_center_index = vertex_count - 1;

    int f = 0;
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int face[3] = { 0, 1 + i, 1 + next };
        set_face(f++, face, 3);
    }

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int face[3] = { base_center_index, 1 + next, 1 + i };
        set_face(f++, face, 3);
    }

    return true;
}

bool polyhedron_t::from_ellipsoid(const ellipsoid_t& e, int lat_div, int long_div) {
    if (lat_div < 2 || long_div < 3 || !e.is_valid())
        return false;

    const int vertex_count = (lat_div - 1) * long_div + 2;
    const int face_count = (lat_div - 2) * long_div * 2 + long_div * 2; 

    allocate(vertex_count, face_count);

    vector3 center = e.center;
    vector3 radii = e.radii;
    quat orientation = e.orientation;

    int v = 0;

    vertices[v++] = center + orientation.rotate(vector3(0, +radii.y, 0));

    for (int i = 1; i < lat_div; ++i) {
        double theta = M_PI * i / lat_div; 
        double sin_theta = std::sin(theta);
        double cos_theta = std::cos(theta);

        for (int j = 0; j < long_div; ++j) {
            double phi = 2.0 * M_PI * j / long_div;
            double sin_phi = std::sin(phi);
            double cos_phi = std::cos(phi);

            vector3 local_pos = {
                radii.x * sin_theta * cos_phi,
                radii.y * cos_theta,
                radii.z * sin_theta * sin_phi
            };
            vertices[v++] = center + orientation.rotate(local_pos);
        }
    }

    vertices[v++] = center + orientation.rotate(vector3(0, -radii.y, 0));

    int f = 0;
    int top = 0;
    int bottom = vertex_count - 1;

    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int face[3] = { top, 1 + next, 1 + j };
        set_face(f++, face, 3);
    }

    for (int i = 0; i < lat_div - 2; ++i) {
        for (int j = 0; j < long_div; ++j) {
            int next_j = (j + 1) % long_div;

            int a = 1 + i * long_div + j;
            int b = 1 + i * long_div + next_j;
            int c = 1 + (i + 1) * long_div + next_j;
            int d = 1 + (i + 1) * long_div + j;

            int face1[3] = { a, b, c };
            int face2[3] = { a, c, d };
            set_face(f++, face1, 3);
            set_face(f++, face2, 3);
        }
    }

    int offset = 1 + (lat_div - 2) * long_div;
    for (int j = 0; j < long_div; ++j) {
        int next = (j + 1) % long_div;
        int face[3] = { bottom, offset + j, offset + next };
        set_face(f++, face, 3);
    }

    return true;
}

bool polyhedron_t::from_disc(const disc_t& disc, int segments) {
    if (segments < 3 || !disc.is_valid())
        return false;

    allocate(segments + 1, segments);

    vector3 center = disc.center;
    vector3 normal = disc.normal.normalized();
    vector3 right = normal.perpendicular().normalized();
    vector3 forward = normal.cross(right).normalized();

    vertices[0] = center;

    for (int i = 0; i < segments; ++i) {
        double angle = (2.0 * M_PI * i) / segments;
        vector3 dir = std::cos(angle) * right + std::sin(angle) * forward;
        vertices[1 + i] = center + dir * disc.radius;
    }

    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        int tri[3] = { 0, 1 + i, 1 + next };
        set_face(i, tri, 3);
    }

    return true;
}

bool polyhedron_t::from_torus(const torus_t& torus, int major_segments, int minor_segments) {
    if (major_segments < 3 || minor_segments < 3 || !torus.is_valid())
        return false;

    const int vertex_count = major_segments * minor_segments;
    const int face_count = major_segments * minor_segments;

    allocate(vertex_count, face_count);

    vector3 right, up, forward;
    torus.get_axes(right, up, forward);

    int v = 0;
    for (int i = 0; i < major_segments; ++i) {
        double theta = 2.0 * M_PI * i / major_segments;
        vector3 major_dir = std::cos(theta) * right + std::sin(theta) * forward;
        vector3 ring_center = torus.center + major_dir * torus.major_radius;

        vector3 tube_right = major_dir.cross(up).normalized();
        vector3 tube_up = up;

        for (int j = 0; j < minor_segments; ++j) {
            double phi = 2.0 * M_PI * j / minor_segments;
            vector3 local_dir = std::cos(phi) * tube_right + std::sin(phi) * tube_up;
            vertices[v++] = ring_center + local_dir * torus.minor_radius;
        }
    }

    int f = 0;
    for (int i = 0; i < major_segments; ++i) {
        int next_i = (i + 1) % major_segments;
        for (int j = 0; j < minor_segments; ++j) {
            int next_j = (j + 1) % minor_segments;

            int a = i * minor_segments + j;
            int b = i * minor_segments + next_j;
            int c = next_i * minor_segments + next_j;
            int d = next_i * minor_segments + j;

            int face[4] = { a, b, c, d };
            set_face(f++, face, 4);
        }
    }

    return true;
}

bool polyhedron_t::from_segment(const vector3& start, const vector3& end, double radius, int segments) {
    if ((end - start).length_squared() < 1e-8 || radius <= 0.0 || segments < 3)
        return false;

    vector3 center = (start + end) * 0.5;
    double height = (end - start).length();
    vector3 axis = (end - start).normalized();

    return from_cylinder(center, radius, height, axis, segments);
}

bool polyhedron_t::from_line(const line_t& line, double extent, double radius, int segments) {
    if (!line.is_valid() || extent <= 0.0 || radius <= 0.0 || segments < 3)
        return false;

    vector3 half = line.direction.normalized() * (extent * 0.5);
    vector3 start = line.point - half;
    vector3 end = line.point + half;

    return from_segment(start, end, radius, segments);
}

bool polyhedron_t::from_triangle(const triangle_t& tri) {
    if (tri.is_degenerate())
        return false;

    allocate(3, 1);
    vertices[0] = tri.a;
    vertices[1] = tri.b;
    vertices[2] = tri.c;

    int indices[3] = { 0, 1, 2 };
    set_face(0, indices, 3);

    return true;
}

bool polyhedron_t::export_to_obj(const char* path) const {
    if (!path || !is_valid()) return false;

    FILE* file = nullptr;
    fopen_s(&file, (s_string("C:\\Users\\Timefall\\Documents\\Perception Engine\\")+path).c_str(), "w");
    if (!file) return false;

    for (size_t i = 0; i < vertex_count; ++i) {
        const vector3& v = vertices[i];
        std::fprintf(file, "v %.10f %.10f %.10f\n", v.x, v.y, v.z);
    }

    for (size_t f = 0; f < face_count; ++f) {
        const polyhedron_face_t& face = faces[f];
        if (face.count < 3) continue;

        std::fprintf(file, "f");
        for (size_t i = 0; i < face.count; ++i)
            std::fprintf(file, " %d", face.indices[i] + 1);
        std::fprintf(file, "\n");
    }

    std::fclose(file);
    return true;
}

template bool polyhedron_t::from_kdop<3>(const kdop_t<3>&);
template bool polyhedron_t::from_kdop<7>(const kdop_t<7>&);
template bool polyhedron_t::from_kdop<13>(const kdop_t<13>&);