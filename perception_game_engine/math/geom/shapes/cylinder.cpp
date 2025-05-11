#include "../geom.h"
#include "../../math_defs.h"

cylinder_t::cylinder_t() : a(vector3::zero()), b(vector3::zero()), radius(0.0) {}

cylinder_t::cylinder_t(const vector3& a_, const vector3& b_, double r)
    : a(a_), b(b_), radius(r) {
}

vector3 cylinder_t::center() const {
    return (a + b) * 0.5;
}

vector3 cylinder_t::axis() const {
    return (b - a).safe_normalized();
}

double cylinder_t::height() const {
    return (b - a).length();
}

double cylinder_t::volume() const {
    double h = height();
    return M_PI * radius * radius * h;
}

double cylinder_t::surface_area() const {
    double h = height();
    return 2.0 * M_PI * radius * (radius + h);
}

vector3 cylinder_t::point_at(double t) const {
    return a + (b - a) * t;
}

vector3 cylinder_t::support(const vector3& dir) const {
    vector3 axis_dir = (b - a).safe_normalized();
    double h_proj = dir.dot(axis_dir);
    vector3 cap = h_proj >= 0.0 ? b : a;

    vector3 radial_dir = dir - axis_dir * h_proj;
    if (radial_dir.length_squared() > 1e-12)
        radial_dir = radial_dir.safe_normalized();
    else
        radial_dir = vector3::zero();

    return cap + radial_dir * radius;
}

void cylinder_t::project_onto_axis(const vector3& axis, double& out_min, double& out_max) const {
    vector3 dir = axis.safe_normalized();
    double d1 = a.dot(dir);
    double d2 = b.dot(dir);
    double r_proj = radius * std::sqrt(1.0 - std::pow(axis.dot((b - a).safe_normalized()), 2.0));

    out_min = std::min(d1, d2) - r_proj;
    out_max = std::max(d1, d2) + r_proj;
}

vector3 cylinder_t::closest_point(const vector3& p) const {
    vector3 ab = b - a;
    double h2 = ab.length_squared();
    if (h2 < 1e-8) return a; 

    double t = (p - a).dot(ab) / h2;
    t = std::clamp(t, 0.0, 1.0);
    vector3 proj = a + ab * t;
    vector3 dir = (p - proj).safe_normalized();
    return proj + dir * std::min(radius, (p - proj).length());
}

double cylinder_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool cylinder_t::contains_point(const vector3& p, double epsilon) const {
    vector3 ab = b - a;
    double h2 = ab.length_squared();
    if (h2 < 1e-8) return false;

    double t = (p - a).dot(ab) / h2;
    if (t < -epsilon || t > 1.0 + epsilon) return false;

    vector3 proj = a + ab * t;
    double dist2 = (p - proj).length_squared();
    return dist2 <= (radius + epsilon) * (radius + epsilon);
}

bool cylinder_t::intersects_sphere(const vector3& center, double r) const {
    double dist2 = (closest_point(center) - center).length_squared();
    double total_r = radius + r;
    return dist2 <= total_r * total_r;
}

bool cylinder_t::intersects_cylinder(const cylinder_t& other) const {
    double r = radius + other.radius;
    segment_t s1(a, b);
    segment_t s2(other.a, other.b);
    vector3 c1 = s1.closest_point(other.a);
    vector3 c2 = s2.closest_point(c1);
    return (c1 - c2).length_squared() <= r * r;
}

bool cylinder_t::intersects_ray(const ray_t& ray, double* out_t) const {
    capsule_t cap(a, b, radius);
    return cap.intersects_ray(ray, out_t);
}

aabb_t cylinder_t::to_aabb() const {
    vector3 dir = (b - a).safe_normalized();
    vector3 up = dir.abs();
    vector3 rvec(radius);
    aabb_t box;
    box.min = vector3::min(a, b) - rvec;
    box.max = vector3::max(a, b) + rvec;
    return box;
}

void cylinder_t::transform(const matrix4x4& m) {
    a = m.transform_point(a);
    b = m.transform_point(b);

    vector3 scale_x = m.transform_direction(vector3::unit_x());
    vector3 scale_y = m.transform_direction(vector3::unit_y());
    vector3 scale_z = m.transform_direction(vector3::unit_z());
    double sx = scale_x.length();
    double sy = scale_y.length();
    double sz = scale_z.length();

    double uniform_scale = (sx + sy + sz) / 3.0;
    radius *= uniform_scale;
}

std::pair<vector3, vector3> cylinder_t::get_segment() const {
    return { a, b };
}

bool cylinder_t::equals(const cylinder_t& other, double epsilon) const {
    return a.equals(other.a, epsilon) &&
        b.equals(other.b, epsilon) &&
        std::abs(radius - other.radius) <= epsilon;
}

bool cylinder_t::is_valid() const {
    return a.is_finite() && b.is_finite() && std::isfinite(radius) && radius >= 0.0;
}

cylinder_t cylinder_t::lerp(const cylinder_t& from, const cylinder_t& to, double t) {
    return cylinder_t(
        vector3::lerp(from.a, to.a, t),
        vector3::lerp(from.b, to.b, t),
        from.radius + (to.radius - from.radius) * t
    );
}