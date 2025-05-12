#pragma once
#include "kdop.h"

template<>
const std::array<vector3, 3>& kdop_t<3>::get_axes() {
    static const std::array<vector3, 3> axes = {
        vector3::unit_x(),
        vector3::unit_y(),
        vector3::unit_z()
    };
    return axes;
}

template<>
const std::array<vector3, 7>& kdop_t<7>::get_axes() {
    static const std::array<vector3, 7> axes = {
        vector3::unit_x(),
        vector3::unit_y(),
        vector3::unit_z(),

        (vector3(1, 1, 0)).normalized(),   
        (vector3(1, -1, 0)).normalized(),
        (vector3(1, 0, 1)).normalized(),   
        (vector3(1, 0, -1)).normalized(), 
    };
    return axes;
}

template<>
const std::array<vector3, 13>& kdop_t<13>::get_axes() {
    static const std::array<vector3, 13> axes = {
        vector3::unit_x(),
        vector3::unit_y(),
        vector3::unit_z(),

        (vector3(1, 1, 0)).normalized(),
        (vector3(1, -1, 0)).normalized(),
        (vector3(0, 1, 1)).normalized(),
        (vector3(0, 1, -1)).normalized(),
        (vector3(1, 0, 1)).normalized(),
        (vector3(1, 0, -1)).normalized(),

        (vector3(1, 1, 1)).normalized(),
        (vector3(-1, 1, 1)).normalized(),
        (vector3(1, -1, 1)).normalized(),
        (vector3(1, 1, -1)).normalized()
    };
    return axes;
}

