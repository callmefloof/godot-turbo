//
// Created by Floof on 16-7-2025.
//
#include "flecs_entity.h"
#include "../../../../core/error/error_macros.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/string/ustring.h"
#include "flecs_component_base.h"
#include "../components/script_visible_component.h"
#include "../components/script_component_registry.h"
#include "../../../../core/object/class_db.h"

 void FlecsEntity::_bind_methods() {
	//fill in methods
 	ClassDB::bind_method(D_METHOD("get_component", "component_type"),&FlecsEntity::get_component);
 	ClassDB::bind_method(D_METHOD("remove_all_components"),&FlecsEntity::remove_all_components);
 	ClassDB::bind_method(D_METHOD("get_component_types"),&FlecsEntity::get_component_types);

 	ClassDB::bind_method(D_METHOD("get_entity_name"),&FlecsEntity::get_entity_name);
 	ClassDB::bind_method(D_METHOD("set_entity_name", "p_name"),&FlecsEntity::set_entity_name);
 	ClassDB::bind_method(D_METHOD("set_component", "component_type"),&FlecsEntity::set_component);
 	ClassDB::bind_method(D_METHOD("remove", "component_type"),&FlecsEntity::set_component);
 	ClassDB::bind_method(D_METHOD("get_component_by_name", "component_type"),&FlecsEntity::get_component_by_name);



}
 void FlecsEntity::remove(const Ref<FlecsComponentBase> &comp) {
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

 void FlecsEntity::remove_all_components() {
	components.clear();
}
 Ref<FlecsComponentBase> FlecsEntity::get_component(const StringName &component_type) const {
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
 PackedStringArray FlecsEntity::get_component_types() const {
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
 StringName FlecsEntity::get_entity_name() const {
	if (entity) {
		return StringName(entity.name().c_str());
	}
	return "ERROR";
}
 void FlecsEntity::set_entity_name(const StringName &p_name) const {
	if (entity) {
		const String string_name = p_name;
		entity.set_name(string_name.ascii().get_data());
		return;
	}
	ERR_PRINT("no entity referenced");
	return;
}
 void FlecsEntity::set_entity(const flecs::entity& p_entity) {
	entity = p_entity;
}
 flecs::entity FlecsEntity::get_entity() const {
	return entity;
}

 void FlecsEntity::set_component(const Ref<FlecsComponentBase>& comp_ref) {
	if (!comp_ref.is_valid()) {
		ERR_PRINT("add_component(): Component is null or invalid.");
		return;
	}

	// Handle dynamic script-visible components
	if (comp_ref->is_dynamic()) {
		const Ref<ScriptVisibleComponentRef> dyn = comp_ref;
		ScriptVisibleComponent& data = dyn->get_data();

		const StringName type_name = data.name;
		const AHashMap<StringName, ScriptComponentRegistry::FieldDef>* schema = ScriptComponentRegistry::get_singleton()->get_schema(type_name);

		if (!schema) {
			ERR_PRINT("add_component(): Unknown script component type: " + type_name);
			return;
		}

		// Fill in missing defaults
		for (AHashMap<StringName, ScriptComponentRegistry::FieldDef>::ConstIterator it = schema->begin(); it != schema->end(); ++it) {
			StringName field_name = it->key;
			ScriptComponentRegistry::FieldDef def = it->value;

			if (!data.fields.has(field_name)) {
				data.fields.insert(field_name, def.default_value);
			} else {
				// Optional: Validate type
				if (data.fields.getptr(field_name)->get_type() != def.type) {
					WARN_PRINT("Field '" + String(field_name) + "' has wrong type — expected " + Variant::get_type_name(def.type));
				}
			}
		}

		// Set in ECS
		// ReSharper disable once CppExpressionWithoutSideEffects
		entity.set<ScriptVisibleComponent>(data);
		dyn->set_data(data); // ensures pointer is synced
		return;
	}
 	components.append(comp_ref);
}
void FlecsEntity::remove(String &component_type) {
	const char* c_component_type =component_type.ascii().get_data();
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
Ref<FlecsComponentBase> FlecsEntity::get_component_by_name(const StringName &component_type) {
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