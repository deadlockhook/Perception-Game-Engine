#pragma once
#include <Windows.h>
#include "inline_crt.h"
#include "../memory/mem.h"
#include "atomic.h"

template<class T>
class s_vector
{
public:
    ~s_vector() {
        free_and_clear();
    }

    s_vector() = default;

    s_vector(size_t count) {
        if (count) {
            m_buffer.allocate(count * sizeof(T));
            m_memory = (T*)m_buffer.data();
            m_capacity = count;
            m_count = count;
        }
    }

    s_vector(size_t count, const T& value) {
        if (count) {
            m_buffer.allocate(count * sizeof(T));
            m_memory = (T*)m_buffer.data();
            m_capacity = count;
            m_count = count;
            for (size_t i = 0; i < m_count; i++)
                m_memory[i] = value;
        }
    }

    s_vector(const s_vector<T>& other) {
        if (other.m_count) {
            m_buffer.allocate(other.m_count * sizeof(T));
            m_memory = (T*)m_buffer.data();
            m_capacity = other.m_count;
            m_count = other.m_count;
            for (size_t i = 0; i < m_count; i++)
                new (&m_memory[i]) T(other.m_memory[i]);
        }
    }

    s_vector(s_vector<T>&& other) noexcept {
        *this = std::move(other);
    }

    s_vector<T>& operator=(const s_vector<T>& other) {
        if (this != &other) {
            free_and_clear();
            if (other.m_count) {
                m_buffer.allocate(other.m_count * sizeof(T));
                m_memory = (T*)m_buffer.data();
                m_capacity = other.m_count;
                m_count = other.m_count;
                for (size_t i = 0; i < m_count; i++)
                    new (&m_memory[i]) T(other.m_memory[i]);
            }
        }
        return *this;
    }

