#pragma once

#ifdef VMM
#define VMM_API API_EXPORT
#include "heap.h"
#else
#define VMM_API API_IMPORT
#include <datatypes.h>
#endif

extern "C" {

	typedef void* heap_handle_t;

	VMM_API heap_handle_t* hcreate();
	VMM_API void hdestroy(heap_handle_t* heap);

	VMM_API void* _halloc(heap_handle_t* heap, size_t size);
	VMM_API void* _hrealloc(heap_handle_t* heap, void* base, size_t new_size);
	VMM_API void  _hfree(heap_handle_t* heap, void* base);

	VMM_API void* halloc(size_t size);
	VMM_API void* hrealloc(void* base, size_t new_size);
	VMM_API void  hfree(void* base);

	VMM_API void* valloc(size_t size);
	VMM_API void* vrealloc(void* base, size_t new_size);
	VMM_API void  vfree(void* base);
} 
