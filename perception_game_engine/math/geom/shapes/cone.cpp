#include "../geom.h"
#include "../../math_defs.h"

cone_t::cone_t()
    : apex(vector3::zero()), direction(vector3::unit_y()), angle_rad(0.0), height(0.0) {
}

cone_t::cone_t(const vector3& a, const vector3& dir, double angle, double h)
    : apex(a), direction(dir.safe_normalized()), angle_rad(angle), height(h) {
}

vector3 cone_t::base_center() const {
    return apex + direction * height;
}

double cone_t::radius() const {
    return std::tan(angle_rad) * height;
}

double cone_t::volume() const {
    double r = radius();
    return (1.0 / 3.0) * M_PI * r * r * height;
}

double cone_t::surface_area() const {
    double r = radius();
    double slant = std::sqrt(r * r + height * height);
    return M_PI * r * (r + slant);
}

bool cone_t::contains_point(const vector3& p, double epsilon) const {
    vector3 v = p - apex;
    double len = v.length();
    if (len < epsilon) return true;

    double h = v.dot(direction);
    if (h < -epsilon || h > height + epsilon) return false;

    double max_angle_cos = std::cos(angle_rad);
    double actual_cos = v.safe_normalized().dot(direction);
    return actual_cos >= max_angle_cos - epsilon;
}

vector3 cone_t::closest_point(const vector3& p) const {
    vector3 ac = p - apex;
    double h = std::clamp(ac.dot(direction), 0.0, height);
    vector3 axis_point = apex + direction * h;
    vector3 radial = (p - axis_point).safe_normalized() * std::min((p - axis_point).length(), std::tan(angle_rad) * h);
    return axis_point + radial;
}

double cone_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool cone_t::intersects_sphere(const vector3& center, double r) const {
    double dist_sq = (closest_point(center) - center).length_squared();
    return dist_sq <= r * r;
}

bool cone_t::intersects_ray(const ray_t& ray, double* out_t) const {
    vector3 base = base_center();
    double r = radius();
    capsule_t cap(apex, base, r);
    return cap.intersects_ray(ray, out_t);
}

aabb_t cone_t::to_aabb() const {
    vector3 base = base_center();
    double r = radius();

    vector3 up = direction.safe_normalized();
    vector3 right = vector3::unit_x();
    if (std::abs(up.dot(right)) > 0.99)
        right = vector3::unit_y();
    vector3 ortho1 = up.cross(right).safe_normalized();
    vector3 ortho2 = up.cross(ortho1);

    vector3 rvec = ortho1 * r + ortho2 * r;
    aabb_t box;
    box.min = vector3::min(apex, base - rvec);
    box.max = vector3::max(apex, base + rvec);
    return box;
}

void cone_t::transform(const matrix4x4& m) {
    apex = m.transform_point(apex);
    vector3 base = base_center();
    base = m.transform_point(base);

    direction = (base - apex).safe_normalized();
    height = (base - apex).length();

    double scale = (
        m.transform_direction(vector3::unit_x()).length() +
        m.transform_direction(vector3::unit_y()).length() +
        m.transform_direction(vector3::unit_z()).length()) / 3.0;

    angle_rad = std::atan(std::tan(angle_rad) * scale); 
}

bool cone_t::equals(const cone_t& other, double epsilon) const {
    return apex.equals(other.apex, epsilon) &&
        direction.equals(other.direction, epsilon) &&
        std::abs(angle_rad - other.angle_rad) <= epsilon &&
        std::abs(height - other.height) <= epsilon;
}

bool cone_t::is_valid() const {
    return apex.is_finite() && direction.is_finite() && std::isfinite(angle_rad) &&
        std::isfinite(height) && height >= 0.0 && angle_rad >= 0.0 && direction.is_normalized();
}

cone_t cone_t::lerp(const cone_t& a, const cone_t& b, double t) {
    return cone_t(
        vector3::lerp(a.apex, b.apex, t),
        vector3::lerp(a.direction, b.direction, t).safe_normalized(),
        a.angle_rad + (b.angle_rad - a.angle_rad) * t,
        a.height + (b.height - a.height) * t
    );
}
