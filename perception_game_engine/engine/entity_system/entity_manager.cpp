#include "entity_manager.h"

void entity_manager_t::init_manager(const s_string& n)
{
	name = n;
	lookup_hash_by_name = fnv1a32(n.c_str());
}

void entity_manager_t::create_entity(
	uint32_t type,
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
	
	if (entity.construct(type, name, on_create, on_destroy, on_input_receive, on_physics_update, on_frame, on_render, on_render_ui, on_serialize, on_deserialize, on_debug_draw, on_ui_inspector, data)) {
		auto& entity_list = entities[type];
		entity_list.push_back(entity);
		return;
	}

	on_fail("Failed to create entity: %s", name.c_str());
}

entity_t* entity_manager_t::get_entity_by_class(class_t* class_ptr)
{
	for (auto& entity : entities) {
		for (auto& e : entity.value) {
			if (e._class == class_ptr) {
				return &e;
			}
		}
	}
	return nullptr;
}

entity_t* entity_manager_t::get_entity_by_name(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());
	for (auto& entity : entities) {
		for (auto& e : entity.value) {
			if (e.lookup_hash_by_name == hash) {
				return &e;
			}
		}
	}
	return nullptr;
}

s_vector<entity_t*> entity_manager_t::get_entities_by_name(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());

	s_vector<entity_t*> result;

	for (auto& entity : entities) {
		for (auto& e : entity.value) {
			if (e.lookup_hash_by_name == hash) {
				result.push_back(&e);
			}
		}
	}

	return result;
}

s_vector<entity_t*> entity_manager_t::get_entities_of_type(uint32_t type)
{
	s_vector<entity_t*> result;
	auto it = entities.find(type);
	if (!it) return result;

	for (int i = 0; i < it->count(); i++)
		result.push_back(&it->at(i));

	return result;
}

bool entity_manager_t::remove_entity_by_class(class_t* class_ptr)
{
	for (auto& pair : entities) {
		auto& vec = pair.value;
		for (size_t i = 0; i < vec.count(); ++i) {
			if (vec[i]._class == class_ptr) {
				vec[i].destroy();
				vec.erase_at(i);
				return true;
			}
		}
	}
	return false;
}

void entity_manager_t::destroy()
{
	for (auto& entity : entities) {
		for (auto& e : entity.value) {
			e.destroy();
		}
	}
}