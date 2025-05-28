#pragma once
#include <datatypes.h>
#include <mtx.h>
#include <io.h>
#include <mem.h>

constexpr int _page_size = 0x1000;

struct page_info_t
{
    void* base_address = nullptr;
    ul64 size_in_pages = 0;
};

class virtual_memory_pool
{
public:
    virtual_memory_pool()
    {
        if (!initialize())
        {
            //throw some error or smth
        }
    }
public:
    bool initialize()
    {
        if (pool)
            return true;

        MEMORYSTATUSEX memStatus = { sizeof(memStatus) };
        GlobalMemoryStatusEx(&memStatus);

        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);

        ul64 granularity = sysInfo.dwAllocationGranularity;
        ul64 aligned_size = (memStatus.ullTotalPhys + granularity - 1) & ~(granularity - 1);

        total_reserved_size = aligned_size;
        pool = virtual_alloc_reserve(nullptr, aligned_size);

        if (!pool)
            return false;

        page_count = total_reserved_size / _page_size;

        metadata_size = page_count * sizeof(page_info_t);
        metadata_page_count = (metadata_size + _page_size - 1) / _page_size;

        pages = reinterpret_cast<page_info_t*>(pool);
        void* committed = virtual_alloc_commit(pages, metadata_page_count * _page_size);

        if (!committed)
        {
            virtual_free_release(pool);
            pool = nullptr;
            return false;
        }

        ZeroMemory(pages, metadata_page_count * _page_size);

