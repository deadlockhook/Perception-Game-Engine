#include "../geom.h"
#include "../../math_defs.h"

sphere_t::sphere_t()
    : center(vector3::zero()), radius(0.0) {
}

sphere_t::sphere_t(const vector3& c, double r)
    : center(c), radius(r) {
}

bool sphere_t::is_valid() const {
    return center.is_finite() && std::isfinite(radius) && radius >= 0.0;
}

double sphere_t::volume() const {
    return (4.0 / 3.0) * M_PI * radius * radius * radius;
}

double sphere_t::surface_area() const {
    return 4.0 * M_PI * radius * radius;
}

bool sphere_t::contains_point(const vector3& point, double epsilon) const {
    return (point - center).length_squared() <= (radius + epsilon) * (radius + epsilon);
}

bool sphere_t::contains_sphere(const sphere_t& other, double epsilon) const {
    double dist = (other.center - center).length();
    return (dist + other.radius) <= (radius + epsilon);
}

double sphere_t::distance_to_point(const vector3& point) const {
    return (point - center).length() - radius;
}

vector3 sphere_t::closest_point(const vector3& point) const {
    vector3 dir = (point - center).safe_normalized();
    return center + dir * radius;
}

bool sphere_t::intersects_sphere(const sphere_t& other) const {
    double r_sum = radius + other.radius;
    return (center - other.center).length_squared() <= r_sum * r_sum;
}

bool sphere_t::intersects_aabb(const aabb_t& box) const {
    vector3 closest = vector3::clamp(center, box.min, box.max);
    double dist_sq = (closest - center).length_squared();
    return dist_sq <= radius * radius;
}

bool sphere_t::intersects_ray(const ray_t& ray, double* out_t) const {
    vector3 m = ray.origin - center;
    double b = m.dot(ray.direction);
    double c = m.length_squared() - radius * radius;

    if (c > 0.0 && b > 0.0)
        return false;

    double discriminant = b * b - c;
    if (discriminant < 0.0)
        return false;

    double t = -b - std::sqrt(discriminant);
    if (out_t) *out_t = t;
    return true;
}

aabb_t sphere_t::to_aabb() const {
    vector3 rvec(radius);
    return aabb_t(center - rvec, center + rvec);
}

void sphere_t::transform(const matrix4x4& m) {
    center = m.transform_point(center);

    double sx = m.transform_direction(vector3::unit_x()).length();
    double sy = m.transform_direction(vector3::unit_y()).length();
    double sz = m.transform_direction(vector3::unit_z()).length();
    double scale = (sx + sy + sz) / 3.0;

    radius *= scale;
}

void sphere_t::expand(double delta) {
    radius += delta;
    if (radius < 0.0)
        radius = 0.0;
}

bool sphere_t::equals(const sphere_t& other, double epsilon) const {
    return center.equals(other.center, epsilon) && std::abs(radius - other.radius) <= epsilon;
}

sphere_t sphere_t::merge(const sphere_t& a, const sphere_t& b) {
    vector3 d = b.center - a.center;
    double dist = d.length();

    if (a.radius >= dist + b.radius) return a;
    if (b.radius >= dist + a.radius) return b;

    double new_radius = (dist + a.radius + b.radius) * 0.5;
    vector3 dir = d.safe_normalized();
    vector3 new_center = a.center;
    if (dist > 1e-8)
        new_center = a.center + dir * (new_radius - a.radius);
    return sphere_t(new_center, new_radius);
}


sphere_t sphere_t::from_aabb(const aabb_t& box) {
    vector3 center = box.center();
    double radius = 0.5 * box.size().length();
    return sphere_t(center, radius);
}
