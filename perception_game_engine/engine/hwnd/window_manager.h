#pragma once
#include <Windows.h>
#include <shellscalingapi.h> 
#pragma comment(lib, "Shcore.lib")
#include "../../crt/atomic.h"
#include "../../crt/s_string.h"
#include "../../crt/s_performance_vector.h"
#include "../input_system/input_system.h"
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")


enum window_flags_t : uint32_t {
    window_flag_none = 0,
    window_flag_borderless = 1 << 0,
    window_flag_resizable = 1 << 1,
    window_flag_always_on_top = 1 << 2,
    window_flag_centered = 1 << 3,
    window_flag_hidden = 1 << 4,
    window_flag_tool_window = 1 << 5,
    window_flag_overlay = 1 << 6,         
    window_flag_input_passthrough = 1 << 7 
};

class window_instance_t {
public:
    ~window_instance_t() { destroy(); }

    static void ensure_class_registered(HINSTANCE hinstance) {
        static atomic_bool_t class_registered;
        
        if (p_get_atomic(class_registered))
            return;

        WNDCLASSEXA wc = { sizeof(WNDCLASSEXA) };
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = window_proc;
        wc.hInstance = hinstance;
        wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
        wc.lpszClassName = "PerceptionWindowClass";

        RegisterClassExA(&wc);

		p_set_atomic(class_registered, true);
    }

    void get_client_size(int& out_width, int& out_height) const {
        RECT rect;
        GetClientRect(_hwnd, &rect);
        out_width = rect.right - rect.left;
        out_height = rect.bottom - rect.top;
    }

    bool create(const char* title, int width, int height, uint32_t flags = window_flag_none);

    void destroy() {
        if (_hwnd) {
            DestroyWindow(_hwnd);
            _hwnd = nullptr;
            _is_open = false;
        }
    }

    bool process_messages();
    void set_fullscreen(bool exclusive);
    void set_resolution(int width, int height);
    void set_resolution_full_size();
    void set_fullscreen_windowed();
    void center_window();
    void register_for_raw_input();


    void minimize() { ShowWindow(_hwnd, SW_MINIMIZE); }
    void maximize() { ShowWindow(_hwnd, SW_MAXIMIZE); }
    void restore() { ShowWindow(_hwnd, SW_RESTORE); }
    void focus() { SetForegroundWindow(_hwnd); }
    bool is_open() const { return _is_open; }
    HWND hwnd() const { return _hwnd; }
    HINSTANCE hinstance() const { return _hinstance; }
	input_system& get_input_system() { return input_system; }

private:
	input_system input_system;
    int _width = 0, _height = 0;
    s_string _title;

    HWND _hwnd = nullptr;
    HINSTANCE _hinstance = nullptr;
    bool _is_open = false;
    
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
       
        window_instance_t* self = reinterpret_cast<window_instance_t*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));

        if (self)
            return self->handle_message(msg, wparam, lparam);

        return DefWindowProcA(hwnd, msg, wparam, lparam);
    }

    LRESULT handle_message(UINT msg, WPARAM wparam, LPARAM lparam) {
        switch (msg) {
        case WM_CLOSE:
            _is_open = false;
            PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED: {
            RECT* suggested = (RECT*)lparam;
            SetWindowPos(_hwnd, nullptr,
                suggested->left, suggested->top,
                suggested->right - suggested->left,
                suggested->bottom - suggested->top,
                SWP_NOZORDER | SWP_NOACTIVATE);
            break;
        }
        case WM_INPUT: {
            input_system.process_raw_input(wparam, lparam);
            return 0;
        }
        case WM_DESTROY:
            _hwnd = nullptr;
            return 0;
        default:
            return DefWindowProcA(_hwnd, msg, wparam, lparam);
        }
    }


};

class window_manager_t {
public:
    window_manager_t()
    {
        if (SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))
            return;

        SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
    }

    window_instance_t* create_window(const char* title, int width, int height, uint32_t flags = window_flag_none) {
        unique_lock<critical_section> lock_guard(&manager_lock);

        auto* win = _windows.emplace_back();
        if (!win->create(title, width, height, flags)) {
            remove_window(win);
            return nullptr;
        }
        return win;
    }

    void destroy_window(window_instance_t* win) {
        if (!win) 
            return;

		unique_lock<critical_section> lock_guard(&manager_lock);
    
        win->destroy();

		remove_window(win);
    }

    s_performance_vector<window_instance_t>& windows() { return _windows; }
private:
	void remove_window(window_instance_t* win) {
		if (!win)
			return;

        const size_t windows_count = _windows.size();

        for (size_t i = 0; i < windows_count; ++i) {
            if (!_windows.is_alive(i))
                continue;

            if (&_windows[i] == win) {
                _windows.remove(i);
                break;
            }
        }
	}
private:
    critical_section manager_lock;
    s_performance_vector<window_instance_t> _windows;
};

extern window_manager_t g_window_manager;