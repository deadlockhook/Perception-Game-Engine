#include "../geom.h"
#include "../../math_defs.h"

disc_t::disc_t()
    : center(vector3::zero()), normal(vector3::unit_y()), radius(0.0) {
}

disc_t::disc_t(const vector3& c, const vector3& n, double r)
    : center(c), normal(n.safe_normalized()), radius(r) {
}

double disc_t::area() const {
    return M_PI * radius * radius;
}

bool disc_t::contains_point(const vector3& p, double epsilon) const {
    vector3 to_p = p - center;
    double dist = std::abs(to_p.dot(normal));
    if (dist > epsilon) return false;

    double r2 = (to_p - normal * dist).length_squared();
    return r2 <= (radius + epsilon) * (radius + epsilon);
}

vector3 disc_t::closest_point(const vector3& p) const {
    vector3 to_p = p - center;
    double h = to_p.dot(normal);
    vector3 proj = p - normal * h;

    vector3 offset = proj - center;
    double d2 = offset.length_squared();

    if (d2 <= radius * radius)
        return proj;

    return center + offset.safe_normalized() * radius;
}

double disc_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool disc_t::intersects_sphere(const vector3& sphere_center, double r) const {
    return distance_to_point(sphere_center) <= r;
}

bool disc_t::intersects_ray(const ray_t& ray, double* out_t) const {
    double denom = normal.dot(ray.direction);
    if (std::abs(denom) < 1e-8) return false;

    double t = normal.dot(center - ray.origin) / denom;
    if (t < 0.0) return false;

    vector3 hit = ray.origin + ray.direction * t;
    if ((hit - center).length_squared() > radius * radius) return false;

    if (out_t) *out_t = t;
    return true;
}

aabb_t disc_t::to_aabb() const {
    vector3 up = normal.safe_normalized();
    vector3 right = up.cross(std::abs(up.dot(vector3::unit_x())) < 0.99 ? vector3::unit_x() : vector3::unit_y()).safe_normalized();
    vector3 forward = up.cross(right);

    vector3 corners[4] = {
        center + right * radius,
        center - right * radius,
        center + forward * radius,
        center - forward * radius
    };

    aabb_t box;
    box.min = box.max = center;
    for (int i = 0; i < 4; ++i)
        box.expand(corners[i]);

    return box;
}


vector3 disc_t::support(const vector3& dir) const {
    vector3 plane_dir = dir - normal * dir.dot(normal);
    return center + plane_dir.safe_normalized() * radius;
}

void disc_t::project_onto_axis(const vector3& axis, double& out_min, double& out_max) const {
    vector3 dir = axis.safe_normalized();
    double center_proj = center.dot(dir);

    double radial_proj = radius * std::sqrt(1.0 - std::pow(normal.dot(dir), 2.0));
    out_min = center_proj - radial_proj;
    out_max = center_proj + radial_proj;
}

void disc_t::transform(const matrix4x4& m) {
    center = m.transform_point(center);
    normal = m.transform_direction(normal).safe_normalized();

    double sx = m.transform_direction(vector3::unit_x()).length();
    double sy = m.transform_direction(vector3::unit_y()).length();
    double sz = m.transform_direction(vector3::unit_z()).length();
    double uniform = (sx + sy + sz) / 3.0;
    radius *= uniform;
}

bool disc_t::equals(const disc_t& other, double epsilon) const {
    return center.equals(other.center, epsilon) &&
        normal.equals(other.normal, epsilon) &&
        std::abs(radius - other.radius) <= epsilon;
}

bool disc_t::is_valid() const {
    return center.is_finite() && normal.is_normalized() &&
        std::isfinite(radius) && radius >= 0.0;
}

disc_t disc_t::lerp(const disc_t& a, const disc_t& b, double t) {
    return disc_t(
        vector3::lerp(a.center, b.center, t),
        vector3::lerp(a.normal, b.normal, t).safe_normalized(),
        a.radius + (b.radius - a.radius) * t
    );
}
