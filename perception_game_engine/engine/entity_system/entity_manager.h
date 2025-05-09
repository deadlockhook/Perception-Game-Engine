#pragma once
#include <cstdint>
#include "../../crt/s_string.h"
#include "../../crt/s_map.h"
#include "../../serialize/fnva_hash.h"
#include "../../exception/fail.h"

typedef class entity_callbacks_t entity_header_t;
using uint32_t = uint32_t;
using layer_id_t = uint32_t;
using class_t = void*;
using user_data_t = void*;

typedef class serializer_t;
typedef class deserializer_t;
typedef class render_context_t;
typedef class ui_context_t;
typedef class input_context_t;
typedef class entity_t;

using construct_fn_t = class_t*(*)(user_data_t*);
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
	__forceinline void call_input(class_t* e, input_context_t* i) const { if (on_input_receive) on_input_receive(e,i); }
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



public:
	uint32_t layer;
	s_string name;
	uint32_t lookup_hash_by_name;
	class_t* _class = nullptr; //passed onto callbacks
	user_data_t* user_data = nullptr; //passed onto callbacks
};

class entity_manager_t
{
public:
	entity_manager_t() = default;
	~entity_manager_t() = default;

	void init_manager(const s_string& n)
	{
		name = n;
		lookup_hash_by_name = fnv1a32(n.c_str());
	}

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

	entity_t* get_entity_by_class(class_t* class_ptr)
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

	entity_t* get_entity_by_name(const s_string& name)
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

	s_vector<entity_t*> get_entities_by_name(const s_string& name)
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

	s_vector<entity_t*> get_entities_of_type(uint32_t type)
	{
		s_vector<entity_t*> result;
		auto it = entities.find(type);
		if (!it) return result;

		for (int i = 0; i < it->count(); i++)
			result.push_back(&it->at(i));

		return result;
	}

	bool remove_entity_by_class(class_t* class_ptr)
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

	void destroy()
	{
		for (auto& entity : entities) {
			for (auto& e : entity.value) {
				e.destroy();
			}
		}
	}
public:
	s_string name;
	uint32_t lookup_hash_by_name;
public:
	s_map<uint32_t, s_vector<entity_t>> entities; 
};

class master_entity_manager
{
public:
	master_entity_manager() = default;
	~master_entity_manager() = default;

	layer_id_t create_layer(const s_string& name)
	{
		auto manager = &layers.push_back(entity_manager_t());
		
		if (!manager) 
			return -1;

		manager->init_manager(name);
		return layers.count() - 1;
	}

	void destroy_layer(layer_id_t id)
	{
		if (id >= layers.count())
			return;

		layers[id].destroy();
		layers.erase_at(id);
	}

	entity_manager_t* get_layer(layer_id_t id)
	{
		if (id >= layers.count()) return nullptr;
		return &layers[id];
	}

	entity_manager_t* get_layer_by_name(const s_string& name) {
		uint32_t hash = fnv1a32(name.c_str());
		for (auto& layer : layers)
			if (layer.lookup_hash_by_name == hash)
				return &layer;
		return nullptr;
	}

	bool has_layer(const s_string& name) {
		return get_layer_by_name(name) != nullptr;
	}

	void on_frame() {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_frame && e._class)
						e.on_frame(e._class);
	}

	void on_physics_update() {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_physics_update && e._class)
						e.on_physics_update(e._class);
	}

	void on_input_receive(input_context_t* ctx) {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_input_receive && e._class)
						e.on_input_receive(e._class, ctx);
	}

	void on_render(render_context_t* ctx) {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_render && e._class)
						e.on_render(e._class, ctx);
	}

	void on_render_ui(render_context_t* ctx) {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_render_ui && e._class)
						e.on_render_ui(e._class, ctx);
	}

	void on_debug_draw(render_context_t* ctx) {
		for (auto& layer : layers)
			for (auto& [_, vec] : layer.entities)
				for (auto& e : vec)
					if (e.on_debug_draw && e._class)
						e.on_debug_draw(e._class, ctx);
	}

public:
	s_vector<entity_manager_t> layers;
};