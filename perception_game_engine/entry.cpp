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
#include "math/transform.h"


int main() {

    quat angles = quat::from_euler(vector3(0,90,0));  // Pitch, Yaw, Roll in radians
    std::cout << "Quaternion: " << angles.x << ", " << angles.y << ", " << angles.z << ", " << angles.w << std::endl;

    vector3 angles_vec = angles.to_euler().to_degrees();  // Back to Euler angles in radians
    std::cout << "Euler angles (radians): " << angles_vec.x << ", " << angles_vec.y << ", " << angles_vec.z << std::endl;


    return 0;
}