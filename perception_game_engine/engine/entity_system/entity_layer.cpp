#include "entity_system.h"
#include "../threading/thread_storage.h"

void entity_layer_t::init_layer(const s_string& n)
{
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;
}

void entity_layer_t::create_entity(
	const s_string& name,
	construct_fn_t on_create,
	destruct_fn_t on_destroy,
	on_input_receive_fn_t on_input_receive,
	on_physics_update_fn_t on_physics_update,
	on_frame_fn_t on_frame,
	on_render_fn_t on_render,
	on_render_ui_fn_t on_render_ui,
	on_serialize_fn_t on_serialize,
	on_deserialize_fn_t on_deserialize,
	on_debug_draw_fn_t on_debug_draw,
	on_ui_inspector_fn_t on_ui_inspector,
	user_data_t* data 
)
{
	entity_t entity;
	
	if (entity.construct(this, name, on_create, on_destroy, on_input_receive, on_physics_update, on_frame, on_render, on_render_ui, on_serialize, on_deserialize, on_debug_draw, on_ui_inspector, data)) {
		entities.push_back(entity);
		return;
	}

	on_fail("Failed to create entity: %s", name.c_str());
}

entity_t* entity_layer_t::get_entity_by_class(class_t* class_ptr)
{
	for (auto& entity : entities) {
		if (entity._class == class_ptr) 
				return &entity;
	}
	return nullptr;
}

entity_t* entity_layer_t::get_entity_by_name(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());
	for (auto& entity : entities) 
			if (entity.lookup_hash_by_name == hash) 
				return &entity;
	
	return nullptr;
}

s_vector<entity_t*> entity_layer_t::get_entities_by_name(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());

	s_vector<entity_t*> result;

	for (auto& entity : entities) 
			if (entity.lookup_hash_by_name == hash) 
				result.push_back(&entity);
			
		
	

	return result;
}

bool entity_layer_t::remove_entity_by_class(class_t* class_ptr)
{
	for (size_t i = 0; i < entities.count(); ++i) {
		if (entities[i]._class == class_ptr) {
			entities[i].destroy();
			entities.erase_at(i);
			return true;
		}
	}
	
	return false;
}

void entity_layer_t::destroy()
{
	auto ts = get_current_thread_storage();
	ts->current_layer = this;

	for (auto& entity : entities) {
		ts->current_entity = &entity;
		entity.destroy();
	}
}