#pragma once
#include <Windows.h>
#include "../../crt/s_map.h"
#include "../../crt/s_performance_vector.h"
#include "../../crt/atomic.h"
#include "../../exception/fail.h"

using input_callback_t = void(*)(void* owner, bool activate);
using mouse_move_callback_t = void(*)(void* owner, int delta_x, int delta_y);
using toggle_state_callback_t = void(*)(void* owner, bool active);

struct binding_t {
    input_callback_t callback;
    void* owner;
};

struct mouse_move_binding_t {
    mouse_move_callback_t callback;
    void* owner;
};

struct toggle_binding_t {
    toggle_state_callback_t callback;
    void* owner;
};

enum input_key_t : uint16_t {
    key_invalid = 0,

    key_mouse_left,
    key_mouse_right,
    key_mouse_middle,
    key_mouse_x1,
    key_mouse_x2,
    key_mouse_wheel_up,
    key_mouse_wheel_down,
    key_keyboard_base = 256,
    key_a = key_keyboard_base + 'A',
    key_b = key_keyboard_base + 'B',
    key_c = key_keyboard_base + 'C',
    key_d = key_keyboard_base + 'D',
    key_e = key_keyboard_base + 'E',
    key_f = key_keyboard_base + 'F',
    key_g = key_keyboard_base + 'G',
    key_h = key_keyboard_base + 'H',
    key_i = key_keyboard_base + 'I',
    key_j = key_keyboard_base + 'J',
    key_k = key_keyboard_base + 'K',
    key_l = key_keyboard_base + 'L',
    key_m = key_keyboard_base + 'M',
    key_n = key_keyboard_base + 'N',
    key_o = key_keyboard_base + 'O',
    key_p = key_keyboard_base + 'P',
    key_q = key_keyboard_base + 'Q',
    key_r = key_keyboard_base + 'R',
    key_s = key_keyboard_base + 'S',
    key_t = key_keyboard_base + 'T',
    key_u = key_keyboard_base + 'U',
    key_v = key_keyboard_base + 'V',
    key_w = key_keyboard_base + 'W',
    key_x = key_keyboard_base + 'X',
    key_y = key_keyboard_base + 'Y',
    key_z = key_keyboard_base + 'Z',

    key_0 = key_keyboard_base + '0',
    key_1 = key_keyboard_base + '1',
    key_2 = key_keyboard_base + '2',
    key_3 = key_keyboard_base + '3',
    key_4 = key_keyboard_base + '4',
    key_5 = key_keyboard_base + '5',
    key_6 = key_keyboard_base + '6',
    key_7 = key_keyboard_base + '7',
    key_8 = key_keyboard_base + '8',
    key_9 = key_keyboard_base + '9',

    key_f1 = key_keyboard_base + VK_F1,
    key_f2 = key_keyboard_base + VK_F2,
    key_f3 = key_keyboard_base + VK_F3,
    key_f4 = key_keyboard_base + VK_F4,
    key_f5 = key_keyboard_base + VK_F5,
    key_f6 = key_keyboard_base + VK_F6,
    key_f7 = key_keyboard_base + VK_F7,
    key_f8 = key_keyboard_base + VK_F8,
    key_f9 = key_keyboard_base + VK_F9,
    key_f10 = key_keyboard_base + VK_F10,
    key_f11 = key_keyboard_base + VK_F11,
    key_f12 = key_keyboard_base + VK_F12,

    key_space = key_keyboard_base + VK_SPACE,
    key_enter = key_keyboard_base + VK_RETURN,
    key_escape = key_keyboard_base + VK_ESCAPE,
    key_tab = key_keyboard_base + VK_TAB,
    key_backspace = key_keyboard_base + VK_BACK,

    key_shift = key_keyboard_base + VK_SHIFT,
    key_ctrl = key_keyboard_base + VK_CONTROL,
    key_alt = key_keyboard_base + VK_MENU,

    key_lshift = key_keyboard_base + VK_LSHIFT,
    key_rshift = key_keyboard_base + VK_RSHIFT,
    key_lctrl = key_keyboard_base + VK_LCONTROL,
    key_rctrl = key_keyboard_base + VK_RCONTROL,
    key_lalt = key_keyboard_base + VK_LMENU,
    key_ralt = key_keyboard_base + VK_RMENU,
    key_capslock = key_keyboard_base + VK_CAPITAL,
	key_numlock = key_keyboard_base + VK_NUMLOCK,
	key_scrolllock = key_keyboard_base + VK_SCROLL,

    key_up = key_keyboard_base + VK_UP,
    key_down = key_keyboard_base + VK_DOWN,
    key_left = key_keyboard_base + VK_LEFT,
    key_right = key_keyboard_base + VK_RIGHT
};

class input_system {
public:

    void initialize(HWND hwnd);

    void bind(input_key_t key, input_callback_t callback, void* owner = nullptr);
    void bind(int key, input_callback_t callback, void* owner = nullptr);
    void bind_mouse_move(mouse_move_callback_t callback, void* owner = nullptr);
    void unbind_mouse_move(mouse_move_callback_t callback, void* owner = nullptr);
	
    void unbind(input_key_t key, input_callback_t callback, void* owner);
    void unbind_all(input_key_t key);
    void rebind(input_key_t key, input_callback_t old_callback, void* old_userdata,
        input_callback_t new_callback, void* new_userdata);

	//for toggle keys like capslock, numlock, scrolllock, etc.
    void bind_toggle(input_key_t key, toggle_state_callback_t callback, void* owner = nullptr);
    void bind_toggle(int key, toggle_state_callback_t callback, void* owner = nullptr);
    void unbind_toggle(input_key_t key, toggle_state_callback_t callback, void* owner);
    void rebind_toggle(input_key_t key, toggle_state_callback_t old_callback, void* old_userdata,
        toggle_state_callback_t new_callback, void* new_userdata);
    void unbind_all_toggle(input_key_t key);
    void unbind_all_toggle();

    void unbind_all();

    void process_raw_input(WPARAM, LPARAM lparam);

private:
    s_map<input_key_t, s_performance_vector<binding_t>> _bindings;
    s_performance_vector<mouse_move_binding_t> _mouse_move_callbacks;
    s_map<input_key_t, s_performance_vector<toggle_binding_t>> _toggle_bindings;
	interlocked_mutex _mutex;

    void handle_mouse_button(USHORT flags, USHORT down_flag, USHORT up_flag, input_key_t key);
    void trigger(input_key_t key, bool activate);
};