#include "../geom.h"
#include "../../math_defs.h"

ellipsoid_t::ellipsoid_t(const vector3& c, const vector3& r, const quat& o)
    : center(c), radii(r), orientation(o) {
}

bool ellipsoid_t::is_valid() const {
    return center.is_finite() && radii.is_finite() && orientation.is_valid() &&
        radii.x > 0 && radii.y > 0 && radii.z > 0;
}

bool ellipsoid_t::is_sphere(double epsilon) const {
    return std::abs(radii.x - radii.y) < epsilon &&
        std::abs(radii.y - radii.z) < epsilon;
}

ellipsoid_t ellipsoid_t::transformed(const matrix4x4& m) const {
    vector3 new_center = m.transform_point(center);
    vector3 new_scale = m.get_scale();
    vector3 scaled_radii = vector3(
        radii.x * new_scale.x,
        radii.y * new_scale.y,
        radii.z * new_scale.z
    );
    return ellipsoid_t(new_center, scaled_radii, orientation); 
}

ellipsoid_t ellipsoid_t::normalized() const {
    return ellipsoid_t(center, vector3(1, 1, 1), orientation);
}

double ellipsoid_t::signed_distance(const vector3& pt) const {
    vector3 local = orientation.inverse().rotate(pt - center);
    vector3 q = local / radii;
    return q.length() - 1.0;
}

vector3 ellipsoid_t::closest_point(const vector3& pt) const {
    vector3 local = orientation.inverse().rotate(pt - center);
    vector3 q = local / radii;

    if (q.length_squared() < 1e-12)
        q = vector3(1, 0, 0);

    vector3 closest_local = q.normalized() * radii;
    return center + orientation.rotate(closest_local);
}

double ellipsoid_t::distance_to_point(const vector3& pt) const {
    return (closest_point(pt) - pt).length();
}

bool ellipsoid_t::contains_point(const vector3& pt) const {
    return signed_distance(pt) <= 0.0;
}

vector3 ellipsoid_t::sample_point(double u, double v) const {
    double cu = std::cos(u), su = std::sin(u);
    double cv = std::cos(v), sv = std::sin(v);

    vector3 local = {
        radii.x * cu * sv,
        radii.y * cv,
        radii.z * su * sv
    };
    return center + orientation.rotate(local);
}

aabb_t ellipsoid_t::compute_aabb() const {
    vector3 axes[3];
    get_axes(axes[0], axes[1], axes[2]);

    vector3 extents =
        axes[0].abs() * radii.x +
        axes[1].abs() * radii.y +
        axes[2].abs() * radii.z;

    return aabb_t::from_center_extents(center, extents);
}

void ellipsoid_t::get_axes(vector3& out_x, vector3& out_y, vector3& out_z) const {
    out_x = orientation.rotate(vector3::unit_x());
    out_y = orientation.rotate(vector3::unit_y());
    out_z = orientation.rotate(vector3::unit_z());
}

bool ellipsoid_t::intersects_ray(const ray_t& ray, double& out_t) const {
    vector3 ro = orientation.inverse().rotate(ray.origin - center) / radii;
    vector3 rd = orientation.inverse().rotate(ray.direction) / radii;

    double a = rd.dot(rd);
    double b = 2.0 * ro.dot(rd);
    double c = ro.dot(ro) - 1.0;

    double discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) return false;

    double sqrt_d = std::sqrt(discriminant);
    double t0 = (-b - sqrt_d) / (2.0 * a);
    double t1 = (-b + sqrt_d) / (2.0 * a);

    if (t1 < 0.0) return false;

    out_t = t0 >= 0.0 ? t0 : t1;
    return true;
}

bool ellipsoid_t::operator==(const ellipsoid_t& rhs) const {
    return center == rhs.center && radii == rhs.radii && orientation == rhs.orientation;
}

bool ellipsoid_t::operator!=(const ellipsoid_t& rhs) const {
    return !(*this == rhs);
}