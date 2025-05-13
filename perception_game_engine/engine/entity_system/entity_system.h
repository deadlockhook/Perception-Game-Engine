#pragma once

#include "../../crt/s_string.h"
#include "../../crt/s_map.h"
#include "../../serialize/fnva_hash.h"
#include "../../exception/fail.h"
#include "../../crt/s_string_pool.h"
#include "../../crt/s_performance_vector.h"
#include "../threading/thread_system.h"

using class_t = void*;
using user_data_t = void*;

class serializer_t;
class deserializer_t;
class render_context_t;
class ui_context_t;
class input_context_t;
class entity_t;
class entity_layer_t;
class component_t;
typedef void(*entity_iter_fn_t)(struct thread_storage_t* storage, entity_t* child, void* userdata);

using construct_fn_t = class_t * (*)(entity_t*, user_data_t*);
using destruct_fn_t = void(*)(entity_t*, class_t*);

using on_input_receive_fn_t = void(*)(entity_t*, class_t*, input_context_t*);
using on_physics_update_fn_t = void(*)(entity_t*, class_t*);
using on_frame_fn_t = void(*)(entity_t*, class_t*);

using on_render_fn_t = void(*)(entity_t*, class_t*, render_context_t*);
using on_render_ui_fn_t = void(*)(entity_t*, class_t*, render_context_t*);

using on_serialize_fn_t = void(*)(entity_t*, class_t*, serializer_t*);
using on_deserialize_fn_t = void(*)(entity_t*, class_t*, deserializer_t*);

using on_debug_draw_fn_t = void(*)(entity_t*, class_t*, render_context_t*);
using on_ui_inspector_fn_t = void(*)(entity_t*, class_t*, render_context_t*);

class component_callbacks_t
{
public:
	component_callbacks_t() = default;
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

struct identifier_t
{
	s_pooled_string name;
	uint32_t lookup_hash_by_name;
};

class component_t : public component_callbacks_t, public identifier_t
{
public:
	bool construct(
		entity_t* owner,
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
	void destroy();
public:
	entity_t* owner = nullptr;
	class_t* _class = nullptr;
	user_data_t* user_data = nullptr;
};

class entity_t : public component_callbacks_t, public identifier_t {
public:
	entity_t() = default;
	~entity_t() = default;

	void destroy();

	bool construct(
		entity_layer_t* owner,
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
	entity_t* get_root();
	void for_each_child_recursive(thread_storage_t* storage, entity_iter_fn_t fn, void* userdata);
	
	template <typename fn_t>
	void for_each_child_recursive(thread_storage_t* storage, fn_t&& fn, void* userdata)
	{
		const size_t count = children.size();

		for (size_t i = 0; i < count; ++i)
		{
			if (!children.is_alive(i))
				continue;

			entity_t* child = children[i];
			fn(storage, child, userdata);

			child->for_each_child_recursive(storage, std::forward<fn_t>(fn), userdata);
		}
	}


	bool add_component(
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

	bool remove_component(const s_string& name);

	component_t* get_component(const s_string& name);

	template<typename T>
	T* get_component_class(const s_string& name) {
		auto* c = get_component(name);
		return c ? reinterpret_cast<T*>(c->_class) : nullptr;
	}
public:
	class_t* _class = nullptr; //passed onto callbacks
	user_data_t* user_data = nullptr; //passed onto callbacks
public:
	entity_t* parent = nullptr; //parent entity
	entity_layer_t* owner_layer = nullptr; //layer pointer
	s_performance_vector<entity_t*> children;
	s_performance_vector<component_t> components; //components attached to this entity
};

class entity_layer_t : public identifier_t
{
public:
	entity_layer_t() = default;
	~entity_layer_t() = default;

	void init_layer(const s_string& n);

	entity_t* create_entity(
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
	bool remove_entity_by_class(class_t* class_ptr);
	void destroy();
public:
	s_performance_vector<entity_t> entities;
	//map with name hashes for quick lookup

};

class level_t : public identifier_t
{
public:
	level_t() = default;
	~level_t() = default;
public:
	void init_level(const s_string& n);

	entity_layer_t* create_layer(const s_string& name);

	void destroy_layer(entity_layer_t* layer);

	entity_layer_t* get_layer(uint32_t index);
	entity_layer_t* get_layer_by_name(const s_string& name);

	bool has_layer(const s_string& name);

	void destroy();

	void set_active(bool active) { this->active = active; }

public:
	bool active = false;
	s_performance_vector<entity_layer_t> layers;
};

class entity_manager
{
public:
	entity_manager() = default;
	~entity_manager() = default;

	level_t* create_level(const s_string& name);

	void destroy_level(level_t* layer);

	level_t* get_level(uint32_t index);
	level_t* get_level_by_name(const s_string& name);

	bool has_level(const s_string& name);

	template <typename context_t, typename component_callback, typename entity_callback>
	void process(context_t* ctx, component_callback component_cb, entity_callback entity_cb);

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

	void execute_start();
	void execute_stop();

private:
	thread_t t_on_frame;
	thread_t t_on_physics_update;
public:
	s_performance_vector<level_t> levels;
};


extern entity_manager g_entity_mgr;

DWORD WINAPI execute_on_physics_update(LPVOID param); 
DWORD WINAPI execute_on_frame(LPVOID param); 