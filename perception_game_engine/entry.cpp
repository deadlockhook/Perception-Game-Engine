#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <sstream>
#include "crt/safe_crt.h"
#include "definitions.h"
#include "exception_handler.h"
#include "crt/s_vector.h"
#include "serialize/fnva_hash.h"
#include "engine/entity_system/entity_system.h"


#include <cstdio>
#include "engine/threading/thread_storage.h"
#include <chrono>

// Test struct
struct test_class {
    int counter = 0;
};

// Callbacks
class_t* on_create_test(user_data_t*) {
    return (class_t*)new test_class();
}

class_t on_destroy_test(class_t* ptr) {
    delete (test_class*)ptr;
    return nullptr;
}

void on_frame_test(class_t* ptr) {
    auto* obj = (test_class*)ptr;
    obj->counter++;
}

// Stress test
void run_entity_stress_test() {
    constexpr int total_entities = 50000;
    auto layer_id = g_entity_mgr.create_layer("stress_layer");
    auto* layer = g_entity_mgr.get_layer(layer_id);

    printf("[test] Creating %d entities...\n", total_entities);
    auto start_create = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < total_entities; ++i) {
        char name[32];
        sprintf_s(name, "entity_%d", i);
        layer->create_entity(i, name, on_create_test, on_destroy_test, nullptr, nullptr, on_frame_test);
    }

    auto end_create = std::chrono::high_resolution_clock::now();
    auto create_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_create - start_create).count();
    printf("[test] Creation time: %lld ms\n", create_duration);

    // Run frame N times
    constexpr int frame_iterations = 100;
    printf("[test] Running %d frames...\n", frame_iterations);

    auto start_frame = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < frame_iterations; ++i) {
        g_entity_mgr.on_frame();
    }
    auto end_frame = std::chrono::high_resolution_clock::now();
    auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_frame - start_frame).count();

    printf("[test] Total frame time: %lld ms (avg: %.2f ms/frame)\n", frame_duration, frame_duration / (float)frame_iterations);

    // Cleanup
    g_entity_mgr.destroy();
}



int main()
{

	std::cout << "non paged 1kb page " << v_mem::allocate_pool(non_paged_pool, 0x400).get_size() << std::endl;
    run_entity_stress_test();
//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



