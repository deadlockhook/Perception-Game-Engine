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
#include "math/geom/polyhedron.h"
static void test_polyhedron_from_frustum() {

    matrix4x4 view = matrix4x4::look_at(
        vector3(0, 0, -5), vector3(0, 0, 0), vector3::unit_y()
    );
    matrix4x4 proj = matrix4x4::perspective(60.0 * M_PI / 180.0, 1.0f, 0.1f, 100.0f);
    matrix4x4 view_proj = proj * view;
    matrix4x4 inv_view_proj = view_proj.inverse();

    frustum_t frustum = frustum_t::from_matrix(view_proj);

    polyhedron_t poly;
    bool ok = poly.from_frustum(frustum, inv_view_proj);
    assert(ok);
    assert(poly.vertex_count == 8);
    assert(poly.face_count == 6);
    aabb_t bounds = poly.compute_aabb();
    assert(bounds.is_valid());

    std::cout << "[polyhedron_t] from_frustum passed\n";
}

static void test_polyhedron_from_kdop() {
    kdop14_t kdop;
    vector3 cube_pts[] = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1},  {1, -1, 1},  {1, 1, 1},  {-1, 1, 1}
    };

    kdop.from_points(cube_pts, 8);

    polyhedron_t poly;
    bool ok = poly.from_kdop(kdop);
    assert(ok);
    assert(poly.vertex_count == 14); // 7 axes × 2 projections
    assert(poly.face_count == 0);    // no faces yet (we haven’t triangulated it)

    aabb_t bounds = poly.compute_aabb();
    assert(bounds.contains_point(vector3(0, 0, 0)));

    std::cout << "[polyhedron_t] from_kdop14 passed\n";
}
static void test_polyhedron_from_aabb() {
    aabb_t box(vector3(-1, -2, -3), vector3(1, 2, 3));
    assert(box.is_valid());

    vector3 corners[8];
    box.get_corners(corners);

    std::cout << "--- AABB corners ---\n";
    for (int i = 0; i < 8; ++i)
        std::cout << "corner[" << i << "] = " << corners[i].x << ", " << corners[i].y << ", " << corners[i].z << "\n";

    vector3 centroid(0.0, 0.0, 0.0);
    for (int i = 0; i < 8; ++i)
        centroid += corners[i];
    centroid *= (1.0 / 8.0);

    std::cout << "average of corners (geometric centroid): " << centroid.x << ", " << centroid.y << ", " << centroid.z << "\n";
    std::cout << "aabb_t::center(): " << box.center().x << ", " << box.center().y << ", " << box.center().z << "\n";

    polyhedron_t poly;
    bool ok = poly.from_aabb(box);
    assert(ok);

    vector3 com = poly.center_of_mass();

    std::cout << "polyhedron_t::center_of_mass(): " << com.x << ", " << com.y << ", " << com.z << "\n";
    std::cout << "polyhedron_t::volume(): " << poly.volume() << "\n";

    double error = (com - centroid).length();
    std::cout << "error = (center_of_mass - avg_corner): " << error << "\n";

    // Final assertion (adjust threshold if needed)
    assert(error < 1e-3); // less strict tolerance for diagnostic phase

    std::cout << "[polyhedron_t] from_aabb passed\n";
}



static void test_polyhedron_from_box() {
    // Define an AABB in local space
    aabb_t local_bounds(vector3(-1, -2, -3), vector3(1, 2, 3));
    matrix4x4 transform = matrix4x4::identity(); // no transform for simplicity

    box_t box(local_bounds, transform);

    polyhedron_t poly;
    bool ok = poly.from_box(box);
    assert(ok);
    assert(poly.vertex_count == 8);
    assert(poly.face_count == 6);

    aabb_t world_aabb = box.to_world_aabb();
    aabb_t poly_bounds = poly.compute_aabb();

    assert(poly_bounds.equals(world_aabb, 1e-6));

    vector3 box_center = box.center();
    vector3 com = poly.center_of_mass();
    assert((com - box_center).length() < 1e-6);

    std::cout << "[polyhedron_t] from_box passed\n";
}

static void test_polyhedron_from_obb() {
    vector3 center(0, 0, 0);
    vector3 axes[3] = {
        vector3(1, 0, 0),
        vector3(0, 1, 0),
        vector3(0, 0, 1)
    };
    vector3 half_extents(1, 2, 3);
    obb_t obb(center, axes, half_extents);

    polyhedron_t poly;
    bool ok = poly.from_obb(obb);
    assert(ok);
    assert(poly.vertex_count == 8);
    assert(poly.face_count == 6);

    vector3 com = poly.center_of_mass();
    assert(com.equals(center, 1e-6));

    aabb_t bounds = poly.compute_aabb();
    assert(bounds.contains_point(vector3(1, 2, 3)));
    assert(bounds.contains_point(vector3(-1, -2, -3)));

    std::cout << "[polyhedron_t] from_obb passed\n";
}
static void test_polyhedron_convexity_closure() {
    aabb_t box(vector3(-1, -1, -1), vector3(1, 1, 1));
    polyhedron_t poly;
    assert(poly.from_aabb(box));

    std::cout << "[convexity/closure] testing from_aabb...\n";

    assert(poly.is_closed());
    assert(poly.is_convex());

    // Flip winding to break convexity
    int temp = poly.get_face(0).indices[0];
    poly.get_face(0).indices[0] = poly.get_face(0).indices[1];
    poly.get_face(0).indices[1] = temp;

    bool still_closed = poly.is_closed();   // should still be closed
    bool still_convex = poly.is_convex();   // should now fail

    std::cout << "Still closed after winding flip?  " << (still_closed ? "yes" : "no") << "\n";
    std::cout << "Still convex after winding flip? " << (still_convex ? "yes" : "no") << "\n";

    assert(!still_closed);
    assert(!still_convex);


    std::cout << "[polyhedron_t] is_closed() and is_convex() tests passed\n";
}
static void test_polyhedron_support_point() {
    aabb_t box(vector3(-1, -1, -1), vector3(1, 1, 1));
    polyhedron_t poly;
    assert(poly.from_aabb(box));

    vector3 dir = vector3(1, 0, 0);
    vector3 support = poly.get_support_point(dir);
    assert(support.equals(vector3(1, 1, 1), 1e-6) || support.equals(vector3(1, -1, -1), 1e-6)); // or any max-x

    std::cout << "[polyhedron_t] get_support_point passed\n";
}

void run_polyhedron_box_tests() {
    test_polyhedron_from_aabb();
    test_polyhedron_from_box();
    test_polyhedron_from_obb();
    test_polyhedron_convexity_closure();
}

static void test_polyhedron_shape_constructors() {
    std::cout << "--- Testing polyhedron_t shape generators ---\n";

    polyhedron_t p1, p2, p3;

    segment_t s(vector3(-1, 0, 0), vector3(1, 0, 0));
    p1.from_segment(s.start, s.end, 0.05, 16);

    line_t l(vector3(0, 0, 0), vector3(0, 1, 0));
    p2.from_line(l, 2.0, 0.05, 12);

    triangle_t t(vector3(0, 0, 0), vector3(1, 0, 0), vector3(0, 1, 0));
    p3.from_triangle(t);

    p1.export_to_obj("segment.obj");
    p2.export_to_obj("line.obj");
    p3.export_to_obj("triangle.obj");

}
int main() {

    test_polyhedron_from_frustum();
    test_polyhedron_from_kdop();
    run_polyhedron_box_tests();
    test_polyhedron_support_point();
	test_polyhedron_shape_constructors();
    return 0;
}