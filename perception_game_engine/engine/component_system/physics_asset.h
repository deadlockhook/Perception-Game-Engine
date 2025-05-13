#pragma once
#include "default_component_hashes.h"
#include "../../math/geom/geom.h"

struct transform_instance_t;
struct physics_asset_t
{
	transform_instance_t* transform_instance = nullptr;
	geometry_collection_t geometry;

	triple_buffer_t<transform_t> previous_local_transform;
	triple_buffer_t<transform_t> previous_world_transform;

	//what more should here be for simple physics requirements maybe like is_collidable or should is_collidable be a completely different asset
};