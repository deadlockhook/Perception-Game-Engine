#pragma once

typedef class entity_t;
typedef class entity_layer_t;
typedef class component_t;
struct thread_storage_t
{
	entity_t* current_entity = nullptr;
	entity_layer_t* current_layer = nullptr;
	component_t* current_component = nullptr;
};
thread_storage_t* get_current_thread_storage();

entity_t* get_current_entity();
entity_layer_t* get_current_layer();
