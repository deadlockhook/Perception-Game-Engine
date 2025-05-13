#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_string_pool.h"

void entity_t::destroy()
{
	detach();
	auto ts = get_current_thread_storage();
	ts->current_layer = owner_layer;
	ts->current_entity = this;

	const size_t component_count = components.size();
	for (size_t i = 0; i < component_count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		components[i].destroy();
	}

	if (on_destroy)
		on_destroy(this, _class);

	components.clear();
}

bool entity_t::construct(entity_layer_t* o,
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
) {
	owner_layer = o;
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
	ts->current_layer = owner_layer;
	ts->current_entity = this;
	if (on_create)
		_class = on_create(this,user_data);

	return true;
}

void entity_t::attach_to(entity_t* e)
{
	if (!e || e == this || parent == e)
		return;

	const size_t child_count = children.size();
	for (size_t i = 0; i < child_count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		if (children[i] == e)
		{
			children.remove(i);
			e->parent = nullptr;
			break;
		}
	}

	if (parent)
	{
		auto& siblings = parent->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i] == this)
			{
				siblings.remove(i);
				break;
			}
		}

		parent = nullptr;
	}

	const size_t e_child_count = e->children.size();
	for (size_t i = 0; i < e_child_count; ++i)
	{
		if (!e->children.is_alive(i))
			continue;

		if (e->children[i] == this)
		{
			e->children.remove(i);
			break;
		}
	}

	parent = e;
	e->children.push_back(this);
}


void entity_t::detach()
{
	if (parent)
	{
		auto& siblings = parent->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i] == this)
			{
				siblings.remove(i);
				break;
			}
		}

		parent = nullptr;
	}
}



entity_t* entity_t::get_root() {
	entity_t* current = this;
	while (current->parent)
		current = current->parent;
	return current;
}


void entity_t::for_each_child_recursive(thread_storage_t* storage, entity_iter_fn_t fn, void* userdata)
{
	const size_t count = children.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		entity_t* child = children[i];
		fn(storage, child, userdata);

		child->for_each_child_recursive(storage, fn, userdata);
	}
}
