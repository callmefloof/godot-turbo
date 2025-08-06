//
// Created by Floof on 14-7-2025.
//

#ifndef VARIANT_TYPE_MAP_H
#define VARIANT_TYPE_MAP_H
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "core/variant/callable.h"
#include "core/math/aabb.h"
#include "core/math/rect2.h"
#include "core/math/rect2i.h"
#include "core/math/color.h"
#include "core/math/transform_3d.h"
#include "core/math/transform_2d.h"
#include "core/io/resource.h"
#include "core/templates/rid.h"
#include "core/object/object.h"
#include "core/math/vector2i.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/vector3i.h"
#include "core/math/vector4.h"
#include "core/math/vector4i.h"
#include "core/math/basis.h"
#include "core/math/quaternion.h"
#include "core/string/string_name.h"
#include "core/string/ustring.h"

template<typename T>
struct VariantTypeMap{
	static constexpr Variant::Type value = Variant::NIL;
};


template<>
struct VariantTypeMap<Ref<RefCounted>>{
	static constexpr Variant::Type value = Variant::OBJECT;
};

template<>
struct VariantTypeMap<Ref<Resource>>{
	static constexpr Variant::Type value = Variant::OBJECT;
};

template<>
struct VariantTypeMap<Ref<RefCounted>*> {
	static constexpr Variant::Type value = Variant::OBJECT;
};

template<>
struct VariantTypeMap<Ref<Resource>*> {
	static constexpr Variant::Type value = Variant::OBJECT;
};

template<>
struct VariantTypeMap<RID> {
	static constexpr Variant::Type value = Variant::RID;
};

template<>
struct VariantTypeMap<bool> {
	static constexpr Variant::Type value = Variant::BOOL;
};

template<>
struct VariantTypeMap<int> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<uint64_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<uint32_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<uint16_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<uint8_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<int64_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<int16_t> {
	static constexpr Variant::Type value = Variant::INT;
};

template<>
struct VariantTypeMap<int8_t> {
	static constexpr Variant::Type value = Variant::INT;
};


template<>
struct VariantTypeMap<AABB> {
	static constexpr Variant::Type value = Variant::AABB;
};

template<>
struct VariantTypeMap<Basis> {
	static constexpr Variant::Type value = Variant::BASIS;
};

template<>
struct VariantTypeMap<Object> {
	static constexpr Variant::Type value = Variant::OBJECT;
};

template<>
struct VariantTypeMap<Object*> {
	static constexpr Variant::Type value = Variant::OBJECT;
};



template<>
struct VariantTypeMap<Callable> {
	static constexpr Variant::Type value = Variant::CALLABLE;
};

template<>
struct VariantTypeMap<Color> {
	static constexpr Variant::Type value = Variant::COLOR;
};

template<>
struct VariantTypeMap<Dictionary> {
	static constexpr Variant::Type value = Variant::DICTIONARY;
};

template<>
struct VariantTypeMap<PackedByteArray> {
	static constexpr Variant::Type value = Variant::PACKED_BYTE_ARRAY;
};

template<>
struct VariantTypeMap<PackedInt32Array> {
	static constexpr Variant::Type value = Variant::PACKED_INT32_ARRAY;
};

template<>
struct VariantTypeMap<PackedFloat64Array> {
	static constexpr Variant::Type value = Variant::PACKED_INT64_ARRAY;
};

template<>
struct VariantTypeMap<float> {
	static constexpr Variant::Type value = Variant::FLOAT;
};

template<>
struct VariantTypeMap<PackedColorArray> {
	static constexpr Variant::Type value = Variant::PACKED_COLOR_ARRAY;
};

template<>
struct VariantTypeMap<PackedFloat32Array> {
	static constexpr Variant::Type value = Variant::PACKED_FLOAT32_ARRAY;
};

template<>
struct VariantTypeMap<PackedVector2Array> {
	static constexpr Variant::Type value = Variant::PACKED_VECTOR2_ARRAY;
};

template<>
struct VariantTypeMap<PackedVector3Array> {
	static constexpr Variant::Type value = Variant::PACKED_VECTOR3_ARRAY;
};

template<>
struct VariantTypeMap<PackedVector4Array> {
	static constexpr Variant::Type value = Variant::PACKED_VECTOR4_ARRAY;
};

template<>
struct VariantTypeMap<Rect2> {
	static constexpr Variant::Type value = Variant::RECT2;
};


template<>
struct VariantTypeMap<NodePath> {
	static constexpr Variant::Type value = Variant::NODE_PATH;
};

template<>
struct VariantTypeMap<Signal> {
	static constexpr Variant::Type value = Variant::SIGNAL;
};

template<>
struct VariantTypeMap<Plane> {
	static constexpr Variant::Type value = Variant::PLANE;
};

template<>
struct VariantTypeMap<Projection> {
	static constexpr Variant::Type value = Variant::PROJECTION;
};

template<>
struct VariantTypeMap<Quaternion> {
	static constexpr Variant::Type value = Variant::QUATERNION;
};

template<>
struct VariantTypeMap<Transform2D> {
	static constexpr Variant::Type value = Variant::TRANSFORM2D;
};

template<>
struct VariantTypeMap<Transform3D> {
	static constexpr Variant::Type value = Variant::TRANSFORM3D;
};

template<>
struct VariantTypeMap<PackedStringArray> {
	static constexpr Variant::Type value = Variant::PACKED_STRING_ARRAY;
};

template<>
struct VariantTypeMap<Rect2i> {
	static constexpr Variant::Type value = Variant::RECT2I;
};

template<>
struct VariantTypeMap<Vector2> {
	static constexpr Variant::Type value = Variant::VECTOR2I;
};

template<>
struct VariantTypeMap<Vector2i> {
	static constexpr Variant::Type value = Variant::VECTOR2I;
};

template<>
struct VariantTypeMap<Vector3> {
	static constexpr Variant::Type value = Variant::VECTOR3;
};


template<>
struct VariantTypeMap<Vector3i> {
	static constexpr Variant::Type value = Variant::VECTOR3I;
};

template<>
struct VariantTypeMap<Vector4> {
	static constexpr Variant::Type value = Variant::VECTOR4I;
};


template<>
struct VariantTypeMap<Vector4i> {
	static constexpr Variant::Type value = Variant::VECTOR4I;
};

template<>
struct VariantTypeMap<StringName> {
	static constexpr Variant::Type value = Variant::STRING_NAME;
};

template<>
struct VariantTypeMap<String> {
	static constexpr Variant::Type value = Variant::STRING;
};



#endif //VARIANT_TYPE_MAP_H