    s_vector<T>& operator=(s_vector<T>&& other) noexcept {
        if (this != &other) {
            free_and_clear();
            m_buffer = std::move(other.m_buffer);
            m_memory = (T*)m_buffer.data();
            m_count = other.m_count;
            m_capacity = other.m_capacity;
            other.m_count = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    void resize(size_t new_count, const T& fill_value = T{}) {
        if (new_count < m_count) {
            for (size_t i = new_count; i < m_count; ++i)
                m_memory[i].~T();
            m_count = new_count;
        }
        else if (new_count > m_count) {
            reserve(new_count);
            for (size_t i = m_count; i < new_count; ++i)
                new (&m_memory[i]) T(fill_value);
            m_count = new_count;
        }
    }

    void reserve(size_t size) {
        if (size > m_capacity) {
            m_buffer.resize(size * sizeof(T));
            m_memory = (T*)m_buffer.data();
            m_capacity = size;
        }
    }

    void clear() {
        for (size_t i = 0; i < m_count; ++i)
            m_memory[i].~T();
        m_count = 0;
    }

    void free_and_clear() {
        for (size_t i = 0; i < m_count; ++i)
            m_memory[i].~T();
        m_buffer.clear();
        m_buffer.free();
        m_memory = nullptr;
        m_count = 0;
        m_capacity = 0;
    }

    T& push_back(const T& value) {
        if (m_count == m_capacity) {
            reserve(m_capacity ? m_capacity * 2 : 1);
        }
        new (&m_memory[m_count]) T(value);
        return m_memory[m_count++];
    }

    void pop_back() {
        if (m_count)
            m_memory[--m_count].~T(); 
    }

    void shrink_to_fit() {
        if (m_capacity > m_count) {
            m_buffer.resize(m_count * sizeof(T));
            m_memory = (T*)m_buffer.data();
            m_capacity = m_count;
        }
    }

    size_t index_of(const T& value) const {
        for (size_t i = 0; i < m_count; ++i)
            if (m_memory[i] == value)
                return i;
        return static_cast<size_t>(-1);
    }

    template<typename... args_t>
    T& emplace_back(args_t&&... args) {
        if (m_count == m_capacity)
            reserve(m_capacity ? m_capacity * 2 : 1);

        new (&m_memory[m_count]) T(std::forward<args_t>(args)...);
        return m_memory[m_count++];
    }

    T* find(const T& value) {
        for (size_t i = 0; i < m_count; ++i)
            if (m_memory[i] == value)
                return &m_memory[i];
        return nullptr;
    }

    const T* find(const T& value) const {
        for (size_t i = 0; i < m_count; ++i)
            if (m_memory[i] == value)
                return &m_memory[i];
        return nullptr;
    }

    size_t remove_all(const T& value) {
        size_t removed = 0;
        for (size_t i = 0; i < m_count; ) {
            if (m_memory[i] == value) {
                erase_at(i);
                ++removed;
            }
            else {
                ++i;
            }
        }
        return removed;
    }

    bool remove(const T& value) {
        size_t index = index_of(value);
        if (index != static_cast<size_t>(-1)) {
            erase_at(index);
            return true;
        }
        return false;
    }

    T& push_front(const T& value) {
        if (m_count == m_capacity) {
            reserve(m_capacity ? m_capacity * 2 : 1);
        }
        for (size_t i = m_count; i > 0; --i) {
            new (&m_memory[i]) T(std::move(m_memory[i - 1]));
            m_memory[i - 1].~T();
        }
        new (&m_memory[0]) T(value);
        ++m_count;
        return m_memory[0];
    }

    void pop_front() {
        if (m_count) {
            for (size_t i = 0; i < m_count - 1; ++i)
                m_memory[i] = m_memory[i + 1];
            --m_count;
        }
    }

    T& back() {
        return m_memory[m_count - 1];
    }

    T& front() {
        return m_memory[0];
    }

    void insert_at(size_t index, const T& value) {
        if (index > m_count)
            return;

        if (m_count == m_capacity)
            reserve(m_capacity ? m_capacity * 2 : 1);

        for (size_t i = m_count; i > index; --i) {
            new (&m_memory[i]) T(std::move(m_memory[i - 1]));
            m_memory[i - 1].~T();
        }
        new (&m_memory[index]) T(value);

        ++m_count;
    }
    void remove_iterator(T* it) {
        if (!it || it < m_memory || it >= m_memory + m_count)
            return;

        size_t index = it - m_memory;
        erase_at(index);
    }
    void erase_at(size_t index) {
        if (index >= m_count)
            return;

        m_memory[index].~T();
        for (size_t i = index; i < m_count - 1; ++i) {
            new (&m_memory[i]) T(std::move(m_memory[i + 1]));
            m_memory[i + 1].~T();
        }
        --m_count;
    }

    bool contains(const T& value) const {
        for (size_t i = 0; i < m_count; ++i) {
            if (m_memory[i] == value)
                return true;
        }
        return false;
    }

    T& operator[](size_t index) {
        return m_memory[index];
    }

    const T& operator[](size_t index) const {
        return m_memory[index];
    }

    bool is_empty() const {
        return m_count == 0;
    }

    void swap(s_vector<T>& other) noexcept {
        std::swap(m_buffer, other.m_buffer);
        std::swap(m_memory, other.m_memory);
        std::swap(m_count, other.m_count);
        std::swap(m_capacity, other.m_capacity);
    }

    T* begin() { return m_memory; }
    T* end() { return m_memory + m_count; }
    const T* begin() const { return m_memory; }
    const T* end() const { return m_memory + m_count; }
    const T* cbegin() const { return begin(); }
    const T* cend() const { return end(); }
    T* rbegin() { return m_memory + m_count - 1; }
    T* rend() { return m_memory - 1; }
    const T* rbegin() const { return m_memory + m_count - 1; }
    const T* rend() const { return m_memory - 1; }

    void assign(const T* arr, size_t count) {
        if (!arr || !count) return;

        clear(); 

        reserve(count);
        for (size_t i = 0; i < count; ++i)
            new (&m_memory[i]) T(arr[i]);

        m_count = count;
    }

    void fill(const T& value) {
        for (size_t i = 0; i < m_count; ++i) {
            m_memory[i].~T();
            new (&m_memory[i]) T(value);
        }
    }

    void reverse() {
        for (size_t i = 0; i < m_count / 2; ++i)
            std::swap(m_memory[i], m_memory[m_count - i - 1]);
    }

    T* next(T* it) {
        return (it + 1 < end()) ? (it + 1) : nullptr;
    }

    const T* next(const T* it) const {
        return (it + 1 < end()) ? (it + 1) : nullptr;
    }

    T& at(size_t index) {
        return m_memory[index];
    }

    const T& at(size_t index) const {
        return m_memory[index];
    }

    T* data() { return m_memory; }
    const T* data() const { return m_memory; }

    size_t count() const { return m_count; }
	size_t size() const { return m_count; }
	size_t capacity() const { return m_capacity; }
	bool empty() const { return m_count == 0; }
private:
    v_heap_mem m_buffer;
    T* m_memory = nullptr;
    size_t m_count = 0;
    size_t m_capacity = 0;
};