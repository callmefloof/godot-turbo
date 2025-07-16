//
// Created by Floof on 16-7-2025.
//
#include "../../../../core/error/error_macros.h"
#include "../../../../core/variant/dictionary.h"
#include "../../../../core/templates/oa_hash_map.h"
#include "../../../../core/string/ustring.h"
#include "flecs_component_base.h"
#include "../components/script_visible_component.h"

inline void FlecsEntity::_bind_methods() {
	//fill in methods

}
inline void FlecsEntity::remove(const Ref<FlecsComponentBase> &comp) {
	if (!components.has(comp)) {
		ERR_PRINT("component type not found in entity");
		return;
	}
	const String string_name = comp->get_type_name();
	const char* c_component_type = string_name.ascii().get_data();
	if (const flecs::entity flecs_comp = entity.world().lookup(c_component_type)) {
		entity = entity.remove(flecs_comp);
		components.erase(comp);
		return;
	}
	ERR_PRINT("component type not found in entity");

}

inline void FlecsEntity::remove_all_components() {
	components.clear();
}
inline Ref<FlecsComponentBase> FlecsEntity::get(const StringName &component_type) const {
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		if (components[i]->get_type_name() == component_type) {
			return components[i];
		}
	}
	ERR_PRINT("component type not found. returning nullptr");
	return Ref<FlecsComponentBase>();
}
inline PackedStringArray FlecsEntity::get_component_types() const {
	PackedStringArray ret;
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		ret.push_back(components[i]->get_type_name());
	}
	return ret;
}
inline StringName FlecsEntity::get_entity_name() const {
	if (entity) {
		return StringName(entity.name().c_str());
	}
	return "ERROR";
}
inline void FlecsEntity::set_entity_name(const StringName &name) const {
	if (entity) {
		const String string_name = name;
		entity = entity.set_name(string_name.ascii().get_data());
		return;
	}
	ERR_PRINT("no entity referenced");
	return;
}
inline void FlecsEntity::set_entity(const flecs::entity& p_entity) const {
	entity = p_entity;
}
inline flecs::entity& FlecsEntity::get_entity() const {
	return entity;
}

void FlecsEntity::set(const Ref<FlecsComponentBase>& comp) {
	if (!comp.is_valid()) {
		ERR_PRINT("add_component(): Component is null or invalid.");
		return;
	}

	// Handle dynamic script-visible components
	if (comp->is_dynamic()) {
		const Ref<ScriptVisibleComponentRef> dyn = comp;
		ScriptVisibleComponent* data = dyn->get_data();

		if (!data) {
			ERR_PRINT("add_component(): ScriptVisibleComponent has no data.");
			return;
		}

		const StringName type_name = data->name;
		const auto* schema = ScriptComponentRegistry::get_singleton()->get_schema(type_name);

		if (!schema) {
			ERR_PRINT("add_component(): Unknown script component type: " + type_name);
			return;
		}

		// Fill in missing defaults
		for (auto& [field_name, def] : *schema) {
			if (!data->fields.has(field_name)) {
				data->fields.insert(field_name, def.default_value);
			} else {
				// Optional: Validate type
				if (data->fields[field_name].get_type() != def.type) {
					WARN_PRINT("Field '" + String(field_name) + "' has wrong type — expected " + Variant::get_type_name(def.type));
				}
			}
		}

		// Set in ECS
		// ReSharper disable once CppExpressionWithoutSideEffects
		entity.set<ScriptVisibleComponent>(*data);
		dyn->set_data(data); // ensures pointer is synced
		return;
	}

	// Static typed component path
	comp->commit_to_entity(Ref<FlecsEntity>(this));
}
void FlecsEntity::remove(const StringName &component_type) {
	const char* c_component_type = String(component_type).ascii().get_data();
	const flecs::entity component = entity.world().lookup(c_component_type);
	if (component.is_valid()) {
		ERR_PRINT("internal flecs component type is invalid. this likely means it wasn't added.");
		return;
	}
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		if (components[i]->get_type_name() != c_component_type) {
			continue;
		}
		// ReSharper disable once CppExpressionWithoutSideEffects
		entity.remove(component);
		components.remove_at(i);
		return;
	}
	ERR_PRINT("component type not found in entity");
}
Ref<FlecsComponentBase> FlecsEntity::get(const StringName &component_type) {
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		if (components[i]->get_type_name() != component_type) {
			continue;
		}
		return components[i];
	}
	ERR_PRINT("component type not found in entity. returning nullptr");
	return Ref<FlecsComponentBase>();
}