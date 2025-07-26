// component_proxy.h
// Refactored to use raw pointers instead of Ref<T>
#pragma once

#include "../../../../core/object/ref_counted.h"  // RefCounted
#include "../../../../core/object/class_db.h"    // ClassDB
#include "../../../../core/object/object.h"
#include "../../../../core/templates/vector.h"
#include "../../../../core/variant/variant.h"
#include "../../../../core/variant/typed_array.h"
#include "../../../../core/templates/rid.h"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "flecs_component.h"
#include "variant_type_map.h"
#include <type_traits>

// DJB2 string hash
constexpr uint32_t godot_djb2_hash(const char *str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = (hash << 5) + hash + static_cast<uint8_t>(*str++);
    }
    return hash;
}
#define FIELD_HASH(name) godot_djb2_hash(name)

// default_value: Returns a sensible default for any type
template<typename T>
constexpr T default_value() {
    if constexpr (std::is_pointer_v<T>) {
        return nullptr;
    } else if constexpr (std::is_same_v<T, String> || std::is_same_v<T, StringName>) {
        return {};
    } else if constexpr (std::is_floating_point_v<T>) {
        return static_cast<T>(0);
    } else if constexpr (std::is_integral_v<T>) {
        return static_cast<T>(0);
    } else {
        return T{};
    }
}

// Trait for Godot Vector<T>
template<typename> struct is_godot_vector : std::false_type {};
template<typename U>
struct is_godot_vector<Vector<U>> : std::true_type {
    using value_type = U;
};

// Sets a field from Variant
template<typename T>
void set_field_value(T &field, const Variant &v, const StringName &name) {
    if constexpr (is_godot_vector<T>::value) {
        if (v.get_type() != Variant::ARRAY) {
            ERR_PRINT("Expected TypedArray for field " + name);
            return;
        }
        TypedArray<typename is_godot_vector<T>::value_type> arr = v;
        field.clear();
        for (int i = 0; i < arr.size(); ++i) {
            field.push_back(arr[i]);
        }
    } else {
        if (VariantTypeMap<T>::value == Variant::NIL) {
            ERR_PRINT("Invalid type for field " + name);
            return;
        }
        field = v;
    }
}

// Property macros simplified
#define DEFINE_PROPERTY(Type, Name, Component) \
Type get_##Name() const { \
    if (auto *c = get_typed_data<Component>()) return c->Name; \
    return default_value<Type>(); \
} \
void set_##Name(Type val) { \
    if (auto *c = get_typed_data<Component>()) c->Name = val; \
}

#define DEFINE_ARRAY_PROPERTY(Type, Name, Component) \
TypedArray<Type> get_##Name() const { \
    TypedArray<Type> arr; \
    if (auto *c = get_typed_data<Component>()) { \
        for (auto &e : c->Name) arr.push_back(e); \
    } \
    return arr; \
} \
void set_##Name(const TypedArray<Type> &arr) { \
    if (auto *c = get_typed_data<Component>()) { \
        c->Name.clear(); \
        for (int i = 0; i < arr.size(); ++i) c->Name.push_back(arr[i]); \
    } \
}

#define BIND_PROPERTY(Type, Name, Class) \
ClassDB::bind_method(D_METHOD("get_" #Name), &Class::get_##Name); \
ClassDB::bind_method(D_METHOD("set_" #Name, "value"), &Class::set_##Name); \
ClassDB::add_property( \
    Class::get_class_static(), \
    PropertyInfo(VariantTypeMap<Type>::value, #Name), \
    StringName("set_" #Name), \
    StringName("get_" #Name) \
);

#define BIND_ARRAY_PROPERTY(Type, Name, Class) \
ClassDB::bind_method(D_METHOD("get_" #Name), &Class::get_##Name); \
ClassDB::bind_method(D_METHOD("set_" #Name, "value"), &Class::set_##Name); \
ClassDB::add_property( \
    Class::get_class_static(), \
    PropertyInfo(Variant::ARRAY, #Name), \
    StringName("set_" #Name), \
    StringName("get_" #Name) \
);

// Component proxy definition using raw pointers

#define DEFINE_COMPONENT_PROXY(ClassName, CompType, PROP_DEFS, PROP_BINDS) \
    GDCLASS(ClassName, FlecsComponent<CompType>); \
public: \
	\
	\
	ClassName(){\
		\
		\
	}\
	\
	~ClassName() = default;\
	\
	\
    PROP_DEFS \
    \
    /* Create and attach component to entity, returns raw pointer for serializability */ \
    static ClassName *create_component(FlecsEntity *owner) { \
        ClassName *inst = memnew(ClassName); \
        auto *ent = owner->get_entity(); \
        ent->set<CompType>({}); \
        inst->set_data(&ent->get_mut<CompType>()); \
        return inst; \
    } \
    static ClassName *create_component(const Ref<FlecsEntity> &owner) { \
        return create_component(owner.ptr()); \
    } \
    \
    static void _bind_methods() { \
        PROP_BINDS \
		ClassDB::bind_static_method(\
			ClassName::get_class_static(),\
			"create_component",\
			static_cast<ClassName *(*)(FlecsEntity *)>(&ClassName::create_component),\
			"owner"\
		);\
        ClassDB::bind_method(D_METHOD("get_type_name"), &ClassName::get_type_name); \
    } \
    \
    CompType *get_data() const { return static_cast<CompType*>(get_data_ptr()); } \
    void set_data(CompType *d) { FlecsComponent<CompType>::set_data(d); } \
    StringName get_type_name() const { return #CompType; } \
\
;