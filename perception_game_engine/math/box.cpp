#include "box.h"

box_t::box_t()
    : local_bounds(vector3(-0.5), vector3(0.5)), transform(matrix4x4::identity()) {
}

box_t::box_t(const aabb_t& bounds, const matrix4x4& transform)
    : local_bounds(bounds), transform(transform) {
}

box_t box_t::from_center_extent(const vector3& center, const vector3& extent) {
    aabb_t local = aabb_t::from_center_extents(vector3::zero(), extent);
    matrix4x4 m = matrix4x4::translate(center);
    return box_t(local, m);
}

vector3 box_t::center() const {
    return transform.transform_point(local_bounds.center());
}

vector3 box_t::extent() const {
    return local_bounds.extent(); 
}

vector3 box_t::size() const {
    return local_bounds.size();
}

double box_t::volume() const {
    vector3 s = local_bounds.size();
    double scale_x = transform.transform_direction(vector3::unit_x()).length();
    double scale_y = transform.transform_direction(vector3::unit_y()).length();
    double scale_z = transform.transform_direction(vector3::unit_z()).length();
    return s.x * scale_x * s.y * scale_y * s.z * scale_z;
}

double box_t::surface_area() const {
    vector3 s = local_bounds.size();
    double scale_x = transform.transform_direction(vector3::unit_x()).length();
    double scale_y = transform.transform_direction(vector3::unit_y()).length();
    double scale_z = transform.transform_direction(vector3::unit_z()).length();

    double sx = s.x * scale_x;
    double sy = s.y * scale_y;
    double sz = s.z * scale_z;

    return 2.0 * (sx * sy + sy * sz + sz * sx);
}

aabb_t box_t::to_world_aabb() const {
    vector3 corners[8];
    get_corners(corners);
    aabb_t result;
    result.min = result.max = corners[0];
    for (int i = 1; i < 8; ++i)
        result.expand(corners[i]);
    return result;
}

void box_t::get_corners(vector3 out[8]) const {
    const vector3& min = local_bounds.min;
    const vector3& max = local_bounds.max;

    out[0] = transform.transform_point(vector3(min.x, min.y, min.z));
    out[1] = transform.transform_point(vector3(max.x, min.y, min.z));
    out[2] = transform.transform_point(vector3(min.x, max.y, min.z));
    out[3] = transform.transform_point(vector3(max.x, max.y, min.z));
    out[4] = transform.transform_point(vector3(min.x, min.y, max.z));
    out[5] = transform.transform_point(vector3(max.x, min.y, max.z));
    out[6] = transform.transform_point(vector3(min.x, max.y, max.z));
    out[7] = transform.transform_point(vector3(max.x, max.y, max.z));
}

bool box_t::contains_point(const vector3& p, double epsilon, bool inclusive) const {
    vector3 local = transform_point_to_local(p);

    if (inclusive) {
        return local.x >= (local_bounds.min.x - epsilon) && local.x <= (local_bounds.max.x + epsilon) &&
            local.y >= (local_bounds.min.y - epsilon) && local.y <= (local_bounds.max.y + epsilon) &&
            local.z >= (local_bounds.min.z - epsilon) && local.z <= (local_bounds.max.z + epsilon);
    }
    else {
        return local.x > (local_bounds.min.x + epsilon) && local.x < (local_bounds.max.x - epsilon) &&
            local.y >(local_bounds.min.y + epsilon) && local.y < (local_bounds.max.y - epsilon) &&
            local.z >(local_bounds.min.z + epsilon) && local.z < (local_bounds.max.z - epsilon);
    }
}


vector3 box_t::closest_point(const vector3& p) const {
    vector3 local = transform_point_to_local(p);
    vector3 clamped = vector3::clamp(local, local_bounds.min, local_bounds.max);
    return transform.transform_point(clamped);
}

double box_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

bool box_t::intersects_ray(const ray_t& ray, double* out_t) const {
    matrix4x4 inv = transform.inverse_affine();
    ray_t local_ray;
    local_ray.origin = inv.transform_point(ray.origin);
    local_ray.direction = inv.transform_direction(ray.direction).safe_normalized();

    double tmin = 0.0, tmax = 0.0;
    bool hit = local_bounds.intersect_ray(local_ray, tmin, tmax);

    if (hit && out_t) {
        *out_t = tmin;
    }
    return hit;
}


vector3 box_t::transform_point_to_local(const vector3& world_point) const {
    matrix4x4 inv = transform.inverse_affine();
    return inv.transform_point(world_point);
}

bool box_t::equals(const box_t& other, double epsilon) const {
    return local_bounds.equals(other.local_bounds, epsilon) &&
        transform.equals(other.transform, static_cast<float>(epsilon));
}

bool box_t::is_valid() const {
    return local_bounds.is_valid() && transform.data(); 
}