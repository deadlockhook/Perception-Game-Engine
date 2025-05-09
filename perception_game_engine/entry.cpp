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

#include <chrono>
#include <iostream>

// Dummy data
struct dummy_data_t {
    int tick_counter = 0;
};

// Callbacks
class_t* on_create_test(user_data_t* data) {
    return (class_t*)new dummy_data_t();
}

class_t on_destroy_test(class_t* ptr) {
    delete reinterpret_cast<dummy_data_t*>(ptr);
    return nullptr;
}

void on_frame_test(class_t* ptr) {
    reinterpret_cast<dummy_data_t*>(ptr)->tick_counter++;
}

void run_stress_test() {
    const int entity_count = 10000;
    const int children_per_entity = 2;
    const int components_per_entity = 3;
    const int frame_iterations = 100;

    std::cout << "[test] Creating layer and entities...\n";
    auto layer_id = g_entity_mgr.create_layer("stress_layer");
    auto* layer = g_entity_mgr.get_layer(layer_id);

    auto start = std::chrono::high_resolution_clock::now();

    // Create all entities
    s_vector<entity_t*> entities;
    for (int i = 0; i < entity_count; ++i) {
        auto name = "entity_" + std::to_string(i);
        auto* e = layer->create_entity(
            name.c_str(),
            on_create_test,
            on_destroy_test,
            nullptr,
            nullptr,
            on_frame_test,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr
        );
        if (e) entities.push_back(e);
    }

    auto after_creation = std::chrono::high_resolution_clock::now();
    std::cout << "[test] Attaching children and components...\n";

    // Attach children
    for (int i = 0; i < entity_count; ++i) {
        auto* parent = entities[i];
        for (int j = 0; j < children_per_entity; ++j) {
            int child_index = (i * children_per_entity + j) % entity_count;
            if (child_index != i) {
                entities[child_index]->attach_to(parent);
            }
        }
    }

    // Add components
    for (auto* e : entities) {
        for (int j = 0; j < components_per_entity; ++j) {
            std::string cname = "component_" + std::to_string(j);
            e->add_component(
                cname.c_str(),
                on_create_test,
                on_destroy_test,
                nullptr,
                nullptr,
                on_frame_test,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            );
        }
    }

    auto after_setup = std::chrono::high_resolution_clock::now();

    std::cout << "[test] Running " << frame_iterations << " frames...\n";
    for (int i = 0; i < frame_iterations; ++i)
        g_entity_mgr.on_frame();

    auto after_frame = std::chrono::high_resolution_clock::now();

    std::cout << "[test] Destroying everything...\n";
    g_entity_mgr.destroy();

    auto after_destroy = std::chrono::high_resolution_clock::now();

    auto ms = [](auto a, auto b) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(b - a).count();
        };

    std::cout << "[result] Creation time: " << ms(start, after_creation) << " ms\n";
    std::cout << "[result] Setup (attach + components): " << ms(after_creation, after_setup) << " ms\n";
    std::cout << "[result] Frame time total: " << ms(after_setup, after_frame) << " ms\n";
    std::cout << "[result] Destroy time: " << ms(after_frame, after_destroy) << " ms\n";
}

int main()
{
    run_stress_test();
//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



