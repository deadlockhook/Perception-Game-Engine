#include "entity_system.h"
#include "../threading/thread_storage.h"
#include "../../crt/s_node_list.h"

entity_manager g_entity_mgr = entity_manager();

entity_layer_t* entity_manager::create_layer(const s_string& name)
{
	entity_layer_t layer;
	layer.init_layer(name);
	return layers.push_back(layer);
}
void entity_manager::destroy_layer(entity_layer_t* layer)
{
	if (!layer)
		return;

	for (auto* n = layers.begin(); n != layers.end(); n = n->next) {
		if (&n->value == layer) {
			n->value.destroy();
			layers.remove_node(n);
			return;
		}
	}
}
void entity_manager::destroy_layer(uint32_t id)
{
	uint32_t index = 0;
	for (auto* n = layers.begin(); n != layers.end(); n = n->next, ++index) {
		if (index == id) {
			n->value.destroy();
			layers.remove_node(n);
			return;
		}
	}
}


entity_layer_t* entity_manager::get_layer(uint32_t id)
{
	uint32_t index = 0;
	for (auto* n = layers.begin(); n != layers.end(); n = n->next, ++index) {
		if (index == id)
			return &n->value;
	}
	return nullptr;
}

entity_layer_t* entity_manager::get_layer_by_name(const s_string& name)
{
	uint32_t hash = fnv1a32(name.c_str());
	for (auto* n = layers.begin(); n != layers.end(); n = n->next) {
		if (n->value.lookup_hash_by_name == hash)
			return &n->value;
	}
	return nullptr;
}

bool entity_manager::has_layer(const s_string& name) {
	return get_layer_by_name(name) != nullptr;
}

void entity_manager::on_frame() {
	auto ts = get_current_thread_storage();

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;
			
			std::cout << "Current entity: " << (void*)(&e)->name.c_str() << " ptr " << &e << std::endl;

			if (e.parent)
				continue;

		
			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_frame) {
					__try { comp.on_frame(comp._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_frame failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_frame ) {
				__try { e.on_frame(e._class); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_frame failed: %s", e.name.c_str());
				}
			}

			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void*) {
				storage->current_entity = child;

				for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
					auto& comp = c_node->value;
					storage->current_component = &comp;
					if (comp.on_frame) {
						__try { comp.on_frame(comp._class); }
						__except (EXCEPTION_EXECUTE_HANDLER) {
							on_fail("child component on_frame failed: %s child entity:%s owner entity:%s",
								comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
						}
					}
				}

				if (child->on_frame) {
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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_physics_update) {
					__try { comp.on_physics_update(comp._class); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_physics_update failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_physics_update ) {
				__try { e.on_physics_update(e._class); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_physics_update failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void*) {
	
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_physics_update) {
							storage->current_entity = child;
							__try { comp.on_physics_update(comp._class); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_physics_update failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_input_receive) {;
					__try { comp.on_input_receive(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_input_receive failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_input_receive ) {
			
				__try { e.on_input_receive(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_input_receive failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
		
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_input_receive) {
							storage->current_entity = child;
							__try { comp.on_input_receive(comp._class, (input_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_input_receive failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_render) {
					;
					__try { comp.on_render(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_render failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_render ) {
		
				__try { e.on_render(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_render failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
	
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_render) {
							storage->current_entity = child;
							__try { comp.on_render(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_render failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_render_ui) {
					;
					__try { comp.on_render_ui(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_render_ui failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_render_ui ) {
		
				__try { e.on_render_ui(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_render_ui failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
		
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_render_ui) {
							storage->current_entity = child;
							__try { comp.on_render_ui(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_render_ui failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_debug_draw) {
					;
					__try { comp.on_debug_draw(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_debug_draw failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_debug_draw ) {
			
				__try { e.on_debug_draw(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_debug_draw failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
		
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_debug_draw) {
							storage->current_entity = child;
							__try { comp.on_debug_draw(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_debug_draw failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_ui_inspector) {
					;
					__try { comp.on_ui_inspector(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_ui_inspector failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_ui_inspector ) {
		
				__try { e.on_ui_inspector(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_ui_inspector failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
			
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_ui_inspector) {
							storage->current_entity = child;
							__try { comp.on_ui_inspector(comp._class, (render_context_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_ui_inspector failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}

					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_serialize) {
					;
					__try { comp.on_serialize(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_serialize failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_serialize ) {
			
				__try { e.on_serialize(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_serialize failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
			
					storage->current_entity = child;
					
					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_serialize) {
							storage->current_entity = child;
							__try { comp.on_serialize(comp._class, (serializer_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_serialize failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* layer_node = layers.begin(); layer_node != layers.end(); layer_node = layer_node->next) {
		auto& layer = layer_node->value;
		ts->current_layer = &layer;

		for (auto* e_node = layer.entities.begin(); e_node != layer.entities.end(); e_node = e_node->next) {
			auto& e = e_node->value;

			if (e.parent)
				continue;

			ts->current_entity = &e;

			for (auto* c_node = e.components.begin(); c_node != e.components.end(); c_node = c_node->next) {
				auto& comp = c_node->value;
				ts->current_component = &comp;
				if (comp.on_deserialize) {
					;
					__try { comp.on_deserialize(comp._class, ctx); }
					__except (EXCEPTION_EXECUTE_HANDLER) {
						on_fail("component on_deserialize failed: %s entity: %s", comp.name.c_str(), e.name.c_str());
					}
				}
			}

			if (e.on_deserialize ) {
				__try { e.on_deserialize(e._class, ctx); }
				__except (EXCEPTION_EXECUTE_HANDLER) {
					on_fail("on_deserialize failed: %s", e.name.c_str());
				}
			}
			e.for_each_child_recursive(ts, [](thread_storage_t* storage, entity_t* child, void* ctx_ptr) {
		
					storage->current_entity = child;

					for (auto* c_node = child->components.begin(); c_node != child->components.end(); c_node = c_node->next) {
						auto& comp = c_node->value;
						storage->current_component = &comp;
						if (comp.on_deserialize) {
							storage->current_entity = child;
							__try { comp.on_deserialize(comp._class, (deserializer_t*)ctx_ptr); }
							__except (EXCEPTION_EXECUTE_HANDLER) {
								on_fail("child component on_deserialize failed: %s child entity:%s owner entity:%s", comp.name.c_str(), child->name.c_str(), child->parent->name.c_str());
							}
						}
					}
					if (child->on_frame) {

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

	for (auto* node = layers.begin(); node != layers.end(); node = node->next) {
		entity_layer_t& layer = node->value;
		ts->current_layer = &layer;
		layer.destroy();
	}

	layers.clear();
}