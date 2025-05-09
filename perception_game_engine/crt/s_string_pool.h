#pragma once
#include "s_string.h"
#include "s_map.h"
#include "s_vector.h"
#include "atomic.h"
#include "../serialize/fnva_hash.h"
class string_pool_t
{
public:
    string_pool_t() = default;
    ~string_pool_t() = default;

    const s_string& intern(const char* str)
    {
        unique_lock<critical_section> lock(&mutex);

        uint32_t hash = fnv1a32(str);
        auto* existing = pool.find(hash);

        if (existing && *existing == str)
            return *existing;

        return pool.insert_and_return(hash, s_string(str));
    }

    const s_string& intern(const s_string& str)
    {
        return intern(str.c_str());
    }

    bool contains(const s_string& str) const
    {
        return pool.contains(fnv1a32(str.c_str()));
    }

    const s_string* get_name_by_hash(uint32_t hash) const
    {
        auto* found = pool.find(hash);
        return found ? found : nullptr;
    }

    void clear()
    {
        unique_lock<critical_section> lock(&mutex);
        pool.clear();
    }

    size_t size() const { return pool.size(); }

private:
    s_map<uint32_t, s_string> pool;
    critical_section mutex;
};


extern string_pool_t g_string_pool;

inline const s_string& intern_string(const char* str) {
    return g_string_pool.intern(str);
}

inline const s_string& intern_string(const s_string& str) {
    return g_string_pool.intern(str);
}

struct s_pooled_string {
    s_string* str = nullptr;
    uint32_t hash = 0;

    s_pooled_string() = default;
    s_pooled_string(const char* raw) {
        hash = fnv1a32(raw);
        str = (s_string*)&intern_string(raw); 
    }

    s_pooled_string(const s_string& s) {
        hash = fnv1a32(s.c_str());
        str = (s_string*)&intern_string(s);
    }

    const char* c_str() const { return str ? str->c_str() : ""; }

    bool operator==(const s_pooled_string& other) const {
        return hash == other.hash && str == other.str;
    }

    bool operator!=(const s_pooled_string& other) const {
        return !(*this == other);
    }

    operator const char* () const { return c_str(); }
    operator const s_string& () const { return *str; }
};

