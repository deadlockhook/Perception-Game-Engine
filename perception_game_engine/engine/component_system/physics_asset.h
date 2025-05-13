#pragma once
#include "../../math/geom/geom.h"

struct physics_asset_t
{
	physics_asset_t() = default;
	~physics_asset_t() = default;
	geometry_collection_t geometry;
	//what more should here be for simple physics requirements maybe like is_collidable or should is_collidable be a completely different asset
};