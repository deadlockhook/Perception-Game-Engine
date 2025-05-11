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
#include "math/geom/kdop.h"


template<typename kdop_t>
static void test_from_points_and_contains() {
    vector3 points[] = {
        {1, 2, 3},
        {-1, -2, -1},
        {0, 0, 0},
        {5, 5, 5}
    };
    const size_t count = sizeof(points) / sizeof(points[0]);

    kdop_t kdop;
    kdop.from_points(points, count);
    assert(kdop.is_valid());

    for (size_t i = 0; i < count; ++i)
        assert(kdop.contains_point(points[i]));
}

template<typename kdop_t>
static void test_to_aabb_projection() {
    vector3 points[] = {
        {-2, 0, 1},
        {4, 1, 3}
    };

    kdop_t kdop;
    kdop.from_points(points, 2);

    aabb_t box = kdop.to_aabb();
    assert(box.is_valid());
    assert(box.contains_point(points[0]));
    assert(box.contains_point(points[1]));
}

template<typename kdop_t>
static void test_support_point() {
    vector3 points[] = {
        {-1, 0, 0},
        {2, 0, 0}
    };

    kdop_t kdop;
    kdop.from_points(points, 2);

    vector3 dir = vector3(1, 0, 0);
    vector3 support = kdop.get_support_point(dir);
    assert(support.x > 0.9); // Should be close to (2,0,0)

    dir = vector3(-1, 0, 0);
    support = kdop.get_support_point(dir);
    assert(support.x < -0.9); // Should be close to (-1,0,0)
}

template<typename kdop_t>
static void run_kdop_extension_tests(const char* label) {
    test_from_points_and_contains<kdop_t>();
    test_to_aabb_projection<kdop_t>();
    test_support_point<kdop_t>();
    std::cout << "[" << label << "] Extension tests passed.\n";
}

int main() {

    run_kdop_extension_tests<kdop6_t>("kdop6_t");
    run_kdop_extension_tests<kdop14_t>("kdop14_t");
    run_kdop_extension_tests<kdop26_t>("kdop26_t");
    return 0;
}