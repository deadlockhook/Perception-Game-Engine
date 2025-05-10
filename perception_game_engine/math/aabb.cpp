#include "aabb.h"
#include "ray.h"    

bool aabb_t::intersect_ray(const ray_t& ray, double& tmin, double& tmax) const {
    return ray.intersects_aabb(*this, tmin, tmax);
}