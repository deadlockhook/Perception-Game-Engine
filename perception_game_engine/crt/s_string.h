#pragma once
#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <ostream>
#include "../memory/mem.h"

class s_string {
public:
    s_string() {
        m_capacity = 16;
        if (m_buffer.allocate(m_capacity))
            m_length = 0;
    }

    s_string(const char* str) {
        m_length = (str ? strlen(str) : 0);
        m_capacity = m_length + 16;
        if (m_buffer.allocate(m_capacity)) {
            if (m_length)
                memcpy(m_buffer.data(), str, m_length);
        }
    }

    s_string(const char* str, size_t len) {
        m_length = len;
        m_capacity = m_length + 16;
        if (m_buffer.allocate(m_capacity)) {
            memcpy(m_buffer.data(), str, m_length);
        }
    }

    s_string(const s_string& other) {
        m_length = other.m_length;
        m_capacity = other.m_capacity;
        if (m_buffer.allocate(m_capacity)) {
            memcpy(m_buffer.data(), other.m_buffer.data(), m_length);
        }
    }

    s_string& operator=(const s_string& other) {
        if (this != &other) {
            v_heap_mem temp;
            if (temp.allocate(other.m_capacity)) {
                memcpy(temp.data(), other.m_buffer.data(), other.m_length);
                m_buffer.swap(temp);
                m_length = other.m_length;
                m_capacity = other.m_capacity;
            }
        }
        return *this;
    }

    s_string(s_string&& other) noexcept {
        m_buffer = std::move(other.m_buffer);
        m_length = other.m_length;
        m_capacity = other.m_capacity;
        other.m_length = 0;
        other.m_capacity = 0;
    }

    s_string& operator=(s_string&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_length = other.m_length;
            m_capacity = other.m_capacity;
            other.m_length = 0;
            other.m_capacity = 0;
        }
        return *this;
    }
    
    s_string(char c) {
        m_length = 1;
        m_capacity = 16;
        if (m_buffer.allocate(m_capacity))
            ((char*)m_buffer.data())[0] = c;
    }


    ~s_string() {
        m_buffer.clear();
        m_buffer.free();
        m_capacity = 0;
    }

    void reserve(size_t size) {
        if (size > m_capacity) {
            v_heap_mem new_buf;
            if (new_buf.allocate(size)) {
                memcpy(new_buf.data(), m_buffer.data(), m_length);
                m_buffer.swap(new_buf);
                m_capacity = size;
            }
        }
    }

    void push_back(char c) {
        if (m_length + 1 >= m_capacity)
            reserve(m_capacity * 2);
        ((char*)m_buffer.data())[m_length++] = c;
    }

    void pop_back() {
        if (m_length > 0) {
            m_length--;
            ((char*)m_buffer.data())[m_length] = '\0';
        }
    }

    void clear() {
        m_buffer.clear();
        m_length = 0;
    }

    void insert(size_t pos, const char* str) {
        size_t len = strlen(str);
        if (m_length + len >= m_capacity)
            reserve(m_length + len + 16);
        memmove((char*)m_buffer.data() + pos + len, (char*)m_buffer.data() + pos, m_length - pos);
        memcpy((char*)m_buffer.data() + pos, str, len);
        m_length += len;
    }

    void erase(size_t pos, size_t len) {
        if (pos + len > m_length) return;
        RtlZeroMemory((char*)m_buffer.data() + pos, len);
        memmove((char*)m_buffer.data() + pos, (char*)m_buffer.data() + pos + len, m_length - pos - len);
        m_length -= len;
        ((char*)m_buffer.data())[m_length] = '\0';
    }

    void replace(size_t pos, size_t len, const char* str) {
        erase(pos, len);
        insert(pos, str);
    }

    void trim() {
        char* data = (char*)m_buffer.data();
        size_t start = 0, end = m_length;
        while (start < m_length && (data[start] == ' ' || data[start] == '\t')) ++start;
        while (end > start && (data[end - 1] == ' ' || data[end - 1] == '\t')) --end;
        size_t new_len = end - start;
        if (start > 0 || end < m_length) {
            memmove(data, data + start, new_len);
            RtlZeroMemory(data + new_len, m_length - new_len);
            m_length = new_len;
        }
    }

    void to_upper() {
        char* data = (char*)m_buffer.data();
        for (size_t i = 0; i < m_length; i++)
            if (data[i] >= 'a' && data[i] <= 'z') data[i] -= 32;
    }

    void to_lower() {
        char* data = (char*)m_buffer.data();
        for (size_t i = 0; i < m_length; i++)
            if (data[i] >= 'A' && data[i] <= 'Z') data[i] += 32;
    }

    const char* c_str() const {
        ((char*)m_buffer.data())[m_length] = '\0';
        return (const char*)m_buffer.data();
    }

    int compare(const char* str) const {
        return strcmp(c_str(), str);
    }

    size_t find(const char* substr) const {
        const char* found = strstr(c_str(), substr);
        return found ? (found - c_str()) : static_cast<size_t>(-1);
    }

    char& operator[](size_t index) { return ((char*)m_buffer.data())[index]; }
    const char& operator[](size_t index) const { return ((char*)m_buffer.data())[index]; }

    bool operator==(const s_string& other) const {
        return m_length == other.m_length && memcmp(m_buffer.data(), other.m_buffer.data(), m_length) == 0;
    }

    bool operator!=(const s_string& other) const {
        return !(*this == other);
    }

    bool operator==(const char* str) const {
        return strcmp(c_str(), str) == 0;
    }

    bool operator!=(const char* str) const {
        return !(*this == str);
    }

    s_string& operator+=(const s_string& other) {
        insert(m_length, other.c_str());
        return *this;
    }

    s_string& operator+=(const char* str) {
        insert(m_length, str);
        return *this;
    }

    s_string operator+(const s_string& other) const {
        s_string result(*this);
        result += other;
        return result;
    }

    s_string operator+(const char* str) const {
        s_string result(*this);
        result += str;
        return result;
    }

    bool operator<(const s_string& other) const {
        return compare(other.c_str()) < 0;
    }

    operator const char* () const {
        return c_str();
    }

    size_t size() const { return m_length; }
    bool empty() const { return m_length == 0; }

    char* begin() { return (char*)m_buffer.data(); }
    char* end() { return (char*)m_buffer.data() + m_length; }
    const char* begin() const { return (const char*)m_buffer.data(); }
    const char* end() const { return (const char*)m_buffer.data() + m_length; }

    class sw_string to_swstring() const;


