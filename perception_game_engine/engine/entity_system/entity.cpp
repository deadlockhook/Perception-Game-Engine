#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_string_pool.h"

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


/*
void entity_t::destroy()
{
	detach();
	detach_all_children();

	auto ts = get_current_thread_storage();
	ts->current_layer = owner_layer;
	ts->current_entity = this;

	const size_t component_count = components.size();
	for (size_t i = 0; i < component_count; ++i)
	{
		if (!components.is_alive(i))
			continue;

		components[i].destroy();
	}

	if (on_destruct)
		on_destruct(this, _class);

	components.clear();
}*/

bool entity_t::construct(entity_layer_t* o,
	const s_string& n, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
) {
	owner_layer = o;
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;

	on_construct = construct_fn;
	on_destruct = deconstruct_fn;

	if (on_construct)
		_class = on_construct(this);

	return true;
}

void entity_t::attach_to(entity_t* new_parent)
{
	if (!new_parent || new_parent == this || parent == new_parent)
		return;

	if (parent)
	{
		auto& siblings = parent->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i] == this)
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
				comp.on_parent_detach(parent, this, comp._class);
		}

		if (on_parent_detach)
			on_parent_detach(parent, this, _class);

		parent = nullptr;
	}

	const size_t new_parent_child_count = new_parent->children.size();
	for (size_t i = 0; i < new_parent_child_count; ++i)
	{
		if (!new_parent->children.is_alive(i))
			continue;

		if (new_parent->children[i] == this)
		{
			new_parent->children.remove(i);
			break;
		}
	}

	parent = new_parent;
	new_parent->children.push_back(this);

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
	if (parent)
	{
		if (parent->is_destroyed_or_pending())
			return;

		auto& siblings = parent->children;
		const size_t sibling_count = siblings.size();

		for (size_t i = 0; i < sibling_count; ++i)
		{
			if (!siblings.is_alive(i))
				continue;

			if (siblings[i] == this)
			{
				siblings.remove(i);
				break;
			}
		}

		parent = nullptr;
	}
}

void entity_t::detach_all_children()
{
	const size_t child_count = children.size();

	for (size_t i = 0; i < child_count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		entity_t* child = children[i];

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

		child->parent = nullptr;
	}
}

entity_t* entity_t::get_root() {
	entity_t* current = this;
	while (current->parent)
		current = current->parent;
	return current;
}


void entity_t::for_each_child_recursive(entity_iter_fn_t fn, void* userdata)
{
	const size_t count = children.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		entity_t* child = children[i];
		fn(child, userdata);

		child->for_each_child_recursive( fn, userdata);
	}
}
