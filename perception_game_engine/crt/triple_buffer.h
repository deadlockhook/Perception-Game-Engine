#pragma once
#include <atomic>

template <typename T>
class triple_buffer_t {
public:

    triple_buffer_t() = default;

    T& wb() { return _buffers[0]; }

    const T& rb() const { return _buffers[2]; }

    void set_available() {
        _buffers[1] = _buffers[0];        
        _has_pending.exchange(true, std::memory_order_release); 
    }

    void sync() {
        if (_has_pending.exchange(false, std::memory_order_acquire)) 
            _buffers[2] = _buffers[1];  
    }

private:
    T _buffers[3]; 

    std::atomic<bool> _has_pending = false;
};