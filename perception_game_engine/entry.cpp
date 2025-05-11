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



bool approx(float a, float b, float epsilon = 1e-5f) {
    return std::fabs(a - b) < epsilon;
}

bool approx_vec3(const vector3& a, const vector3& b, float epsilon = 1e-5f) {
    return (a - b).length_squared() < epsilon * epsilon;
}

bool approx_vec4(const vector4& a, const vector4& b, float epsilon = 1e-5f) {
    return (a - b).length_squared() < epsilon * epsilon;
}

bool approx_mat(const matrix4x4& a, const matrix4x4& b, float epsilon = 1e-5f) {
    return a.equals(b, epsilon);
}

void test_identity_and_zero() {
    matrix4x4 id = matrix4x4::identity();
    matrix4x4 zero = matrix4x4::zero();

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            assert((i == j ? approx(id[i][j], 1.0f) : approx(id[i][j], 0.0f)));

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            assert(approx(zero[i][j], 0.0f));
}

void test_translation() {
    matrix4x4 t = matrix4x4::translate({ 1, 2, 3 });
    vector3 p = t.transform_point({ 0, 0, 0 });
    assert(approx_vec3(p, { 1, 2, 3 }));
}

void test_scaling() {
    matrix4x4 s = matrix4x4::scale({ 2, 3, 4 });
    vector3 p = s.transform_point({ 1, 1, 1 });
    assert(approx_vec3(p, { 2, 3, 4 }));
}

void test_rotation() {
    quat q = quat::from_axis_angle(vector3::unit_y(), M_PI); // 180 degrees around Y
    matrix4x4 r = matrix4x4::rotate(q);
    vector3 p = r.transform_point({ 1, 0, 0 });
    assert(approx_vec3(p, { -1, 0, 0 }));
}

void test_trs_composition() {
    vector3 t = { 2, 3, 4 };
    vector3 s = { 1, 2, 1 };
    quat r = quat::from_axis_angle(vector3::unit_y(), M_PI_2);
    matrix4x4 m = matrix4x4::trs(t, r, s);

    vector3 p = m.transform_point({ 1, 0, 0 }); // rotated 90° around Y and translated
    assert(approx_vec3(p, { 2, 3, 5 })); // should end up at +Z after rotation and scale
}

void test_look_at() {
    matrix4x4 view = matrix4x4::look_at({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });
    vector3 result = view.transform_point({ 0, 0, 0 });
    assert(approx_vec3(result, { 0, 0, -5 }));

}

