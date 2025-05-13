#pragma once
#include "s_vector.h"
#include "../engine/threading/thread_system.h"

template <typename T>
struct s_stable_vector_iterator
{
    T* ptr = nullptr;
    size_t index = 0;
};

template <typename T>
class s_performance_vector
{
public:
    s_performance_vector() = default;
    ~s_performance_vector() = default;

    T* push_back(const T& value)
    {
        size_t index;
        if (!free_indices.empty())
        {
            index = free_indices.back();
            free_indices.pop_back();

            data[index] = value;
            alive_bits[index / 64] |= (1ull << (index % 64));
        }
        else
        {
            data.push_back(value);
            index = data.size() - 1;

            const size_t bit_index = index % 64;
            const size_t block_index = index / 64;

            if (block_index >= alive_bits.size())
                alive_bits.push_back(0);

            alive_bits[block_index] |= (1ull << bit_index);
        }

        return &data[index];
    }

    template <typename... Args>
    T* emplace_back(Args&&... args)
    {
        size_t index;
        if (!free_indices.empty())
        {
            index = free_indices.back();
            free_indices.pop_back();

            data[index] = T(std::forward<Args>(args)...);
            alive_bits[index / 64] |= (1ull << (index % 64));
        }
        else
        {
            data.emplace_back(std::forward<Args>(args)...);
            index = data.size() - 1;

            const size_t bit_index = index % 64;
            const size_t block_index = index / 64;

            if (block_index >= alive_bits.size())
                alive_bits.push_back(0);

            alive_bits[block_index] |= (1ull << bit_index);
        }

        return &data[index];
    }
    void remove(size_t index)
    {
        if (index < data.size())
        {
            const size_t bit_index = index % 64;
            const size_t block_index = index / 64;

            alive_bits[block_index] &= ~(1ull << bit_index);
            free_indices.push_back(index);
        }
    }

    void clear()
    {
        data.clear();
        alive_bits.clear();
        free_indices.clear();
    }

    void reserve(size_t n)
    {
        data.reserve(n);
        alive_bits.reserve((n + 63) / 64);
    }

    size_t size() const { return data.size(); }

    bool is_alive(size_t index) const
    {
        const size_t block_index = index / 64;
        const size_t bit_index = index % 64;

        if (block_index >= alive_bits.size())
            return false;

        return (alive_bits[block_index] & (1ull << bit_index)) != 0;
    }

    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    template <typename Fn>
    void for_each_alive(Fn&& fn)
    {
        const size_t n = data.size();
        const size_t block_count = alive_bits.size();
        constexpr size_t BITS_PER_BLOCK = 64;

        T* data_ptr = data.data();

        for (size_t block_idx = 0; block_idx < block_count; ++block_idx)
        {
            uint64_t mask = alive_bits[block_idx];

            if (mask == 0)
                continue;

            if (block_idx + 1 < block_count)
                _mm_prefetch(reinterpret_cast<const char*>(&data_ptr[(block_idx + 1) * BITS_PER_BLOCK]), _MM_HINT_T0);

            while (mask != 0)
            {
                uint32_t bit_pos = _tzcnt_u64(mask);
                size_t index = block_idx * BITS_PER_BLOCK + bit_pos;

                if (index >= n) break;

                fn(data_ptr[index], index);

                mask &= (mask - 1);
            }
        }
    }
    template <typename Fn>
    void parallel_for_each_alive(uint32_t thread_count, size_t chunk_size, Fn&& fn)
    {
        const size_t total = size();
        if (total == 0 || thread_count <= 1 || total <= chunk_size)
        {
            for_each_alive(fn);
            return;
        }

        struct thread_data_t
        {
            s_performance_vector<T>* vec;
            size_t chunk_size;
            std::atomic<size_t>* chunk_index;
            Fn* fn;
        };

        auto worker_fn = [](void* param) -> uint32_t
            {
                auto* data = reinterpret_cast<thread_data_t*>(param);
                auto* vec = data->vec;
                size_t chunk_size = data->chunk_size;
                std::atomic<size_t>& chunk_index = *data->chunk_index;
                Fn& fn = *data->fn;

                const size_t total = vec->size();
                const size_t block_count = vec->alive_bits.size();

                while (true)
                {
                    size_t start = chunk_index.fetch_add(chunk_size);
                    if (start >= total)
                        break;

                    const size_t end = std::min(start + chunk_size, total);
                    const size_t block_start = start / 64;
                    const size_t block_end = (end + 63) / 64;

                    for (size_t block_idx = block_start; block_idx < block_end; ++block_idx)
                    {
                        uint64_t mask = vec->alive_bits[block_idx];
                        if (mask == 0)
                            continue;

                        while (mask != 0)
                        {
                            uint32_t bit_pos = _tzcnt_u64(mask);
                            size_t index = block_idx * 64 + bit_pos;

                            if (index >= end)
                                break;
                            if (index < start)
                            {
                                mask &= (mask - 1);
                                continue;
                            }

                            fn((*vec)[index], index);

                            mask &= (mask - 1);
                        }
                    }
                }

                return 0;
            };

        std::atomic<size_t> chunk_index{ 0 };
        s_vector<thread_t> threads;
        threads.reserve(thread_count);

        thread_data_t thread_data{ this, chunk_size, &chunk_index, &fn };

        for (uint32_t i = 0; i < thread_count; ++i)
        {
            thread_t t;
            t.create(worker_fn, &thread_data);
            threads.push_back(t);
        }

        for (auto& t : threads)
            t.join();
    }


    s_stable_vector_iterator<T> begin()
    {
        const size_t count = data.size();
        for (size_t i = 0; i < count; ++i)
        {
            if (is_alive(i))
                return { &data[i], i };
        }
        return { nullptr, size_t(-1) };
    }

    s_stable_vector_iterator<T> end()
    {
        return { nullptr, size_t(-1) };
    }

    s_stable_vector_iterator<T> next(const s_stable_vector_iterator<T>& current)
    {
        const size_t count = data.size();
        for (size_t i = current.index + 1; i < count; ++i)
        {
            if (is_alive(i))
                return { &data[i], i };
        }
        return { nullptr, size_t(-1) };
    }

private:
    s_vector<T> data;
    s_vector<uint64_t> alive_bits;
    s_vector<size_t> free_indices;
};