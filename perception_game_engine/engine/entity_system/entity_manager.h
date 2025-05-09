#pragma once
#include <cstdint>
#include "../../crt/s_string.h"
#include "../../crt/s_map.h"
#include "../../serialize/fnva_hash.h"
#include "../../exception/fail.h"

using class_t = void*;
using user_data_t = void*;

typedef class serializer_t;
typedef class deserializer_t;
typedef class render_context_t;
typedef class ui_context_t;
typedef class input_context_t;
typedef class entity_t;
typedef class component_t;
typedef void(*entity_iter_fn_t)(entity_t* child, void* userdata);

using construct_fn_t = class_t * (*)(user_data_t*);
using destruct_fn_t = class_t(*)(class_t*);

using on_input_receive_fn_t = void(*)(class_t*, input_context_t*);
using on_physics_update_fn_t = void(*)(class_t*);
using on_frame_fn_t = void(*)(class_t*);

using on_render_fn_t = void(*)(class_t*, render_context_t*);
using on_render_ui_fn_t = void(*)(class_t*, render_context_t*);

using on_serialize_fn_t = void(*)(class_t*, serializer_t*);
using on_deserialize_fn_t = void(*)(class_t*, deserializer_t*);

using on_debug_draw_fn_t = void(*)(class_t*, render_context_t*);
using on_ui_inspector_fn_t = void(*)(class_t*, render_context_t*);

class entity_callbacks_t
{
public:
	entity_callbacks_t() = default;
public:
	//add try and catch properly here
	__forceinline class_t* call_construct(user_data_t* data) const { if (on_create) return on_create(data); return nullptr; }
	__forceinline void call_destruct(class_t* e) const { if (on_destroy) on_destroy(e); }
	__forceinline void call_input(class_t* e, input_context_t* i) const { if (on_input_receive) on_input_receive(e, i); }
	__forceinline void call_physics(class_t* e) const { if (on_physics_update) on_physics_update(e); }
	__forceinline void call_frame(class_t* e) const { if (on_frame) on_frame(e); }
	__forceinline void call_render(class_t* e, render_context_t* ctx) const { if (on_render) on_render(e, ctx); }
	__forceinline void call_render_ui(class_t* e, render_context_t* ctx) const { if (on_render_ui) on_render_ui(e, ctx); }
	__forceinline void call_serialize(class_t* e, serializer_t* s) const { if (on_serialize) on_serialize(e, s); }
	__forceinline void call_deserialize(class_t* e, deserializer_t* d) const { if (on_deserialize) on_deserialize(e, d); }
	__forceinline void call_debug_draw(class_t* e, render_context_t* ctx) const { if (on_debug_draw) on_debug_draw(e, ctx); }
	__forceinline void call_ui_inspector(class_t* e, render_context_t* ctx) const { if (on_ui_inspector) on_ui_inspector(e, ctx); }
public:
	construct_fn_t on_create; //construction function
	destruct_fn_t on_destroy; //destruction function

	on_input_receive_fn_t on_input_receive; //input receive function
	on_physics_update_fn_t on_physics_update; //physics update function
	on_frame_fn_t on_frame; //frame function

	on_render_fn_t on_render; //render function
	on_render_ui_fn_t on_render_ui; //render ui function

	on_serialize_fn_t on_serialize; //serialization function
	on_deserialize_fn_t on_deserialize; //deserialization function

	on_debug_draw_fn_t on_debug_draw; //debug draw function

	on_ui_inspector_fn_t on_ui_inspector; //editor inspector panel
};

class entity_t : public entity_callbacks_t {
public:
	entity_t() = default;
	~entity_t() = default;

	void destroy()
	{
		__try {
			if (on_destroy)
				on_destroy(_class);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			on_fail("Entity destructor failed! %s ", name.c_str()); //add more proper logging
		}
	}

	bool construct(
		uint32_t l,
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
		layer = l;
		name = n;
		lookup_hash_by_name = fnv1a32(n.c_str());
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

	void attach_to(entity_t* e) {
		if (e && !parent) {
			parent = e;
			e->children.push_back(this);
		}
	}

	void detach() {
		if (parent) {
			auto& siblings = parent->children;
			for (size_t i = 0; i < siblings.count(); ++i) {
				if (siblings[i] == this) {
					siblings.erase_at(i);
					break;
				}
			}
			parent = nullptr;
		}
	}

	void reparent(entity_t* new_parent) {

		if (new_parent == this || parent == new_parent)
			return;

		detach();
		attach_to(new_parent);
	}

	entity_t* get_root() {
		entity_t* current = this;
		while (current->parent)
			current = current->parent;
		return current;
	}

	void for_each_child_recursive(entity_iter_fn_t fn, void* userdata) {
		for (entity_t* child : children) {
			fn(child, userdata);
			child->for_each_child_recursive(fn, userdata);
		}
	}

public:
	uint32_t layer;
	s_string name;
	uint32_t lookup_hash_by_name;
	class_t* _class = nullptr; //passed onto callbacks
	user_data_t* user_data = nullptr; //passed onto callbacks
public:
	entity_t* parent = nullptr; //parent entity
	s_vector<entity_t*> children;
};

class entity_manager_t
{
public:
	entity_manager_t() = default;
	~entity_manager_t() = default;

	void init_manager(const s_string& n);

	void create_entity(
		uint32_t type,
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
	);
	entity_t* get_entity_by_class(class_t* class_ptr);
	entity_t* get_entity_by_name(const s_string& name);
	s_vector<entity_t*> get_entities_by_name(const s_string& name);
	s_vector<entity_t*> get_entities_of_type(uint32_t type);
	bool remove_entity_by_class(class_t* class_ptr);
	void destroy();
public:
	s_string name;
	uint32_t lookup_hash_by_name = 0;
public:
	s_map<uint32_t, s_vector<entity_t>> entities;
};

class master_entity_manager
{
public:
	master_entity_manager() = default;
	~master_entity_manager() = default;

	uint32_t create_layer(const s_string& name);

	void destroy_layer(uint32_t id);

	entity_manager_t* get_layer(uint32_t id);
	entity_manager_t* get_layer_by_name(const s_string& name);

	bool has_layer(const s_string& name);

	void on_frame();
	void on_physics_update();
	void on_input_receive(input_context_t* ctx);
	void on_render(render_context_t* ctx);
	void on_render_ui(render_context_t* ctx);
	void on_debug_draw(render_context_t* ctx);
	void on_ui_inspector(render_context_t* ctx);
	void on_serialize(serializer_t* s);
	void on_deserialize(deserializer_t* d);
	void destroy();

public:
	s_vector<entity_manager_t> layers;
};

extern master_entity_manager g_master_entity_manager;