        return true;
    }

    void* realloc(void* ptr, ul64 new_size)
    {
        if (!ptr)
            return allocate(new_size);

        if (new_size == 0)
        {
            free(ptr);
            return nullptr;
        }

        mgr_lock.lock_exclusive();

        ul64 offset = static_cast<char*>(ptr) - static_cast<char*>(pool);
        ul64 page_index = offset / _page_size;

        if (page_index < metadata_page_count || page_index >= page_count) {
            mgr_lock.unlock_exclusive();
            return nullptr;
        }

        ul64 old_page_count = pages[page_index].size_in_pages;

        if (old_page_count == 0) {
            mgr_lock.unlock_exclusive();
            return nullptr;
        }

        ul64 new_page_count = (new_size + _page_size - 1) / _page_size;

        if (new_page_count <= old_page_count)
        {
            mgr_lock.unlock_exclusive();
            return ptr;
        }

        bool can_extend = true;
        for (ul64 i = 0; i < (new_page_count - old_page_count); ++i)
        {
            if (pages[page_index + old_page_count + i].base_address != nullptr)
            {
                can_extend = false;
                break;
            }
        }

        if (can_extend)
        {
            void* extend_start = static_cast<char*>(pool) + (page_index + old_page_count) * _page_size;
            ul64 extra_size = (new_page_count - old_page_count) * _page_size;

            void* committed = virtual_alloc_commit(extend_start, extra_size);

            if (!committed) {
                mgr_lock.unlock_exclusive();
                return nullptr;
            }


            for (ul64 i = 0; i < (new_page_count - old_page_count); ++i)
                pages[page_index + old_page_count + i].base_address = ptr;

            pages[page_index].size_in_pages = new_page_count;

            mgr_lock.unlock_exclusive();
            return ptr;
        }

        mgr_lock.unlock_exclusive();

        void* new_ptr = allocate(new_size);

        if (!new_ptr) {
            return nullptr;
        }

        ul64 old_size = old_page_count * _page_size;
        memcpy(new_ptr, ptr, old_size);
        free(ptr);

        return new_ptr;
    }

    void* allocate(ul64 size)
    {
        pltf_lock_guard lock(mgr_lock);

        if (!pool || size == 0)
            return nullptr;

        ul64 aligned_size = (size + _page_size - 1) & ~(_page_size - 1);
        ul64 required_pages = aligned_size / _page_size;

        if (required_pages + metadata_page_count > page_count)
            return nullptr;

        for (ul64 i = metadata_page_count; i <= page_count - required_pages; ++i)
        {
            bool all_free = true;
            for (ul64 j = 0; j < required_pages; ++j)
            {
                if (pages[i + j].base_address != nullptr)
                {
                    all_free = false;
                    i += j;
                    break;
                }
            }

            if (all_free)
            {
                void* base = static_cast<char*>(pool) + i * _page_size;
                void* committed = virtual_alloc_commit(base, aligned_size);

                if (!committed)
                {
                    return nullptr;
                }

                for (ul64 j = 0; j < required_pages; ++j)
                    pages[i + j].base_address = base;

                pages[i].size_in_pages = required_pages;

                return base;
            }
        }

        return nullptr;
    }

    bool shrink(void* ptr, ul64 new_size)
    {
        pltf_lock_guard lock(mgr_lock);

        if (!ptr || new_size == 0)
            return false;

        ul64 offset = static_cast<char*>(ptr) - static_cast<char*>(pool);
        ul64 page_index = offset / _page_size;

        if (page_index < metadata_page_count || page_index >= page_count)
            return false;

        ul64 current_pages = pages[page_index].size_in_pages;

        if (current_pages == 0)
            return false;

        ul64 new_pages = (new_size + _page_size - 1) / _page_size;

        if (new_pages >= current_pages)
            return true;

        ul64 pages_to_free = current_pages - new_pages;
        void* decommit_base = static_cast<char*>(ptr) + new_pages * _page_size;

        if (!decomit(decommit_base, pages_to_free * _page_size))
            return false;

        for (ul64 i = new_pages; i < current_pages; ++i)
            pages[page_index + i].base_address = nullptr;

        pages[page_index].size_in_pages = new_pages;

        return true;
    }

    void free(void* address)
    {
        pltf_lock_guard lock(mgr_lock);

        if (!address || !pool)
            return;

        ul64 offset = static_cast<char*>(address) - static_cast<char*>(pool);
        ul64 page_index = offset / _page_size;

        if (page_index < metadata_page_count || page_index >= page_count || pages[page_index].base_address != address)
            return;

        ul64 count = pages[page_index].size_in_pages;

        if (count == 0)
            return;

        for (ul64 i = 0; i < count; ++i)
            pages[page_index + i].base_address = nullptr;

        pages[page_index].size_in_pages = 0;

        decomit(address, count * _page_size);
    }
    void get_stats()
    {
        pltf_shared_guard lock(mgr_lock);

        ul64 committed_pages = 0;

        for (ul64 i = metadata_page_count; i < page_count; ++i)
        {
            if (pages[i].base_address != nullptr)
                committed_pages++;
        }

        ul64 metadata_bytes = metadata_page_count * _page_size;
        ul64 committed_bytes = committed_pages * _page_size;
        ul64 total_reserved_bytes = total_reserved_size;

        std::cout << "\n=== Memory Pool Stats ===\n";
        std::cout << "Total reserved: " << total_reserved_bytes / (1024 * 1024) << " MB (" << total_reserved_bytes << " bytes)\n";
        std::cout << "Committed:      " << committed_bytes / (1024 * 1024) << " MB (" << committed_pages << " pages, " << committed_bytes << " bytes)\n";
        std::cout << "Free pages:     " << (page_count - committed_pages - metadata_page_count) << "\n";
        std::cout << "Metadata:       " << metadata_bytes / 1024 << " KB (" << metadata_page_count << " pages, " << metadata_bytes << " bytes)\n";
        std::cout << "===========================\n";
    }
private:
    void* pool = nullptr;
    page_info_t* pages = nullptr;
    pltf_mutex mgr_lock;
    ul64 page_count = 0;
    ul64 metadata_page_count = 0;
    ul64 metadata_size = 0;
    ul64 total_reserved_size = 0;
};
