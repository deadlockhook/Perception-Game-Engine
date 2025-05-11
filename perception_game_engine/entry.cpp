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
    return std::abs(a - b) < epsilon;
}

bool approx_vec(const vector3& a, const vector3& b, double epsilon = 1e-6) {
    return (a - b).length_squared() < epsilon * epsilon;
}

void test_distance_and_projection() {
    plane_t p = plane_t::from_point_normal(vector3(0, 0, 0), vector3(0, 1, 0));
    assert(approx(p.distance_to_point(vector3(0, 5, 0)), 5.0));
    assert(approx(p.distance_to_point(vector3(0, -1, 0)), -1.0));

    vector3 proj = p.project_point(vector3(0, 5, 0));
    assert(approx_vec(proj, vector3(0, 0, 0)));
}

void test_ray_and_segment_intersection() {
    plane_t p = plane_t::from_point_normal(vector3(0, 0, 0), vector3(0, 1, 0));
    ray_t r(vector3(0, 5, 0), vector3(0, -1, 0));
    double t = 0.0;
    assert(p.intersects_ray(r, &t));
    assert(approx(t, 5.0));

    vector3 hit;
    assert(p.intersects_segment(vector3(0, 1, 0), vector3(0, -1, 0), &hit));
    assert(approx_vec(hit, vector3(0, 0, 0)));

    assert(!p.intersects_segment(vector3(0, 1, 0), vector3(0, 0.5, 0)));
}

void test_side_classification() {
    plane_t p = plane_t::from_point_normal(vector3(0, 0, 0), vector3(0, 1, 0));

    assert(p.classify_point(vector3(0, 1, 0)) == plane_t::side_t::front);
    assert(p.classify_point(vector3(0, -1, 0)) == plane_t::side_t::back);
    assert(p.classify_point(vector3(0, 0, 0)) == plane_t::side_t::intersecting);

    vector3 tri1[3] = { {1, 1, 0}, {0, 1, 0}, {-1, 1, 0} };
    vector3 tri2[3] = { {1, -1, 0}, {0, -1, 0}, {-1, -1, 0} };
    vector3 tri3[3] = { {-1, -1, 0}, {1, 1, 0}, {0, -1, 0} };

    assert(p.classify_triangle(tri1) == plane_t::side_t::front);
    assert(p.classify_triangle(tri2) == plane_t::side_t::back);
    assert(p.classify_triangle(tri3) == plane_t::side_t::intersecting);
}

void test_triangle_clipping() {
    plane_t p = plane_t::from_point_normal(vector3(0, 0, 0), vector3(0, 1, 0));

    vector3 tri[3] = { {-1, -1, 0}, {1, -1, 0}, {0, 1, 0} };
    vector3 out[4];

    int count = p.clip_triangle(tri, out);
    assert(count == 3);

    for (int i = 0; i < count; ++i)
        assert(p.is_in_front(out[i]));
}

void test_equals_and_flipped() {
    plane_t a = plane_t::from_point_normal(vector3(0, 0, 0), vector3(0, 1, 0));
    plane_t b = a.normalized();
    plane_t c = a.flipped().flipped();

    assert(a.equals(b));
    assert(a.equals(c));
}

int main() {
    test_distance_and_projection();
    test_ray_and_segment_intersection();
    test_side_classification();
    test_triangle_clipping();
    test_equals_and_flipped();
    std::cout << "✅ disc_t: All tests passed.\n";
    return 0;
}
