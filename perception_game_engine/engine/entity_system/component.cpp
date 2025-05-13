#include "entity_system.h"
#include "../threading/thread_storage.h"

void component_t::destroy()
{
	auto ts = get_current_thread_storage();
	ts->current_layer = owner->owner_layer;
	ts->current_entity = owner;
	ts->current_component = this;

	if (on_destroy)
		on_destroy(owner,_class);
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

	auto ts = get_current_thread_storage();
	ts->current_layer = owner->owner_layer;
	ts->current_entity = owner;

	if (on_create)
		_class = on_create(owner, user_data);

	return _class != nullptr;
}

bool entity_t::add_component(
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
	user_data_t* data)
{
	auto string_hash = fnv1a32(name.c_str());

	const size_t count = components.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		if (components[i].lookup_hash_by_name == string_hash)
		{
			on_fail("Failed to create component, already exists ", name.c_str());
			return false;
		}
	}

	component_t component;
	if (component.construct(this, name, on_create, on_destroy, on_input_receive, on_physics_update, on_frame, on_render, on_render_ui, on_serialize, on_deserialize, on_debug_draw, on_ui_inspector, data))
	{
		components.push_back(component);
		return true;
	}

	on_fail("Failed to create component: ", name.c_str());
	return false;
}

component_t* entity_t::get_component(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());

	const size_t count = components.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		if (components[i].lookup_hash_by_name == hash)
			return &components[i];
	}

	return nullptr;
}


bool entity_t::remove_component(const s_string& name)
{
	auto string_hash = fnv1a32(name.c_str());

	const size_t count = components.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		if (components[i].lookup_hash_by_name == string_hash)
		{
			components[i].destroy();
			components.remove(i);
			return true;
		}
	}

	on_fail("Failed to remove component: %s", name.c_str());
	return false;
}
