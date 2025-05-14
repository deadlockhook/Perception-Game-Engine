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


	}
	return false;
}

void entity_layer_t::destroy(bool force)
{
	if (is_destroyed_or_pending())
		return;

	if (force)
	{
		const size_t entity_count = entities.size();
		for (size_t e = 0; e < entity_count; ++e)
		{
			if (!entities.is_alive(e))
				continue;

			entity_t& entity = entities[e];
			entity.destroy(true);
			entities.remove(e);
		}

		post_destruction(); 
	}
	else
	{
		mark_for_destruction();
	}
}


