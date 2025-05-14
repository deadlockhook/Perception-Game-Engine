#pragma once
#include <cstdint>
#include <windows.h>
#include <atomic>
#include "atomic_datatype.h"

#ifdef _WIN64
#pragma comment(lib, "winmm.lib")
#endif

inline void p_sleep(uint32_t ms)
{
#ifdef _WIN64
    Sleep(ms);
#else
   
#endif
}

inline uint64_t p_query_counter()
{
#ifdef _WIN64
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return static_cast<uint64_t>(counter.QuadPart);
#else
    
    return 0;
#endif
}

inline uint64_t p_query_frequency()
{
#ifdef _WIN64
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return static_cast<uint64_t>(frequency.QuadPart);
#else

    return 1000000000; 
#endif
}

inline double p_time_since(uint64_t start)
{
#ifdef _WIN64
    uint64_t now = p_query_counter();
    uint64_t frequency = p_query_frequency();
    return (now - start) * 1000.0 / static_cast<double>(frequency);
#else
  
    return 0.0;
#endif
}

inline double p_current_time_ms()
{
#ifdef _WIN64
    uint64_t now = p_query_counter();
    uint64_t frequency = p_query_frequency();
    return now * 1000.0 / static_cast<double>(frequency);
#else

    return 0.0;
#endif
}

inline void* p_create_thread(void* (*thread_func)(void*), void* param, uint32_t creation_flags, uint32_t* out_thread_id)
{
#ifdef _WIN64
    DWORD tid = 0;
    HANDLE handle = CreateThread(
        nullptr,
        0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(thread_func),
        reinterpret_cast<LPVOID>(param),
        creation_flags,
        &tid
    );
    if (out_thread_id)
        *out_thread_id = tid;
    return handle;
#else
    return nullptr;
#endif
}

inline void p_close_handle(void* handle)
{
#ifdef _WIN64
    if (handle)
    {
        CloseHandle(handle);
    }
#endif
}

inline void p_wait_for_single_object(void* handle)
{
#ifdef _WIN64
    if (handle)
    {
        WaitForSingleObject(handle, INFINITE);
    }
#endif
}

inline void p_terminate_thread(void* handle, uint32_t exit_code)
{
#ifdef _WIN64
    if (handle)
    {
        TerminateThread(handle, exit_code);
    }
#endif
}

inline void p_set_thread_priority_high()
{
#ifdef _WIN64
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif
}

inline void p_set_thread_priority_time_critical()
{
#ifdef _WIN64
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
}

inline void p_set_thread_priority_normal()
{
#ifdef _WIN64
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
#endif
}

inline void p_set_thread_affinity(uint32_t core_id)
{
#ifdef _WIN64
    if (core_id < sizeof(DWORD_PTR) * 8)
    {
        DWORD_PTR affinity_mask = 1ull << core_id;
        SetThreadAffinityMask(GetCurrentThread(), affinity_mask);
    }
#endif
}

inline void p_set_timer_resolution(uint32_t ms)
{
#ifdef _WIN64
    timeBeginPeriod(ms);
#endif
}

inline void p_reset_timer_resolution(uint32_t ms)
{
#ifdef _WIN64
    timeEndPeriod(ms);
#endif
}

