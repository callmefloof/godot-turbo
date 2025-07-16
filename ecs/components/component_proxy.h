//
// Created by Floof on 13-7-2025.
//
#pragma once
#include "../../../../core/object/class_db.h"
#include "../../../../core/object/object.h"
#include "../../../../core/templates/vector.h"
#include "../../../../core/variant/variant.h"
#include "../../../../core/variant/typed_array.h"
#include "variant_type_map.h"
#include <type_traits>

//name hashing
//compliant with the definition "uint32_t String::hash(const char *p_cstr)" in core/string/ustring.h
constexpr uint32_t godot_djb2_hash(const char *str) {
	uint32_t hash = 5381;
	while (*str) {
		hash = ((hash << 5) + hash) + static_cast<uint8_t>(*str++);
	}
	return hash;
}
#define FIELD_HASH(name) godot_djb2_hash(name)

//get the default value for a type
template<typename T>
constexpr T default_value() {
	if constexpr (std::is_base_of_v<T,Object*>) {
		return nullptr;
	} else if constexpr (std::is_base_of_v<T,Object>) {
		return T();
	} else if constexpr (std::is_floating_point_v<T>) {
		return 0.0;
	} else if constexpr (std::is_integral_v<T>) {
		return 0;
	} else {
		return T{};
	}
}
//Returns the default value of a given type
#define RETURN_DEFAULT(T) return default_value<T>();

// Trait to detect Vector<T>
template<typename T>
struct is_godot_vector : std::false_type {};

template<typename T>
struct is_godot_vector<Vector<T>> : std::true_type {
	using inner_type = T;
};

// Helper function to set field value
template<typename T>
void set_field_value(T* data_field, const Variant &value, const StringName &field_name) {
	if constexpr (is_godot_vector<T>::value) {
		using Inner = typename is_godot_vector<T>::inner_type;
		if (value.get_type() != Variant::ARRAY) {
			ERR_PRINT("Expected TypedArray for field " + field_name);
			return;
		}
		TypedArray<Inner> arr = value;
		data_field->clear();
		for (int i = 0; i < arr.size(); i++) {
			data_field->push_back(arr[i]);
		}
	} else {
		if (VariantTypeMap<T>::value == Variant::NIL) {
			ERR_PRINT("Invalid type for field " + field_name);
			return;
		}
		*data_field = value; // This uses Variant's implicit conversion if available
	}
}

#define DEFINE_PROPERTY(type, name, component_type)\
type get_##name() {\
	auto typed = get_typed_data<component_type>();\
	if (typed) {\
		return typed->name;\
	}\
	RETURN_DEFAULT(type)\
}\
void set_##name(type value) {\
	auto typed = get_typed_data<component_type>();\
	if (typed) {\
	typed->name = value;\
	}\
}

#define DEFINE_PROPERTY_ARRAY(type, name, component_type)\
TypedArray<type> get_##name() {\
TypedArray<type> arr;\
auto typed = get_typed_data<component_type>();\
if (typed) { \
for (int i = 0; i < typed->name.size(); i++) { \
arr.push_back(typed->name[i]);\
} \
} \
return arr; \
}\
void set_##name(const TypedArray<type> &value) {\
auto typed = get_typed_data<component_type>();\
if (!typed) { return; }\
typed->name.clear();\
for (int i = 0; i < value.size(); i++) {\
typed->name.push_back(value.get(i));\
}\
}


#define BIND_PROPERTY(type, name, class_name)\
ClassDB::bind_method(D_METHOD("get_"#name), &class_name::get_##name);\
ClassDB::bind_method(D_METHOD("set_"#name, "value"), &class_name::set_##name);\
::ClassDB::add_property(class_name::get_class_static(), PropertyInfo(VariantTypeMap<type>::value, #name), _scs_create("set_"#name), _scs_create("get_"#name));\


#define BIND_VECTOR_PROPERTY(type, name, class_name)\
ClassDB::bind_method(D_METHOD("get_"#name), &class_name::get_##name);\
ClassDB::bind_method(D_METHOD("set_"#name), &class_name::set_##name);\
::ClassDB::add_property(class_name::get_class_static(), PropertyInfo(Variant::ARRAY, #name), _scs_create("set_"#name), _scs_create("get_"#name));\

// a macro collection to simplify exposing types to another scripting language
#define DEFINE_COMPONENT_PROXY(CLASS, TYPE, FIELD_PROPERTIES,FIELD_BINDINGS)\
class CLASS : public FlecsComponent<TYPE> {\
public:\
\
	FIELD_PROPERTIES\
\
	static Ref<CLASS> create_component(const Ref<FlecsEntity> &owner){ \
		auto class_ref = Ref<CLASS>(memnew(CLASS));\
		const flecs::entity& entity = owner->get_entity();\
		entity.set<TYPE>({});\
		class_ref->set_data(&entity.get_mut<TYPE>());\
		return class_ref;\
	}\
	\
	static uint64_t get_type_hash_static() { \
		static int type_marker; \
		return reinterpret_cast<uint64_t>(&type_marker); \
	} \
	Ref<FlecsComponentBase> clone() const { \
		Ref<CLASS> new_ref;\
		new_ref.instantiate();\
		if (data) {\
			/* Step 1: cast void* to actual type*/\
		const auto typed = get_typed_data<TYPE>();\
\
		/* Step 2: allocate memory and copy*/\
		TYPE *copied = memnew(TYPE);\
		*copied = *typed; /* copy the struct contents*/\
\
		new_ref->set_data(copied);\
		}\
		return new_ref;\
	}	\
	void set_data(TYPE* d) { \
		this->data = d; \
		this->component_type_hash = get_type_hash_static(); \
	} \
	\
	StringName get_type_name() const override {\
		return #TYPE;\
	}\
	\
	static void _bind_methods(){ \
		FIELD_BINDINGS \
		ClassDB::bind_static_method(CLASS::get_class_static(),"create_component",&CLASS::create_component,"owner");\
		\
		ClassDB::bind_method(D_METHOD("get_type_name"), &CLASS::get_type_name);\
	} \
};