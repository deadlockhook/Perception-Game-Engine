#pragma once
#include "s_vector.h"

template<typename key_t, typename value_t>
class s_map {
public:
    struct map_entry_t {
        key_t key;
        value_t value;
    };

    s_map() = default;

    void insert(const key_t& key, const value_t& value) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key) {
                m_data[i].value = value;
                return;
            }
        }
        m_data.push_back({ key, value });
    }

    bool get(const key_t& key, value_t& out_value) const {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key) {
                out_value = m_data[i].value;
                return true;
            }
        }
        return false;
    }

    template<typename... args_t>
    value_t& emplace(const key_t& key, args_t&&... args) {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return m_data[i].value;

        m_data.push_back({ key, value_t(std::forward<args_t>(args)...) });
        return m_data.back().value;
    }

    template<typename cmp_t>
    void sort(cmp_t cmp) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            for (size_t j = i + 1; j < m_data.count(); ++j) {
                if (cmp(m_data[j], m_data[i]))
                    std::swap(m_data[i], m_data[j]);
            }
        }
    }

    bool contains(const key_t& key) const {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key)
                return true;
        }
        return false;
    }

    void erase(const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key) {
                m_data.erase_at(i);
                return;
            }
        }
    }

    void clear() {
        m_data.clear();
    }

    size_t count() const {
        return m_data.count();
    }

    size_t find_index(const key_t& key) const {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return i;
        return static_cast<size_t>(-1);
    }

    void merge(const s_map<key_t, value_t>& other) {
        for (size_t i = 0; i < other.count(); ++i)
            insert(other.begin()[i].key, other.begin()[i].value);
    }

    s_vector<key_t> keys() const {
        s_vector<key_t> result;
        for (size_t i = 0; i < m_data.count(); ++i)
            result.push_back(m_data[i].key);
        return result;
    }

    s_vector<value_t> values() const {
        s_vector<value_t> result;
        for (size_t i = 0; i < m_data.count(); ++i)
            result.push_back(m_data[i].value);
        return result;
    }

    value_t& operator[](const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key)
                return m_data[i].value;
        }

        m_data.push_back({ key_t(key), value_t{} });
        return m_data.back().value;
    }

    value_t& at(const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return m_data[i].value;
        static value_t default_val{};
        return default_val;
    }

    const value_t& at(const key_t& key) const {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return m_data[i].value;
        static value_t default_val{};
        return default_val;
    }

    bool is_empty() const {
        return m_data.count() == 0;
    }

    void swap(s_map<key_t, value_t>& other) {
        m_data.swap(other.m_data);
    }

    template<typename func_t>
    void for_each(func_t fn) {
        for (size_t i = 0; i < m_data.count(); ++i)
            fn(m_data[i].key, m_data[i].value);
    }

    template<typename pred_t>
    void remove_if(pred_t predicate) {
        for (size_t i = 0; i < m_data.count();) {
            if (predicate(m_data[i].key, m_data[i].value)) {
                m_data.erase_at(i);
            }
            else {
                ++i;
            }
        }
    }

    value_t& ensure(const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return m_data[i].value;

        return insert_and_return(key, value_t{});
    }

    value_t& insert_and_return(const key_t& key, const value_t& value) {
        m_data.push_back({ key, value });
        return m_data.back().value;
    }

    bool extract(const key_t& key, value_t& out_value) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].key == key) {
                out_value = m_data[i].value;
                m_data.erase_at(i);
                return true;
            }
        }
        return false;
    }

    s_map<key_t, value_t> clone() const {
        s_map<key_t, value_t> result;
        for (size_t i = 0; i < m_data.count(); ++i)
            result.insert(m_data[i].key, m_data[i].value);
        return result;
    }

    bool equals(const s_map<key_t, value_t>& other) const {
        if (count() != other.count()) return false;
        for (size_t i = 0; i < m_data.count(); ++i) {
            const key_t& k = m_data[i].key;
            if (!other.contains(k) || other.at(k) != m_data[i].value)
                return false;
        }
        return true;
    }

    void reserve(size_t count) {
        m_data.reserve(count);
    }
    
    void shrink_to_fit() {
        m_data.shrink_to_fit();
    }

    bool empty() const { return is_empty(); }
    size_t capacity() const { return m_data.capacity(); }

    bool remove_by_value(const value_t& value) {
        for (size_t i = 0; i < m_data.count(); ++i) {
            if (m_data[i].value == value) {
                m_data.erase_at(i);
                return true;
            }
        }
        return false;
    }

    value_t* find(const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return &m_data[i].value;
        return nullptr;
    }

    const value_t* find(const key_t& key) const {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return &m_data[i].value;
        return nullptr;
    }

    size_t remove_all_by_value(const value_t& value) {
        size_t removed = 0;
        for (size_t i = 0; i < m_data.count();) {
            if (m_data[i].value == value) {
                m_data.erase_at(i);
                ++removed;
            }
            else {
                ++i;
            }
        }
        return removed;
    }

    map_entry_t& operator[](size_t index) { return m_data[index]; }
    const map_entry_t& operator[](size_t index) const { return m_data[index]; }

    s_map<key_t, value_t>& operator+=(const s_map<key_t, value_t>& other) {
        merge(other);
        return *this;
    }

    bool operator==(const s_map<key_t, value_t>& other) const {
        return equals(other);
    }

    bool operator!=(const s_map<key_t, value_t>& other) const {
        return !(*this == other);
    }

    value_t* try_get(const key_t& key) {
        for (size_t i = 0; i < m_data.count(); ++i)
            if (m_data[i].key == key)
                return &m_data[i].value;
        return nullptr;
    }

    map_entry_t* rbegin() { return m_data.end() - 1; }
    map_entry_t* rend() { return m_data.begin() - 1; }

    const map_entry_t* rbegin() const { return m_data.end() - 1; }
    const map_entry_t* rend() const { return m_data.begin() - 1; }

    value_t& first() { return m_data[0].value; }
    value_t& last() { return m_data[m_data.count() - 1].value; }
 
    const map_entry_t* cbegin() const { return m_data.begin(); }
    const map_entry_t* cend() const { return m_data.end(); }

    size_t size() const { return m_data.count(); }

    map_entry_t* begin() { return m_data.begin(); }
    map_entry_t* end() { return m_data.end(); }
    const map_entry_t* begin() const { return m_data.begin(); }
    const map_entry_t* end() const { return m_data.end(); }

private:
    s_vector<map_entry_t> m_data;
};

