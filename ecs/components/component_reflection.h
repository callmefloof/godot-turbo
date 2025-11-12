#pragma once

#include "core/variant/dictionary.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include <functional>

// Forward declarations
namespace FlecsReflection {

// Type-erased serialization function signature
using SerializeFn = std::function<::Dictionary(const void* data)>;
using DeserializeFn = std::function<void(void* data, const ::Dictionary& dict)>;

struct ComponentMeta {
	flecs::entity_t component_id = 0;
	StringName name;
	SerializeFn serialize = nullptr;
	DeserializeFn deserialize = nullptr;
	size_t size = 0;
	size_t alignment = 0;
};

// Central registry for component metadata
class Registry {
private:
	HashMap<StringName, ComponentMeta> name_to_meta;
	HashMap<flecs::entity_t, ComponentMeta*> id_to_meta;

public:
	static Registry& get() {
		static Registry instance;
		return instance;
	}

	void register_component(const StringName& name, const ComponentMeta& meta) {
		name_to_meta[name] = meta;
		if (meta.component_id != 0) {
			id_to_meta[meta.component_id] = &name_to_meta[name];
		}
	}

	void bind_component_id(const StringName& name, flecs::entity_t id) {
		if (ComponentMeta* meta = name_to_meta.getptr(name)) {
			meta->component_id = id;
			id_to_meta[id] = meta;
		}
	}

	ComponentMeta* get_by_name(const StringName& name) {
		return name_to_meta.getptr(name);
	}

	ComponentMeta* get_by_id(flecs::entity_t id) {
		if (ComponentMeta** ptr = id_to_meta.getptr(id)) {
			return *ptr;
		}
		return nullptr;
	}

	// Serialize component from entity
	::Dictionary serialize(const flecs::entity& e, flecs::entity_t component_id) {
		ComponentMeta* meta = get_by_id(component_id);
		if (!meta || !meta->serialize) {
			return ::Dictionary();
		}

		const void* data = e.get(component_id);
		if (!data) {
			return ::Dictionary();
		}

		return meta->serialize(data);
	}

	// Deserialize component to entity
	void deserialize(flecs::entity& e, flecs::entity_t component_id, const ::Dictionary& dict) {
		ComponentMeta* meta = get_by_id(component_id);
		if (!meta || !meta->deserialize) {
			return;
		}

		void* data = e.get_mut(component_id);
		if (data) {
			meta->deserialize(data, dict);
		}
	}

	void clear() {
		name_to_meta.clear();
		id_to_meta.clear();
	}
};

// Helper template for automatic registration
template<typename T>
struct ComponentRegistrar {
	static void register_type(const StringName& name,
							   SerializeFn serialize_fn = nullptr,
							   DeserializeFn deserialize_fn = nullptr) {
		ComponentMeta meta;
		meta.name = name;
		meta.serialize = serialize_fn;
		meta.deserialize = deserialize_fn;
		meta.size = sizeof(T);
		meta.alignment = alignof(T);

		Registry::get().register_component(name, meta);
	}
};

// Macro for simple component registration without serialization
#define FLECS_COMPONENT(TYPE) \
	namespace { \
		struct TYPE##_AutoRegister { \
			TYPE##_AutoRegister() { \
				FlecsReflection::ComponentRegistrar<TYPE>::register_type(#TYPE); \
			} \
		}; \
		static TYPE##_AutoRegister TYPE##_auto_reg; \
	}

// Macro for component registration with custom serialization
#define FLECS_COMPONENT_SERIALIZABLE(TYPE, SERIALIZE_FN, DESERIALIZE_FN) \
	namespace { \
		struct TYPE##_AutoRegister { \
			TYPE##_AutoRegister() { \
				FlecsReflection::ComponentRegistrar<TYPE>::register_type( \
					#TYPE, \
					SERIALIZE_FN, \
					DESERIALIZE_FN \
				); \
			} \
		}; \
		static TYPE##_AutoRegister TYPE##_auto_reg; \
	}

} // namespace FlecsReflection