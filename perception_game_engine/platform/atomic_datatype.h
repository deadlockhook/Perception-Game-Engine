#pragma once

#include <cstdint>
#include <type_traits>
#include <cstring>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

template <typename T>
struct p_atomic
{
    static_assert(std::is_trivially_copyable<T>::value, "p_atomic<T> requires trivially copyable types.");

#if defined(_MSC_VER)
    volatile T value = {};
#else
    T value = {};
#endif

    __forceinline void store(T v, std::memory_order = std::memory_order_relaxed)
    {
#if defined(_MSC_VER)
        volatile T* ptr = const_cast<volatile T*>(&value);

        if constexpr (sizeof(T) == 1)
            _InterlockedExchange8(reinterpret_cast<volatile char*>(ptr), v);
        else if constexpr (sizeof(T) == 2)
            _InterlockedExchange16(reinterpret_cast<volatile short*>(ptr), v);
        else if constexpr (sizeof(T) == 4)
            _InterlockedExchange(reinterpret_cast<volatile long*>(ptr), static_cast<long>(v));
        else if constexpr (sizeof(T) == 8)
            _InterlockedExchange64(reinterpret_cast<volatile long long*>(ptr), static_cast<long long>(v));
        else
            static_assert(sizeof(T) == 0, "Unsupported atomic size.");
#elif defined(__GNUC__) || defined(__clang__)
        __atomic_store_n(&value, v, __ATOMIC_RELAXED);
#else
#error Unsupported platform
#endif
    }

    // Atomic load
    __forceinline T load(std::memory_order = std::memory_order_relaxed) const
    {
        return value;
#if defined(_MSC_VER)
        volatile T* ptr = const_cast<volatile T*>(&value);

        if constexpr (sizeof(T) == 1)
            return _InterlockedOr8(reinterpret_cast<volatile char*>(ptr), 0);
        else if constexpr (sizeof(T) == 2)
            return _InterlockedOr16(reinterpret_cast<volatile short*>(ptr), 0);
        else if constexpr (sizeof(T) == 4)
            return static_cast<T>(_InterlockedOr(reinterpret_cast<volatile long*>(ptr), 0));
        else if constexpr (sizeof(T) == 8)
            return static_cast<T>(_InterlockedOr64(reinterpret_cast<volatile long long*>(ptr), 0));
        else
            static_assert(sizeof(T) == 0, "Unsupported atomic size.");
#elif defined(__GNUC__) || defined(__clang__)
        return __atomic_load_n(&value, __ATOMIC_RELAXED);
#else
#error Unsupported platform
#endif
    }

    __forceinline T exchange(T v, std::memory_order = std::memory_order_relaxed)
    {
#if defined(_MSC_VER)
        volatile T* ptr = const_cast<volatile T*>(&value);

        if constexpr (sizeof(T) == 1)
            return _InterlockedExchange8(reinterpret_cast<volatile char*>(ptr), v);
        else if constexpr (sizeof(T) == 2)
            return _InterlockedExchange16(reinterpret_cast<volatile short*>(ptr), v);
        else if constexpr (sizeof(T) == 4)
            return static_cast<T>(_InterlockedExchange(reinterpret_cast<volatile long*>(ptr), static_cast<long>(v)));
        else if constexpr (sizeof(T) == 8)
            return static_cast<T>(_InterlockedExchange64(reinterpret_cast<volatile long long*>(ptr), static_cast<long long>(v)));
        else
            static_assert(sizeof(T) == 0, "Unsupported atomic size.");
#elif defined(__GNUC__) || defined(__clang__)
        return __atomic_exchange_n(&value, v, __ATOMIC_RELAXED);
#else
#error Unsupported platform
#endif
    }
};

template <>
struct p_atomic<float>
{
    p_atomic() = default;
    p_atomic(float f) { store(f); }

    p_atomic<uint32_t> u32;

    __forceinline void store(float v, std::memory_order order = std::memory_order_relaxed)
    {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        u32.store(bits, order);
    }

    __forceinline float load(std::memory_order order = std::memory_order_relaxed) const
    {
        uint32_t bits = u32.load(order);
        float result;
        std::memcpy(&result, &bits, sizeof(result));
        return result;
    }

    __forceinline float exchange(float v, std::memory_order order = std::memory_order_relaxed)
    {
        uint32_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        uint32_t old_bits = u32.exchange(bits, order);

        float result;
        std::memcpy(&result, &old_bits, sizeof(result));
        return result;
    }
};

template <>
struct p_atomic<double>
{
    p_atomic() = default;
    p_atomic(double d) { store(d); }

    p_atomic<uint64_t> u64;

    __forceinline void store(double v, std::memory_order order = std::memory_order_relaxed)
    {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        u64.store(bits, order);
    }

    __forceinline double load(std::memory_order order = std::memory_order_relaxed) const
    {
        uint64_t bits = u64.load(order);
        double result;
        std::memcpy(&result, &bits, sizeof(result));
        return result;
    }

    __forceinline double exchange(double v, std::memory_order order = std::memory_order_relaxed)
    {
        uint64_t bits;
        std::memcpy(&bits, &v, sizeof(bits));
        uint64_t old_bits = u64.exchange(bits, order);

        double result;
        std::memcpy(&result, &old_bits, sizeof(result));
        return result;
    }
};

using atomic_bool_t = p_atomic<bool>;
using atomic_uint32_t = p_atomic<uint32_t>;
using atomic_uint64_t = p_atomic<uint64_t>;
using atomic_int32_t = p_atomic<int32_t>;
using atomic_int64_t = p_atomic<int64_t>;
using atomic_ptr_t = p_atomic<void*>;
using atomic_uintptr_t = p_atomic<uintptr_t>;
using atomic_intptr_t = p_atomic<intptr_t>;

template<typename T>
__forceinline void p_set_atomic(p_atomic<T>& var, T value, std::memory_order order = std::memory_order_relaxed)
{
    var.store(value, order);
}

template<typename T>
__forceinline T p_get_atomic(const p_atomic<T>& var, std::memory_order order = std::memory_order_relaxed)
{
    return var.load(order);
}