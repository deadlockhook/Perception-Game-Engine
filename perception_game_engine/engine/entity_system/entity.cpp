#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_string_pool.h"
#include "../../crt/s_node_list.h"

void entity_t::destroy()
{
	detach();
	auto ts = get_current_thread_storage();
	ts->current_layer = owner_layer;

	__try {
		ts->current_entity = this;

		for (auto* n = components.begin(); n != components.end(); n = n->next)
			n->value.destroy();

		if (on_destroy)
			on_destroy(_class);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		on_fail("Entity destructor failed! %s", name.c_str()); 
	}

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

	__try {
		auto ts = get_current_thread_storage();
		ts->current_layer = owner_layer;
		ts->current_entity = this;
		if (on_create)
			_class = on_create(user_data);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		on_fail("Entity constructor failed: %s", name.c_str());
		_class = nullptr;
		return false;
	}

	return true;
}

void entity_t::attach_to(entity_t* e) {

	if (!e || e == this || parent == e)
		return;

	for (auto* n = children.begin(); n != children.end(); n = n->next) {
		if (n->value == e) {
			children.remove_node(n);
			e->parent = nullptr;
			break;
		}
	}

	if (parent) {
		auto& siblings = parent->children;
		for (auto* n = siblings.begin(); n != siblings.end(); n = n->next) {
			if (n->value == this) {
				siblings.remove_node(n);
				break;
			}
		}
		parent = nullptr;
	}

	for (auto* n = e->children.begin(); n != e->children.end(); n = n->next) {
		if (n->value == this) {
			e->children.remove_node(n);
			break;
		}
	}

	parent = e;
	e->children.push_back(this);
}


void entity_t::detach() {
	if (parent) {
		auto& siblings = parent->children;
		for (auto* n = siblings.begin(); n != siblings.end(); n = n->next) {
			if (n->value == this) {
				siblings.remove_node(n);
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

void entity_t::for_each_child_recursive(thread_storage_t* storage, entity_iter_fn_t fn, void* userdata) {
	FOR_EACH_NODE(children, child) {
		fn(storage, child, userdata);
		child->for_each_child_recursive(storage, fn, userdata);
	}
}