#include "entity_system.h"
#include "../threading/thread_storage.h"

void level_t::init_level(const s_string& n)
{
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;
}

entity_layer_t* level_t::create_layer(const s_string& name)
{
	entity_layer_t* layer = layers.emplace_back();
	layer->init_layer(name);
	return layer;
}

void level_t::destroy_layer(entity_layer_t* layer)
{
	if (!layer)
		return;

	const size_t count = layers.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!layers.is_alive(i))
			continue;

		if (&layers[i] == layer)
		{
			layers[i].destroy();
			layers.remove(i);
			return;
		}
	}
}

entity_layer_t* level_t::get_layer(uint32_t id)
{
	const size_t count = layers.size();
	uint32_t alive_index = 0;

	for (size_t i = 0; i < count; ++i)
	{
		if (!layers.is_alive(i))
			continue;

		if (alive_index == id)
			return &layers[i];

		++alive_index;
	}

	return nullptr;
}

entity_layer_t* level_t::get_layer_by_name(const s_string& name)
{
	const uint32_t hash = fnv1a32(name.c_str());
	const size_t count = layers.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!layers.is_alive(i))
			continue;

		if (layers[i].lookup_hash_by_name == hash)
			return &layers[i];
	}

	return nullptr;
}

bool level_t::has_layer(const s_string& name) {
	return get_layer_by_name(name) != nullptr;
}

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
}