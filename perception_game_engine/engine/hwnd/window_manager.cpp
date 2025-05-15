#include "window_manager.h"

window_manager_t g_window_manager = window_manager_t();

bool window_instance_t::process_messages() {
    MSG msg;
   
    while (PeekMessageA(&msg, _hwnd, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            _is_open = false;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return _is_open;
}

bool window_instance_t::create(const char* title, int width, int height, uint32_t flags) {

    if (_hwnd)
        return false;

    if (width == 0 || height == 0)
    {
		HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfoA(monitor, &mi);
		width = mi.rcMonitor.right - mi.rcMonitor.left;
		height = mi.rcMonitor.bottom - mi.rcMonitor.top;
		
        if (width == 0 || height == 0)
			return false;
    }

    _title = title;
    _width = width;
    _height = height;

    _hinstance = GetModuleHandleA(nullptr);
    ensure_class_registered(_hinstance);

    DWORD style = WS_OVERLAPPEDWINDOW;

    if (flags & window_flag_borderless) {
        style = WS_POPUP;
    }
    if (flags & window_flag_resizable) {
        style |= WS_THICKFRAME;
    }

    DWORD ex_style = 0;
    if (flags & window_flag_always_on_top)
        ex_style |= WS_EX_TOPMOST;
    if (flags & window_flag_tool_window)
        ex_style |= WS_EX_TOOLWINDOW;

    RECT rect = { 0, 0, width, height };
    AdjustWindowRectEx(&rect, style, FALSE, ex_style);

    int pos_x = CW_USEDEFAULT;
    int pos_y = CW_USEDEFAULT;

    if (flags & window_flag_centered) {
        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);
        pos_x = (screen_width - (rect.right - rect.left)) / 2;
        pos_y = (screen_height - (rect.bottom - rect.top)) / 2;
    }

    _hwnd = CreateWindowExA(
        ex_style,
        "PerceptionWindowClass",
        _title.c_str(),
        style,
        pos_x, pos_y,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        _hinstance,
        this
    );

    SetWindowLongPtrA(_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    if (!_hwnd)
        return false;

	input_system.initialize(_hwnd);

    if (!(flags & window_flag_hidden))
        ShowWindow(_hwnd, SW_SHOW);

    _is_open = true;
    return true;
}

void window_instance_t::set_fullscreen(bool exclusive) {
    if (!_hwnd)
        return;

    if (exclusive) {
        DEVMODEA dev_mode = {};
        dev_mode.dmSize = sizeof(DEVMODEA);
        dev_mode.dmPelsWidth = _width;
        dev_mode.dmPelsHeight = _height;
        dev_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

        ChangeDisplaySettingsA(&dev_mode, CDS_FULLSCREEN);

        SetWindowLongPtrA(_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(_hwnd, nullptr, 0, 0, _width, _height, SWP_NOZORDER | SWP_FRAMECHANGED);
    }
    else {
        ChangeDisplaySettingsA(nullptr, 0);

        SetWindowLongPtrA(_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        RECT rect = { 0, 0, _width, _height };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        SetWindowPos(_hwnd, nullptr, CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
            SWP_NOZORDER | SWP_FRAMECHANGED);
    }
}

void window_instance_t::set_resolution(int width, int height) {
    if (!_hwnd) return;

    _width = width;
    _height = height;

    DWORD style = GetWindowLongPtrA(_hwnd, GWL_STYLE);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, style, FALSE);

    int win_width = rect.right - rect.left;
    int win_height = rect.bottom - rect.top;

    SetWindowPos(_hwnd, nullptr, 0, 0, win_width, win_height,
        SWP_NOZORDER | SWP_NOMOVE | SWP_FRAMECHANGED);
}


void window_instance_t::set_resolution_full_size() {
    if (!_hwnd) return;

    HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoA(monitor, &mi);

    _width = mi.rcMonitor.right - mi.rcMonitor.left;
    _height = mi.rcMonitor.bottom - mi.rcMonitor.top;

    SetWindowLongPtrA(_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

    SetWindowPos(_hwnd, HWND_TOP,
        mi.rcMonitor.left,
        mi.rcMonitor.top,
        _width,
        _height,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}

void window_instance_t::register_for_raw_input() {
    RAWINPUTDEVICE rid[2] = {};
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x02;
    rid[0].dwFlags = RIDEV_INPUTSINK;
    rid[0].hwndTarget = _hwnd;
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x06;
    rid[1].dwFlags = RIDEV_INPUTSINK;
    rid[1].hwndTarget = _hwnd;
    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
}

void window_instance_t::center_window() {
    if (!_hwnd) return;

    HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoA(monitor, &mi);

    RECT rect;
    GetWindowRect(_hwnd, &rect);

    int win_width = rect.right - rect.left;
    int win_height = rect.bottom - rect.top;

    int pos_x = mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - win_width) / 2;
    int pos_y = mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - win_height) / 2;

    SetWindowPos(_hwnd, nullptr, pos_x, pos_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

void window_instance_t::set_fullscreen_windowed() {
    if (!_hwnd) return;

    HMONITOR monitor = MonitorFromWindow(_hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    GetMonitorInfoA(monitor, &mi);

    SetWindowLongPtrA(_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

    SetWindowPos(_hwnd, HWND_TOP,
        mi.rcMonitor.left,
        mi.rcMonitor.top,
        mi.rcMonitor.right - mi.rcMonitor.left,
        mi.rcMonitor.bottom - mi.rcMonitor.top,
        SWP_NOZORDER | SWP_FRAMECHANGED);
}