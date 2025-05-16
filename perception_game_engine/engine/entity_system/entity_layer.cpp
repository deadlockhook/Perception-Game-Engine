#include "entity_system.h"
#include "../threading/thread_storage.h"

void entity_layer_t::init_layer(const s_performance_struct_t& owner_level, const s_string& n)
{
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;
	this->owner_level = owner_level;
}
s_performance_vector<component_indexed_t>& entity_layer_t::get_components_by_hash(uint32_t hash)
{
	return map_components[hash];
}

s_performance_struct_t entity_layer_t::create_entity(
	const s_string& name, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
)
{
	entity_t entity;
	if (entity.construct(s_performance_struct_t{ index }, s_performance_struct_t{ owner_level.index }, name, construct_fn, deconstruct_fn))
	{
		auto perf_index = entities.push_back_perf_struct(entity);
		
		auto _entity = get_entity(perf_index.index);
		_entity->index = perf_index.index;
		_entity->self.entity.index = perf_index.index;

		return perf_index;
	}
	
	on_fail("Failed to create entity: %s", name.c_str());
	return s_performance_struct_t{invalid_index};
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


