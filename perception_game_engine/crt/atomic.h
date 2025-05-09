#pragma once
#include <Windows.h>
#include <intrin.h>
#include <iostream>
#include "inline_crt.h"

class critical_section
{
public:

    critical_section() {

        if (is_initialized)
            return;

        InitializeCriticalSection(&cs);
        is_initialized = true;
    };

    int try_lock() {
        return TryEnterCriticalSection(&cs);
    }

    void lock() {
        EnterCriticalSection(&cs);
    }

    void release() {
        LeaveCriticalSection(&cs);
    }

    ~critical_section() {
        DeleteCriticalSection(&cs);
    }

public:
    bool is_initialized = false;
    CRITICAL_SECTION cs = { 0 };
};

class interlocked_mutex {
public:
    __forceinline  interlocked_mutex() : flag(0) {}

    __forceinline void lock() {
        while (InterlockedExchange(&flag, 1) == 1)
        {

        }
    }

    __forceinline bool try_lock() {
        return InterlockedExchange(&flag, 1) == 0;
    }

    __forceinline void release() {
        InterlockedExchange(&flag, 0);
    }

private:
    volatile LONG flag;
};

template<class lock>
class unique_lock
{
public:
    unique_lock(lock* _section) {
        lock_object = _section;
        lock_object->lock();
    }

    ~unique_lock() {
        lock_object->release();
    }
private:
    lock* lock_object;
};

template<class t>
class shared_variable : public interlocked_mutex {
public:
    shared_variable() {
        unique_lock lock_guard(this);
        RtlZeroMemory(&data, sizeof(t));
    }

    shared_variable(const t& value) {
        unique_lock lock_guard(this);
        memcpy(&data, &value, sizeof(t));
    }

    shared_variable(const shared_variable<t>& other) {
        if (this != &other) {
            unique_lock lock_guard_this(this);
            unique_lock lock_guard_other(&other);
            memcpy(&data, &other.data, sizeof(t));
        }
    }

    shared_variable(shared_variable<t>&& other) noexcept {
        unique_lock lock_guard(this);
        data = std::move(other.data);
    }

    shared_variable(std::initializer_list<t> init_list) {
        unique_lock lock_guard(this);
        data = t(init_list);
    }

    shared_variable<t>& operator=(const shared_variable<t>& other) {
        if (this != &other) {
            unique_lock lock_guard_this(this);
            unique_lock lock_guard_other(&other);
            memcpy(&data, &other.data, sizeof(t));
        }
        return *this;
    }

    shared_variable<t>& operator=(shared_variable<t>&& other) noexcept {
        if (this != &other) {
            unique_lock lock_guard(this);
            data = std::move(other.data);
        }
        return *this;
    }

    shared_variable<t>& operator=(const t& value) noexcept {
        unique_lock lock_guard(this);
        memcpy(&data, &value, sizeof(t));
        return *this;
    }


    t get() {
        t out;
        unique_lock lock_guard(this);
        memcpy(&out, &data, sizeof(t));
        return out;
    }

    void set(const t& value) {
        unique_lock lock_guard(this);
        memcpy(&data, &value, sizeof(t));
    }

    ~shared_variable() = default;

private:
    t data;
};

class shared_pointer {
private:
    volatile uintptr_t ptr;

public:
    __forceinline shared_pointer() : ptr(0) {}

    __forceinline void set(uintptr_t new_value) {
        InterlockedExchange64(reinterpret_cast<volatile LONG64*>(&ptr), static_cast<LONG64>(new_value));
    }

    __forceinline uintptr_t get() const {
        return ptr;
    }
};
