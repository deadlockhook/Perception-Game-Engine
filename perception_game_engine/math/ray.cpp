#include "ray.h"
#include "aabb.h"

ray_t::ray_t(const vector3& origin, const vector3& direction)
    : origin(origin), direction(direction.normalized()) {
}

vector3 ray_t::point_at(double t) const {
    return origin + direction * t;
}

bool ray_t::intersects_aabb(const aabb_t& box, double& out_tmin, double& out_tmax) const {
    out_tmin = 0.0;
    out_tmax = std::numeric_limits<double>::max();

    for (int i = 0; i < 3; ++i) {
        double inv_dir = 1.0 / direction[i];
        double t0 = (box.min[i] - origin[i]) * inv_dir;
        double t1 = (box.max[i] - origin[i]) * inv_dir;

        if (inv_dir < 0.0)
            std::swap(t0, t1);

        out_tmin = std::max(out_tmin, t0);
        out_tmax = std::min(out_tmax, t1);

        if (out_tmax < out_tmin)
            return false; 
    }
    return true;
}

