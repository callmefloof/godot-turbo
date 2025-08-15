// component_proxy.h
// Refactored to use raw pointers instead of Ref<T>
#pragma once
#include "core/object/ref_counted.h"  // RefCounted
#include "core/object/class_db.h"    // ClassDB
#include "core/object/object.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "core/variant/typed_array.h"
#include "core/templates/rid.h"
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

#define DEFINE_PROPERTY(Type, Name, Component) \
Type get_##Name() const { \
	if (!owner.is_alive()) { \
	if (!Engine::get_singleton()->is_editor_hint()) { \
	ERR_PRINT("Owner entity is not alive"); \
	} \
	return Type(); \
	} \
	if (!owner.has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("Entity does not have component " #Component); \
		} \
		return Type(); \
	} \
		return owner.get<Component>().Name; \
	} \
	void set_##Name(Type val) { \
	if (!owner.is_alive()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("Owner entity is not alive"); \
		} \
		return; \
	} \
	if (!owner.has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("Entity does not have component " #Component); \
		} \
		return; \
	} \
	owner.get_mut<Component>().Name = val; \
	owner.modified<Component>();\
}


#define DEFINE_SINGLETON_PROPERTY(Type, Name, Component) \
Type get_##Name() const { \
	if (!world) { \
	if (!Engine::get_singleton()->is_editor_hint()) { \
	ERR_PRINT("World is not alive"); \
	} \
	return Type(); \
	} \
	if (!world->has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("World does not have component " #Component); \
		} \
		return Type(); \
	} \
		return world->get<Component>().Name; \
	} \
	void set_##Name(Type val) { \
	if (!world) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("World is not alive"); \
		} \
		return; \
	} \
	if (!world->has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("World does not have component " #Component); \
		} \
		return; \
	} \
	owner.get_mut<Component>().Name = val; \
	owner.modified<Component>();\
}

#define DEFINE_SINGLETON_ARRAY_PROPERTY(Type, Name, Component) \
TypedArray<Type> get_##Name() const { \
	TypedArray<Type> arr; \
	if (!world->c_ptr()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("World is not alive"); \
		} \
		return arr; \
	} \
	if (!world->has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
		ERR_PRINT("World does not have component " #Component); \
		} \
		return arr; \
	} \
	auto c = try_get_typed_data<Component>(); \
	if(!c) {\
		ERR_PRINT("World data is null, returning empty array.");\
		return arr;\
	}\
	for (auto &e : c->Name) { arr.push_back(e); }\
	return arr; \
} \
void set_##Name(const TypedArray<Type> &arr) { \
	if (!world->c_ptr()) { \
	if (!Engine::get_singleton()->is_editor_hint()) { \
	ERR_PRINT("World is not alive"); \
	} \
	return; \
	} \
	if (!world->has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("World does not have component " #Component); \
		} \
		return; \
	} \
	auto c = try_get_typed_data<Component>(); \
	if(!c){\
		ERR_PRINT("Component data is null, array data was not set.");\
		return;\
	}\
	c->Name.clear(); \
	for (int i = 0; i < arr.size(); ++i) { c->Name.push_back(arr[i]); } \
	world->modified<Component>();\
}\

