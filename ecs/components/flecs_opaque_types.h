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
#include "core/math/basis.h"
#include "core/math/rect2.h"
#include "core/math/quaternion.h"
#include "core/object/object_id.h"
#include "core/string/string_name.h"
#include "core/templates/rid.h"
#include "core/variant/variant.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"
#include "core/io/json.h"
#include "servers/rendering_server.h"

namespace FlecsOpaqueTypes {

// ============================================================================
// SERIALIZATION HELPERS
// ============================================================================

// Helper to serialize Vector2
inline int serialize_vector2(const flecs::serializer* s, const Vector2* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	return 0;
}

// Helper to serialize Vector3
inline int serialize_vector3(const flecs::serializer* s, const Vector3* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	s->member("z");
	s->value(data->z);
	return 0;
}

// Helper to serialize Vector4
inline int serialize_vector4(const flecs::serializer* s, const Vector4* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	s->member("z");
	s->value(data->z);
	s->member("w");
	s->value(data->w);
	return 0;
}

// Helper to serialize Color
inline int serialize_color(const flecs::serializer* s, const Color* data) {
	s->member("r");
	s->value(data->r);
	s->member("g");
	s->value(data->g);
	s->member("b");
	s->value(data->b);
	s->member("a");
	s->value(data->a);
	return 0;
}

// Helper to serialize Quaternion
inline int serialize_quaternion(const flecs::serializer* s, const Quaternion* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	s->member("z");
	s->value(data->z);
	s->member("w");
	s->value(data->w);
	return 0;
}

// Helper to serialize Plane
inline int serialize_plane(const flecs::serializer* s, const Plane* data) {
	s->member("normal_x");
	s->value(data->normal.x);
	s->member("normal_y");
	s->value(data->normal.y);
	s->member("normal_z");
	s->value(data->normal.z);
	s->member("d");
	s->value(data->d);
	return 0;
}

// Helper to serialize AABB
inline int serialize_aabb(const flecs::serializer* s, const AABB* data) {
	s->member("position_x");
	s->value(data->position.x);
	s->member("position_y");
	s->value(data->position.y);
	s->member("position_z");
	s->value(data->position.z);
	s->member("size_x");
	s->value(data->size.x);
	s->member("size_y");
	s->value(data->size.y);
	s->member("size_z");
	s->value(data->size.z);
	return 0;
}

// Helper to serialize Rect2
inline int serialize_rect2(const flecs::serializer* s, const Rect2* data) {
	s->member("position_x");
	s->value(data->position.x);
	s->member("position_y");
	s->value(data->position.y);
	s->member("size_x");
	s->value(data->size.x);
	s->member("size_y");
	s->value(data->size.y);
	return 0;
}

// Helper to serialize Transform2D
inline int serialize_transform2d(const flecs::serializer* s, const Transform2D* data) {
	s->member("x_x");
	s->value(data->columns[0].x);
	s->member("x_y");
	s->value(data->columns[0].y);
	s->member("y_x");
	s->value(data->columns[1].x);
	s->member("y_y");
	s->value(data->columns[1].y);
	s->member("origin_x");
	s->value(data->columns[2].x);
	s->member("origin_y");
	s->value(data->columns[2].y);
	return 0;
}

// Helper to serialize Basis
inline int serialize_basis(const flecs::serializer* s, const Basis* data) {
	s->member("x_x");
	s->value(data->rows[0].x);
	s->member("x_y");
	s->value(data->rows[0].y);
	s->member("x_z");
	s->value(data->rows[0].z);
	s->member("y_x");
	s->value(data->rows[1].x);
	s->member("y_y");
	s->value(data->rows[1].y);
	s->member("y_z");
	s->value(data->rows[1].z);
	s->member("z_x");
	s->value(data->rows[2].x);
	s->member("z_y");
	s->value(data->rows[2].y);
	s->member("z_z");
	s->value(data->rows[2].z);
	return 0;
}

// Helper to serialize Transform3D
inline int serialize_transform3d(const flecs::serializer* s, const Transform3D* data) {
	s->member("basis");
	serialize_basis(s, &data->basis);
	s->member("origin_x");
	s->value(data->origin.x);
	s->member("origin_y");
	s->value(data->origin.y);
	s->member("origin_z");
	s->value(data->origin.z);
	return 0;
}

// Helper to serialize Projection
inline int serialize_projection(const flecs::serializer* s, const Projection* data) {
	for (int i = 0; i < 4; i++) {
		s->member("x");
		s->value(data->columns[i].x);
		s->member("y");
		s->value(data->columns[i].y);
		s->member("z");
		s->value(data->columns[i].z);
		s->member("w");
		s->value(data->columns[i].w);
	}
	return 0;
}

// Helper to serialize String
inline int serialize_string(const flecs::serializer* s, const String* data) {
	CharString utf8 = data->utf8();
	const char* str = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &str);
}

// Helper to serialize StringName
inline int serialize_string_name(const flecs::serializer* s, const StringName* data) {
	String str = *data;
	CharString utf8 = str.utf8();
	const char* cstr = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &cstr);
}

// Helper to serialize RID (read-only - just serialize the ID)
inline int serialize_rid(const flecs::serializer* s, const RID* data) {
	uint64_t id = data->get_id();
	return s->value(ecs_id(ecs_u64_t), &id);
}

// Helper to serialize ObjectID
inline int serialize_object_id(const flecs::serializer* s, const ObjectID* data) {
	uint64_t id = (uint64_t)*data;
	return s->value(ecs_id(ecs_u64_t), &id);
}

// Helper to serialize Variant (unwrap the variant)
inline int serialize_variant(const flecs::serializer* s, const Variant* data) {
	// Serialize variant as its string representation for simplicity
	String str = data->stringify();
	CharString utf8 = str.utf8();
	const char* cstr = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &cstr);
}

