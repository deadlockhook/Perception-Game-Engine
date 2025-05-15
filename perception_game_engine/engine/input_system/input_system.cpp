#include "input_system.h"

void input_system::initialize(HWND hwnd) {

    RAWINPUTDEVICE rid[2] = {
        { 0x01, 0x02, RIDEV_INPUTSINK, hwnd },
        { 0x01, 0x06, RIDEV_INPUTSINK, hwnd }
    };
    RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
}

void input_system::bind(input_key_t key, input_callback_t callback, void* owner ) {
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    _bindings[key].push_back({ callback, owner });
}

void input_system::bind(int key, input_callback_t callback, void* owner) {
    return bind((input_key_t)key, callback,  owner);
}

void input_system::unbind_mouse_move(mouse_move_callback_t callback, void* owner)
{
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    for (size_t i = 0; i < _mouse_move_callbacks.size(); ++i) {
		if (!_mouse_move_callbacks.is_alive(i))
			continue;
		auto& b = _mouse_move_callbacks.at(i);
		if (b.callback == callback && b.owner == owner) {
			_mouse_move_callbacks.remove(i);
			break;
		}
	}
}
void input_system::bind_mouse_move(mouse_move_callback_t callback, void* owner ) {

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    _mouse_move_callbacks.push_back({ callback, owner });
}

void input_system::unbind(input_key_t key, input_callback_t callback, void* owner) {

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* vec = _bindings.find(key);
    if (!vec) return;

    for (size_t i = 0; i < vec->size(); ++i) {
        if (!vec->is_alive(i))
            continue;

        auto& b = vec->at(i);
        if (b.callback == callback && b.owner == owner) {
            vec->remove(i);
            break;
        }
    }
}

void input_system::unbind_all(input_key_t key) {

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* vec = _bindings.find(key);
    if (vec) {
        vec->clear();
    }
}

void input_system::rebind(input_key_t key, input_callback_t old_callback, void* old_userdata,
    input_callback_t new_callback, void* new_userdata) {

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* vec = _bindings.find(key);
    if (!vec) return;

    for (size_t i = 0; i < vec->size(); ++i) {
        if (!vec->is_alive(i))
            continue;

        auto& b = vec->at(i);
        if (b.callback == old_callback && b.owner == old_userdata) {
            b.callback = new_callback;
            b.owner = new_userdata;
            return;
        }
    }
}

void input_system::bind_toggle(input_key_t key, toggle_state_callback_t callback, void* owner ) {

	if (key != key_capslock && key != key_numlock && key != key_scrolllock)
		return on_fail("input_system::bind_toggle only supports capslock, numlock and scrolllock!");

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    _toggle_bindings[key].push_back({ callback, owner });
}

void input_system::bind_toggle(int key, toggle_state_callback_t callback, void* owner ) {

	if (key != key_capslock && key != key_numlock && key != key_scrolllock)
		return on_fail("input_system::bind_toggle only supports capslock, numlock and scrolllock!");

    return bind_toggle(input_key_t(key), callback, owner);
}

void input_system::unbind_toggle(input_key_t key, toggle_state_callback_t callback, void* owner) {

    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* callbacks = _toggle_bindings.find(key);

    if (!callbacks)
        return;

    for (size_t i = 0; i < callbacks->size(); ++i) {
        if (!callbacks->is_alive(i))
            continue;

        auto& b = callbacks->at(i);
        if (b.callback == callback && b.owner == owner) {
            callbacks->remove(i);
            break;
        }
    }
}

void input_system::rebind_toggle(input_key_t key, toggle_state_callback_t old_callback, void* old_userdata,
    toggle_state_callback_t new_callback, void* new_userdata) {

	if (key != key_capslock && key != key_numlock && key != key_scrolllock)
		return on_fail("input_system::rebind_toggle only supports capslock, numlock and scrolllock!");
  
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* callbacks = _toggle_bindings.find(key);

    if (!callbacks)
        return;

    for (size_t i = 0; i < callbacks->size(); ++i) {
        if (!callbacks->is_alive(i))
            continue;

        auto& b = callbacks->at(i);
        if (b.callback == old_callback && b.owner == old_userdata) {
            b.callback = new_callback;
            b.owner = new_userdata;
            return;
        }
    }
}

void input_system::unbind_all_toggle(input_key_t key) {
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* callbacks = _toggle_bindings.find(key);

    if (callbacks)
        callbacks->clear();

}

void input_system::unbind_all_toggle() {
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    _toggle_bindings.clear();
}

void input_system::unbind_all() {
    unique_lock<interlocked_mutex> lock_guard(&_mutex);

    _bindings.clear();
    _mouse_move_callbacks.clear();
    _toggle_bindings.clear();
}