#define DEFINE_ARRAY_PROPERTY(Type, Name, Component) \
TypedArray<Type> get_##Name() const { \
	TypedArray<Type> arr; \
	if (!owner.is_alive()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("Owner entity is not alive"); \
		} \
		return arr; \
	} \
	if (!owner.has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
		ERR_PRINT("Entity does not have component " #Component); \
		} \
		return arr; \
	} \
	auto c = try_get_typed_data<Component>(); \
	if(!c) {\
		ERR_PRINT("Component data is null, returning empty array.");\
		return arr;\
	}\
	for (auto &e : c->Name) { arr.push_back(e); }\
	return arr; \
} \
void set_##Name(const TypedArray<Type> &arr) { \
	if (!owner.is_alive()) { \
	if (!Engine::get_singleton()->is_editor_hint()) { \
	ERR_PRINT("Owner entity is not alive"); \
	} \
	return; \
	} \
	if (!owner.has<Component>()) { \
		if (!Engine::get_singleton()->is_editor_hint()) { \
			ERR_PRINT("Entity does not have component " #Component); \
		} \
		return; \
	} \
	auto c = try_get_typed_data<Component>(); \
	if(!c){\
		ERR_PRINT("Component data is null, array data was not set.");\
		return;\
	}\
	c->Name.clear(); \
	for (int i = 0; i < arr.size(); ++i) { c->Name.push_back(arr[i]); } \
	owner.modified<Component>();\
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

#define COMPONENT_FACTORY(CompType)\
static Ref<CompType##Ref> create_component(const Ref<FlecsEntity> &p_owner) {\
	if (!p_owner.is_valid()) {\
		ERR_FAIL_COND_V_MSG(true, Ref<CompType##Ref>(), "owner is not valid");\
	}\
	\
	Ref<CompType##Ref> inst = Ref<CompType##Ref>(memnew(CompType##Ref));\
	\
	inst->set_owner(p_owner);\
	\
	flecs::entity ent = p_owner->get_entity();\
	inst->set_internal_owner(ent);\
	flecs::world* world = p_owner->get_internal_world();\
	inst->set_internal_world(world);\
	\
	inst->set_component(ent.world().component<CompType>());\
	\
	if(ent.has<CompType>()) {\
		inst->set_data(ent.get_mut<CompType>());\
	}\
	else {\
		CompType comp;\
		inst->set_data(comp);\
	}\
	\
	if(!p_owner->has_component(#CompType)){\
		p_owner->set_component(inst);\
	}\
	\
	return inst;\
}

#define SINGLETON_FACTORY(CompType)\
static Ref<FlecsWorld> create_singleton_component(FlecsWorld *p_world) {\
	if (!p_world) {\
		ERR_FAIL_COND_V_MSG(!p_world,p_world, "world is not valid");\
	}\
	flecs::world *world = p_world->get_world_ref();\
	if(!world) {\
		ERR_FAIL_COND_V_MSG(!world, p_world, "world is null");\
	}\
	bool has_comp_type = world->has<CompType>();\
	if(!has_comp_type) {\
		world->set<CompType>({});\
	}\
	if(!p_world->has_component(#CompType)){\
		Ref<CompType##Ref> comp;\
		comp.instantiate(); \
		p_world->set_component(comp);\
	}\
	return p_world;\
}\

#define TAG_FACTORY(TagType) \
static Ref<TagType##Ref> create_tag(const Ref<FlecsEntity> &p_owner) {\
	if (!p_owner.is_valid()) {\
		ERR_FAIL_COND_V_MSG(true, Ref<TagType##Ref>(), "owner is not valid");\
	}\
	\
	Ref<TagType##Ref> inst = Ref<TagType##Ref>(memnew(TagType##Ref));\
	\
	inst->set_owner(p_owner);\
	\
	flecs::entity ent = p_owner->get_entity();\
	inst->set_internal_owner(ent);\
	\
	flecs::world* world = p_owner->get_internal_world();\
	inst->set_internal_world(world);\
	inst->set_component(ent.world().component<TagType>());\
	\
	if(!p_owner->has_component(#TagType)){\
		p_owner->set_component(inst);\
	}\
	p_owner->get_entity().add<TagType>();\
	\
	return inst;\
}
//     
// 


#define DEFINE_COMPONENT_PROXY(CompType, PROP_DEFS, PROP_BINDS) \
GDCLASS(CompType##Ref, FlecsComponentBase); \
public: \
	\
	\
	CompType##Ref() = default; \
	\
	~CompType##Ref() = default;\
	\
	PROP_DEFS \
    \
    \
    COMPONENT_FACTORY(CompType)\
    static void _bind_methods() { \
        PROP_BINDS \
		ClassDB::bind_static_method(\
			CompType##Ref::get_class_static(),\
			"create_component",\
			&CompType##Ref::create_component,\
			"owner"\
		);\
    } \
    \
    void set_data(CompType &d) { FlecsComponent<CompType>::set_data(d); } \
    StringName get_type_name() const { return #CompType; } \
\
;


#define DEFINE_SINGLETON_COMPONENT_PROXY(CompType, PROP_DEFS, PROP_BINDS) \
GDCLASS(CompType##Ref, FlecsComponentBase); \
public: \
	\
	\
	CompType##Ref() = default; \
	\
	~CompType##Ref() = default;\
	\
	PROP_DEFS \
    \
    SINGLETON_FACTORY(CompType)\
    static void _bind_methods() { \
        PROP_BINDS \
		ClassDB::bind_static_method(\
			CompType##Ref::get_class_static(),\
			"create_singleton_component",\
			&CompType##Ref::create_singleton_component,\
			"world"\
		);\
    } \
    \
    void set_data(CompType &d) { FlecsSingletonComponent<CompType>::set_data(d); } \
    StringName get_type_name() const { return #CompType; } \
\
;


#define DEFINE_TAG_PROXY(CompType) \
    GDCLASS(CompType##Ref, FlecsComponentBase); \
public: \
	\
	CompType##Ref() = default; \
	\
	~CompType##Ref() = default;\
	\
    TAG_FACTORY(CompType)\
    static void _bind_methods() { \
		\
		ClassDB::bind_static_method(\
			CompType##Ref::get_class_static(),\
			"create_tag",\
			&CompType##Ref::create_tag,\
			"owner"\
		);\
    } \
    \
    StringName get_type_name() const { return #CompType; } \
\
;