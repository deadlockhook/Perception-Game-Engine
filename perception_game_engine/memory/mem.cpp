#include "mem.h"
#include <iostream>

v_mem v_mem::allocate_pool(memory_pool_type type, size_t size, bool zero_on_destruction)
{
	if (size == 0 || size == ULLONG_MAX)
		return {};

	if (size < 0x1000)
		size = PAGE_SIZE_4KB;

	uint32_t protection = 0;
	bool paged = false;

	switch (type)
	{
	case paged_pool:
		protection = PAGE_READWRITE;
		paged = true;
		break;
	case paged_pool_execute:
		protection = PAGE_EXECUTE_READWRITE;
		paged = true;
		break;
	case non_paged_pool:
		protection = PAGE_READWRITE;
		break;
	case non_paged_pool_execute:
		protection = PAGE_EXECUTE_READWRITE;
		break;
	default:
		return {};
	}

	void* memory = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, protection);

	if (!memory)
		return {};

	if (!paged)
	{
		if (!VirtualLock(memory, size)) {
			VirtualFree(memory, 0, MEM_RELEASE);
			return {};
		}
	}

	return v_mem(memory, size, type, paged, zero_on_destruction);
}

void v_mem::zero_and_free()
{
	if (memory && size)
		RtlSecureZeroMemory(memory, size);

	free();
}

void v_mem::free()
{
	if (memory)
	{
		__try {
			if (!paged)
				VirtualUnlock(memory, size);

			if (zero_on_destruction)
				RtlSecureZeroMemory(memory, size);

			VirtualFree(memory, 0, MEM_RELEASE);
			memory = nullptr;
			size = 0;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			memory = nullptr;
			size = 0;
		}
	}
}

bool v_heap_mem::resize(size_t new_size)
{
	if (new_size == 0 || new_size == ULLONG_MAX)
		return false;

	if (!memory)
		return allocate(new_size);

	void* new_memory = HeapReAlloc(heap, 0, memory, new_size);

	if (!new_memory)
		return false;

	memory = new_memory;
	size = new_size;
	return true;
}
bool v_heap_mem::allocate(size_t size)
{
	if (size == 0 || size == ULLONG_MAX)
		return false;

	if (memory)
		free();

	heap = GetProcessHeap();
	if (!heap)
		return false;

	memory = HeapAlloc(heap, 0, size);
	this->size = size;
	//std::cout << " new memory allocated " << memory << std::endl;
	return memory != nullptr;
}
void v_heap_mem::zero()
{
	if (memory && size)
		RtlSecureZeroMemory(memory, size);
}

void v_heap_mem::clear()
{
	if (memory && size)
		RtlSecureZeroMemory(memory, size);
}

void v_heap_mem::free()
{
	if (memory && heap)
	{
		//std::cout << " free memory " << memory << std::endl;

		clear();
		HeapFree(heap, 0, memory);
		memory = nullptr;
		heap = nullptr;
		size = 0;
	}
}