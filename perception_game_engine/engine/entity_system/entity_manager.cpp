#include "entity_system.h"
#include "../threading/thread_storage.h"


entity_manager g_entity_mgr = entity_manager();

uint32_t entity_manager::create_layer(const s_string& name)
{
	auto manager = &layers.push_back(entity_layer_t());

	if (!manager)
		return -1;

	manager->init_layer(name);
	return layers.count() - 1;
}

void entity_manager::destroy_layer(uint32_t id)
{
	if (id >= layers.count())
		return;

	layers[id].destroy();
	layers.erase_at(id);
}

entity_layer_t* entity_manager::get_layer(uint32_t id)
{
	if (id >= layers.count()) return nullptr;
	return &layers[id];
}

entity_layer_t* entity_manager::get_layer_by_name(const s_string& name) {
	uint32_t hash = fnv1a32(name.c_str());
	for (auto& layer : layers)
		if (layer.lookup_hash_by_name == hash)
			return &layer;
	return nullptr;
}

bool entity_manager::has_layer(const s_string& name) {
	return get_layer_by_name(name) != nullptr;
}

void entity_manager::on_frame() {
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_frame && comp._class) {
					__try { comp.on_frame(comp._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_frame failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_frame && e._class) {
				__try { e.on_frame(e._class); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_frame failed: %s", e.name.c_str());
				}
			}

			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void*) {
				if (child->on_frame && child->_class) {

					storage->current_entity = child;
					for (auto& comp : child->components) {
						if (comp.on_frame && comp._class) {
							storage->current_entity = child;
							__try { comp.on_frame(comp._class); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_frame failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_frame(child->_class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_frame failed: %s", child->name.c_str());
					}


				}
				}, nullptr);


		}

	}
}

void entity_manager::on_physics_update() {
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;

		for (auto& e : layer.entities) {
			if (e.parent) continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_physics_update && comp._class) {
					__try { comp.on_physics_update(comp._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_physics_update failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_physics_update && e._class) {
				__try { e.on_physics_update(e._class); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_physics_update failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void*) {
				if (child->on_physics_update && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_physics_update && comp._class) {
							storage->current_entity = child;
							__try { comp.on_physics_update(comp._class); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_physics_update failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_physics_update(child->_class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_physics_update failed: %s", child->name.c_str());
					}
				}
				}, nullptr);
		}

	}
}

void entity_manager::on_input_receive(input_context_t* ctx) {
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_input_receive && comp._class) {;
					__try { comp.on_input_receive(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_input_receive failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_input_receive && e._class) {
			
				__try { e.on_input_receive(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_input_receive failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_input_receive && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_input_receive && comp._class) {
							storage->current_entity = child;
							__try { comp.on_input_receive(comp._class, (input_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_input_receive failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_input_receive(child->_class, (input_context_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_input_receive failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::on_render(render_context_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {

			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_render && comp._class) {
					;
					__try { comp.on_render(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_render failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_render && e._class) {
		
				__try { e.on_render(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_render failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_render && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_render && comp._class) {
							storage->current_entity = child;
							__try { comp.on_render(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_render failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_render(child->_class, (render_context_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_render failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::on_render_ui(render_context_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_render_ui && comp._class) {
					;
					__try { comp.on_render_ui(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_render_ui failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_render_ui && e._class) {
		
				__try { e.on_render_ui(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_render_ui failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_render_ui && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_render_ui && comp._class) {
							storage->current_entity = child;
							__try { comp.on_render_ui(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_render_ui failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_render_ui(child->_class, (render_context_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_render_ui failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}


void entity_manager::on_debug_draw(render_context_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
			if (e.parent)
				continue;
			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_debug_draw && comp._class) {
					;
					__try { comp.on_debug_draw(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_debug_draw failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_debug_draw && e._class) {
			
				__try { e.on_debug_draw(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_debug_draw failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_debug_draw && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_debug_draw && comp._class) {
							storage->current_entity = child;
							__try { comp.on_debug_draw(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_debug_draw failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_debug_draw(child->_class, (render_context_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("on_debug_draw failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::on_ui_inspector(render_context_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
			
			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_ui_inspector && comp._class) {
					;
					__try { comp.on_ui_inspector(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_ui_inspector failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_ui_inspector && e._class) {
		
				__try { e.on_ui_inspector(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_ui_inspector failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_ui_inspector && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_ui_inspector && comp._class) {
							storage->current_entity = child;
							__try { comp.on_ui_inspector(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_ui_inspector failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}


					__try { child->on_ui_inspector(child->_class, (render_context_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_ui_inspector failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::on_serialize(serializer_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
		
			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_serialize && comp._class) {
					;
					__try { comp.on_serialize(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_serialize failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_serialize && e._class) {
			
				__try { e.on_serialize(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_serialize failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_serialize && child->_class) {
					storage->current_entity = child;
					
					for (auto& comp : child->components) {
						if (comp.on_serialize && comp._class) {
							storage->current_entity = child;
							__try { comp.on_serialize(comp._class, (serializer_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_serialize failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_serialize(child->_class, (serializer_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_serialize failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::on_deserialize(deserializer_t* ctx)
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		for (auto& e : layer.entities) {
			if (e.parent) 
				continue;

			ts->current_entity = &e;

			for (auto& comp : e.components) {
				if (comp.on_deserialize && comp._class) {
					;
					__try { comp.on_deserialize(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_deserialize failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_deserialize && e._class) {
				__try { e.on_deserialize(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_deserialize failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
				if (child->on_deserialize && child->_class) {
					storage->current_entity = child;

					for (auto& comp : child->components) {
						if (comp.on_deserialize && comp._class) {
							storage->current_entity = child;
							__try { comp.on_deserialize(comp._class, (deserializer_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_deserialize failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					__try { child->on_deserialize(child->_class, (deserializer_t*)ctx_ptr); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("child on_deserialize failed: %s", child->name.c_str());
					}
				}
				}, ctx);
		}

	}
}

void entity_manager::destroy()
{
	auto ts = get_current_thread_storage();

	for (auto& layer : layers) {
		ts->current_layer = &layer;
		layer.destroy();
	}

	layers.clear();
}