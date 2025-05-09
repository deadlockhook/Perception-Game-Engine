#pragma once

#include "../../crt/s_string.h"
#include "../../crt/s_map.h"
#include "../../serialize/fnva_hash.h"
#include "../../exception/fail.h"
#include "../../crt/s_string_pool.h"

using class_t = void*;
using user_data_t = void*;

typedef class serializer_t;
typedef class deserializer_t;
typedef class render_context_t;
typedef class ui_context_t;
typedef class input_context_t;
typedef class entity_t;
typedef class entity_layer_t;
typedef class component_t;
typedef void(*entity_iter_fn_t)(struct thread_storage_t* storage, entity_t* child, void* userdata);

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

class component_callbacks_t
{
public:
	component_callbacks_t() = default;
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

class component_t : public component_callbacks_t
{
public:

	void destroy();
public:
	s_pooled_string name;
	uint32_t lookup_hash_by_name;
	entity_t* owner = nullptr;
	class_t* _class = nullptr;
};

class entity_t : public component_callbacks_t {
public:
	entity_t() = default;
	~entity_t() = default;

	void destroy();

	bool construct(
		entity_layer_t* owner,
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
	);

	void attach_to(entity_t* e);
	void detach();
	void reparent(entity_t* new_parent);
	entity_t* get_root();
	void for_each_child_recursive(thread_storage_t* storage, entity_iter_fn_t fn, void* userdata);

public:
	uint32_t layer;
	s_pooled_string name;
	uint32_t lookup_hash_by_name;
	class_t* _class = nullptr; //passed onto callbacks
	user_data_t* user_data = nullptr; //passed onto callbacks
public:
	entity_t* parent = nullptr; //parent entity
	entity_layer_t* owner_layer = nullptr; //layer pointer
	s_vector<entity_t*> children;
	s_vector<component_t*> components; //components attached to this entity
};

class entity_layer_t
{
public:
	entity_layer_t() = default;
	~entity_layer_t() = default;

	void init_layer(const s_string& n);

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
	s_pooled_string name;
	uint32_t lookup_hash_by_name = 0;
public:
	s_map<uint32_t, s_vector<entity_t>> entities;
};

class entity_manager
{
public:
	entity_manager() = default;
	~entity_manager() = default;

	uint32_t create_layer(const s_string& name);

	void destroy_layer(uint32_t id);

	entity_layer_t* get_layer(uint32_t id);
	entity_layer_t* get_layer_by_name(const s_string& name);

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
	s_vector<entity_layer_t> layers;
};

extern entity_manager g_entity_mgr;