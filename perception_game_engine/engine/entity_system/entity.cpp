#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_string_pool.h"

entity_t* relative_entity_t::get()
{
	return g_entity_mgr.get_entity_by_perf_index(owner_level.index, owner_layer.index, entity.index);
}

component_t* component_indexed_t::get()
{
	auto owner_entity = g_entity_mgr.get_entity_by_perf_index(owner.owner_level.index, owner.owner_layer.index, owner.entity.index);
	
	if (!owner_entity)
		return nullptr;

	return owner_entity->get_component(comp);
}

void relative_entity_t::set(entity_t* ent)
{
	if (!ent)
	{
		entity = s_performance_struct_t{ invalid_index };
		owner_layer = s_performance_struct_t{ invalid_index };
		owner_level = s_performance_struct_t{ invalid_index };
		return;
	}

	entity = s_performance_struct_t{ ent->index };
	owner_layer = ent->owner_layer;
	owner_level = ent->owner_level;
}

void entity_t::destroy(bool force)
{
	if (force)
	{
		if (is_destroyed())
			return; 
	}

	detach();
	detach_all_children();

	if (force)
	{
		const size_t component_count = components.size();
		for (size_t c = 0; c < component_count; ++c)
		{
			if (!components.is_alive(c))
				continue;

			component_t& comp = components[c];

			if (comp.is_destroyed())
				continue;

			comp.destroy(true);
			components.remove(c);
		}

		if (on_destruct)
			on_destruct(this, _class);

		post_destruction();
	}
	else
	{
		mark_for_destruction();
	}
}


bool entity_t::construct(const s_performance_struct_t& owner_layer, const s_performance_struct_t& owner_level,
	const s_string& n, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
) {
	this->owner_layer = owner_layer;
	this->owner_level = owner_level;
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;

	parent = relative_entity_t();
	self = relative_entity_t(s_performance_struct_t{ index }, owner_layer, owner_level);

	on_construct = construct_fn;
	on_destruct = deconstruct_fn;

	if (on_construct)
		_class = on_construct(this);

	return true;
}

void entity_t::attach_to(entity_t* new_parent)
{
	if (!new_parent || new_parent == this || parent == new_parent->self)
		return;

	if (parent.valid())
	{
		auto parent_entity = parent.get();

		auto& siblings = parent_entity->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i] == self)
			{
				siblings.remove(i);
				break;
			}
		}

		const size_t component_count = components.size();
		for (size_t c = 0; c < component_count; ++c)
		{
			if (!components.is_alive(c))
				continue;

			auto& comp = components[c];

			if (comp.on_parent_detach)
				comp.on_parent_detach(parent_entity, this, comp._class);
		}

		if (on_parent_detach)
			on_parent_detach(parent_entity, this, _class);

		parent = relative_entity_t();
	}

	const size_t new_parent_child_count = new_parent->children.size();
	for (size_t i = 0; i < new_parent_child_count; ++i)
	{
		if (!new_parent->children.is_alive(i))
			continue;

		auto child = new_parent->children[i];

		if (child == self)
		{
			new_parent->children.remove(i);
			break;
		}
	}

	parent.set(new_parent);
	new_parent->children.push_back(relative_entity_t{ s_performance_struct_t{index}, owner_layer, owner_level});

	const size_t component_count = components.size();
	for (size_t c = 0; c < component_count; ++c)
	{
		if (!components.is_alive(c))
			continue;

		auto& comp = components[c];

		if (comp.on_parent_attach)
			comp.on_parent_attach(new_parent, this, comp._class);
	}

	if (on_parent_attach)
		on_parent_attach(new_parent, this, _class);
}



void entity_t::detach()
{
	if (parent.valid())
	{
		auto parent_entity = parent.get();

		if (parent_entity->is_destroyed_or_pending())
			return;

		auto& siblings = parent_entity->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i].is_equal(s_performance_struct_t{ this->index }, this->owner_layer, this->owner_level))
			{
				siblings.remove(i);
				break;
			}
		}

		parent = relative_entity_t();
	}
}

void entity_t::detach_all_children()
{
	const size_t child_count = children.size();

	for (size_t i = 0; i < child_count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		auto child = children[i].get();

		if (!child)
			continue;

		if (child->is_destroyed_or_pending())
			continue;

		if (!child)
			continue;

		const size_t component_count = child->components.size();
		for (size_t c = 0; c < component_count; ++c)
		{
			if (!child->components.is_alive(c) )
				continue;

			auto& comp = child->components[c];

			if (comp.is_destroyed_or_pending())
				continue;

			if (comp.on_parent_detach)
				comp.on_parent_detach(this, child, comp._class);
		}

		if (child->on_parent_detach)
			child->on_parent_detach(this, child, child->_class);

		child->parent = relative_entity_t();
	}
}

entity_t* entity_t::get_root()
{
	if (parent.valid())
	{
		auto parent_entity = parent.get();
		
		if (parent_entity->is_destroyed_or_pending())
			return nullptr;

		return parent_entity->get_root();
	}
	return this;
}



void entity_t::for_each_child_recursive(entity_iter_fn_t fn, void* userdata)
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

		child->for_each_child_recursive( fn, userdata);
	}
}
