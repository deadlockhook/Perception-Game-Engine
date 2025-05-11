#include "plane.h"
#include "matrix4x4.h"
#include "ray.h"
#include "aabb.h"

plane_t plane_t::transform(const matrix4x4& mat) const {
    matrix4x4 inv_transpose = mat.inverse_affine().transposed();
    vector4 p = to_vector4();
    vector4 tp = inv_transpose.transform_vec4(p);
    return plane_t(vector3(tp.x, tp.y, tp.z), tp.w).normalized();
}

bool plane_t::intersects_ray(const ray_t& ray, double* out_t) const {
    double denom = normal.dot(ray.direction);
    if (std::abs(denom) < 1e-8) return false;

    double t = -(normal.dot(ray.origin) + d) / denom;
    if (t < 0.0) return false;

    if (out_t) *out_t = t;
    return true;
}

bool plane_t::intersects_segment(const vector3& p0, const vector3& p1, vector3* out_point) const {
    vector3 dir = p1 - p0;
    double denom = normal.dot(dir);
    if (std::abs(denom) < 1e-8) return false;

    double t = -(normal.dot(p0) + d) / denom;
    if (t < 0.0 || t > 1.0) return false;

    if (out_point)
        *out_point = p0 + dir * t;
    return true;
}
plane_t::side_t plane_t::classify_point(const vector3& p, double epsilon) const {
    double dist = distance_to_point(p);
    if (dist > epsilon) return side_t::front;
    if (dist < -epsilon) return side_t::back;
    return side_t::intersecting;
}

plane_t::side_t plane_t::classify_triangle(const vector3 tri[3], double epsilon) const {
    int front = 0, back = 0;
    for (int i = 0; i < 3; ++i) {
        double d = distance_to_point(tri[i]);
        if (d > epsilon) ++front;
        else if (d < -epsilon) ++back;
    }
    if (front > 0 && back == 0) return side_t::front;
    if (back > 0 && front == 0) return side_t::back;
    return side_t::intersecting;
}

int plane_t::clip_triangle(const vector3 tri[3], vector3 out[4]) const {
    const vector3& a = tri[0];
    const vector3& b = tri[1];
    const vector3& c = tri[2];

    double da = distance_to_point(a);
    double db = distance_to_point(b);
    double dc = distance_to_point(c);

    bool ina = da >= 0.0;
    bool inb = db >= 0.0;
    bool inc = dc >= 0.0;

    int count = 0;
    auto interp = [&](const vector3& p0, const vector3& p1, double d0, double d1) {
        double t = d0 / (d0 - d1);
        return p0 + (p1 - p0) * t;
        };

    if (ina) out[count++] = a;
    if (ina != inb) out[count++] = interp(a, b, da, db);
    if (inb) out[count++] = b;
    if (inb != inc) out[count++] = interp(b, c, db, dc);
    if (inc) out[count++] = c;
    if (inc != ina && count < 4) out[count++] = interp(c, a, dc, da);

    return count;
}

bool halfspace_t::contains(const vector3& p, double epsilon) const {
    return distance_to_point(p) >= -epsilon;
}

bool halfspace_t::intersects_sphere(const vector3& center, double radius, double epsilon) const {
    return distance_to_point(center) >= -radius - epsilon;
}

bool halfspace_t::intersects_aabb(const aabb_t& box, double epsilon) const {

    vector3 corners[8] = {
        {box.min.x, box.min.y, box.min.z},
        {box.max.x, box.min.y, box.min.z},
        {box.min.x, box.max.y, box.min.z},
        {box.max.x, box.max.y, box.min.z},
        {box.min.x, box.min.y, box.max.z},
        {box.max.x, box.min.y, box.max.z},
        {box.min.x, box.max.y, box.max.z},
        {box.max.x, box.max.y, box.max.z},
    };

    for (int i = 0; i < 8; ++i) {
        if (contains(corners[i], epsilon))
            return true;
    }
    return false;
}

double halfspace_t::signed_distance(const vector3& p) const {
    return distance_to_point(p);
}

vector3 halfspace_t::closest_point(const vector3& p) const {
    return project_point(p);
}

void halfspace_t::flip() {
    plane_t flipped_plane = flipped(); 
    normal = flipped_plane.normal;
    d = flipped_plane.d;
}
