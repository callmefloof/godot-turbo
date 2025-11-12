#pragma once

#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/math/color.h"
#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "core/math/projection.h"
#include "core/object/object_id.h"
#include "core/string/string_name.h"
#include "core/templates/rid.h"

namespace FlecsOpaqueTypes {

// Register all common Godot types as opaque to Flecs
// Note: For most Godot types, simply registering them as components is sufficient
// Flecs will treat them as opaque by default if they don't have reflection metadata
inline void register_opaque_types(flecs::world &world) {
	// Math types - register as regular components (Flecs treats them as opaque)
	world.component<Transform2D>();
	world.component<Transform3D>();
	world.component<Vector2>();
	world.component<Vector3>();
	world.component<Vector4>();
	world.component<Color>();
	world.component<AABB>();
	world.component<Plane>();
	world.component<Projection>();

	// Godot ID types
	world.component<ObjectID>();
	world.component<RID>();

	// String types
	world.component<StringName>();
	world.component<String>();

	// Primitive types
	world.component<uint32_t>();
	world.component<int64_t>();
	world.component<float>();
	world.component<bool>();
}

} // namespace FlecsOpaqueTypes