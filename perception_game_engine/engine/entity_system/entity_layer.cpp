#include "entity_system.h"
#include "../threading/thread_storage.h"

void entity_layer_t::init_layer(const s_string& n)
{
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;
}

entity_t* entity_layer_t::create_entity(
	const s_string& name, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
)
{
	entity_t entity;
	if (entity.construct(this, name, construct_fn, deconstruct_fn)) {
		return entities.push_back(entity);
	}
	on_fail("Failed to create entity: %s", name.c_str());
	return nullptr;
}


entity_t* entity_layer_t::get_entity_by_class(class_t* class_ptr)
{
	const size_t count = entities.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!entities.is_alive(i))
			continue;

		if (entities[i]._class == class_ptr)
			return &entities[i];
	}
	return nullptr;
}


entity_t* entity_layer_t::get_entity_by_name(const s_string& name)
{
	const uint32_t hash = fnv1a32(name.c_str());
	const size_t count = entities.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!entities.is_alive(i))
			continue;

		if (entities[i].lookup_hash_by_name == hash)
			return &entities[i];
	}
	return nullptr;
}


bool entity_layer_t::remove_entity_by_class(class_t* class_ptr)
{
	const size_t count = entities.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!entities.is_alive(i))
			continue;

		if (entities[i]._class == class_ptr)
		{
			entities[i].destroy();
			entities.remove(i);
			return true;
		}
	}
	return false;
}

void entity_layer_t::destroy()
{
	auto ts = get_current_thread_storage();
	ts->current_layer = this;

	const size_t count = entities.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!entities.is_alive(i))
			continue;

		ts->current_entity = &entities[i];
		entities[i].destroy();
	}

	entities.clear();
}

