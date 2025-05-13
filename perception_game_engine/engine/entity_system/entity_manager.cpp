#include "entity_system.h"
#include "../threading/thread_storage.h"

entity_manager g_entity_mgr = entity_manager();

level_t* entity_manager::create_level(const s_string& name)
{
	level_t* layer = levels.emplace_back();
	layer->init_level(name);
	return layer;
}

void entity_manager::destroy_level(level_t* layer)
{
	if (!layer)
		return;

	const size_t count = levels.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		if (&levels[i] == layer)
		{
			levels[i].destroy();
			levels.remove(i);
			return;
		}
	}
}

level_t* entity_manager::get_level(uint32_t id)
{
	const size_t count = levels.size();
	uint32_t alive_index = 0;

	for (size_t i = 0; i < count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		if (alive_index == id)
			return &levels[i];

		++alive_index;
	}

	return nullptr;
}

level_t* entity_manager::get_level_by_name(const s_string& name)
{
	const uint32_t hash = fnv1a32(name.c_str());
	const size_t count = levels.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		if (levels[i].lookup_hash_by_name == hash)
			return &levels[i];
	}

	return nullptr;
}

bool entity_manager::has_level(const s_string& name) {
	return get_level_by_name(name) != nullptr;
}

template <typename context_t, typename component_callback, typename entity_callback>
void entity_manager::process(context_t* ctx, component_callback component_cb, entity_callback entity_cb)
{
	auto ts = get_current_thread_storage();
	
	const size_t levels_count = levels.size();

	for (size_t i = 0; i < levels_count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		level_t& level = levels[i];

		const size_t layer_count = level.layers.size();

		for (size_t i = 0; i < layer_count; ++i)
		{
			if (!level.layers.is_alive(i))
				continue;

			entity_layer_t& layer = level.layers[i];
			ts->current_layer = &layer;

			const size_t entity_count = layer.entities.size();

			for (size_t j = 0; j < entity_count; ++j)
			{
				if (!layer.entities.is_alive(j))
					continue;

				entity_t& e = layer.entities[j];
				if (e.parent)
					continue;

				ts->current_entity = &e;

				const size_t component_count = e.components.size();
				for (size_t k = 0; k < component_count; ++k)
				{
					if (!e.components.is_alive(k))
						continue;

					component_t& comp = e.components[k];
					ts->current_component = &comp;

					component_cb(&e, &comp, ctx);
				}

				entity_cb(&e, ctx);

				e.for_each_child_recursive(ts,
					[&component_cb, &entity_cb](thread_storage_t* storage, entity_t* child, void* context_ptr)
					{
						storage->current_entity = child;

						const size_t child_component_count = child->components.size();
						for (size_t c = 0; c < child_component_count; ++c)
						{
							if (!child->components.is_alive(c))
								continue;

							component_t& comp = child->components[c];
							storage->current_component = &comp;

							component_cb(child, &comp, context_ptr);
						}

						entity_cb(child, context_ptr);
					}, ctx);
			}
		}
	}
}


void entity_manager::on_frame()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_frame)
				comp->on_frame(e, comp->_class);
			
		},
		[](entity_t* e, void*)
		{
			if (e->on_frame)
				e->on_frame(e, e->_class); 
		}
	);
}


void entity_manager::on_physics_update()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_physics_update)
				comp->on_physics_update(e, comp->_class);
		},
		[](entity_t* e, void*)
		{
			if (e->on_physics_update)
				e->on_physics_update(e, e->_class);
		}
	);
}

void entity_manager::on_input_receive(input_context_t* ctx)
{
	process<input_context_t>(ctx,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_input_receive)
				comp->on_input_receive(e, comp->_class, static_cast<input_context_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_input_receive)
				e->on_input_receive(e, e->_class, static_cast<input_context_t*>(context));
		}
	);
}

void entity_manager::on_render(render_context_t* ctx)
{
	process<render_context_t>(ctx,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_render)
				comp->on_render(e, comp->_class, static_cast<render_context_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_render)
				e->on_render(e, e->_class, static_cast<render_context_t*>(context));
		}
	);
}

void entity_manager::on_render_ui(render_context_t* ctx)
{
	process<render_context_t>(ctx,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_render_ui)
				comp->on_render_ui(e, comp->_class, static_cast<render_context_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_render_ui)
				e->on_render_ui(e, e->_class, static_cast<render_context_t*>(context));
		}
	);
}

void entity_manager::on_debug_draw(render_context_t* ctx)
{
	process<render_context_t>(ctx,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_debug_draw)
				comp->on_debug_draw(e, comp->_class, static_cast<render_context_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_debug_draw)
				e->on_debug_draw(e, e->_class, static_cast<render_context_t*>(context));
		}
	);
}

void entity_manager::on_ui_inspector(render_context_t* ctx)
{
	process<render_context_t>(ctx,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_ui_inspector)
				comp->on_ui_inspector(e, comp->_class, static_cast<render_context_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_ui_inspector)
				e->on_ui_inspector(e, e->_class, static_cast<render_context_t*>(context));
		}
	);
}

void entity_manager::on_serialize(serializer_t* s)
{
	process<serializer_t>(s,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_serialize)
				comp->on_serialize(e, comp->_class, static_cast<serializer_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_serialize)
				e->on_serialize(e, e->_class, static_cast<serializer_t*>(context));
		}
	);
}

void entity_manager::on_deserialize(deserializer_t* d)
{
	process<deserializer_t>(d,
		[](entity_t* e, component_t* comp, void* context)
		{
			if (comp->on_deserialize)
				comp->on_deserialize(e, comp->_class, static_cast<deserializer_t*>(context));
		},
		[](entity_t* e, void* context)
		{
			if (e->on_deserialize)
				e->on_deserialize(e, e->_class, static_cast<deserializer_t*>(context));
		}
	);
}

void entity_manager::destroy()
{
	execute_stop();

	auto ts = get_current_thread_storage();

	const size_t count = levels.size();
	for (size_t i = 0; i < count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		level_t& level = levels[i];
		ts->current_level = &level;
		level.destroy();
	}

	levels.clear();
}


void entity_manager::execute_stop()
{
	t_on_frame.terminate_and_close();
	t_on_physics_update.terminate_and_close();
}

class test_component
{
public:
	float xyz[3];
};

test_component test;

class_t* on_create_test(entity_t* e, user_data_t* data)
{
	return (class_t*)&test;
}

void entity_manager::execute_start()
{
	auto level = create_level("Test Level");

	auto layer = level->create_layer("Test Layer");

	for (int test_entity_index = 0; test_entity_index < 100000; ++test_entity_index) {
	     auto entity = layer->create_entity("Test Entity");
		 entity->add_component("test", on_create_test);
		 entity->add_component("test2", on_create_test);
		 entity->add_component("test3", on_create_test);
		 entity->add_component("test4", on_create_test);
		 entity->add_component("test5", on_create_test);
		 entity->add_component("test6", on_create_test);
		 entity->add_component("test7", on_create_test);
		 entity->add_component("test8", on_create_test);
		 entity->add_component("test9", on_create_test);
		 entity->add_component("test10", on_create_test);
		 //entity->add_component("test2", on_create_test);
		 //entity->add_component("test3", on_create_test);
	}

	std::cout << " count " << layer->entities.size() << "\n";

	t_on_frame.create(execute_on_frame, nullptr);
	//t_on_physics_update.create(execute_on_physics_update, nullptr);
}