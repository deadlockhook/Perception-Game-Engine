#include "obb.h"
#include "math_defs.h"

obb_t::obb_t()
    : center(vector3::zero()), half_extents(vector3::one()) {
    axes[0] = vector3::unit_x();
    axes[1] = vector3::unit_y();
    axes[2] = vector3::unit_z();
}

obb_t::obb_t(const vector3& c, const vector3 axes_[3], const vector3& he)
    : center(c), half_extents(he) {
    axes[0] = axes_[0].normalized();
    axes[1] = axes_[1].normalized();
    axes[2] = axes_[2].normalized();
}

obb_t::obb_t(const vector3& c, const quat& q, const vector3& he)
    : center(c), half_extents(he) {
    axes[0] = q * vector3::unit_x();
    axes[1] = q * vector3::unit_y();
    axes[2] = q * vector3::unit_z();
}

void obb_t::transform(const matrix4x4& m) {
    center = m.transform_point(center);
    for (int i = 0; i < 3; ++i)
        axes[i] = m.transform_direction(axes[i]).normalized();
}

bool obb_t::contains_point(const vector3& p) const {
    vector3 d = p - center;
    for (int i = 0; i < 3; ++i) {
        double dist = d.dot(axes[i]);
        if (std::abs(dist) > half_extents[i])
            return false;
    }
    return true;
}

aabb_t obb_t::to_aabb() const {
    vector3 extent = vector3::zero();
    for (int i = 0; i < 3; ++i)
        extent += axes[i].abs() * half_extents[i];
    return aabb_t::from_center_extents(center, extent);
}

