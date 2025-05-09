#include "entity_system.h"
#include "../threading/thread_storage.h"

void component_t::destroy()
{
	auto ts = get_current_thread_storage();
	ts->current_layer = owner->owner_layer;
	ts->current_entity = owner;
	ts->current_component = this;

	__try {

		if (on_destroy)
			on_destroy(_class);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		on_fail("Component destructor failed! %s ", name.c_str()); //add more proper logging
	}
}

bool component_t::construct(
	entity_t* _owner,
	const s_string& n,
	construct_fn_t _on_create,
	destruct_fn_t _on_destroy,
	on_input_receive_fn_t _on_input_receive,
	on_physics_update_fn_t _on_physics_update,
	on_frame_fn_t _on_frame,
	on_render_fn_t _on_render,
	on_render_ui_fn_t _on_render_ui,
	on_serialize_fn_t _on_serialize,
	on_deserialize_fn_t _on_deserialize,
	on_debug_draw_fn_t _on_debug_draw,
	on_ui_inspector_fn_t _on_ui_inspector,
	user_data_t* data
)
{
	owner = _owner;
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;

	user_data = data;

	on_create = _on_create;
	on_destroy = _on_destroy;
	on_input_receive = _on_input_receive;
	on_physics_update = _on_physics_update;
	on_frame = _on_frame;
	on_render = _on_render;
	on_render_ui = _on_render_ui;
	on_serialize = _on_serialize;
	on_deserialize = _on_deserialize;
	on_debug_draw = _on_debug_draw;
	on_ui_inspector = _on_ui_inspector;

	__try {
		auto ts = get_current_thread_storage();
		ts->current_layer = owner->owner_layer;
		ts->current_entity = owner;
		if (on_create)
			_class = on_create(user_data);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		on_fail("Entity constructor failed: %s", name.c_str());
		_class = nullptr;
		return false;
	}

	return _class != nullptr;
}

bool entity_t::add_component(
	const s_string& name,
	construct_fn_t on_create = nullptr,
	destruct_fn_t on_destroy = nullptr,
	on_input_receive_fn_t on_input_receive = nullptr,
	on_physics_update_fn_t on_physics_update = nullptr,
	on_frame_fn_t on_frame = nullptr,
	on_render_fn_t on_render = nullptr,
	on_render_ui_fn_t on_render_ui = nullptr,
	on_serialize_fn_t on_serialize = nullptr,
	on_deserialize_fn_t on_deserialize = nullptr,
	on_debug_draw_fn_t on_debug_draw = nullptr,
	on_ui_inspector_fn_t on_ui_inspector = nullptr,
	user_data_t* data = nullptr
)
{
	auto string_hash = fnv1a32(name.c_str());

	for (auto& c : components) {
		if (c.lookup_hash_by_name == string_hash)
		{
			on_fail("Failed to create component: %s, already exists", name.c_str());
			return false;
		}
	}

	component_t component;
	if (component.construct(this, name, on_create, on_destroy, on_input_receive, on_physics_update, on_frame, on_render, on_render_ui, on_serialize, on_deserialize, on_debug_draw, on_ui_inspector, data)) {
		components.push_back(component);
		return true;
	}
	on_fail("Failed to create component: %s", name.c_str());
	return true;
}

bool entity_t::remove_component(const s_string& name)
{
	auto string_hash = fnv1a32(name.c_str());

	for (size_t i = 0; i < components.count(); ++i) {
		if (components[i].lookup_hash_by_name == string_hash) {
			components[i].destroy();
			components.erase_at(i);
			return true;
		}
	}

	on_fail("Failed to remove component: %s", name.c_str());
	return false;
}