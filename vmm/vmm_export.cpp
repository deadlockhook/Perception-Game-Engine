#include "vmm_export.h"

heap_handle_t* hcreate() {
	heap_allocator_t* heap = (heap_allocator_t*)general_heap.allocate(sizeof(heap_allocator_t));
	*heap = heap_allocator_t();
	return (heap_handle_t*)heap;
}

void hdestroy(heap_handle_t* heap) {
	if (heap) {
		heap_allocator_t* h = (heap_allocator_t*)*heap;
		h->destroy();
		general_heap.free(h);
		*heap = nullptr;
	}
}

void* _halloc(heap_handle_t* heap, size_t size) {
	heap_allocator_t* h = (heap_allocator_t*)*heap;
	return h->allocate(size);
}

void* _hrealloc(heap_handle_t* heap, void* base, size_t new_size) {
	heap_allocator_t* h = (heap_allocator_t*)*heap;
	return h->realloc(base, new_size);
}

void _hfree(heap_handle_t* heap, void* base) {
	heap_allocator_t* h = (heap_allocator_t*)*heap;
	return h->free(base);
}

void* halloc(size_t size) {
	return general_heap.allocate(size);
}

void* hrealloc(void* base, size_t new_size) {
	return general_heap.realloc(base, new_size);
}

void hfree(void* base) {
	return general_heap.free(base);
}

void* valloc(size_t size) {
	return memory_pool.allocate(size);
}

void vfree(void* base) {
	return memory_pool.free(base);
}

void* vrealloc(void* base, size_t new_size) {
	return memory_pool.realloc(base, new_size);
}


