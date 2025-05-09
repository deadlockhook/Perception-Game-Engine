#include "thread_storage.h"
#include "../../crt/s_map.h"

s_map<ULONG, thread_storage_t> threads_storage;

thread_storage_t* get_current_thread_storage()
{
	return &threads_storage[GetCurrentThreadId()];
}
component_t* get_current_component()
{
	thread_storage_t* ts = get_current_thread_storage();

	if (ts)
		return ts->current_component;
	return nullptr;
}
entity_t* get_current_entity()
{
	thread_storage_t* ts = get_current_thread_storage();
	
	if (ts)
		return ts->current_entity;

	return nullptr;
}

entity_layer_t* get_current_layer()
{
	thread_storage_t* ts = get_current_thread_storage();
	
	if (ts)
		return ts->current_layer;

	return nullptr;
}