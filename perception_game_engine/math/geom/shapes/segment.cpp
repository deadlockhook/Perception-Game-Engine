#include "../geom.h"
#include "../../math_defs.h"

segment_t::segment_t()
    : start(vector3::zero()), end(vector3::zero()) {
}

segment_t::segment_t(const vector3& s, const vector3& e)
    : start(s), end(e) {
}

vector3 segment_t::center() const {
    return (start + end) * 0.5;
}

vector3 segment_t::direction() const {
    return (end - start).safe_normalized();
}

double segment_t::length() const {
    return (end - start).length();
}

double segment_t::length_squared() const {
    return (end - start).length_squared();
}

vector3 segment_t::point_at(double t) const {
    return start + (end - start) * t;
}

double segment_t::project_point_t(const vector3& p) const {
    vector3 seg = end - start;
    double len_sq = seg.length_squared();
    if (len_sq < 1e-12) return 0.0;
    return (p - start).dot(seg) / len_sq;
}

vector3 segment_t::closest_point(const vector3& p) const {
    double t = project_point_t(p);
    t = std::clamp(t, 0.0, 1.0);
    return point_at(t);
}

double segment_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool segment_t::intersects_sphere(const vector3& center, double radius) const {
    double dist_sq = (closest_point(center) - center).length_squared();
    return dist_sq <= radius * radius;
}

bool segment_t::intersects_aabb(const aabb_t& box) const {
    ray_t ray(start, (end - start).safe_normalized());
    double tmin = 0.0, tmax = 0.0;
    bool hit = box.intersect_ray(ray, tmin, tmax);

    if (!hit) return false;

    double seg_len = (end - start).length();
    return tmin <= seg_len && tmax >= 0.0;
}

ray_t segment_t::to_ray() const {
    return ray_t(start, (end - start).safe_normalized());
}

std::pair<vector3, vector3> segment_t::as_pair() const {
    return { start, end };
}

void segment_t::transform(const matrix4x4& m) {
    start = m.transform_point(start);
    end = m.transform_point(end);
}

bool segment_t::equals(const segment_t& other, double epsilon) const {
    return start.equals(other.start, epsilon) && end.equals(other.end, epsilon);
}
