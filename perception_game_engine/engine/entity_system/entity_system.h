#pragma once

#include "../../crt/s_string.h"
#include "../../crt/s_map.h"
#include "../../serialize/fnva_hash.h"
#include "../../exception/fail.h"
#include "../../crt/s_string_pool.h"
#include "../../crt/s_performance_vector.h"
#include "../threading/thread_system.h"
#include "../../math/vector3.h"
#include "../../math/quat.h"
#include "../global_vars.h"
#include "../../memory/destructable_object.h"


using class_t = void*;

class serializer_t;
class deserializer_t;
class render_context_t;
class ui_context_t;
class input_context_t;
class entity_t;
class entity_layer_t;
class component_t;
class level_t;

struct current_execution_data_t
{
	entity_t* current_entity = nullptr;
	entity_layer_t* current_layer = nullptr;
	level_t* current_level = nullptr;
};

typedef void(*entity_iter_fn_t)(entity_t* child, void* userdata);

using construct_fn_t = class_t * (*)(entity_t*);
using destruct_fn_t = void(*)(entity_t*, class_t*);

using parent_attach_fn_t = void(*)(entity_t* parent, entity_t* self, class_t*);
using parent_detach_fn_t = void(*)(entity_t* parent, entity_t* self, class_t*);

using on_input_receive_fn_t = void(*)(entity_t*, class_t*, input_context_t*);
using on_physics_update_fn_t = void(*)(entity_t*, class_t*);
using on_frame_fn_t = void(*)(entity_t*, class_t*);

using on_render_fn_t = void(*)(entity_t*, class_t*, render_context_t*);
using on_render_ui_fn_t = void(*)(entity_t*, class_t*, render_context_t*);

using on_serialize_fn_t = void(*)(entity_t*, class_t*, serializer_t*);
using on_deserialize_fn_t = void(*)(entity_t*, class_t*, deserializer_t*);

using on_debug_draw_fn_t = void(*)(entity_t*, class_t*, render_context_t*);
using on_ui_inspector_fn_t = void(*)(entity_t*, class_t*, render_context_t*);


//components
struct transform_instance_t;
struct physics_asset_t;

using component_callback_fn = void(*)(entity_t*, class_t*, void* ctx);

struct component_callback_t
{
	component_callback_fn callback;
	void* ctx;
};

class component_callbacks_t
{
public:
	component_callbacks_t() = default;
public:

	construct_fn_t on_construct; //construction function
	destruct_fn_t on_destruct; //destruction function

	parent_attach_fn_t on_parent_attach; //for components that need to do something when the entity is attached to a parent
	parent_detach_fn_t on_parent_detach; //for components that need to do something before the entity is destroyed

	on_input_receive_fn_t on_input_receive; //input receive function

	on_physics_update_fn_t on_physics_start;
	on_physics_update_fn_t on_physics_update;
	on_physics_update_fn_t on_physics_end;

	on_frame_fn_t on_frame_start; //frame function
	on_frame_fn_t on_frame_update; //frame function
	on_frame_fn_t on_frame_end; //frame function

	on_render_fn_t on_render; //render function
	on_render_ui_fn_t on_render_ui; //render ui function

	on_serialize_fn_t on_serialize; //serialization function
	on_deserialize_fn_t on_deserialize; //deserialization function

	on_debug_draw_fn_t on_debug_draw; //debug draw function

	on_ui_inspector_fn_t on_ui_inspector; //editor inspector panel

	void set_construct_callback(construct_fn_t fn) { on_construct = fn; }
	void set_destruct_callback(destruct_fn_t fn) { on_destruct = fn; }

	void set_on_parent_attach(parent_attach_fn_t fn) { on_parent_attach = fn; }
	void set_on_parent_detach(parent_detach_fn_t fn) { on_parent_detach = fn; }

	void set_input_callback(on_input_receive_fn_t fn) { on_input_receive = fn; }

	void set_physics_start_callback(on_physics_update_fn_t fn) { on_physics_start = fn; }
	void set_physics_update_callback(on_physics_update_fn_t fn) { on_physics_update = fn; }
	void set_physics_end_callback(on_physics_update_fn_t fn) { on_physics_end = fn; }

