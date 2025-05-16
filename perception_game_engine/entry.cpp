#include "platform/platform.h"
#include "engine/setup.h"
#include "crt/s_vector.h"

#include <chrono>

#include <cassert>
#include <iostream>
#include "crt/s_string_pool.h"
#include <random>
#include "engine/hwnd/window_manager.h"

#include <iostream>
#include <cassert>

void s_vector_stress_test() {
    std::cout << "Running s_vector<s_string> stress test...\n";

    s_vector<s_string> vec;

    // Massive push_back to force reallocs
    for (int i = 0; i < 10000; ++i) {
        vec.push_back(s_string(("Test String " + std::to_string(i)).c_str()));
        assert(vec.back() == ("Test String " + std::to_string(i)).c_str());
    }

    // Verify all elements are correct
    for (int i = 0; i < vec.count(); ++i) {
        std::string expected = "Test String " + std::to_string(i);
        assert(vec[i] == expected.c_str());
    }

    // Shrink to fit test
    size_t original_capacity = vec.capacity();
    vec.shrink_to_fit();
    assert(vec.capacity() == vec.count());
    std::cout << "Shrink to fit reduced capacity from " << original_capacity << " to " << vec.capacity() << "\n";


    // Erase a few elements and test compaction
    for (int i = 0; i < 100; ++i) {
        vec.erase_at(0);
    }
    std::cout << "After erasing 100 elements, new count is " << vec.count() << "\n";

    // Pop_back stress test
    while (!vec.empty()) {
        vec.pop_back();
    }
    assert(vec.count() == 0);

    // Reserve explicit test
    vec.reserve(5000);
    assert(vec.capacity() >= 5000);
    std::cout << "Reserve to 5000 successful.\n";
 
    // Emplace back test with temporary strings
    for (int i = 0; i < 1000; ++i) {
        vec.emplace_back(s_string(("Emplaced " + std::to_string(i)).c_str()));
    }
  
    for (int i = 0; i < vec.count(); ++i) {
        std::string expected = "Emplaced " + std::to_string(i);
        assert(vec[i] == expected.c_str());
    }
   
    std::cout << "s_vector<s_string> stress test PASSED ✅\n";
}

int main() {
   // s_vector_stress_test();
   engine_execute();
    return 0;
}