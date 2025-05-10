#pragma once
#include <Windows.h>
#include "../../crt/s_node_list.h"
#include "../../crt/atomic.h"
#include "../../crt/s_string.h"

/*
enum window_flags_t : uint32_t {
    window_flag_none = 0,
    window_flag_borderless = 1 << 0,
    window_flag_resizable = 1 << 1,
    window_flag_always_on_top = 1 << 2,
    window_flag_centered = 1 << 3,
    window_flag_hidden = 1 << 4,
    window_flag_tool_window = 1 << 5, 
};

class window_instance_t {
public:
    ~window_instance_t(); //use for destruction
    bool create(const wchar_t* title, int width, int height, uint32_t flags = window_flag_none);
    void destroy();

    HWND hwnd() const { return _hwnd; }
    HINSTANCE hinstance() const { return _hinstance; }

    bool process_messages(); 
    bool is_open() const { return _is_open; }
private:
    int _width = 0, _height = 0;
    s_string _title;
private:
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam);

    HWND _hwnd = nullptr;
    HINSTANCE _hinstance = nullptr;
    bool _is_open = false;
};

class window_manager_t {
public:
    window_manager_t() = default;

    window_instance_t* create_window(const wchar_t* title, int width, int height, uint32_t flags = window_flag_none);
    void destroy_window(window_instance_t* win);

    s_node_list_t<window_instance_t>& windows() { return _windows; }


private:
    critical_section manager_lock;
    s_node_list_t<window_instance_t> _windows;
};
*/
//extern window_manager_t g_window_manager;