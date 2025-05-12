#pragma once
#include "../entity_system/entity_system.h"
#include "../../math/transform.h"

class component_transform_t {
public:
    component_transform_t() = default;

    transform_t local_transform;

    transform_t world_transform;

    bool dirty = true;

    component_transform_t* parent = nullptr;
    s_vector<component_transform_t*> children;

    s_pooled_string name;

    void set_local_transform(const transform_t& t) {
        local_transform = t;
        mark_dirty();
    }

    void mark_dirty() {
        if (!dirty) {
            dirty = true;
            for (auto* child : children)
                child->mark_dirty();
        }
    }

    const transform_t& get_world_transform() {
        if (dirty) {
            if (parent) {
                world_transform = parent->get_world_transform().combine(local_transform);
            }
            else {
                world_transform = local_transform;
            }
            dirty = false;
        }
        return world_transform;
    }


    void attach_to(component_transform_t* new_parent) {
        if (parent) {
            auto& siblings = parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }

        parent = new_parent;

        if (parent) {
            parent->children.push_back(this);
        }

        mark_dirty();
    }

    void detach() {
        if (parent) {
            auto& siblings = parent->children;
            siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
            parent = nullptr;
            mark_dirty();
        }
    }

    void translate_local(const vector3& delta) {
        local_transform.position += delta;
        mark_dirty();
    }

    void rotate_local(const quat& delta_rotation) {
        local_transform.rotation = delta_rotation * local_transform.rotation;
        mark_dirty();
    }

    void scale_local(const vector3& scale_factor) {
        local_transform.scale = local_transform.scale * scale_factor;
        mark_dirty();
    }

 
    void translate_world(const vector3& delta_world) {
        vector3 local_delta = parent ? parent->get_world_transform().inverse().transform_direction(delta_world) : delta_world;
        local_transform.position += local_delta;
        mark_dirty();
    }

    void set_world_position(const vector3& world_pos) {
        if (parent) {
            vector3 local_pos = parent->get_world_transform().inverse().transform_point(world_pos);
            local_transform.position = local_pos;
        }
        else {
            local_transform.position = world_pos;
        }
        mark_dirty();
    }

    // --- Direction accessors ---

    vector3 get_forward() const {
        return get_world_transform().rotation * vector3(0, 0, 1);
    }

    vector3 get_right() const {
        return get_world_transform().rotation * vector3(1, 0, 0);
    }

    vector3 get_up() const {
        return get_world_transform().rotation * vector3(0, 1, 0);
    }

    vector3 get_world_scale() const {
        return get_world_transform().scale;
    }


    void set_relative_to(const component_transform_t* target, const transform_t& relative_transform) {
        attach_to(const_cast<component_transform_t*>(target));
        local_transform = relative_transform;
        mark_dirty();
    }


    void print_world_transform() const {
        const transform_t& w = get_world_transform();
        std::cout << "[Transform] World Pos: (" << w.position.x << ", " << w.position.y << ", " << w.position.z << ")\n";
        std::cout << "[Transform] World Scale: (" << w.scale.x << ", " << w.scale.y << ", " << w.scale.z << ")\n";
        vector3 euler_deg = w.rotation.to_euler().to_degrees();
        std::cout << "[Transform] World Rot (Euler degrees): (" << euler_deg.x << ", " << euler_deg.y << ", " << euler_deg.z << ")\n";
    }
};