private:
    v_heap_mem m_buffer;
    size_t m_length = 0;
    size_t m_capacity = 0;
};

class sw_string {
public:
    sw_string() {
        m_capacity = 16;
        if (m_buffer.allocate(m_capacity * sizeof(wchar_t)))
            m_length = 0;
    }

    sw_string(const wchar_t* str) {
        m_length = (str ? wcslen(str) : 0);
        m_capacity = m_length + 16;
        if (m_buffer.allocate(m_capacity * sizeof(wchar_t))) {
            if (m_length)
                memcpy(m_buffer.data(), str, m_length * sizeof(wchar_t));
        }
    }

    sw_string(const wchar_t* str, size_t len) {
        m_length = len;
        m_capacity = m_length + 16;
        if (m_buffer.allocate(m_capacity * sizeof(wchar_t))) {
            memcpy(m_buffer.data(), str, m_length * sizeof(wchar_t));
        }
    }

    sw_string(const sw_string& other) {
        m_length = other.m_length;
        m_capacity = other.m_capacity;
        if (m_buffer.allocate(m_capacity * sizeof(wchar_t))) {
            memcpy(m_buffer.data(), other.m_buffer.data(), m_length * sizeof(wchar_t));
        }
    }

    sw_string(sw_string&& other) noexcept {
        m_buffer = std::move(other.m_buffer);
        m_length = other.m_length;
        m_capacity = other.m_capacity;
        other.m_length = 0;
        other.m_capacity = 0;
    }

