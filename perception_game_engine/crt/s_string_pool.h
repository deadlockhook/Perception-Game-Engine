#pragma once
#include "s_string.h"
#include "s_node_list.h"
#include "atomic.h"
#include "../serialize/fnva_hash.h"

#pragma once
#include "s_string.h"
#include "s_node_list.h"
#include "atomic.h"
#include "../serialize/fnva_hash.h"

class string_pool_t {
public:
    string_pool_t() = default;
    ~string_pool_t() = default;

    const s_string& intern(const char* str) {
        uint32_t hash = fnv1a32(str);
        unique_lock<critical_section> lock(&mutex);

        FOR_EACH_NODE(pool, existing) {
            if (fnv1a32(existing.c_str()) == hash && existing == str)
                return existing;
        }

        return pool.push_back_ref(s_string(str));
    }

    const s_string& intern(const s_string& str) {
        return intern(str.c_str());
    }

    bool contains(const s_string& str) const {
        uint32_t hash = fnv1a32(str.c_str());
        FOR_EACH_NODE(pool, existing) {
            if (fnv1a32(existing.c_str()) == hash && existing == str)
                return true;
        }
        return false;
    }

    const s_string* get_name_by_hash(uint32_t hash) const {
        FOR_EACH_NODE(pool, existing) {
            if (fnv1a32(existing.c_str()) == hash)
                return &existing;
        }
        return nullptr;
    }

    void clear() {
        unique_lock<critical_section> lock(&mutex);
        pool.clear();
    }

    size_t size() const { return pool.size(); }

private:
    s_node_list_t<s_string> pool;
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
    ~s_pooled_string() = default;

    s_pooled_string(const char* raw) {
        hash = fnv1a32(raw);
        str = (s_string*)&intern_string(raw);
    }

    const char* c_str() const { return str ? str->c_str() : ""; }
    operator const char* () const { return c_str(); }
    operator const s_string& () const { return *str; }
};
