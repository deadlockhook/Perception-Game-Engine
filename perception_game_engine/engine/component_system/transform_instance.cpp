#include "transform_instance.h"
#include "physics_asset.h"

class_t* transform_instance_t::create_transform(entity_t* e) {
    return (class_t*)new transform_instance_t();
}

void transform_instance_t::on_frame_update(entity_t* e, class_t* c)
{
	auto* t = reinterpret_cast<transform_instance_t*>(c);
    t->world_transform.sync();
	t->local_transform.sync();
}

void transform_instance_t::on_physics_update(entity_t* e, class_t* c)
{
    if (e->get_component<physics_asset_t>(physics_asset_t_register_hash))
        return;

    auto* t = reinterpret_cast<transform_instance_t*>(c);
    
    t->update_transform(e);
}

void transform_instance_t::destroy_transform(entity_t* e, class_t* c) {
    delete reinterpret_cast<transform_instance_t*>(c);
}

void transform_instance_t::set_all_transforms_available(entity_layer_t* layer)
{
    auto& transform_components = layer->get_components_by_hash(transform_instance_t_register_hash);

    for (size_t i = 0; i < transform_components.size(); ++i) {
        if (!transform_components.is_alive(i))
            continue;

        auto* transform = reinterpret_cast<transform_instance_t*>(transform_components[i]->_class);
       
        if (transform->transform_updated_this_tick)
        {
            transform->local_transform.set_available();
            transform->world_transform.set_available();
			transform->transform_updated_this_tick = false;
        }
    }
}