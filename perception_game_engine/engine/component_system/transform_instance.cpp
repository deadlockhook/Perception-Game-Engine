#include "transform_instance.h"
#include "physics_asset.h"

class_t* transform_instance_t::create_transform(entity_t* e) {
    return (class_t*)new transform_instance_t();
}

void transform_instance_t::on_frame_update(entity_t* e, class_t* c)
{
	auto* t = reinterpret_cast<transform_instance_t*>(c);
	auto last_physics_tick = g_vars.get_update_end_tickcount(thread_ids_t::thread_id_physics);
	
    if (last_physics_tick == 0)
		return;

	auto& curr_data = t->get_transform_data_for_tick(last_physics_tick);
	auto& prev_data = t->get_transform_data_for_tick(last_physics_tick - 1);

    //interp and stuff will happen here
}

void transform_instance_t::on_physics_update(entity_t* e, class_t* c)
{
    if (e->get_component<physics_asset_t>(physics_asset_t_register_hash))
        return;

    auto* t = reinterpret_cast<transform_instance_t*>(c);
	t->update_transform(e, g_vars.get_update_end_tickcount(thread_ids_t::thread_id_physics));
}

void transform_instance_t::on_parent_detach(entity_t* parent, entity_t* self, class_t* c) {
    auto* transform = reinterpret_cast<transform_instance_t*>(c);
	std::cout << "on_parent_detach" << std::endl;
    transform->parent_change_request = { nullptr, true };
    transform->flags.fetch_or(transform_flag_world_dirty | transform_flag_parent_change, std::memory_order_relaxed);
}

void transform_instance_t::on_parent_attach(entity_t* new_parent, entity_t* self, class_t* c) {
    auto* transform = reinterpret_cast<transform_instance_t*>(c);
	std::cout << "on_parent_attach" << std::endl;
    transform->parent_change_request = { new_parent, false };
    transform->flags.fetch_or(transform_flag_world_dirty | transform_flag_parent_change, std::memory_order_relaxed);
}


void transform_instance_t::destroy_transform(entity_t* e, class_t* c) {
    delete reinterpret_cast<transform_instance_t*>(c);
}