	void set_frame_start_callback(on_frame_fn_t fn) { on_frame_start = fn; }
	void set_frame_update_callback(on_frame_fn_t fn) { on_frame_update = fn; }
	void set_frame_end_callback(on_frame_fn_t fn) { on_frame_end = fn; }

	void set_render_callback(on_render_fn_t fn) { on_render = fn; }
	void set_render_ui_callback(on_render_ui_fn_t fn) { on_render_ui = fn; }
	void set_serialize_callback(on_serialize_fn_t fn) { on_serialize = fn; }
	void set_deserialize_callback(on_deserialize_fn_t fn) { on_deserialize = fn; }
	void set_debug_draw_callback(on_debug_draw_fn_t fn) { on_debug_draw = fn; }
	void set_ui_inspector_callback(on_ui_inspector_fn_t fn) { on_ui_inspector = fn; }
};

struct identifier_t
{
	s_pooled_string name;
	uint32_t lookup_hash_by_name;
};

struct relative_entity_t {
	relative_entity_t()
	{
		entity = s_performance_struct_t{ invalid_index };
		owner_layer = s_performance_struct_t{ invalid_index };
		owner_level = s_performance_struct_t{ invalid_index };
	}

	static relative_entity_t invalid() { return relative_entity_t(); }

	relative_entity_t(const s_performance_struct_t& entity, const s_performance_struct_t& owner_layer, const s_performance_struct_t& owner_level)
	{
		this->entity = entity;
		this->owner_layer = owner_layer;
		this->owner_level = owner_level;
	}

	relative_entity_t(const relative_entity_t& other)
	{
		entity = other.entity;
		owner_layer = other.owner_layer;
		owner_level = other.owner_level;
	}

	relative_entity_t(relative_entity_t&& other)
	{
		entity = other.entity;
		owner_layer = other.owner_layer;
		owner_level = other.owner_level;
	}

	relative_entity_t& operator=(const relative_entity_t& other)
	{
		entity = other.entity;
		owner_layer = other.owner_layer;
		owner_level = other.owner_level;
		return *this;
	}

	relative_entity_t& operator=(relative_entity_t&& other)
	{
		entity = other.entity;
		owner_layer = other.owner_layer;
		owner_level = other.owner_level;
		return *this;
	}

	bool operator==(const relative_entity_t& other) const
	{
		return entity.index == other.entity.index && owner_layer.index == other.owner_layer.index && owner_level.index == other.owner_level.index;
	}

	bool operator!=(const relative_entity_t& other) const
	{
		return entity.index != other.entity.index || owner_layer.index != other.owner_layer.index || owner_level.index != other.owner_level.index;
	}

	bool is_equal(const relative_entity_t& other) const
	{
		return entity.index == other.entity.index && owner_layer.index == other.owner_layer.index && owner_level.index == other.owner_level.index;
	}

	bool is_equal(const s_performance_struct_t& other_entity, const s_performance_struct_t& other_owner_layer, const s_performance_struct_t& other_owner_level) const
	{
		return entity.index == other_entity.index && owner_layer.index == other_owner_layer.index && owner_level.index == other_owner_level.index;
	}

	s_performance_struct_t entity;
	s_performance_struct_t owner_layer;
	s_performance_struct_t owner_level;
	void set(entity_t* ent);
	bool valid() { return entity.index != invalid_index; }
	entity_t* get();
};

struct component_indexed_t
{
	relative_entity_t owner;
	s_performance_struct_t comp;
	component_indexed_t() = default;
	component_indexed_t(const s_performance_struct_t& self, const relative_entity_t& owner)
	{
		this->comp = self;
		this->owner = owner;
	}
	component_indexed_t(const component_indexed_t& other)
	{
		comp = other.comp;
		owner = other.owner;
	}
	component_indexed_t(component_indexed_t&& other)
	{
		comp = other.comp;
		owner = other.owner;
	}
	component_indexed_t& operator=(const component_indexed_t& other)
	{
		comp = other.comp;
		owner = other.owner;
		return *this;
	}
	component_indexed_t& operator=(component_indexed_t&& other)
	{
		comp = other.comp;
		owner = other.owner;
		return *this;
	}
	bool operator==(const component_indexed_t& other) const
	{
		return comp.index == other.comp.index && owner.is_equal(other.owner);
	}
	bool operator!=(const component_indexed_t& other) const
	{
		return comp.index != other.comp.index || owner != other.owner;
	}
	bool is_equal(const component_indexed_t& other) const
	{
		return comp.index == other.comp.index && owner.is_equal(other.owner);
	}
	bool is_equal(const s_performance_struct_t& other_self, const relative_entity_t& other_owner) const
	{
		return comp.index == other_self.index && owner.is_equal(other_owner);
	}

