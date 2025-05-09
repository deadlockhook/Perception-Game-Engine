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
#include <random>

#include "crt/s_string_pool.h"
#include <cstdio>
#include <chrono>


struct test_class {
    int counter = 0;
};

class_t* on_create_test(user_data_t*) {
    return (class_t*)new test_class();
}

class_t on_destroy_test(class_t* ptr) {
    delete (test_class*)ptr;
    return nullptr;
}

void on_frame_test(class_t* ptr) {
    ((test_class*)ptr)->counter++;
}

void run_stress_test_with_pooled_names() {
    const int entity_count = 50000;
    const int frame_count = 100;
    const int name_repeat = 10;

    s_string name_variants[name_repeat];
    for (int i = 0; i < name_repeat; ++i) {
        char buf[32];
        sprintf_s(buf, "entity_%d", i);
        name_variants[i] = s_string(buf);
    }

    printf("[test] Interning %d strings...\n", entity_count);
    auto intern_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < entity_count; ++i) {
        intern_string(name_variants[i % name_repeat]);
    }

    auto intern_end = std::chrono::high_resolution_clock::now();
    auto intern_time = std::chrono::duration_cast<std::chrono::milliseconds>(intern_end - intern_start).count();
    printf("[test] Interning done in %lld ms\n", intern_time);
    printf("[test] Final pool size: %zu\n", g_string_pool.size());

    // Now do entity creation
    uint32_t layer_id = g_entity_mgr.create_layer("test");
    auto* layer = g_entity_mgr.get_layer(layer_id);

    printf("[test] Creating %d entities...\n", entity_count);
    auto create_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < entity_count; ++i) {
        layer->create_entity(
            1, name_variants[i % name_repeat],
            on_create_test,
            on_destroy_test,
            nullptr, nullptr,
            on_frame_test,
            nullptr, nullptr,
            nullptr, nullptr,
            nullptr, nullptr,
            nullptr
        );
    }

    auto create_end = std::chrono::high_resolution_clock::now();
    auto create_time = std::chrono::duration_cast<std::chrono::milliseconds>(create_end - create_start).count();
    printf("[test] Creation time: %lld ms\n", create_time);

    // Frame test
    printf("[test] Running %d frames...\n", frame_count);
    auto frame_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < frame_count; ++i) {
        g_entity_mgr.on_frame();
    }

    auto frame_end = std::chrono::high_resolution_clock::now();
    auto frame_time = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start).count();
    printf("[test] Total frame time: %lld ms (avg: %.2f ms/frame)\n", frame_time, frame_time / (float)frame_count);

    // Cleanup
    printf("[test] Destroying all entities...\n");
    auto destroy_start = std::chrono::high_resolution_clock::now();
    g_entity_mgr.destroy();
    auto destroy_end = std::chrono::high_resolution_clock::now();
    auto destroy_time = std::chrono::duration_cast<std::chrono::milliseconds>(destroy_end - destroy_start).count();
    printf("[test] Destruction time: %lld ms\n", destroy_time);
}


int main()
{

	std::cout << "non paged 1kb page " << v_mem::allocate_pool(non_paged_pool, 0x400).get_size() << std::endl;
    run_stress_test_with_pooled_names();
//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



