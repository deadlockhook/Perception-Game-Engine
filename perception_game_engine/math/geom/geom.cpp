#include "geom.h"

void geometry_collection_t::update_bounds()
{
	bounds = compute_aabb();
}

aabb_t geometry_collection_t::compute_aabb() const {
    aabb_t result;
    bool initialized = false;

    auto merge_if_valid = [&](const aabb_t& box) {
        if (!box.is_valid()) return;
        if (!initialized) {
            result = box;
            initialized = true;
        }
        else {
            result += box;
        }
        };

    for (const auto& c : capsules)   merge_if_valid(c.to_aabb());
    for (const auto& s : spheres)    merge_if_valid(s.to_aabb());
    for (const auto& e : ellipsoids) merge_if_valid(e.compute_aabb());
    for (const auto& t : triangles)  merge_if_valid(aabb_t::from_points(t.a, t.b, t.c));
    for (const auto& s : segments)   merge_if_valid(aabb_t(s.start, s.end));
    for (const auto& c : cylinders)  merge_if_valid(c.to_aabb());
    for (const auto& c : cones)      merge_if_valid(c.to_aabb());
    for (const auto& d : discs)      merge_if_valid(d.to_aabb());
    for (const auto& l : lines)      merge_if_valid(aabb_t(l.point, l.point));
    for (const auto& t : torii)      merge_if_valid(t.compute_aabb());

    return initialized ? result : aabb_t();
}


