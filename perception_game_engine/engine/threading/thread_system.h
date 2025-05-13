#pragma once
#include "../../platform/platform.h"

#include "../../platform/platform.h"

struct thread_t
{
    void* handle = nullptr;
    uint32_t thread_id = 0;

    void close()
    {
        if (handle)
        {
            p_close_handle(handle);
            handle = nullptr;
            thread_id = 0;
        }
    }

    bool valid() const
    {
        return handle != nullptr;
    }

    void join()
    {
        if (handle)
        {
            p_wait_for_single_object(handle);
        }
    }

    void terminate_and_close(uint32_t exit_code = 0)
    {
        if (handle)
        {
            p_terminate_thread(handle, exit_code);
            p_close_handle(handle);
            handle = nullptr;
            thread_id = 0;
        }
    }

    void terminate(uint32_t exit_code = 0)
    {
        if (handle)
        {
            p_terminate_thread(handle, exit_code);
        }
    }

    void detach()
    {
        close();
    }

    template <typename Function, typename Param>
    void create(Function thread_func, Param param, uint32_t creation_flags = 0)
    {
        if (handle)
        {
            terminate_and_close();
        }

        handle = p_create_thread(reinterpret_cast<void* (*)(void*)>(thread_func), reinterpret_cast<void*>(param), creation_flags, &thread_id);
    }

    ~thread_t()
    {
        close();
    }
};
