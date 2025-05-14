#pragma once
#include "default_component_hashes.h"
#include "../entity_system/entity_system.h"
#include "../../math/transform.h"
#include "../../math/vector3.h"
#include "../../math/quat.h"
#include "../../math/aabb.h"
#include "../../crt/triple_buffer.h"
#include "../global_vars.h"

enum transform_flags_t : uint32_t {
    transform_flag_none = 0,
    transform_flag_world_dirty = 1 << 0,
    transform_flag_position_delta = 1 << 1,
    transform_flag_rotation_delta = 1 << 2,
    transform_flag_scale_delta = 1 << 3,
    transform_flag_absolute_position = 1 << 4,
    transform_flag_absolute_rotation = 1 << 5,
    transform_flag_absolute_scale = 1 << 6,
    transform_flag_parent_change = 1 << 7, 
};

struct transform_instance_t {

    std::atomic<uint32_t> flags = transform_flag_none;

    struct transform_data_t {
        transform_t local_transform;
        transform_t world_transform;
    };

    transform_data_t transform_data[max_physics_ticks];
    transform_data_t interp_transform;

    vector3 queued_position_delta = vector3::zero();
    quat queued_rotation_delta = quat::identity();
    vector3 queued_scale_factor = vector3(1.0f, 1.0f, 1.0f);
    vector3 queued_absolute_position;
    quat queued_absolute_rotation;
    vector3 queued_absolute_scale;
   
    parent_change_request_t parent_change_request;

    __forceinline transform_data_t& get_transform_data_for_tick(uint64_t tick) {
        return transform_data[tick % max_physics_ticks];
    }

    __forceinline const transform_data_t& get_transform_data_for_tick(uint64_t tick) const {
        return transform_data[tick % max_physics_ticks];
    }

    static class_t* create_transform(entity_t* e);
    static void on_physics_update(entity_t* e, class_t* c);
    static void on_frame_update(entity_t* e, class_t* c);
    static void destroy_transform(entity_t* e, class_t* c);

    static void on_parent_detach(entity_t* parent, entity_t* self, class_t* c);
    static void on_parent_attach(entity_t* new_parent, entity_t* self, class_t* c);


    void queue_translate(const vector3& delta) {
        queued_position_delta += delta;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_position_delta, std::memory_order_relaxed);
    }

    void queue_rotate(const quat& delta) {
        queued_rotation_delta = delta * queued_rotation_delta;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_rotation_delta, std::memory_order_relaxed);
    }

    void queue_scale(const vector3& factor) {
        queued_scale_factor = queued_scale_factor * factor;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_scale_delta, std::memory_order_relaxed);
    }

    void queue_set_position(const vector3& pos) {
        queued_absolute_position = pos;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_absolute_position, std::memory_order_relaxed);
    }

    void queue_set_rotation(const quat& rot) {
        queued_absolute_rotation = rot;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_absolute_rotation, std::memory_order_relaxed);
    }

    void queue_set_scale(const vector3& scl) {
        queued_absolute_scale = scl;
        flags.fetch_or(transform_flag_world_dirty | transform_flag_absolute_scale, std::memory_order_relaxed);
    }
  
    void update_transform(entity_t* e, uint64_t tick) {
        uint32_t current_flags = flags.load(std::memory_order_acquire);

        if (current_flags & transform_flag_parent_change) {
            auto& data = transform_data[tick % max_physics_ticks];

            if (parent_change_request.detach) {
                data.local_transform = data.world_transform;
            }
            else if (parent_change_request.new_parent) {
                if (auto* parent_transform = parent_change_request.new_parent->get_component<transform_instance_t>(transform_instance_t_register_hash)) {
                    auto parent_world_inv = parent_transform->transform_data[tick % max_physics_ticks].world_transform.inverse();
                    data.local_transform = parent_world_inv.combine(data.world_transform);
                }
            }

            flags.fetch_and(~transform_flag_parent_change, std::memory_order_relaxed);
        }

        if (current_flags & transform_flag_world_dirty) {
            auto& data = transform_data[tick % max_physics_ticks];

            if (current_flags & transform_flag_absolute_position) {
                data.local_transform.position = queued_absolute_position;
            }
            if (current_flags & transform_flag_absolute_rotation) {
                data.local_transform.rotation = queued_absolute_rotation;
            }
            if (current_flags & transform_flag_absolute_scale) {
                data.local_transform.scale = queued_absolute_scale;
            }

            if (current_flags & transform_flag_position_delta) {
                data.local_transform.position += queued_position_delta;
            }
            if (current_flags & transform_flag_rotation_delta) {
                data.local_transform.rotation = queued_rotation_delta * data.local_transform.rotation;
            }
            if (current_flags & transform_flag_scale_delta) {
                data.local_transform.scale = data.local_transform.scale * queued_scale_factor;
            }

            if (e && e->parent) {
                if (auto* parent_transform = e->parent->get_component<transform_instance_t>(transform_instance_t_register_hash)) {
                    auto parent_world = parent_transform->transform_data[tick % max_physics_ticks].world_transform;
                    data.world_transform = parent_world.combine(data.local_transform);
                }
                else {
                    data.world_transform = data.local_transform;
                }
            }
            else {
                data.world_transform = data.local_transform;
            }

            queued_position_delta = vector3::zero();
            queued_rotation_delta = quat::identity();
            queued_scale_factor = vector3(1.0f, 1.0f, 1.0f);

            flags.fetch_and(~(transform_flag_world_dirty |
                transform_flag_position_delta |
                transform_flag_rotation_delta |
                transform_flag_scale_delta |
                transform_flag_absolute_position |
                transform_flag_absolute_rotation |
                transform_flag_absolute_scale), std::memory_order_relaxed);

            const size_t child_count = e->children.size();
            for (size_t i = 0; i < child_count; ++i) {
                if (!e->children.is_alive(i))
                    continue;

                entity_t* child_entity = e->children[i];
                if (!child_entity)
                    continue;

                auto* child_transform = child_entity->get_component<transform_instance_t>(transform_instance_t_register_hash);
                if (child_transform) {
                    child_transform->flags.fetch_or(transform_flag_world_dirty, std::memory_order_relaxed);
                }
            }
        }
    }


    // Debug print
    void print(entity_t* self, uint64_t tick) const {
        auto& data = transform_data[tick % max_physics_ticks];

        const transform_t& w = data.world_transform;
        vector3 euler = w.rotation.to_euler().to_degrees();

        std::cout << "[transform] Pos: (" << w.position.x << ", " << w.position.y << ", " << w.position.z << ")\n";
        std::cout << "           Rot (deg): (" << euler.x << ", " << euler.y << ", " << euler.z << ")\n";
        std::cout << "           Scale: (" << w.scale.x << ", " << w.scale.y << ", " << w.scale.z << ")\n";
    }
};