	bool valid() { return comp.index != invalid_index; }
	component_t* get();
};

class component_t : public s_performance_struct_t, public component_callbacks_t, public identifier_t, public destruction_queue_t
{
public:
	component_t() = default;

	template <typename context_t, typename component_callback>
	void process(entity_t* owner, context_t* ctx, component_callback component_cb);

	bool construct(
		const relative_entity_t& owner,
		const s_string& n, construct_fn_t construct_fn = nullptr, destruct_fn_t deconstruct_fn = nullptr
	);

	void destroy(bool force = false);
public:
	relative_entity_t owner;
	component_indexed_t self;
	class_t* _class = nullptr;
};

struct parent_change_request_t {
	entity_t* new_parent;
	bool detach;
};


class entity_t : public s_performance_struct_t, public component_callbacks_t, public identifier_t, public destruction_queue_t {
public:
	entity_t() = default;
	~entity_t() = default;

	void destroy(bool force = false);

	bool construct(
		const s_performance_struct_t& owner_layer, const s_performance_struct_t& owner_level,
		const s_string& n, construct_fn_t construct_fn = nullptr, destruct_fn_t deconstruct_fn = nullptr
	);

	template <typename context_t, typename component_callback, typename entity_callback>
	void process(context_t* ctx, component_callback component_cb, entity_callback entity_cb);

	void attach_to(entity_t* e);
	void detach();
	void detach_all_children();


	entity_t* get_root();
	void for_each_child_recursive(entity_iter_fn_t fn, void* userdata);

	template <typename fn_t>
	void for_each_child_recursive(fn_t&& fn, void* userdata)
	{
		const size_t count = children.size();

		for (size_t i = 0; i < count; ++i)
		{
			if (!children.is_alive(i))
				continue;

			entity_t* child = children[i].get();

			if (!child)
				continue;

			fn(child, userdata);

			child->for_each_child_recursive(std::forward<fn_t>(fn), userdata);
		}
	}

	s_performance_struct_t add_component(
		const s_string& name, construct_fn_t construct_fn = nullptr, destruct_fn_t deconstruct_fn = nullptr
	);

	s_performance_struct_t add_transform_component(const vector3& position = vector3(0.0f, 0.0f, 0.0f), const quat& rotation = quat::identity(),
		const vector3& scale = vector3(1.0f, 1.0f, 1.0f));
	
	bool remove_component(const s_string& name);
	bool remove_component(uint32_t hash);

	component_t* get_component(const s_performance_struct_t& perf_index)
	{
		if (perf_index.index >= components.size())
			return nullptr;
		
		auto& comp = components[perf_index.index];
		
		if (!components.is_alive(perf_index.index))
			return nullptr;
		
		if (comp.is_destroyed_or_pending())
			return nullptr;

		return &comp;
	}


	template<typename T>
	T* get_component(const s_performance_struct_t& perf_index)
	{
		if (perf_index.index >= components.size())
			return nullptr;

		auto& comp = components[perf_index.index];
		
		if (!components.is_alive(perf_index.index))
			return nullptr;

		if (comp.is_destroyed_or_pending())
			return nullptr;

		return reinterpret_cast<T*>(comp._class);
	}

	template<typename T>
	T* get_component(uint32_t hash)
	{
		const size_t count = components.size();
		for (size_t i = 0; i < count; ++i)
		{
			if (!components.is_alive(i))
				continue;

			auto& comp = components[i];

			if (comp.is_destroyed_or_pending())
				continue;

			if (comp.lookup_hash_by_name == hash)
				return reinterpret_cast<T*>(comp._class);
		}

		return nullptr;
	}
public:
	class_t* _class = 0; //passed onto callbacks

	relative_entity_t parent;
	relative_entity_t self;

	s_performance_struct_t owner_layer; //layer pointer
	s_performance_struct_t owner_level; //layer pointer
	s_performance_vector<relative_entity_t> children;
	s_performance_vector<component_t> components; //components attached to this entity
};

