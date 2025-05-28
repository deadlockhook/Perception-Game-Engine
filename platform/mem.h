#pragma once


#ifdef PLATFORM_WINDOWS

#include <Windows.h>

void* virtual_alloc_commit(void* address, size_t size) {
    return VirtualAlloc(address, size, MEM_COMMIT, PAGE_READWRITE);
}

void* virtual_alloc_reserve(void* address, size_t size) {
    return VirtualAlloc(address, size, MEM_RESERVE, PAGE_NOACCESS);
}

void virtual_free_release(void* address) { VirtualFree(address, 0, MEM_RELEASE); }

BOOL decomit(void* address, size_t size) { return VirtualFree(address, size, MEM_DECOMMIT); }

#endif