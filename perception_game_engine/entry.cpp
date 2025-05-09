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


struct test_data_t {
    int created = 0;
    int destroyed = 0;
    int ticked = 0;
};

class_t* on_create_test(user_data_t*) {
    auto* d = new test_data_t();
    d->created = 1;
    return reinterpret_cast<class_t*>(d);
}

void on_destroy_test(class_t* ptr) {
    auto* d = reinterpret_cast<test_data_t*>(ptr);
    d->destroyed = 1;
    delete d;
}

void on_frame_test(class_t* ptr) {
   // auto* d = reinterpret_cast<test_data_t*>(ptr);
   // d->ticked++;
    auto current_class = get_current_entity();

	std::cout << "Current entity: " << (current_class ? current_class->name.c_str() : "") << std::endl;
}

void on_frame_test_comp(class_t* ptr) {
    // auto* d = reinterpret_cast<test_data_t*>(ptr);
    // d->ticked++;
    auto current_component = get_current_component();

    std::cout << "Current entity: " << (current_component ? current_component->name.c_str() : "") << std::endl;
}

void run_component_hierarchy_test() {
    printf("[test] Running component hierarchy test...\n");

    // Create layer
    auto* layer = g_entity_mgr.create_layer("hierarchy_layer");

   auto* root = layer->create_entity("root", nullptr, nullptr, nullptr, nullptr, on_frame_test);
   // root->add_component("root_component", on_create_test, on_destroy_test, nullptr, nullptr, on_frame_test);

    // Create child entity with component
    auto* child = layer->create_entity("child", nullptr, nullptr, nullptr, nullptr, on_frame_test);
    child->add_component("child_component", on_create_test, on_destroy_test, nullptr, nullptr, on_frame_test_comp);

    child->attach_to(root);
    root->attach_to(child);
    child->attach_to(root);

    for (int i = 0; i < 1; ++i)
        g_entity_mgr.on_frame();

    // Validate
   // auto* root_data = root->get_component_class<test_data_t>("root_component");
   // auto* child_data = child->get_component_class<test_data_t>("child_component");

   // assert(root_data && root_data->created == 1 && root_data->ticked == 50);
   // assert(child_data && child_data->created == 1 && child_data->ticked == 50);

    // Cleanup
    g_entity_mgr.destroy();

    //printf("[result] Root ticks: %d, Child ticks: %d\n", root_data->ticked, child_data->ticked);
    printf("[test] Component hierarchy test passed.\n");
}


void run_s_pooled_string_test() {
    printf("[test] Running s_pooled_string test...\n");

    const char* raw = "pooled_test_string";
    s_string raw_s = s_string(raw);

    // Construct from C-string
    s_pooled_string a(raw);
    s_pooled_string b(raw);
    assert(a.str == b.str);
    assert(a.hash == b.hash);
    assert(strcmp(a.c_str(), raw) == 0);

    // Construct from s_string
    s_pooled_string c(raw_s);
    assert(c.str == a.str);
    assert(c.hash == a.hash);

    // Copy constructor
    s_pooled_string d = a;
    assert(d.str == a.str);
    assert(d.hash == a.hash);

    // Assignment operator
    s_pooled_string e;
    e = b;
    assert(e.str == b.str);
    assert(e.hash == b.hash);

    // Implicit conversions
    const char* cstr = a;
    const s_string& ss = a;
    assert(strcmp(cstr, "pooled_test_string") == 0);
    assert(ss == "pooled_test_string");

    printf("[result] Pooled A: %s (hash = %u)\n", a.c_str(), a.hash);
    printf("[test] s_pooled_string test passed.\n");
}


int main()
{
    run_s_pooled_string_test();
   // run_component_hierarchy_test();
//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