class entity_layer_t : public s_performance_struct_t, public identifier_t, public destruction_queue_t
{
public:
	entity_layer_t() = default;
	~entity_layer_t() = default;

	void init_layer(const s_performance_struct_t& owner_level, const s_string& n);
	
	template <typename context_t, typename component_callback, typename entity_callback>
	void process(context_t* ctx, component_callback component_cb, entity_callback entity_cb);

	s_performance_struct_t create_entity(const s_string& name, construct_fn_t construct_fn = nullptr, destruct_fn_t deconstruct_fn = nullptr);

	s_performance_vector<component_indexed_t>& get_components_by_hash(uint32_t hash);
	
	__forceinline entity_t* get_entity(const uint64_t& layer_perf_index)
	{
		if (layer_perf_index >= entities.size())
			return nullptr;
		
		auto& entity = entities[layer_perf_index];
		
		if (!entities.is_alive(layer_perf_index) || entity.is_destroyed_or_pending())
			return nullptr;

		return &entity;
	}

	void destroy(bool force = false);
public:
	s_performance_struct_t owner_level;
	s_performance_vector<entity_t> entities;
	s_map<uint32_t, s_performance_vector<component_indexed_t>> map_components;
};

class level_t : public s_performance_struct_t, public identifier_t, public destruction_queue_t
{
public:
	level_t() = default;
	~level_t() = default;
public:
	void init_level(const s_string& n);

	s_performance_struct_t create_layer(const s_string& name);
	void destroy_layer(const s_performance_struct_t& layer_perf_index);
	
	__forceinline entity_layer_t* get_layer(const uint64_t& layer_perf_index)
	{
		if (layer_perf_index >= layers.size())
			return nullptr;

		auto& layer = layers[layer_perf_index];

		if (!layers.is_alive(layer_perf_index) || layer.is_destroyed_or_pending())
			return nullptr;

		return &layer;
	}

	void destroy(bool force = false);

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

	s_performance_struct_t create_level(const s_string& name);
	void destroy_level(const s_performance_struct_t& level_perf_index, bool force = false);

	template <typename context_t, typename component_callback, typename entity_callback>
	void process(context_t* ctx, component_callback component_cb, entity_callback entity_cb);

	__forceinline level_t* get_level(const uint64_t& level_perf_index)
	{
		if (level_perf_index >= levels.size())
			return nullptr;

		auto& level = levels[level_perf_index];

		if (!levels.is_alive(level_perf_index) || level.is_destroyed_or_pending())
			return nullptr;

		return &level;
	}

	__forceinline entity_t* get_entity_by_relation(const relative_entity_t& relation)
	{
		auto level = get_level(relation.owner_level.index);
		
		if (!level)
			return nullptr;

		auto layer = level->get_layer(relation.owner_layer.index);
	
		if (!layer)
			return nullptr;

		return layer->get_entity(relation.entity.index);
	}

	__forceinline entity_t* get_entity_by_perf_index(const uint64_t& level_perf_index, const uint64_t& layer_perf_index
		, const uint64_t& entity_perf_index)
	{
		auto level = get_level(level_perf_index);

		if (!level)
			return nullptr;

		auto layer = level->get_layer(layer_perf_index);

		if (!layer)
			return nullptr;

		return layer->get_entity(entity_perf_index);
	}

	void on_physics_start();
	void on_physics_update();
	void on_physics_end();

	void on_frame_start();
	void on_frame_update();
	void on_frame_end();

	void on_input_receive(input_context_t* ctx);
	void on_render(render_context_t* ctx);
	void on_render_ui(render_context_t* ctx);
	void on_debug_draw(render_context_t* ctx);
	void on_ui_inspector(render_context_t* ctx);
	void on_serialize(serializer_t* s);
	void on_deserialize(deserializer_t* d);
	void destroy();

	void on_gc();

	void execute_start();
	void execute_stop();

private:
	thread_t t_on_frame;
	thread_t t_on_physics_update;
	thread_t t_on_gc;
public:
	s_performance_vector<level_t> levels;
};


extern entity_manager g_entity_mgr;

DWORD WINAPI execute_on_physics_update(LPVOID param);
DWORD WINAPI execute_on_frame(LPVOID param);
DWORD WINAPI execute_gc(LPVOID param);