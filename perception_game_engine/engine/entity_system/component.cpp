#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../component_system/transform_instance.h"
#include "../component_system/physics_asset.h"

void component_t::destroy(bool force)
{
	if (force)
	{
		if (is_destroyed())
			return; 

		if (on_destruct)
			on_destruct(owner, _class);

		_class = nullptr;
		owner = nullptr;
		on_destruct = nullptr;

		post_destruction();
	}
	else
		mark_for_destruction();
	
}

bool component_t::construct(
	entity_t* _owner,
	const s_string& n, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
)
{
	owner = _owner;
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;

	on_construct = construct_fn;
	on_destruct = deconstruct_fn;

	if (on_construct)
		_class = on_construct(owner);

	return _class != nullptr;
}

component_t* entity_t::add_component(
	const s_string& name, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn)
{
	auto string_hash = fnv1a32(name.c_str());

	const size_t count = components.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		if (components[i].lookup_hash_by_name == string_hash)
		{
			on_fail("Failed to create component, already exists ", name.c_str());
			return nullptr;
		}
	}

	component_t component;
	if (component.construct(this, name, construct_fn, deconstruct_fn))
	{
		auto comp = components.push_back(component);
		owner_layer->map_components[string_hash].push_back(comp);
		return comp;
	}

	on_fail("Failed to create component: ", name.c_str());
	return nullptr;
}


bool entity_t::remove_component(uint32_t string_hash)
{
	const size_t count = components.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		if (components[i].is_destroyed_or_pending())
			continue;

		if (components[i].lookup_hash_by_name == string_hash)
		{
			auto& hash_map = owner_layer->map_components[string_hash];

			const size_t has_compcount = hash_map.size();
			for (size_t j = 0; j < has_compcount; ++j)
			{
				if (!hash_map.is_alive(j))
					continue;

				if (hash_map[i] == &components[i])
				{
					hash_map.remove(j);
					break;
				}
			}

			components[i].mark_for_destruction();
			return true;
		}
	}

	on_fail("Failed to remove component: %s", name.c_str());
	return false;
}
bool entity_t::remove_component(const s_string& name)
{
	return remove_component(fnv1a32(name.c_str()));
}

component_t* entity_t::add_transform_component(const vector3& position, const quat& rotation,
	const vector3& scale)
{
	
	auto transform = add_component(transform_instance_t_register, transform_instance_t::create_transform, transform_instance_t::destroy_transform);

	if (!transform)
		return nullptr;

	transform->set_physics_update_callback(transform_instance_t::on_physics_update);
	transform->set_frame_update_callback(transform_instance_t::on_frame_update);
	transform->set_on_parent_attach(transform_instance_t::on_parent_attach);
	transform->set_on_parent_detach(transform_instance_t::on_parent_detach);

	auto transform_instance = reinterpret_cast<transform_instance_t*>(transform->_class);
	transform_instance->queue_set_position(position);
	transform_instance->queue_set_rotation(rotation);
	transform_instance->queue_set_scale(scale);

	return transform;
}