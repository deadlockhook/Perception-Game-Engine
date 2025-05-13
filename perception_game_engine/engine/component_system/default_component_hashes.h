#pragma once
#include "../../serialize/fnva_hash.h"

constexpr auto physics_asset_t_register = "physics_asset";
constexpr auto physics_asset_t_register_hash = fnv1a32(physics_asset_t_register);

constexpr auto transform_instance_t_register = "transform_instance";
constexpr auto transform_instance_t_register_hash = fnv1a32(transform_instance_t_register);
