#include "../geom.h"
#include "../../math_defs.h"

torus_t::torus_t(const vector3& c, const quat& o, double R, double r)
    : center(c), orientation(o), major_radius(R), minor_radius(r) {
}

bool torus_t::is_valid() const {
    return center.is_finite() && orientation.is_valid() &&
        major_radius > 0.0 && minor_radius > 0.0 &&
        std::isfinite(major_radius) && std::isfinite(minor_radius);
}

torus_t torus_t::transformed(const matrix4x4& m) const {
    vector3 new_center = m.transform_point(center);
    quat new_orientation = orientation; 
    vector3 scale = m.get_scale(); 

    double scale_uniform = (scale.x + scale.y + scale.z) / 3.0;
    return torus_t(new_center, new_orientation, major_radius * scale_uniform, minor_radius * scale_uniform);
}

double torus_t::signed_distance(const vector3& pt) const {
    vector3 local = orientation.inverse().rotate(pt - center);
    double xz_len = std::sqrt(local.x * local.x + local.z * local.z);
    double q = std::sqrt((xz_len - major_radius) * (xz_len - major_radius) + local.y * local.y);
    return q - minor_radius;
}

vector3 torus_t::closest_point(const vector3& pt) const {
    vector3 local = orientation.inverse().rotate(pt - center);

    double len_xz = std::sqrt(local.x * local.x + local.z * local.z);
    if (len_xz < 1e-6)
        return center + orientation.rotate(vector3(major_radius, 0, 0)); 

    double scale = major_radius / len_xz;
    vector3 circle_center_local(local.x * scale, 0, local.z * scale);
    vector3 dir_to_point = (local - circle_center_local).normalized();
    vector3 surface_local = circle_center_local + dir_to_point * minor_radius;

    return center + orientation.rotate(surface_local);
}

double torus_t::distance_to_point(const vector3& pt) const {
    return (closest_point(pt) - pt).length();
}

bool torus_t::contains_point(const vector3& pt) const {
    return std::abs(signed_distance(pt)) < 1e-6;
}

aabb_t torus_t::compute_aabb() const {
    vector3 axes[3];
    get_axes(axes[0], axes[1], axes[2]);

    double total_radius = major_radius + minor_radius;
    vector3 extent = vector3::zero();

    for (int i = 0; i < 3; ++i)
        extent += axes[i] * total_radius;

    return aabb_t::from_center_extents(center, vector3(total_radius, minor_radius, total_radius));
}

vector3 torus_t::sample_point(double u, double v) const {
    double cu = std::cos(u), su = std::sin(u);
    double cv = std::cos(v), sv = std::sin(v);

    double x = (major_radius + minor_radius * cv) * cu;
    double y = minor_radius * sv;
    double z = (major_radius + minor_radius * cv) * su;

    vector3 local(x, y, z);
    return center + orientation.rotate(local);
}

void torus_t::get_axes(vector3& out_x, vector3& out_y, vector3& out_z) const {
    out_x = orientation.rotate(vector3::unit_x());
    out_y = orientation.rotate(vector3::unit_y());
    out_z = orientation.rotate(vector3::unit_z());
}

bool torus_t::operator==(const torus_t& other) const {
    return center == other.center &&
        orientation == other.orientation &&
        std::abs(major_radius - other.major_radius) < 1e-8 &&
        std::abs(minor_radius - other.minor_radius) < 1e-8;
}

bool torus_t::operator!=(const torus_t& other) const {
    return !(*this == other);
}