bool obb_t::intersects(const obb_t& b) const {
    constexpr double EPSILON = 1e-8;
    double ra, rb;
    double R[3][3], AbsR[3][3];

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            R[i][j] = axes[i].dot(b.axes[j]);
            AbsR[i][j] = std::abs(R[i][j]) + EPSILON; 
        }

    vector3 t = b.center - center;
    vector3 T(t.dot(axes[0]), t.dot(axes[1]), t.dot(axes[2]));

    for (int i = 0; i < 3; ++i) {
        ra = half_extents[i];
        rb = b.half_extents[0] * AbsR[i][0] + b.half_extents[1] * AbsR[i][1] + b.half_extents[2] * AbsR[i][2];
        if (std::abs(T[i]) > ra + rb)
            return false;
    }

    for (int i = 0; i < 3; ++i) {
        ra = half_extents[0] * AbsR[0][i] + half_extents[1] * AbsR[1][i] + half_extents[2] * AbsR[2][i];
        rb = b.half_extents[i];
        double t_proj = std::abs(T[0] * R[0][i] + T[1] * R[1][i] + T[2] * R[2][i]);
        if (t_proj > ra + rb)
            return false;
    }

    ra = half_extents[1] * AbsR[2][0] + half_extents[2] * AbsR[1][0];
    rb = b.half_extents[1] * AbsR[0][2] + b.half_extents[2] * AbsR[0][1];
    if (std::abs(T[2] * R[1][0] - T[1] * R[2][0]) > ra + rb) return false;

    ra = half_extents[1] * AbsR[2][1] + half_extents[2] * AbsR[1][1];
    rb = b.half_extents[0] * AbsR[0][2] + b.half_extents[2] * AbsR[0][0];
    if (std::abs(T[2] * R[1][1] - T[1] * R[2][1]) > ra + rb) return false;

    ra = half_extents[1] * AbsR[2][2] + half_extents[2] * AbsR[1][2];
    rb = b.half_extents[0] * AbsR[0][1] + b.half_extents[1] * AbsR[0][0];
    if (std::abs(T[2] * R[1][2] - T[1] * R[2][2]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[2][0] + half_extents[2] * AbsR[0][0];
    rb = b.half_extents[1] * AbsR[1][2] + b.half_extents[2] * AbsR[1][1];
    if (std::abs(T[0] * R[2][0] - T[2] * R[0][0]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[2][1] + half_extents[2] * AbsR[0][1];
    rb = b.half_extents[0] * AbsR[1][2] + b.half_extents[2] * AbsR[1][0];
    if (std::abs(T[0] * R[2][1] - T[2] * R[0][1]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[2][2] + half_extents[2] * AbsR[0][2];
    rb = b.half_extents[0] * AbsR[1][1] + b.half_extents[1] * AbsR[1][0];
    if (std::abs(T[0] * R[2][2] - T[2] * R[0][2]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[1][0] + half_extents[1] * AbsR[0][0];
    rb = b.half_extents[1] * AbsR[2][2] + b.half_extents[2] * AbsR[2][1];
    if (std::abs(T[1] * R[0][0] - T[0] * R[1][0]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[1][1] + half_extents[1] * AbsR[0][1];
    rb = b.half_extents[0] * AbsR[2][2] + b.half_extents[2] * AbsR[2][0];
    if (std::abs(T[1] * R[0][1] - T[0] * R[1][1]) > ra + rb) return false;

    ra = half_extents[0] * AbsR[1][2] + half_extents[1] * AbsR[0][2];
    rb = b.half_extents[0] * AbsR[2][1] + b.half_extents[1] * AbsR[2][0];
    if (std::abs(T[1] * R[0][2] - T[0] * R[1][2]) > ra + rb) return false;

    return true; 
}


bool obb_t::intersects_ray(const ray_t& ray, double* out_t) const {
    vector3 p = ray.origin - center;
    vector3 p_local, d_local;

    for (int i = 0; i < 3; ++i) {
        d_local[i] = ray.direction.dot(axes[i]);
        p_local[i] = p.dot(axes[i]);
    }

    double tmin = -std::numeric_limits<double>::infinity();
    double tmax = std::numeric_limits<double>::infinity();

    for (int i = 0; i < 3; ++i) {
        if (std::abs(d_local[i]) < 1e-12) {
            if (std::abs(p_local[i]) > half_extents[i])
                return false;
        }
        else {
            double ood = 1.0 / d_local[i];
            double t1 = (-half_extents[i] - p_local[i]) * ood;
            double t2 = (half_extents[i] - p_local[i]) * ood;
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax)
                return false;
        }
    }

    if (out_t) *out_t = tmin;
    return true;
}

void obb_t::get_corners(vector3 out[8]) const {
    const vector3 x = axes[0] * half_extents.x;
    const vector3 y = axes[1] * half_extents.y;
    const vector3 z = axes[2] * half_extents.z;

    out[0] = center - x - y - z;
    out[1] = center + x - y - z;
    out[2] = center - x + y - z;
    out[3] = center + x + y - z;
    out[4] = center - x - y + z;
    out[5] = center + x - y + z;
    out[6] = center - x + y + z;
    out[7] = center + x + y + z;
}

vector3 obb_t::closest_point(const vector3& p) const {
    vector3 d = p - center;
    vector3 q = center;
    for (int i = 0; i < 3; ++i) {
        double dist = d.dot(axes[i]);
        dist = clamp(dist, -half_extents[i], half_extents[i]);
        q += axes[i] * dist;
    }
    return q;
}

double obb_t::distance_to_point(const vector3& p) const {
    return (closest_point(p) - p).length();
}

double obb_t::volume() const {
    return 8.0 * half_extents.x * half_extents.y * half_extents.z;
}

double obb_t::surface_area() const {
    double x = half_extents.x * 2.0;
    double y = half_extents.y * 2.0;
    double z = half_extents.z * 2.0;
    return 2.0 * (x * y + y * z + z * x);
}

vector3 obb_t::support(const vector3& dir) const {
    vector3 result = center;
    for (int i = 0; i < 3; ++i) {
        double sign = (dir.dot(axes[i]) >= 0.0) ? 1.0 : -1.0;
        result += axes[i] * (half_extents[i] * sign);
    }
    return result;
}

inline bool obb_t::equals(const obb_t& rhs, double epsilon) const {
    return center.equals(rhs.center, epsilon) &&
        axes[0].equals(rhs.axes[0], epsilon) &&
        axes[1].equals(rhs.axes[1], epsilon) &&
        axes[2].equals(rhs.axes[2], epsilon) &&
        half_extents.equals(rhs.half_extents, epsilon);
}

bool obb_t::operator==(const obb_t& rhs) const {
    return center == rhs.center &&
        axes[0] == rhs.axes[0] &&
        axes[1] == rhs.axes[1] &&
        axes[2] == rhs.axes[2] &&
        half_extents == rhs.half_extents;
}

bool obb_t::is_valid(double epsilon) const {
    return half_extents.x >= 0.0 && half_extents.y >= 0.0 && half_extents.z >= 0.0 &&
        axes[0].is_normalized(epsilon) &&
        axes[1].is_normalized(epsilon) &&
        axes[2].is_normalized(epsilon);
}

void obb_t::project_onto_axis(const vector3& axis, double& out_min, double& out_max) const {
    double center_proj = center.dot(axis);
    double radius = 0.0;
    for (int i = 0; i < 3; ++i)
        radius += std::abs(axis.dot(axes[i])) * half_extents[i];

    out_min = center_proj - radius;
    out_max = center_proj + radius;
}

obb_t obb_t::from_points(const vector3* points, int count) {
    if (!points || count == 0)
        return obb_t(); 

    vector3 center_sum = vector3::zero();
    for (int i = 0; i < count; ++i)
        center_sum += points[i];
    vector3 center = center_sum / static_cast<double>(count);

    vector3 local_min(std::numeric_limits<double>::infinity());
    vector3 local_max(-std::numeric_limits<double>::infinity());

    for (int i = 0; i < count; ++i) {
        vector3 p = points[i] - center;
        local_min.x = std::min(local_min.x, p.x);
        local_min.y = std::min(local_min.y, p.y);
        local_min.z = std::min(local_min.z, p.z);
        local_max.x = std::max(local_max.x, p.x);
        local_max.y = std::max(local_max.y, p.y);
        local_max.z = std::max(local_max.z, p.z);
    }

    vector3 half_extents = (local_max - local_min) * 0.5;
    vector3 box_center_local = (local_min + local_max) * 0.5;

    vector3 final_center = center + box_center_local;

    vector3 axes[3] = {
        vector3::unit_x(),
        vector3::unit_y(),
        vector3::unit_z()
    };

    return obb_t(final_center, axes, half_extents);
}
