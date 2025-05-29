#pragma once
#include "vmm.h"

class heap_allocator_t
{
public:

    heap_allocator_t()
    {
		initialize();
    }

    ~heap_allocator_t()
    {
      //  destroy();
    }

public:
    static constexpr size_t default_page_size = 64 * 1024;
    static constexpr size_t block_align_granule = 16;

    void initialize()
    {
        if (mem_pool)
            return;

        mem_pool = &memory_pool;
        page_list = nullptr;
    }

    void* allocate(size_t raw_size)
    {
        pltf_lock_guard lock(heap_lock);
        size_t size = align_up(raw_size, block_align_granule);

        for (page_header_t* pg = page_list; pg; pg = pg->next)
        {
            if (void* p = allocate_in_page(pg, size))
                return p;
        }

        page_header_t* pg = allocate_new_page(max(size + sizeof(block_header_t), default_page_size));
        if (!pg) return nullptr;
        return allocate_in_page(pg, size);
    }

    void  free(void* ptr)
    {
        pltf_lock_guard lock(heap_lock);

        if (!ptr)
            return;

        auto* bh = reinterpret_cast<block_header_t*>((char*)ptr - sizeof(block_header_t));
        bh->used = false;
        coalesce(bh);
    }

    void* realloc(void* ptr, size_t new_size)
    {
        if (!ptr)
            return allocate(new_size);

        if (new_size == 0)
        {
            free(ptr);
            return nullptr;
        }

        heap_lock.lock_exclusive();
        auto* bh = reinterpret_cast<block_header_t*>((char*)ptr - sizeof(block_header_t));
        size_t old_sz = bh->size;
        size_t need = align_up(new_size, block_align_granule);
      
        if (need <= old_sz)
        {
            split_block(bh, need);
            heap_lock.unlock_exclusive();
            return ptr;
        }

        if (!bh->next || !bh->next->used)
        {
            size_t avail = bh->size + (bh->next ? sizeof(block_header_t) + bh->next->size : 0);
            if (avail >= need)
            {
                if (bh->next)
                {
                    block_header_t* nxt = bh->next;
                    bh->size += sizeof(block_header_t) + nxt->size;
                    bh->next = nxt->next;
                }

                split_block(bh, need);
                bh->used = true;
                heap_lock.unlock_exclusive();
                return ptr;
            }
        }

        heap_lock.unlock_exclusive();

        void* newp = allocate(new_size);
        if (!newp) return nullptr;
        memcpy(newp, ptr, old_sz);
        free(ptr);
        return newp;
    }

    void print_stats() 
    {
        pltf_shared_guard lock(heap_lock);

        constexpr size_t page_size = 0x1000; 
        size_t pages_count = 0, blocks_total = 0, blocks_used = 0;
        size_t used_bytes = 0;

        for (page_header_t* pg = page_list; pg; pg = pg->next)
        {
            ++pages_count;

            for (block_header_t* bh = pg->first; bh; bh = bh->next)
            {
                ++blocks_total;
                if (bh->used)
                {
                    ++blocks_used;
                    used_bytes += bh->size;
                }
            }
        }

        size_t committed_bytes = pages_count * page_size;

        std::cout << "--- heap_allocator_t stats ---\n";
        std::cout << " pages:        " << pages_count << "\n";
        std::cout << " blocks total: " << blocks_total << "\n";
        std::cout << " blocks used:  " << blocks_used << "\n";
        std::cout << " committed:    " << (committed_bytes / (1024 * 1024)) << " MB\n";
        std::cout << " used:         " << (used_bytes / (1024 * 1024.0)) << " MB\n";
        std::cout << "------------------------------\n";
    }

    void destroy()
    {
        pltf_lock_guard lock(heap_lock);

        while (page_list)
        {
            page_header_t* pg = page_list;
            page_list = pg->next;

            if (mem_pool && pg->base)
                mem_pool->free(pg->base);
        }

        page_list = nullptr;
        mem_pool = nullptr;
    }


private:
    struct block_header_t {
        block_header_t* next;
        size_t       size;
        bool         used;
    };

    struct page_header_t {
        page_header_t* next;
        block_header_t* first;
        void* base;
        size_t        capacity;
    };

    virtual_memory_pool* mem_pool = nullptr;
    page_header_t* page_list = nullptr;
    pltf_mutex heap_lock;

    static size_t align_up(size_t v, size_t a)
    {
        return (v + a - 1) & ~(a - 1);
    }

    page_header_t* allocate_new_page(size_t want)
    {
        void* raw = mem_pool->allocate(align_up(want + sizeof(page_header_t), default_page_size));
        if (!raw)
            return nullptr;

        auto* pg = reinterpret_cast<page_header_t*>(raw);
        pg->base = raw;
        pg->next = page_list;
        pg->capacity = align_up(want + sizeof(page_header_t), default_page_size) - sizeof(page_header_t);
        page_list = pg;

        auto* bh = reinterpret_cast<block_header_t*>((char*)raw + sizeof(page_header_t));
        bh->next = nullptr;
        bh->size = pg->capacity - sizeof(block_header_t);
        bh->used = false;
        pg->first = bh;

        return pg;
    }

    void* allocate_in_page(page_header_t* pg, size_t size)
    {
        for (block_header_t* bh = pg->first; bh; bh = bh->next)
        {
            if (!bh->used && bh->size >= size)
            {
                split_block(bh, size);
                bh->used = true;
                return (char*)bh + sizeof(block_header_t);
            }
        }
        return nullptr;
    }

    void split_block(block_header_t* bh, size_t want)
    {
        if (bh->size >= want + sizeof(block_header_t) + block_align_granule)
        {
            auto* tail = reinterpret_cast<block_header_t*>(
                (char*)bh + sizeof(block_header_t) + want
                );
            tail->size = bh->size - want - sizeof(block_header_t);
            tail->used = false;
            tail->next = bh->next;

            bh->size = want;
            bh->next = tail;
        }
    }

    void coalesce(block_header_t* freed)
    {
        if (freed->next && !freed->next->used)
        {
            freed->size += sizeof(block_header_t) + freed->next->size;
            freed->next = freed->next->next;
        }

        for (page_header_t* pg = page_list; pg; pg = pg->next)
        {
            block_header_t* prev = nullptr;
            for (block_header_t* bh = pg->first; bh; bh = bh->next)
            {
                if (bh == freed)
                {
                    if (prev && !prev->used)
                    {
                        prev->size += sizeof(block_header_t) + freed->size;
                        prev->next = freed->next;
                        freed = prev;
                    }
                    break;
                }
                prev = bh;
            }
        }

        for (page_header_t* pg = page_list, *prev_pg = nullptr; pg; prev_pg = pg, pg = pg->next)
        {
            bool all_free = true;
            for (block_header_t* bh = pg->first; bh; bh = bh->next)
            {
                if (bh->used)
                {
                    all_free = false;
                    break;
                }
            }

            if (all_free)
            {
                if (prev_pg)
                    prev_pg->next = pg->next;
                else
                    page_list = pg->next;

                mem_pool->free(pg->base);
                return;
            }
        }
    }

};

inline heap_allocator_t general_heap;