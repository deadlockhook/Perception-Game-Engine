#pragma once
#include "common.h"
#include "crt/safe_crt.h"

class exception_handler {
public:
    using fn_exception_handler_callback = bool(*)(EXCEPTION_POINTERS* exception_info);
    using fn_unhandled_exception_handler_callback = bool(*)(EXCEPTION_POINTERS* exception_info);
public:
    exception_handler();
    ~exception_handler();

    void disable();
    void enable();

     bool check_exception_handles();

    void add_exception_handler(fn_exception_handler_callback callback);
    void add_unhandled_exception_handler(fn_unhandled_exception_handler_callback callback);
    void remove_exception_handler(fn_exception_handler_callback callback);
    void remove_unhandled_exception_handler(fn_unhandled_exception_handler_callback callback);

    void on_exception(const char* str);
    void write_to_exception_file(bool clear = true);
    void print_exceptions_on_console(bool clear = true);

    template <typename func_t, typename... args_t>
    __declspec(noinline) auto seh_call(func_t&& func, args_t&&... args) -> decltype(func(args...)) {
        using return_type = decltype(func(args...));
        __try {
            return func(std::forward<args_t>(args)...);
        }
        __except (seh_exception_filter(GetExceptionInformation())) {
            if constexpr (!std::is_void_v<return_type>) {
                return return_type();
            }
        }
    }

private:
    static LONG WINAPI vectored_exception_handler(EXCEPTION_POINTERS* exception_info);
    static LONG WINAPI unhandled_exception_filter(EXCEPTION_POINTERS* exception_info);
    static LONG WINAPI seh_exception_filter(EXCEPTION_POINTERS* exception_info);
private:
    s_vector<s_string> exception_stack;
    s_vector<fn_exception_handler_callback> exception_callbacks;
    s_vector<fn_unhandled_exception_handler_callback> unhandled_exception_callbacks;
    s_string exception_storage_file_path;
    interlocked_mutex mtx;
    void* veh_handle = nullptr;
private:
    struct handlers_validity_test {
        shared_variable<bool> caught_by_veh;
    } handlers_validity_test;
};

extern exception_handler g_exception_handler;

