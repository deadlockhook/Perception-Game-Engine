#include "entity_system.h"
#include "../threading/thread_storage.h"

entity_manager g_entity_mgr = entity_manager();

s_performance_struct_t entity_manager::create_level(const s_string& name)
{
	s_performance_struct_t level_perf_index = levels.push_back_perf_struct(level_t());
	level_t& level = levels[level_perf_index.index];
	level.init_level(name);
	return level;
}

void entity_manager::destroy_level(const s_performance_struct_t& level_perf_index, bool force)
{
	const size_t count = levels.size();

	for (size_t i = 0; i < count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		auto& level = levels[i];

		if (level.is_destroyed_or_pending())
			continue;

		if (level.index == level_perf_index.index)
		{
			if (force)
			{
				level.destroy(true);
				levels.remove(i);
			}
			else
			{
				level.mark_for_destruction();
			}
			return;
		}
	}
}


template <typename context_t, typename component_callback>
void component_t::process(entity_t* owner, context_t* ctx, component_callback component_cb)
{
	if (is_destroyed_or_pending())
		return;

	component_cb(owner, this, ctx);
}

int somecounter = 0;
template <typename context_t, typename component_callback, typename entity_callback>
void entity_t::process(context_t* ctx, component_callback component_cb, entity_callback entity_cb)
{

	if (is_destroyed_or_pending())
		return;

	if (parent.valid()) {
		somecounter = 1;
		return;
	}


	const size_t component_count = components.size();

	for (size_t k = 0; k < component_count; ++k)
	{
		if (!components.is_alive(k))
			continue;

		components[k].process(this, ctx, component_cb);
	}

	entity_cb(this, ctx);

	for_each_child_recursive(
		[&component_cb, &entity_cb](entity_t* child, void* context_ptr)
		{
			const size_t child_component_count = child->components.size();
			auto& components = child->components;

			for (size_t c = 0; c < child_component_count; ++c)
			{
				if (!components.is_alive(c))
					continue;

				components[c].process(child, context_ptr, component_cb);
			}

			entity_cb(child, context_ptr);
		}, ctx);
}
template <typename context_t, typename component_callback, typename entity_callback>
void entity_layer_t::process(context_t* ctx, component_callback component_cb, entity_callback entity_cb)
{
	const size_t entity_count = entities.size();

	for (size_t j = 0; j < entity_count; ++j)
	{
		if (!entities.is_alive(j))
			continue;

		entities[j].process(ctx, component_cb, entity_cb);
	}
}

template <typename context_t, typename component_callback, typename entity_callback>
void entity_manager::process(context_t* ctx, component_callback component_cb, entity_callback entity_cb)
{

	entity_t current_entity;

	const size_t levels_count = levels.size();

	for (size_t i = 0; i < levels_count; ++i)
	{
		if (!levels.is_alive(i))
			continue;

		level_t& level = levels[i];

		if (level.is_destroyed_or_pending())
			continue;

		auto& layers = level.layers;

		const size_t layer_count = layers.size();

		for (size_t i = 0; i < layer_count; ++i)
		{
			if (!layers.is_alive(i))
				continue;

			entity_layer_t& layer = layers[i];

			if (layer.is_destroyed_or_pending())
				continue;

			layer.process(ctx, component_cb, entity_cb);
		}
	}
}


void entity_manager::on_frame_start()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_frame_start)
				comp->on_frame_start(e, comp->_class);

		},
		[](entity_t* e, void*)
		{
			if (e->on_frame_start)
				e->on_frame_start(e, e->_class);
		}
	);
}

void entity_manager::on_frame_update()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_frame_update)
				comp->on_frame_update(e, comp->_class);

		},
		[](entity_t* e, void*)
		{
			if (e->on_frame_update)
				e->on_frame_update(e, e->_class);
		}
	);
}

void entity_manager::on_frame_end()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_frame_end)
				comp->on_frame_end(e, comp->_class);

		},
		[](entity_t* e, void*)
		{
			if (e->on_frame_end)
				e->on_frame_end(e, e->_class);
		}
	);
}

