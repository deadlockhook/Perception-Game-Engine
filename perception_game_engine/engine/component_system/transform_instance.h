#pragma once
#include "default_component_hashes.h"
#include "../entity_system/entity_system.h"
#include "../../math/transform.h"
#include "../../math/vector3.h"
#include "../../math/quat.h"
#include "../../math/aabb.h"
#include "../../crt/triple_buffer.h"



enum transform_flags_t : uint32_t {
    transform_flag_none = 0,
    transform_flag_dirty = 1 << 0,
};

struct transform_instance_t {

    triple_buffer_t<transform_t> local_transform; 
    triple_buffer_t<transform_t> world_transform;  

    std::atomic<uint32_t> flags = transform_flag_dirty;

    bool transform_updated_this_tick = false;

    static class_t* create_transform(entity_t* e);
    static void on_physics_update(entity_t* e, class_t* c);
    static void on_frame_update(entity_t* e, class_t* c);

    static void destroy_transform(entity_t* e, class_t* c);
    static void sync_all_transforms(entity_layer_t* layer);
    static void set_all_transforms_available(entity_layer_t* layer);

    void update_transform(entity_t* e)
    {
        if (is_dirty()) {

            transform_t parent_world = transform_t::identity();

            if (e && e->parent) {
                auto* parent_transform = e->parent->get_component<transform_instance_t>(transform_instance_t_register_hash);

                if (parent_transform)
                    parent_world = parent_transform->world_transform.wb();

            }
	
            world_transform.wb() = parent_world.combine(local_transform.wb());
            clear_dirty();
            transform_updated_this_tick = true;
        }
    }

    void mark_dirty() {
        flags.fetch_or(transform_flag_dirty, std::memory_order_relaxed);
    }

    void clear_dirty() {
        flags.fetch_and(~transform_flag_dirty, std::memory_order_relaxed);
    }

    bool is_dirty() const {
        return (flags.load(std::memory_order_acquire) & transform_flag_dirty) != 0;
    }

  
    void set_local_transform(const transform_t& t) {
        local_transform.wb() = t;  
        mark_dirty();
    }

    void translate_local(const vector3& delta) {
        local_transform.wb().position += delta;
        mark_dirty();
    }

    void rotate_local(const quat& delta) {
        local_transform.wb().rotation = delta * local_transform.wb().rotation;
        mark_dirty();
    }

    void scale_local(const vector3& factor) {
        local_transform.wb().scale = local_transform.wb().scale * factor;
        mark_dirty();
    }

    void sync_world_to_local(entity_t* self, const transform_t& new_world) {
         //redo
    }

    void print(entity_t* self) const {
        const transform_t& w = const_cast<transform_instance_t*>(this)->world_transform.wb();
        vector3 euler = w.rotation.to_euler().to_degrees();

        std::cout << "[transform] Pos: (" << w.position.x << ", " << w.position.y << ", " << w.position.z << ")\n";
        std::cout << "           Rot (deg): (" << euler.x << ", " << euler.y << ", " << euler.z << ")\n";
        std::cout << "           Scale: (" << w.scale.x << ", " << w.scale.y << ", " << w.scale.z << ")\n";
    }
};