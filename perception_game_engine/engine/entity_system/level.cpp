#include "entity_system.h"
#include "../threading/thread_storage.h"

void level_t::init_level(const s_string& n)
{
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;
}

s_performance_struct_t level_t::create_layer(const s_string& name)
{
	s_performance_struct_t layer_perf_index = layers.push_back_perf_struct(entity_layer_t());
	entity_layer_t& layer = layers[layer_perf_index.index];
	layer.init_layer(s_performance_struct_t{ this->index }, name);
	return layer_perf_index;
}

void level_t::destroy_layer(const s_performance_struct_t& layer_perf_index)
{
	const size_t count = layers.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!layers.is_alive(i))
			continue;

		auto& layer = layers[i];

		if (layer.is_destroyed_or_pending())
			continue;

		if (layer.index == layer_perf_index.index)
		{
			layer.mark_for_destruction();
			return;
		}
	}
}

void level_t::destroy(bool force)
{
	if (force)
	{
		if (is_destroyed())
			return; 

		const size_t layer_count = layers.size();
		for (size_t l = 0; l < layer_count; ++l)
		{
			if (!layers.is_alive(l))
				continue;

			entity_layer_t& layer = layers[l];

			if (layer.is_destroyed())
				continue;

			layer.destroy(true);
			layers.remove(l);    
		}

		post_destruction(); 
	}
	else
		mark_for_destruction();
	
}



/*
void level_t::destroy()
{
	auto ts = get_current_thread_storage();

	const size_t count = layers.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!layers.is_alive(i))
			continue;

		entity_layer_t& layer = layers[i];
		ts->current_layer = &layer;
		layer.destroy();
	}

	layers.clear();
}*/