void input_system::process_raw_input(WPARAM, LPARAM lparam) {
   
    UINT size = 0;
    GetRawInputData((HRAWINPUT)lparam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
   
    if (size == 0)
        return;

    uint8_t* buffer = (uint8_t*)_alloca(size);
    if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) != size)
        return;

    const RAWINPUT* ri = (const RAWINPUT*)buffer;

    if (ri->header.dwType == RIM_TYPEMOUSE) {
        const RAWMOUSE& mouse = ri->data.mouse;

        if (mouse.lLastX != 0 || mouse.lLastY != 0) {
            {
                unique_lock<interlocked_mutex> lock_guard(&_mutex);

                size_t mouse_move_callbacks = _mouse_move_callbacks.size();

                for (int i = 0; i < _mouse_move_callbacks.size(); ++i) {
                    auto& cb = _mouse_move_callbacks[i];

                    if (!_mouse_move_callbacks.is_alive(i))
                        continue;

                    cb.callback(cb.owner, mouse.lLastX, mouse.lLastY);
                }
            }
        }

        handle_mouse_button(mouse.usButtonFlags, RI_MOUSE_BUTTON_1_DOWN, RI_MOUSE_BUTTON_1_UP, key_mouse_left);
        handle_mouse_button(mouse.usButtonFlags, RI_MOUSE_BUTTON_2_DOWN, RI_MOUSE_BUTTON_2_UP, key_mouse_right);
        handle_mouse_button(mouse.usButtonFlags, RI_MOUSE_BUTTON_3_DOWN, RI_MOUSE_BUTTON_3_UP, key_mouse_middle);
        handle_mouse_button(mouse.usButtonFlags, RI_MOUSE_BUTTON_4_DOWN, RI_MOUSE_BUTTON_4_UP, key_mouse_x1);
        handle_mouse_button(mouse.usButtonFlags, RI_MOUSE_BUTTON_5_DOWN, RI_MOUSE_BUTTON_5_UP, key_mouse_x2);

        if (mouse.usButtonFlags & RI_MOUSE_WHEEL) {
            SHORT wheel = (SHORT)mouse.usButtonData;
            if (wheel > 0) trigger(key_mouse_wheel_up, true);
            if (wheel < 0) trigger(key_mouse_wheel_down, true);
        }
    }
    else if (ri->header.dwType == RIM_TYPEKEYBOARD) {
        const RAWKEYBOARD& keyboard = ri->data.keyboard;

        uint8_t vkey = (uint8_t)keyboard.VKey;
        if (vkey == 255) return;

        if (vkey == VK_SHIFT) 
            vkey = (keyboard.MakeCode == 0x2A) ? VK_LSHIFT : VK_RSHIFT;
        else if (vkey == VK_CONTROL) 
            vkey = (keyboard.Flags & RI_KEY_E0) ? VK_RCONTROL : VK_LCONTROL;
        else if (vkey == VK_MENU) 
            vkey = (keyboard.Flags & RI_KEY_E0) ? VK_RMENU : VK_LMENU;
        
        bool down = !(keyboard.Flags & RI_KEY_BREAK);

        trigger(static_cast<input_key_t>(key_keyboard_base + vkey), down);

        if (down) {
            if (vkey == VK_CAPITAL || vkey == VK_NUMLOCK || vkey == VK_SCROLL) {
                {
                    unique_lock<interlocked_mutex> lock_guard(&_mutex);
                    bool toggled_on = (GetKeyState(vkey) & 0x0001) != 0;

                    auto* toggle_callbacks = _toggle_bindings.find(static_cast<input_key_t>(key_keyboard_base + vkey));
                    if (toggle_callbacks) {
                        for (size_t i = 0; i < toggle_callbacks->size(); ++i) {
                            if (!toggle_callbacks->is_alive(i))
                                continue;

                            auto& tcb = toggle_callbacks->at(i);
                            tcb.callback(tcb.owner, toggled_on);
                        }
                    }
                }
            }
        }
    }
}


void input_system::handle_mouse_button(USHORT flags, USHORT down_flag, USHORT up_flag, input_key_t key) {

    if (flags & down_flag)
        trigger(key, true);

    if (flags & up_flag)
        trigger(key, false);
}

void input_system::trigger(input_key_t key, bool activate) {

	unique_lock<interlocked_mutex> lock_guard(&_mutex);

    auto* callbacks = _bindings.find(key);

    if (!callbacks)
        return;

    for (size_t i = 0; i < callbacks->size(); ++i) {

        if (!callbacks->is_alive(i))
            continue;

        auto& b = callbacks->at(i);
        b.callback(b.owner, activate);
    }
}