void entity_manager::on_physics_start()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_physics_start)
				comp->on_physics_start(e, comp->_class);
		},
		[](entity_t* e, void*)
		{
			if (e->on_physics_start)
				e->on_physics_start(e, e->_class);
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

void entity_manager::on_physics_end()
{
	process<void>(nullptr,
		[](entity_t* e, component_t* comp, void*)
		{
			if (comp->on_physics_end)
				comp->on_physics_end(e, comp->_class);
		},
		[](entity_t* e, void*)
		{
			if (e->on_physics_end)
				e->on_physics_end(e, e->_class);
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

void entity_manager::on_gc()
{
	const size_t levels_count = levels.size();

	for (size_t l = 0; l < levels_count; ++l)
	{
		if (!levels.is_alive(l))
			continue;

		level_t& level = levels[l];

		if (level.is_destroyed())
			continue;

		if (level.is_destroy_pending())
		{
			if (level.can_destroy()) {
				level.destroy(true);
				levels.remove(l);
				std::cout << "level destroyed" << std::endl;
			}
			continue;
		}

		const size_t layer_count = level.layers.size();

		for (size_t la = 0; la < layer_count; ++la)
		{
			if (!level.layers.is_alive(la))
				continue;

			entity_layer_t& layer = level.layers[la];

			if (layer.is_destroyed())
				continue;

			if (layer.is_destroy_pending())
			{
				if (layer.can_destroy()) {
					layer.destroy(true);
					level.layers.remove(la);
					std::cout << "layer destroyed" << std::endl;
				}
				continue;
			}

			const size_t entities_count = layer.entities.size();

			for (size_t ea = 0; ea < entities_count; ++ea)
			{
				if (!layer.entities.is_alive(ea))
					continue;

				entity_t& entity = layer.entities[ea];

				if (entity.is_destroyed())
					continue;

				if (entity.is_destroy_pending())
				{
					if (entity.can_destroy()) {
						entity.destroy(true);
						layer.entities.remove(ea);
						std::cout << "entity destroyed" << std::endl;
					}
					continue;
				}

				const size_t component_count = entity.components.size();
				for (size_t c = 0; c < component_count; ++c)
				{
					if (!entity.components.is_alive(c))
						continue;

					component_t& comp = entity.components[c];

					if (comp.is_destroyed())
						continue;

					if (comp.is_destroy_pending())
					{
						if (comp.can_destroy())
						{
							comp.destroy(true);
							entity.components.remove(c);
							std::cout << "component destroyed" << std::endl;
						}
					}
				}
			}
		}
	}
}

void entity_manager::destroy()
{
	execute_stop();

	const size_t levels_count = levels.size();

	for (size_t l = 0; l < levels_count; ++l)
	{
		if (!levels.is_alive(l))
			continue;

		level_t& level = levels[l];

		if (level.is_destroyed())
			continue;

		level.destroy(true);
	}
}


void entity_manager::execute_stop()
{
	t_on_frame.terminate_and_close();
	t_on_physics_update.terminate_and_close();
	t_on_gc.terminate_and_close();
}

class test_component
{
public:
	float xyz[3];
};

test_component test;

class_t* on_create_test(entity_t* e)
{
	return (class_t*)&test;
}

void entity_manager::execute_start()
{
	auto level = get_level(create_level("Test Level").index);

	auto layer = level->get_layer(level->create_layer("Test Layer").index);

	for (int i = 0; i < 1000000; i++)
	{
		auto entity = layer->get_entity(layer->create_entity("Test Entity").index);
		auto transform = entity->add_transform_component(vector3(0.0f, 0.0f, 0.0f), quat::identity(), vector3(1.0f, 1.0f, 1.0f));
		auto test_component = entity->add_component("test", on_create_test, nullptr);
	}

	auto transform_entity = layer->get_entity(layer->create_entity("transform1").index);

	auto transform_entity2 = layer->get_entity(layer->create_entity("transform2").index);

	auto transform = transform_entity->add_transform_component(vector3(4.0f, 30.0f, 10.0f), quat::identity(), vector3(1.0f, 1.0f, 1.0f));
	//auto transform2 = transform_entity2->add_transform_component(vector3(40.0f, 0.0f, 0.0f), quat::identity(), vector3(1.0f, 1.0f, 1.0f));

	std::cout << " level pointer " << level << std::endl;
	//std::cout << " layer pointer" << layer << std::endl;
	//std::cout << " transform entity 1 pointer " << transform_entity << std::endl;
	//std::cout << " transform pointer " << transform << std::endl;

	//level->destroy();

	//static auto transform2 = transform_entity2->add_transform_component(vector3(40.0f, 0.0f, 0.0f), quat::identity(), vector3(1.0f, 1.0f, 1.0f));
	//transform_entity->attach_to(transform_entity2);

	t_on_frame.create(execute_on_frame, nullptr);
	t_on_physics_update.create(execute_on_physics_update, nullptr);
	t_on_gc.create(execute_gc, nullptr);

}