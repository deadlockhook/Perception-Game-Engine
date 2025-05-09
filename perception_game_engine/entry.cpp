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


int main()
{
	g_string_pool.intern("enemy");
    uint32_t hash = fnv1a32("enemy");
    const s_string* result = g_string_pool.get_name_by_hash(hash);
    if (result)
        printf("Found string: %s\n", result->c_str());
    else
        printf("String not found in pool.\n");


//	protect_region = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    system("pause");
	return 0;
}



