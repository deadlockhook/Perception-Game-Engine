#pragma once
#include <Windows.h>
#include <cstdint>
#include "../crt/atomic.h"

#define m_alloc malloc
#define m_free free
#define m_set memset
#define m_realloc realloc

#define PAGE_SIZE_4KB 0x1000

enum memory_pool_type
{
    paged_pool,
    paged_pool_execute,
    non_paged_pool,
    non_paged_pool_execute,
};

class v_mem
{
public:
    v_mem() = default;
    v_mem(void* memory, size_t size, memory_pool_type type, bool paged, bool zero_on_destruction) : memory(memory), size(size), type(type), paged(paged), zero_on_destruction(zero_on_destruction) {}

    ~v_mem() {
        free();
    }

    operator void* () const { return memory; }
    operator bool() const { return memory != nullptr; }

    static v_mem allocate_pool(memory_pool_type type, size_t size, bool zero_on_destruction = false);
    void free();

    void zero_and_free();
	void* data() const { return memory; }
	size_t get_size() const { return size; }

private:
    void* memory = nullptr;
    size_t size = 0;
    memory_pool_type type = paged_pool;
    bool paged = false;
    bool zero_on_destruction = false;
};

class v_heap_mem
{
public:

    v_heap_mem() = default;
    v_heap_mem(const v_heap_mem&) = delete;
    v_heap_mem& operator=(const v_heap_mem&) = delete;

    v_heap_mem(size_t size)
    {
        allocate(size);
    }

    ~v_heap_mem()
    {
        free();
    }

    static void init();

    v_heap_mem(v_heap_mem&& other) noexcept
    {
        *this = std::move(other);
    }

    v_heap_mem& operator=(v_heap_mem&& other) noexcept
    {
        if (this != &other)
        {
            free();
            memory = other.memory;
            size = other.size;

            other.memory = nullptr;
            other.size = 0;
        }
        return *this;
    }

    void swap(v_heap_mem& other) noexcept
    {
        std::swap(memory, other.memory);
        std::swap(size, other.size);
    }
public:

    template<typename T>
    T* as() const { return static_cast<T*>(memory); }

    template<typename T = uint8_t>
    T* at(size_t index)
    {
        return (index * sizeof(T) < size) ? reinterpret_cast<T*>(static_cast<uint8_t*>(memory) + index * sizeof(T)) : nullptr;
    }
public:
    void* release_ownership()
    {
        void* ptr = memory;
        memory = nullptr;
        size = 0;
        return ptr;
    }
public:

    bool allocate(size_t size);
    bool resize(size_t new_size);
    void free();
    void clear();
    void zero();

    operator void* () const { return memory; }
    operator bool() const { return memory != nullptr; }

    void* data() const { return memory; }
    size_t get_size() const { return size; }


public:
    void acquire_mutex() { m_lock.lock(); }
    void release_mutex() { m_lock.release(); }
private:
    void* memory = nullptr;
    size_t size = 0;
    critical_section m_lock;
};