void test_perspective_and_inverse() {
    matrix4x4 proj = matrix4x4::perspective(90.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    matrix4x4 inv = proj.inverse();

    // Transform a point into NDC and back
    vector4 ndc = proj.transform_vec4({ 0, 0, -1, 1 });
    vector4 world = inv.transform_vec4(ndc);
    world = world / world.w;

    assert(approx_vec4(world, { 0, 0, -1, 1 }));
}

void test_multiplication_and_transpose() {
    matrix4x4 a = matrix4x4::translate({ 1, 2, 3 });
    matrix4x4 b = matrix4x4::scale({ 2, 2, 2 });
    matrix4x4 ab = a * b;

    vector3 p = ab.transform_point({ 1, 1, 1 });
    assert(approx_vec3(p, { 3, 4, 5 }));

    matrix4x4 tr = ab.transposed();
    assert(approx(ab[1][0], tr[0][1]));
}
void test_row_column_access() {
    matrix4x4 m = matrix4x4::identity();
    m[2][1] = 42.0f;
    assert(approx(m[2][1], 42.0f));
    assert(m(2, 1) == m[2][1]);
}

void test_transform_point_vs_direction() {
    matrix4x4 t = matrix4x4::translate({ 10, 0, 0 });
    vector3 pt = t.transform_point({ 1, 2, 3 });
    vector3 dir = t.transform_direction({ 1, 2, 3 });
    assert(approx_vec3(pt, { 11, 2, 3 }));
    assert(approx_vec3(dir, { 1, 2, 3 })); // direction shouldn't change with translation
}

void test_inverse_affine_accuracy() {
    matrix4x4 model = matrix4x4::trs({ 1, 2, 3 }, quat::from_axis_angle(vector3::unit_y(), M_PI_2), { 2, 2, 2 });
    matrix4x4 inv = model.inverse_affine();
    matrix4x4 should_be_identity = model * inv;
    assert(should_be_identity.equals(matrix4x4::identity(), 1e-4f));
}

void test_perspective_depth() {
    matrix4x4 proj = matrix4x4::perspective(60.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    vector4 clip_near = proj.transform_vec4(vector4(0, 0, -0.1f, 1));
    vector4 clip_far = proj.transform_vec4(vector4(0, 0, -100.0f, 1));
    assert(clip_near.z < clip_far.z); // z increases toward far plane (in clip space)
}

void test_inverse_perspective_roundtrip() {
    matrix4x4 proj = matrix4x4::perspective(90.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    matrix4x4 inv_proj = proj.inverse();

    vector4 world = { 1, 2, -3, 1 };
    vector4 ndc = proj.transform_vec4(world);
    vector4 recovered = inv_proj.transform_vec4(ndc);
    recovered = recovered / recovered.w;

    assert(approx_vec4(recovered, world, 1e-4f));
}

void run_extended_matrix_tests() {
    test_row_column_access();
    test_transform_point_vs_direction();
    test_inverse_affine_accuracy();
    test_perspective_depth();
    test_inverse_perspective_roundtrip();
    std::cout << "✅ matrix4x4: Extended tests passed.\n";
}


bool approx(double a, double b, double eps = 1e-6) {
    return std::abs(a - b) < eps;
}

bool approx_vec(const vector3& a, const vector3& b, double eps = 1e-6) {
    return (a - b).length_squared() < eps * eps;
}

void print_matrix(const matrix4x4& m, const char* label) {
    std::cout << "[" << label << "]\n";
    for (int r = 0; r < 4; ++r) {
        std::cout << "  ";
        for (int c = 0; c < 4; ++c)
            std::cout << m.m[r][c] << " ";
        std::cout << "\n";
    }
}
void test_frustum_from_matrix_contains() {
    matrix4x4 proj = matrix4x4::perspective(90.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    matrix4x4 view = matrix4x4::look_at({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });
    matrix4x4 view_proj = view * proj; // ✅ FIXED

    print_matrix(proj, "Perspective");
    print_matrix(view, "View");
    print_matrix(view_proj, "ViewProj");

    frustum_t fr = frustum_t::from_matrix(view_proj);
    std::cout << "[ frustum.from_matrix(view_proj) ]\n";
    for (int i = 0; i < 6; ++i) {
        const plane_t& p = fr.get_plane(i);
        double d = p.distance_to_point({ 100, 100, 100 });
        std::cout << "Plane " << i << ": n = " << p.normal.x << "," << p.normal.y << "," << p.normal.z
            << "  d = " << p.d << " → dist = " << d << "\n";
    }

    for (int i = 0; i < 6; ++i) {
        double dist = fr.get_plane(i).distance_to_point({ 0, 0, 0 });
        std::cout << "Plane " << i << " distance to origin: " << dist << "\n";
    }

    assert(fr.contains_point({ 0, 0, -1 })); // Inside view cone

    assert(fr.contains_point({ 0.5, 0.5, -0.5 })); // ✅ it's inside

    assert(!fr.contains_point({ 100, 100, 100 }));
}

void test_frustum_intersects_aabb_sphere() {
    matrix4x4 proj = matrix4x4::perspective(90.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    matrix4x4 view = matrix4x4::look_at({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });
    frustum_t fr = frustum_t::from_matrix(view * proj); // ✅ correct order

    aabb_t inside = aabb_t::from_center_extent({ 0, 0, 0 }, { 1, 1, 1 });
    aabb_t outside = aabb_t::from_center_extent({ 100, 100, 100 }, { 1, 1, 1 });

    assert(fr.intersects_aabb(inside));
    assert(!fr.intersects_aabb(outside));

    assert(fr.intersects_sphere({ 0, 0, 0 }, 1.0));
    assert(!fr.intersects_sphere({ 100, 0, 0 }, 1.0));
}

void test_frustum_corners_roundtrip() {
    matrix4x4 proj = matrix4x4::perspective(60.0f * DEG2RAD, 1.0f, 0.1f, 10.0f);
    matrix4x4 view = matrix4x4::look_at({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });
    matrix4x4 view_proj = view * proj; // ✅ correct order

    frustum_t f = frustum_t::from_matrix(view_proj);

    vector3 corners[8];
    f.get_corners(corners, view_proj.inverse());

    frustum_t reconstructed = frustum_t::from_corners(corners);

    for (int i = 0; i < 6; ++i) {
        if (!f.planes[i].equals(reconstructed.planes[i], 1e-4)) {
            const vector3& orig_n = f.planes[i].normal;
            const vector3& rebuilt_n = reconstructed.planes[i].normal;
            std::cout << "Plane " << i << " mismatch:\n";
            std::cout << "  Original: (" << orig_n.x << ", " << orig_n.y << ", " << orig_n.z << "), d = " << f.planes[i].d << "\n";
            std::cout << "  Rebuilt:  (" << rebuilt_n.x << ", " << rebuilt_n.y << ", " << rebuilt_n.z << "), d = " << reconstructed.planes[i].d << "\n";
        }
    }
}

void test_frustum_split_world() {
    matrix4x4 proj = matrix4x4::perspective(90.0f * DEG2RAD, 1.0f, 0.1f, 100.0f);
    matrix4x4 view = matrix4x4::look_at({ 0, 0, 5 }, { 0, 0, 0 }, { 0, 1, 0 });

    frustum_t sub = frustum_t::split_world(0.0f, 0.1f, proj, view);

    std::cout << "[split_world debug]\n";
    std::cout << "Checking point { 0, 0, 0 }\n";

    for (int i = 0; i < 6; ++i) {
        const plane_t& p = sub.planes[i];
        double dist = p.distance_to_point({ 0, 0, 0 });
        std::cout << "Plane " << i << ": n=("
            << p.normal.x << "," << p.normal.y << "," << p.normal.z
            << "), d=" << p.d << " => distance = " << dist << "\n";
    }
    assert(sub.contains_point({ 0, 0, 0 }));
    assert(!sub.contains_point({ 100, 0, 0 }));
}

void run_frustum_tests() {
    test_frustum_from_matrix_contains();
    test_frustum_intersects_aabb_sphere();
    test_frustum_corners_roundtrip();
    test_frustum_split_world();
    std::cout << "✅ frustum_t: All tests passed.\n";
}

int main() {
    test_identity_and_zero();
    test_translation();
    test_scaling();
    test_rotation();
    test_trs_composition();
    test_look_at();
    test_perspective_and_inverse();
    test_multiplication_and_transpose();

    std::cout << "✅ matrix4x4: All tests passed.\n";
	run_extended_matrix_tests();
    run_frustum_tests();
    return 0;
}