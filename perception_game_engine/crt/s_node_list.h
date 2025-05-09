#pragma once

#include <Windows.h>
#include <functional>

#define FOR_EACH_NODE(list, var) for (auto* _n = (list).begin(); _n != (list).end(); _n = _n->next) if (auto& var = _n->value; true)

template<typename T>
struct node_t {
    T value;
    node_t* prev = nullptr;
    node_t* next = nullptr;
};

template<typename T>
class s_node_list_t {
public:
    s_node_list_t() = default;
    ~s_node_list_t() { clear(); }

    T* push_back(const T& value) {
        auto* n = new node_t<T>{ value };
        if (!head) head = tail = n;
        else {
            tail->next = n;
            n->prev = tail;
            tail = n;
        }
        ++count;
        return &n->value;
    }

    T& push_back_ref(const T& value) {
        auto* n = new node_t<T>{ value };
        if (!head) head = tail = n;
        else {
            tail->next = n;
            n->prev = tail;
            tail = n;
        }
        ++count;
        return n->value;
    }

    T* push_front(const T& value) {
        auto* n = new node_t<T>{ value };
        if (!head) {
            head = tail = n;
        }
        else {
            n->next = head;
            head->prev = n;
            head = n;
        }
        ++count;
        return &n->value;
    }

    void pop_front() {
        if (!head) return;
        auto* next = head->next;
        delete head;
        head = next;
        if (head) head->prev = nullptr;
        else tail = nullptr;
        --count;
    }

    void pop_back() {
        if (!tail) return;
        auto* prev = tail->prev;
        delete tail;
        tail = prev;
        if (tail) tail->next = nullptr;
        else head = nullptr;
        --count;
    }

    bool remove(const T& value) {
        for (auto* n = head; n; n = n->next) {
            if (n->value == value) {
                unlink_node(n);
                delete n;
                --count;
                return true;
            }
        }
        return false;
    }

    node_t<T>* insert_after(node_t<T>* node, const T& value) {
        if (!node) return nullptr;
        auto* n = new node_t<T>{ value };
        n->prev = node;
        n->next = node->next;
        if (node->next) node->next->prev = n;
        else tail = n;
        node->next = n;
        ++count;
        return n;
    }

    node_t<T>* insert_before(node_t<T>* node, const T& value) {
        if (!node) return nullptr;
        auto* n = new node_t<T>{ value };
        n->next = node;
        n->prev = node->prev;
        if (node->prev) node->prev->next = n;
        else head = n;
        node->prev = n;
        ++count;
        return n;
    }

    node_t<T>* find(const T& value) const {
        for (auto* n = head; n; n = n->next) {
            if (n->value == value)
                return n;
        }
        return nullptr;
    }

    bool contains(const T& value) const {
        return find(value) != nullptr;
    }

    void clear() {
        while (head) {
            auto* next = head->next;
            delete head;
            head = next;
        }
        tail = nullptr;
        count = 0;
    }

    void for_each(std::function<void(T&)> func) {
        for (auto* n = head; n; n = n->next)
            func(n->value);
    }
    
    void remove_node(node_t<T>* node) {
        if (!node) return;
        unlink_node(node);
        delete node;
        --count;
    }

    node_t<T>* begin() const { return head; }
    node_t<T>* rbegin() const { return tail; }
    node_t<T>* end() const { return nullptr; }

    size_t size() const { return count; }
    bool is_empty() const { return count == 0; }

private:
    void unlink_node(node_t<T>* n) {
        if (n->prev) n->prev->next = n->next;
        else head = n->next;

        if (n->next) n->next->prev = n->prev;
        else tail = n->prev;
    }

private:
    node_t<T>* head = nullptr;
    node_t<T>* tail = nullptr;
    size_t count = 0;
};
