#pragma once

class entity_t;
class entity_layer_t;
class component_t;
class level_t;
struct thread_storage_t
{
	level_t* current_level = nullptr;
	entity_layer_t* current_layer = nullptr;
	entity_t* current_entity = nullptr;
	component_t* current_component = nullptr;

};
thread_storage_t* get_current_thread_storage();

entity_t* get_current_entity();
component_t* get_current_component();
entity_layer_t* get_current_layer();