// Helper to serialize Dictionary
inline int serialize_dictionary(const flecs::serializer* s, const Dictionary* data) {
	// Serialize as JSON string representation
	String str = JSON::stringify(*data);
	CharString utf8 = str.utf8();
	const char* cstr = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &cstr);
}

// Helper to serialize Array
inline int serialize_array(const flecs::serializer* s, const Array* data) {
	// Serialize as JSON string representation
	String str = JSON::stringify(*data);
	CharString utf8 = str.utf8();
	const char* cstr = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &cstr);
}

// Helper to serialize Vector<RID>
inline int serialize_vector_rid(const flecs::serializer* s, const Vector<RID>* data) {
	s->member("size");
	s->value((int32_t)data->size());
	for (int i = 0; i < data->size(); i++) {
		s->member("element");
		s->value((*data)[i].get_id());
	}
	return 0;
}

// Helper to serialize Vector<Plane>
inline int serialize_vector_plane(const flecs::serializer* s, const Vector<Plane>* data) {
	s->member("size");
	s->value((int32_t)data->size());
	for (int i = 0; i < data->size(); i++) {
		s->member("element");
		serialize_plane(s, &(*data)[i]);
	}
	return 0;
}

// Helper to serialize Vector2i
inline int serialize_vector2i(const flecs::serializer* s, const Vector2i* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	return 0;
}

// Helper to serialize Vector3i
inline int serialize_vector3i(const flecs::serializer* s, const Vector3i* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	s->member("z");
	s->value(data->z);
	return 0;
}

// Helper to serialize Vector4i
inline int serialize_vector4i(const flecs::serializer* s, const Vector4i* data) {
	s->member("x");
	s->value(data->x);
	s->member("y");
	s->value(data->y);
	s->member("z");
	s->value(data->z);
	s->member("w");
	s->value(data->w);
	return 0;
}

// Helper to serialize Rect2i
inline int serialize_rect2i(const flecs::serializer* s, const Rect2i* data) {
	s->member("position_x");
	s->value(data->position.x);
	s->member("position_y");
	s->value(data->position.y);
	s->member("size_x");
	s->value(data->size.x);
	s->member("size_y");
	s->value(data->size.y);
	return 0;
}

// ============================================================================
// OPAQUE TYPE REGISTRATION
// ============================================================================

// Register all common Godot types as opaque to Flecs with proper serialization
inline void register_opaque_types(flecs::world &world) {
	// Vector2 - struct with x, y
	world.component<Vector2>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector2>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector2);
		});
	
	// Vector3 - struct with x, y, z
	world.component<Vector3>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector3>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector3);
		});
	
	// Vector4 - struct with x, y, z, w
	world.component<Vector4>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector4>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector4);
		});
	
	// Color - struct with r, g, b, a
	world.component<Color>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Color>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_color);
		});
	
	// Quaternion - struct with x, y, z, w
	world.component<Quaternion>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Quaternion>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_quaternion);
		});
	
	// Plane - struct with normal (Vector3) and d (float)
	world.component<Plane>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Plane>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_plane);
		});
	
	// AABB - struct with position and size (both Vector3)
	world.component<AABB>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<AABB>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_aabb);
		});
	
	// Rect2 - struct with position and size (both Vector2)
	world.component<Rect2>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Rect2>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_rect2);
		});
	
	// Transform2D - 2x3 matrix
	world.component<Transform2D>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Transform2D>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_transform2d);
		});
	
	// Basis - 3x3 matrix
	world.component<Basis>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Basis>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_basis);
		});
	
	// Transform3D - Basis + origin
	world.component<Transform3D>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Transform3D>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_transform3d);
		});
	
	// Projection - 4x4 matrix
	world.component<Projection>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Projection>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_projection);
		});
	
	// String - Godot string type
	world.component<String>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<String>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_string);
		});
	
	// StringName - Godot interned string
	world.component<StringName>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<StringName>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_string_name);
		});
	
	// RID - Resource ID (read-only, serialize as uint64)
	world.component<RID>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<RID>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_rid);
		});
	
	// ObjectID - Godot object identifier
	world.component<ObjectID>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<ObjectID>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_object_id);
		});
	
	// Variant - Godot's universal type (unwrap on serialize)
	world.component<Variant>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Variant>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_variant);
		});
	
	// Dictionary - Godot's hash map
	world.component<Dictionary>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Dictionary>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_dictionary);
		});
	
	// Array - Godot's dynamic array
	world.component<Array>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Array>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_array);
		});
	
	// Vector<RID> - Godot's templated vector with RID
	world.component<Vector<RID>>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector<RID>>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector_rid);
		});
	
	// Vector<Plane> - Godot's templated vector with Plane
	world.component<Vector<Plane>>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector<Plane>>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector_plane);
		});
	
	// Vector2i, Vector3i, Vector4i, Rect2i
	world.component<Vector2i>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector2i>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector2i);
		});
	
	world.component<Vector3i>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector3i>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector3i);
		});
	
	world.component<Vector4i>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector4i>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_vector4i);
		});
	
	world.component<Rect2i>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Rect2i>()
				.as_type(ecs_id(EcsOpaque))
				.serialize(serialize_rect2i);
		});
	
	// Primitive types (register for completeness)
	world.component<uint32_t>();
	world.component<int64_t>();
	world.component<double>();
	world.component<float>();
	world.component<bool>();
	world.component<int>();
	world.component<int32_t>();
	
	// RenderingServer enums
	world.component<RS::MultimeshTransformFormat>();
}

} // namespace FlecsOpaqueTypes