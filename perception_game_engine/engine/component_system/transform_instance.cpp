#include "transform_instance.h"
#include "physics_asset.h"

class_t* transform_instance_t::create_transform(entity_t* e) {
    return (class_t*)new transform_instance_t();
}


void transform_instance_t::on_frame_update_internal(entity_t* e, class_t* c)
{

    auto& curr_data = get_transform_data_for_tick(g_vars.engine.frame_thread.last_physics_tick);
    auto& prev_data = get_transform_data_for_tick(g_vars.engine.frame_thread.last_physics_tick - 1);

    double physics_update_end_time = g_vars.engine.physics_thread.physics_update_end_time[g_vars.engine.frame_thread.last_physics_tick % max_physics_ticks];

    double lerp_alpha = clamp(
        g_vars.engine.physics_thread.frame_data.time_to_ticks(g_vars.engine.frame_thread.frame_data.current_time - physics_update_end_time),
        0.0, 1.0
    );

   // std::cout << "interp pos x  " << e->name.str->c_str() << "\n";
}

void transform_instance_t::on_frame_update(entity_t* e, class_t* c)
{
	auto* t = reinterpret_cast<transform_instance_t*>(c);
    t->on_frame_update_internal(e, c);

   // if (g_vars.engine.frame_thread.last_physics_tick == 0)
		//return;

    /*
    if (curr_data.world_transform.position.x == 0.0)
    {
        t->queue_set_position(vector3(4.0,7.0,9.0));
    }

    if (curr_data.world_transform.position.x == 4.0)
    {
        t->queue_set_position(vector3(0.0, 0.0, 0.0));
    }*/

  //  std::cout << "interp pos x  " << e->name.str << "\n";

 //   std::cout << "interp pos x  " << e->name.str->c_str() << "\n";
  
   // if (false) {
      //  std::cout << "interp pos x  " << curr_data.local_transform.position.x << " ";
      //  std::cout << "interp pos y  " << curr_data.local_transform.position.y << " ";
      //  std::cout << "interp pos z  " << curr_data.local_transform.position.z << std::endl;
   // }

    //interp and stuff will happen here
}

void transform_instance_t::on_physics_update(entity_t* e, class_t* c)
{
    auto* t = reinterpret_cast<transform_instance_t*>(c);

	t->on_physics_update_internal(e, c);
}

void transform_instance_t::on_physics_update_internal(entity_t* e, class_t* c)
{
    auto& new_tick_data = get_transform_data_for_tick(g_vars.engine.physics_thread.frame_data.tick_count);
    auto& prev_tick_data = get_transform_data_for_tick(g_vars.engine.physics_thread.frame_data.tick_count - 1);

    new_tick_data = prev_tick_data;

    update_transform(e, g_vars.engine.physics_thread.frame_data.tick_count);
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
