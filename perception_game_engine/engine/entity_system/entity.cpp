#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_string_pool.h"

void entity_t::destroy()
{
	detach();
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
}

bool entity_t::construct(entity_layer_t* o,
	const s_string& n, construct_fn_t construct_fn, destruct_fn_t deconstruct_fn
) {
	owner_layer = o;
	name = s_pooled_string(n);
	lookup_hash_by_name = name.hash;

	on_construct = construct_fn;
	on_destruct = deconstruct_fn;

	auto ts = get_current_thread_storage();
	ts->current_layer = owner_layer;
	ts->current_entity = this;
	if (on_construct)
		_class = on_construct(this);

	return true;
}

void entity_t::attach_to(entity_t* e)
{
	if (!e || e == this || parent == e)
		return;

	const size_t child_count = children.size();
	for (size_t i = 0; i < child_count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		if (children[i] == e)
		{
			children.remove(i);
			e->parent = nullptr;
			break;
		}
	}

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

		parent = nullptr;
	}

	const size_t e_child_count = e->children.size();
	for (size_t i = 0; i < e_child_count; ++i)
	{
		if (!e->children.is_alive(i))
			continue;

		if (e->children[i] == this)
		{
			e->children.remove(i);
			break;
		}
	}

	parent = e;
	e->children.push_back(this);
}


void entity_t::detach()
{
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

		parent = nullptr;
	}
}



entity_t* entity_t::get_root() {
	entity_t* current = this;
	while (current->parent)
		current = current->parent;
	return current;
}


void entity_t::for_each_child_recursive(thread_storage_t* storage, entity_iter_fn_t fn, void* userdata)
{
	const size_t count = children.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!children.is_alive(i))
			continue;

		entity_t* child = children[i];
		fn(storage, child, userdata);

		child->for_each_child_recursive(storage, fn, userdata);
	}
}