    sw_string& operator=(sw_string&& other) noexcept {
        if (this != &other) {
            m_buffer = std::move(other.m_buffer);
            m_length = other.m_length;
            m_capacity = other.m_capacity;
            other.m_length = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    sw_string(wchar_t c) {
        m_length = 1;
        m_capacity = 16;
        if (m_buffer.allocate(m_capacity * sizeof(wchar_t)))
            ((wchar_t*)m_buffer.data())[0] = c;
    }

    sw_string& operator=(const sw_string& other) {
        if (this != &other) {
            v_heap_mem temp;
            if (temp.allocate(other.m_capacity * sizeof(wchar_t))) {
                memcpy(temp.data(), other.m_buffer.data(), other.m_length * sizeof(wchar_t));
                m_buffer.swap(temp);
                m_length = other.m_length;
                m_capacity = other.m_capacity;
            }
        }
        return *this;
    }

    ~sw_string() {
        m_buffer.clear();
        m_buffer.free();
        m_capacity = 0;
    }

    void reserve(size_t size) {
        if (size > m_capacity) {
            v_heap_mem new_buf;
            if (new_buf.allocate(size * sizeof(wchar_t))) {
                memcpy(new_buf.data(), m_buffer.data(), m_length * sizeof(wchar_t));
                m_buffer.swap(new_buf);
                m_capacity = size;
            }
        }
    }

    void push_back(wchar_t c) {
        if (m_length + 1 >= m_capacity)
            reserve(m_capacity * 2);
        ((wchar_t*)m_buffer.data())[m_length++] = c;
    }

    void pop_back() {
        if (m_length > 0) {
            m_length--;
            ((wchar_t*)m_buffer.data())[m_length] = L'\0';
        }
    }

    void clear() {
        m_buffer.clear();
        m_length = 0;
    }

    void insert(size_t pos, const wchar_t* str) {
        size_t len = wcslen(str);
        if (m_length + len >= m_capacity)
            reserve(m_length + len + 16);
        memmove((wchar_t*)m_buffer.data() + pos + len, (wchar_t*)m_buffer.data() + pos, (m_length - pos) * sizeof(wchar_t));
        memcpy((wchar_t*)m_buffer.data() + pos, str, len * sizeof(wchar_t));
        m_length += len;
    }

    void erase(size_t pos, size_t len) {
        if (pos + len > m_length) return;
        RtlZeroMemory((wchar_t*)m_buffer.data() + pos, len * sizeof(wchar_t));
        memmove((wchar_t*)m_buffer.data() + pos, (wchar_t*)m_buffer.data() + pos + len, (m_length - pos - len) * sizeof(wchar_t));
        m_length -= len;
        ((wchar_t*)m_buffer.data())[m_length] = L'\0';
    }

    void replace(size_t pos, size_t len, const wchar_t* str) {
        erase(pos, len);
        insert(pos, str);
    }

    void trim() {
        wchar_t* data = (wchar_t*)m_buffer.data();
        size_t start = 0, end = m_length;
        while (start < m_length && (data[start] == L' ' || data[start] == L'\t')) ++start;
        while (end > start && (data[end - 1] == L' ' || data[end - 1] == L'\t')) --end;
        size_t new_len = end - start;
        if (start > 0 || end < m_length) {
            memmove(data, data + start, new_len * sizeof(wchar_t));
            RtlZeroMemory(data + new_len, (m_length - new_len) * sizeof(wchar_t));
            m_length = new_len;
        }
    }

    void to_upper() {
        wchar_t* data = (wchar_t*)m_buffer.data();
        for (size_t i = 0; i < m_length; i++)
            if (data[i] >= L'a' && data[i] <= L'z') data[i] -= 32;
    }

    void to_lower() {
        wchar_t* data = (wchar_t*)m_buffer.data();
        for (size_t i = 0; i < m_length; i++)
            if (data[i] >= L'A' && data[i] <= L'Z') data[i] += 32;
    }

    const wchar_t* c_str() const {
        ((wchar_t*)m_buffer.data())[m_length] = L'\0';
        return (const wchar_t*)m_buffer.data();
    }

    int compare(const wchar_t* str) const {
        return wcscmp(c_str(), str);
    }

    size_t find(const wchar_t* substr) const {
        const wchar_t* found = wcsstr(c_str(), substr);
        return found ? (found - c_str()) : static_cast<size_t>(-1);
    }

    class s_string to_sstring() const;

    wchar_t& operator[](size_t index) { return ((wchar_t*)m_buffer.data())[index]; }
    const wchar_t& operator[](size_t index) const { return ((wchar_t*)m_buffer.data())[index]; }

    bool operator==(const sw_string& other) const {
        return m_length == other.m_length && memcmp(m_buffer.data(), other.m_buffer.data(), m_length * sizeof(wchar_t)) == 0;
    }

    bool operator!=(const sw_string& other) const {
        return !(*this == other);
    }

    bool operator==(const wchar_t* str) const {
        return wcscmp(c_str(), str) == 0;
    }

    bool operator!=(const wchar_t* str) const {
        return !(*this == str);
    }

    sw_string& operator+=(const sw_string& other) {
        insert(m_length, other.c_str());
        return *this;
    }

    sw_string& operator+=(const wchar_t* str) {
        insert(m_length, str);
        return *this;
    }

    sw_string operator+(const sw_string& other) const {
        sw_string result(*this);
        result += other;
        return result;
    }

    sw_string operator+(const wchar_t* str) const {
        sw_string result(*this);
        result += str;
        return result;
    }

    bool operator<(const sw_string& other) const {
        return compare(other.c_str()) < 0;
    }

    operator const wchar_t* () const {
        return c_str();
    }

    size_t size() const { return m_length; }
    bool empty() const { return m_length == 0; }

    wchar_t* begin() { return (wchar_t*)m_buffer.data(); }
    wchar_t* end() { return (wchar_t*)m_buffer.data() + m_length; }
    const wchar_t* begin() const { return (const wchar_t*)m_buffer.data(); }
    const wchar_t* end() const { return (const wchar_t*)m_buffer.data() + m_length; }
   

private:
    v_heap_mem m_buffer;
    size_t m_length = 0;
    size_t m_capacity = 0;
};

inline std::ostream& operator<<(std::ostream& os, const s_string& str) {
    os << str.c_str();
    return os;
}

inline std::wostream& operator<<(std::wostream& os, const sw_string& str) {
    os << str.c_str();
    return os;
}

