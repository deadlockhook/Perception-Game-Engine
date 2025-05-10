#pragma once
#include <cstdint>

enum draw_type_t : uint8_t {
    draw_none = 0,

    // Debug primitives (3D)
    draw_line3d,
    draw_box3d,
    draw_sphere3d,
    draw_triangle3d,
    draw_arrow3d,
    draw_grid3d,
    draw_point3d,
    draw_axis3d,
    draw_aabb,
    draw_camera_frustum,

    // Geometry
    draw_mesh,
    draw_model_handle,

    // Text
    draw_text3d,
    draw_text_screen,
    draw_text_centered,

    // 2D primitives (screen-space)
    draw_line2d,
    draw_rect2d,
    draw_circle2d,
    draw_image2d,

    draw_type_count
};
