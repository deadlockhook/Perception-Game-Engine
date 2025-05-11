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

bool approx(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) <= epsilon;
}

bool approx_vec(const vector3& a, const vector3& b, double epsilon = 1e-6) {
    return (a - b).length_squared() <= epsilon * epsilon;
}

void test_geometry() {
    triangle_t tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));

    assert(approx_vec(tri.normal(), vector3(0, 0, 1)));
    assert(approx_vec(tri.centroid(), vector3(1.0 / 3.0, 1.0 / 3.0, 0)));
    assert(approx(tri.area(), 0.5));
    assert(approx(tri.perimeter(), 1 + std::sqrt(2) + 1));
    assert(!tri.is_degenerate());
}

void test_barycentric_conversion() {
    triangle_t tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));

    double u, v, w;
    vector3 point(0.25, 0.25, 0);
    tri.barycentric_coords(point, u, v, w);
    assert(approx(u + v + w, 1.0));
    vector3 back = tri.from_barycentric(u, v, w);
    assert(approx_vec(back, point));
}

void test_closest_point_and_distance() {
    triangle_t tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));

    vector3 p(0.5, 0.5, 1);
    vector3 closest = tri.closest_point(p);
    assert(approx_vec(closest, vector3(0.5, 0.5, 0)));

    double dist = tri.distance_to_point(p);
    assert(approx(dist, 1.0));
}

void test_ray_intersection() {
    triangle_t tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));
    ray_t ray(vector3(0.25, 0.25, -1), vector3(0, 0, 1));
    double t;
    assert(tri.intersects_ray(ray, &t));
    assert(t > 0);

    ray_t miss(vector3(2, 2, -1), vector3(0, 0, 1));
    assert(!tri.intersects_ray(miss));
}

void test_accessors_and_flipping() {
    triangle_t tri(vector3(1, 2, 3), vector3(4, 5, 6), vector3(7, 8, 9));

    assert(approx_vec(tri.get_vertex(0), tri.a));
    assert(approx_vec(tri.get_vertex(1), tri.b));
    assert(approx_vec(tri.get_vertex(2), tri.c));

    assert(approx_vec(tri.get_edge(0), tri.b - tri.a));
    assert(approx_vec(tri.get_edge(1), tri.c - tri.b));
    assert(approx_vec(tri.get_edge(2), tri.a - tri.c));

    triangle_t flipped = tri.flipped();
    assert(approx_vec(flipped.a, tri.a));
    assert(approx_vec(flipped.b, tri.c));
    assert(approx_vec(flipped.c, tri.b));
}

void test_facing_and_equality() {
    triangle_t tri(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));

    assert(tri.is_front_facing(vector3(0, 0, 1)) == false);
    assert(tri.is_front_facing(vector3(0, 0, -1)) == true);

    triangle_t same = tri;
    triangle_t slightly_diff(vector3(0.000001, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));
    assert(tri.equals(same));
    assert(tri.equals(slightly_diff, 1e-4));
    assert(!tri.equals(slightly_diff, 1e-8));
}

int main() {
    test_geometry();
    test_barycentric_conversion();
    test_closest_point_and_distance();
    test_ray_intersection();
    test_accessors_and_flipping();
    test_facing_and_equality();


    std::cout << "tests complete\n";

   // run_component_hierarchy_test();
//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



