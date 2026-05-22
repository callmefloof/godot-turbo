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
#include "core/string/node_path.h"
#include "core/string/string_name.h"
#include "core/templates/rid.h"
#include "core/variant/callable.h"
#include "core/variant/variant.h"
#include "core/variant/dictionary.h"
#include "core/variant/array.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#ifdef FAR
#undef FAR
#endif

#include "core/variant/variant_parser.h"
#include "core/io/json.h"
#include "servers/rendering/rendering_server.h"

namespace FlecsOpaqueTypes {

template <typename T>
inline int serialize_variant_text(const flecs::serializer* s, const T* data) {
	Variant value = *data;
	String text;
	Error err = VariantWriter::write_to_string(value, text);
	if (err != OK) {
		return -1;
	}

	CharString utf8 = text.utf8();
	const char* str = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &str);
}

inline bool parse_variant_text(const char *p_text, Variant &r_value) {
	VariantParser::StreamString stream;
	stream.s = String::utf8(p_text ? p_text : "");
	String error;
	int error_line = 0;
	return VariantParser::parse(&stream, r_value, error, error_line) == OK;
}

template <typename T>
inline void assign_variant_text(T *dst, const char *value) {
	Variant parsed;
	if (parse_variant_text(value, parsed)) {
		*dst = parsed;
	}
}

inline void assign_variant_text_value(Variant *dst, const char *value) {
	Variant parsed;
	if (parse_variant_text(value, parsed)) {
		*dst = parsed;
	}
}

#define GODOT_VARIANT_OPAQUE(TYPE) \
	world.component<TYPE>() \
		.opaque([](flecs::world& w) { \
			return flecs::opaque<TYPE>() \
				.as_type(ecs_id(ecs_string_t)) \
				.serialize(serialize_variant_text<TYPE>) \
				.assign_string(assign_variant_text<TYPE>); \
		})

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

inline void assign_object_id_uint(ObjectID *dst, uint64_t value) {
	*dst = ObjectID(value);
}

inline int serialize_vector_rid_text(const flecs::serializer* s, const Vector<RID>* data) {
	Array array;
	for (int i = 0; i < data->size(); i++) {
		array.push_back((*data)[i]);
	}
	String text;
	if (VariantWriter::write_to_string(array, text) != OK) {
		return -1;
	}
	CharString utf8 = text.utf8();
	const char* str = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &str);
}

inline void assign_vector_rid_text(Vector<RID> *dst, const char *value) {
	Variant parsed;
	if (!parse_variant_text(value, parsed) || parsed.get_type() != Variant::ARRAY) {
		return;
	}
	Array array = parsed;
	dst->resize(array.size());
	for (int i = 0; i < array.size(); i++) {
		dst->write[i] = array[i];
	}
}

inline int serialize_vector_plane_text(const flecs::serializer* s, const Vector<Plane>* data) {
	Array array;
	for (int i = 0; i < data->size(); i++) {
		array.push_back((*data)[i]);
	}
	String text;
	if (VariantWriter::write_to_string(array, text) != OK) {
		return -1;
	}
	CharString utf8 = text.utf8();
	const char* str = utf8.get_data();
	return s->value(ecs_id(ecs_string_t), &str);
}

inline void assign_vector_plane_text(Vector<Plane> *dst, const char *value) {
	Variant parsed;
	if (!parse_variant_text(value, parsed) || parsed.get_type() != Variant::ARRAY) {
		return;
	}
	Array array = parsed;
	dst->resize(array.size());
	for (int i = 0; i < array.size(); i++) {
		dst->write[i] = array[i];
	}
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
	// Store Godot values in Flecs JSON as Godot Variant text. This is less
	// inspectable than per-member math structs, but it round-trips the full
	// Variant family including packed arrays and nested containers.
	world.component<Variant>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Variant>()
				.as_type(ecs_id(ecs_string_t))
				.serialize(serialize_variant_text<Variant>)
				.assign_string(assign_variant_text_value);
		});
	GODOT_VARIANT_OPAQUE(String);
	GODOT_VARIANT_OPAQUE(StringName);
	GODOT_VARIANT_OPAQUE(NodePath);
	GODOT_VARIANT_OPAQUE(RID);
	GODOT_VARIANT_OPAQUE(Callable);
	GODOT_VARIANT_OPAQUE(Signal);
	GODOT_VARIANT_OPAQUE(Dictionary);
	GODOT_VARIANT_OPAQUE(Array);
	GODOT_VARIANT_OPAQUE(Vector2);
	GODOT_VARIANT_OPAQUE(Vector2i);
	GODOT_VARIANT_OPAQUE(Rect2);
	GODOT_VARIANT_OPAQUE(Rect2i);
	GODOT_VARIANT_OPAQUE(Vector3);
	GODOT_VARIANT_OPAQUE(Vector3i);
	GODOT_VARIANT_OPAQUE(Transform2D);
	GODOT_VARIANT_OPAQUE(Vector4);
	GODOT_VARIANT_OPAQUE(Vector4i);
	GODOT_VARIANT_OPAQUE(Plane);
	GODOT_VARIANT_OPAQUE(Quaternion);
	GODOT_VARIANT_OPAQUE(AABB);
	GODOT_VARIANT_OPAQUE(Basis);
	GODOT_VARIANT_OPAQUE(Transform3D);
	GODOT_VARIANT_OPAQUE(Projection);
	GODOT_VARIANT_OPAQUE(Color);
	GODOT_VARIANT_OPAQUE(PackedByteArray);
	GODOT_VARIANT_OPAQUE(PackedInt32Array);
	GODOT_VARIANT_OPAQUE(PackedInt64Array);
	GODOT_VARIANT_OPAQUE(PackedFloat32Array);
	GODOT_VARIANT_OPAQUE(PackedFloat64Array);
	GODOT_VARIANT_OPAQUE(PackedStringArray);
	GODOT_VARIANT_OPAQUE(PackedVector2Array);
	GODOT_VARIANT_OPAQUE(PackedVector3Array);
	GODOT_VARIANT_OPAQUE(PackedColorArray);
	GODOT_VARIANT_OPAQUE(PackedVector4Array);

	world.component<ObjectID>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<ObjectID>()
				.as_type(ecs_id(ecs_u64_t))
				.serialize(serialize_object_id)
				.assign_uint(assign_object_id_uint);
		});

	world.component<Vector<RID>>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector<RID>>()
				.as_type(ecs_id(ecs_string_t))
				.serialize(serialize_vector_rid_text)
				.assign_string(assign_vector_rid_text);
		});

	world.component<Vector<Plane>>()
		.opaque([](flecs::world& w) {
			return flecs::opaque<Vector<Plane>>()
				.as_type(ecs_id(ecs_string_t))
				.serialize(serialize_vector_plane_text)
				.assign_string(assign_vector_plane_text);
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
	world.component<RSE::MultimeshTransformFormat>();
}

} // namespace FlecsOpaqueTypes

#undef GODOT_VARIANT_OPAQUE
