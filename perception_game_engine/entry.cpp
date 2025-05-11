#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <sstream>
#include "crt/safe_crt.h"
#include "definitions.h"
#include "exception/exception_handler.h"
#include "crt/s_vector.h"
#include "serialize/fnva_hash.h"
#include "engine/entity_system/entity_system.h"


#include <cstdio>
#include "engine/threading/thread_storage.h"
#include <chrono>
#include <random>

#include "crt/s_string_pool.h"
#include <cassert>
#include "crt/s_node_list.h"
#include "math/pixel_vector2.h"
#include "math/vector2.h"
#include "math/math_defs.h"
#include "math/vector4.h"
#include "math/aabb.h"
#include "math/ray.h"
#include "math/frustum.h"
#include "math/obb.h"
#include "math/geom/geom.h"
#include "math/box.h"
#include <cassert>
#include <cmath>
#include <iostream>


bool approx(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) <= epsilon;
}

bool approx_vec(const vector3& a, const vector3& b, double epsilon = 1e-6) {
    return (a - b).length_squared() <= epsilon * epsilon;
}

void test_geometry() {
    disc_t d(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);
    assert(approx(d.area(), M_PI * 4.0));
    assert(d.is_valid());
}

void test_containment_and_distance() {
    disc_t d(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);

    assert(d.contains_point(vector3(0, 0, 0)));
    assert(d.contains_point(vector3(1, 0, 1)));
    assert(!d.contains_point(vector3(3, 0, 0)));
    assert(!d.contains_point(vector3(0, 1, 0))); // off-plane

    vector3 p = vector3(3, 0, 0);
    vector3 cp = d.closest_point(p);
    assert(approx_vec(cp, vector3(2, 0, 0)));
    assert(approx(d.distance_to_point(p), 1.0));
}

void test_ray_and_sphere_intersections() {
    disc_t d(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);

    ray_t hit_ray(vector3(0, 5, 0), vector3(0, -1, 0));
    ray_t miss_ray(vector3(5, 5, 0), vector3(0, -1, 0));
    double t = -1;

    assert(d.intersects_ray(hit_ray, &t));
    assert(t > 0);
    assert(!d.intersects_ray(miss_ray));

    assert(d.intersects_sphere(vector3(0, 0.1, 0), 0.2));
    assert(!d.intersects_sphere(vector3(10, 0, 0), 0.5));
}

void test_aabb_and_transform() {
    disc_t d(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);
    aabb_t box = d.to_aabb();

    assert(box.contains_point(vector3(1, 0, 1)));
    assert(box.contains_point(vector3(-2, 0, -2)));

    matrix4x4 m = matrix4x4::translate(vector3(5, 0, 0));
    d.transform(m);

    assert(approx_vec(d.center, vector3(5, 0, 0)));
    assert(d.radius > 1.5);
    assert(d.normal.is_normalized());
}

void test_support_and_projection() {
    disc_t d(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);

    vector3 s = d.support(vector3(1, 0, 0));
    assert(approx_vec(s, vector3(2, 0, 0)));

    double min, max;
    d.project_onto_axis(vector3(1, 0, 0), min, max);
    assert(approx(min, -2.0));
    assert(approx(max, 2.0));

    d.project_onto_axis(vector3(0, 1, 0), min, max);
    assert(approx(min, 0.0));
    assert(approx(max, 0.0)); // flat projection
}

void test_equals_and_lerp() {
    disc_t a(vector3(0, 0, 0), vector3(0, 1, 0), 2.0);
    disc_t b = a;
    disc_t c(vector3(0, 0, 0), vector3(0.01, 0.999, 0), 2.01);

    assert(a.equals(b));
    assert(!a.equals(c, 1e-6));
    assert(a.equals(c, 1.5e-2));

    disc_t mid = disc_t::lerp(a, c, 0.5);
    assert(approx(mid.radius, 2.005));
    assert(mid.normal.is_normalized());
    assert(approx_vec(mid.center, vector3(0, 0, 0)));
}

int main() {
    test_geometry();
    test_containment_and_distance();
    test_ray_and_sphere_intersections();
    test_aabb_and_transform();
    test_support_and_projection();
    test_equals_and_lerp();

    std::cout << "✅ disc_t: All tests passed.\n";
    return 0;
}
