#pragma once
#include "s_string.h"
#include "atomic.h"
#include "../serialize/fnva_hash.h"
#include "s_performance_vector.h"

struct pooled_string_t
{
    pooled_string_t() = default;
    ~pooled_string_t() = default;

    pooled_string_t(const s_string& s) {
        str = s;
        hash = fnv1a32(s.c_str());
    }

    s_string str;
    uint32_t hash = 0;
};

class string_pool_t {
public:
    string_pool_t() = default;
    ~string_pool_t() = default;

    const s_string& intern(const char* str) {
        uint32_t hash = fnv1a32(str);
        unique_lock<critical_section> lock(&mutex);

        const size_t count = pool.size();
        for (size_t i = 0; i < count; ++i) {
            if (!pool.is_alive(i))
                continue;

            if (pool[i]->hash == hash)
                return pool[i]->str;
        }

        pooled_string_t* new_entry = pool.push_back(new pooled_string_t(s_string(str)));
        return new_entry->str;
    }

    const s_string& intern(const s_string& str) {
        return intern(str.c_str());
    }

    bool contains(const s_string& str) const {
        uint32_t hash = fnv1a32(str.c_str());

        const size_t count = pool.size();
        for (size_t i = 0; i < count; ++i) {
            if (!pool.is_alive(i))
                continue;

            if (pool[i]->hash == hash)
                return true;
        }
        return false;
    }

    const s_string* get_name_by_hash(uint32_t hash) const {
        const size_t count = pool.size();
        for (size_t i = 0; i < count; ++i) {
            if (!pool.is_alive(i))
                continue;

            if (pool[i]->hash == hash)
                return &pool[i]->str;
        }
        return nullptr;
    }

    void clear() {
        unique_lock<critical_section> lock(&mutex);
        for (size_t i = 0; i < pool.size(); ++i) {
            if (pool.is_alive(i))
                delete pool[i];
        }
        pool.clear();
    }


    size_t size() const { return pool.size(); }

private:
    s_performance_vector<pooled_string_t*> pool;
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
    const s_string* str = nullptr;
    uint32_t hash = 0;

    s_pooled_string() = default;
    ~s_pooled_string() = default;

    s_pooled_string(const char* raw) {
        hash = fnv1a32(raw);
        str = &intern_string(raw);
    }

    const char* c_str() const { return str ? str->c_str() : ""; }
    operator const char* () const { return c_str(); }
    operator const s_string& () const { return *str; }
};

