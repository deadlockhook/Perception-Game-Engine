#include "entity_manager.h"

master_entity_manager g_master_entity_manager = master_entity_manager();

uint32_t master_entity_manager::create_layer(const s_string& name)
{
	auto manager = &layers.push_back(entity_manager_t());

	if (!manager)
		return -1;

	manager->init_manager(name);
	return layers.count() - 1;
}

void master_entity_manager::destroy_layer(uint32_t id)
{
	if (id >= layers.count())
		return;

	layers[id].destroy();
	layers.erase_at(id);
}

entity_manager_t* master_entity_manager::get_layer(uint32_t id)
{
	if (id >= layers.count()) return nullptr;
	return &layers[id];
}

entity_manager_t* master_entity_manager::get_layer_by_name(const s_string& name) {
	uint32_t hash = fnv1a32(name.c_str());
	for (auto& layer : layers)
		if (layer.lookup_hash_by_name == hash)
			return &layer;
	return nullptr;
}

bool master_entity_manager::has_layer(const s_string& name) {
	return get_layer_by_name(name) != nullptr;
}

void master_entity_manager::on_frame() {
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {

				if (e.parent)
					continue; 

				if (e.on_frame && e._class) {
					__try { e.on_frame(e._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_frame failed: %s", e.name.c_str());
					}
				}

				e.for_each_child_recursive([](entity_t* child, void*) {
					if (child->on_frame && child->_class) {
						__try { child->on_frame(child->_class); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_frame failed: %s", child->name.c_str());
						}
					}
					}, nullptr);
			}
		}
	}
}

void master_entity_manager::on_physics_update() {
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_physics_update && e._class) {
					__try { e.on_physics_update(e._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_physics_update failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void*) {
					if (child->on_physics_update && child->_class) {
						__try { child->on_physics_update(child->_class); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_physics_update failed: %s", child->name.c_str());
						}
					}
					}, nullptr);
			}
		}
	}
}

void master_entity_manager::on_input_receive(input_context_t* ctx) {
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_input_receive && e._class) {
					__try { e.on_input_receive(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_input_receive failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_input_receive && child->_class) {
						__try { child->on_input_receive(child->_class, (input_context_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_input_receive failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}

void master_entity_manager::on_render(render_context_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_render && e._class) {
					__try { e.on_render(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_render failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_render && child->_class) {
						__try { child->on_render(child->_class, (render_context_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("on_render failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}

void master_entity_manager::on_render_ui(render_context_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_render_ui && e._class) {
					__try { e.on_render_ui(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_render_ui failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_render_ui && child->_class) {
						__try { child->on_render_ui(child->_class, (render_context_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("on_render_ui failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}


void master_entity_manager::on_debug_draw(render_context_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_debug_draw && e._class) {
					__try { e.on_debug_draw(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_debug_draw failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_debug_draw && child->_class) {
						__try { child->on_debug_draw(child->_class, (render_context_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("on_debug_draw failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}

void master_entity_manager::on_ui_inspector(render_context_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_ui_inspector && e._class) {
					__try { e.on_ui_inspector(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_ui_inspector failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_ui_inspector && child->_class) {
						__try { child->on_ui_inspector(child->_class, (render_context_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_ui_inspector failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}

void master_entity_manager::on_serialize(serializer_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_serialize && e._class) {
					__try { e.on_serialize(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_serialize failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_serialize && child->_class) {
						__try { child->on_serialize(child->_class, (serializer_t*)ctx_ptr); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_serialize failed: %s", child->name.c_str());
						}
					}
				}, ctx);
			}
		}
	}
}

void master_entity_manager::on_deserialize(deserializer_t* ctx)
{
	for (auto& layer : layers) {
		for (auto& [_, vec] : layer.entities) {
			for (auto& e : vec) {
				if (e.parent) continue;
				if (e.on_deserialize && e._class) {
					__try { e.on_deserialize(e._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_deserialize failed: %s", e.name.c_str());
					}
				}
				e.for_each_child_recursive([](entity_t* child, void* ctx_ptr) {
					if (child->on_deserialize && child->_class) {
						__try { child->on_deserialize(child->_class, (deserializer_t*)ctx); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child on_deserialize failed: %s", child->name.c_str());
						}
					}
					}, ctx);
			}
		}
	}
}

void master_entity_manager::destroy()
{
	for (auto& layer : layers)
		layer.destroy();

	layers.clear();
}