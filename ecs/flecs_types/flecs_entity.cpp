//
// Created by Floof on 16-7-2025.
//
#include "flecs_entity.h"
#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "flecs_component_base.h"
#include "../components/script_visible_component.h"
#include "../components/script_component_registry.h"
#include "core/object/class_db.h"
#include "flecs_world.h"

 void FlecsEntity::_bind_methods() {
	//fill in methods
 	ClassDB::bind_method(D_METHOD("get_component", "component_type"),&FlecsEntity::get_component);
 	ClassDB::bind_method(D_METHOD("remove_all_components"),&FlecsEntity::remove_all_components);
 	ClassDB::bind_method(D_METHOD("get_component_types"),&FlecsEntity::get_component_types);
 	ClassDB::bind_method(D_METHOD("get_entity_name"),&FlecsEntity::get_entity_name);
 	ClassDB::bind_method(D_METHOD("set_entity_name", "p_name"),&FlecsEntity::set_entity_name);
 	ClassDB::bind_method(D_METHOD("set_component", "component_type"),&FlecsEntity::set_component);
 	ClassDB::bind_method(D_METHOD("get_component_by_name", "component_type"),&FlecsEntity::get_component_by_name);
	ClassDB::bind_method(D_METHOD("has_component", "component_type"),&FlecsEntity::has_component);
	ClassDB::bind_method(D_METHOD("remove_with_component", "component"),&FlecsEntity::remove_with_component);
	ClassDB::bind_method(D_METHOD("remove", "component_name"),&FlecsEntity::remove);
	ClassDB::bind_method(D_METHOD("get_parent"), &FlecsEntity::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "parent"), &FlecsEntity::set_parent);
	ClassDB::bind_method(D_METHOD("get_children"), &FlecsEntity::get_children);
	ClassDB::bind_method(D_METHOD("add_child", "child"), &FlecsEntity::add_child);
	ClassDB::bind_method(D_METHOD("remove_child", "child"), &FlecsEntity::remove_child);
	ClassDB::bind_method(D_METHOD("remove_all_children"), &FlecsEntity::remove_all_children);
	ClassDB::bind_method(D_METHOD("add_relationship", "pair"), &FlecsEntity::add_relationship);
	ClassDB::bind_method(D_METHOD("remove_relationship", "first_entity", "second_entity"), &FlecsEntity::remove_relationship);
	ClassDB::bind_method(D_METHOD("get_relationship", "first_entity", "second_entity"), &FlecsEntity::get_relationship);
	ClassDB::bind_method(D_METHOD("get_relationships"), &FlecsEntity::get_relationships);
	



}
 void FlecsEntity::remove_with_component(const Ref<FlecsComponentBase> &comp) {
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
	Vector<Ref<FlecsComponentBase>> comp_copy = components;
	for(int i = 0; i < comp_copy.size(); i++){
		remove_with_component(comp_copy[i]);
	}
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
bool FlecsEntity::has_component(const StringName &component_type) const {
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			continue;
		}
		if (components[i].is_null()) {
			continue;
		}
		if (components[i]->get_type_name() == component_type) {
			return true;
		}
	}
	return false;
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
		entity.set<ScriptVisibleComponent>(data);
		dyn->set_data(data); // ensures pointer is synced
		return;
	}
	
	
 	components.append(comp_ref);
}

void FlecsEntity::remove(const String &component_type) {
	const char* c_component_type = component_type.ascii().get_data();
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

Ref<FlecsEntity> FlecsEntity::get_parent() const {
	return gd_parent;
}

void FlecsEntity::set_parent(const Ref<FlecsEntity> &p_parent) {
	if (p_parent.is_valid()) {
		parent = p_parent->get_internal_entity();
		gd_parent = p_parent;
	} else {
		parent = flecs::entity();
		gd_parent = Ref<FlecsEntity>();
	}
	entity.add(flecs::ChildOf, parent);
}

flecs::entity FlecsEntity::get_internal_parent() const {
	return parent;
}

flecs::entity FlecsEntity::get_internal_entity() const {
	return entity;
}

Ref<FlecsEntity> FlecsEntity::get_child(int index) const {
	if (index < 0 || index >= children.size()) {
		ERR_PRINT("Index out of bounds for children array.");
		return Ref<FlecsEntity>();
	}
	return children[index];
}

void FlecsEntity::set_children(const TypedArray<FlecsEntity> &p_children) {
	// Clear existing children.
	remove_all_children();

	for(int i = 0; i < p_children.size(); i++) {
		Variant v = p_children[i];
		Object *obj = v;
		String errormsg = "Expected FlecsEntity object, got: " + Variant::get_type_name(v.get_type()) + " at index " + itos(i);
		if(!obj || !Object::cast_to<FlecsEntity>(obj)) {
			ERR_PRINT(errormsg);
			continue;
		}
		FlecsEntity *child = Object::cast_to<FlecsEntity>(obj);
		if (child) {
			add_child(child);
		}
	}
}

void FlecsEntity::add_child(const Ref<FlecsEntity> &child) {
	if (!child.is_valid()) {
		ERR_PRINT("Cannot add an invalid child entity.");
		return;
	}
	for(auto &existing_child : children) {
		if (existing_child == child) {
			return;
		}
	}
	children.append(child);
	if(entity.has(flecs::ChildOf, child->get_internal_entity())) {
		return;
	}
	if(child->get_internal_entity().parent() == entity) {
		return;
	}
	entity.add(flecs::ChildOf, child->get_internal_entity());
}

void FlecsEntity::remove_child(const Ref<FlecsEntity> &child) {
	if (!child.is_valid()) {
		ERR_PRINT("Cannot remove an invalid child entity.");
		return;
	}
	if (children.erase(child)) {
		entity.remove(flecs::ChildOf, child->get_internal_entity());
	} else {
		ERR_PRINT("Child entity not found in children array.");
	}
}

TypedArray<FlecsEntity> FlecsEntity::get_children() const {
	TypedArray<FlecsEntity> child_array;
	for (const auto &child : children) {
		child_array.append(child);
	}
	return child_array;
}

void FlecsEntity::add_component(const Ref<FlecsComponentBase> &comp_ref) {
	if (!comp_ref.is_valid()) {
		ERR_PRINT("add_component(): Component is null or invalid.");
		return;
	}

	// Check if the component is already added
	if (has_component(comp_ref->get_type_name())) {
		ERR_PRINT("Component already exists in entity.");
		return;
	}

	set_component(comp_ref);
}

flecs::world* FlecsEntity::get_internal_world() const {
	return world;
}

void FlecsEntity::set_internal_world(flecs::world* p_world) {
	if (!p_world) {
		ERR_PRINT("FlecsEntity: set_internal_world() called with null world.");
		return;
	}
	world = p_world;
}


void FlecsEntity::remove_all_children() {
	children.clear();
}

void FlecsEntity::add_relationship(FlecsPair *pair) {
	if (!pair) {
		ERR_PRINT("FlecsEntity::add_relationship called with null pair");
		return;
	}
	relationships.append(pair);
}

void FlecsEntity::remove_relationship(const StringName &first_entity, const StringName &second_entity) {
	int8_t index = -1;
	FlecsPair *pair = nullptr;
	for (int i = 0; i < relationships.size(); i++) {
		pair = relationships[i].ptr();
		if (pair->get_first()->get_name() == first_entity && pair->get_second()->get_name() == second_entity) {
			index = i;
			break;
		}
	}
	if(!pair){
		ERR_PRINT("FlecsEntity::remove_relationship: pair not found for " + first_entity + " and " + second_entity);
		return;
	}
	world->remove(pair->get_first()->get_entity(),pair->get_second()->get_entity());
	memdelete(pair->get_first());
	memdelete(pair->get_second());
	pair->set_first(nullptr);
	pair->set_second(nullptr);
	relationships.erase(pair);
	memdelete(pair);
	return;
}

Ref<FlecsPair> FlecsEntity::get_relationship(const StringName &first_entity, const StringName &second_entity) const {
	for (const auto &pair : relationships) {
		if (pair.is_null()) {
			ERR_PRINT("FlecsEntity::get_relationship: pair is null, skipping.");
			continue;
		}
		if (pair->get_first()->get_name() == first_entity && pair->get_second()->get_name() == second_entity) {
			return pair;
		}
	}
	ERR_PRINT("FlecsEntity::get_relationship: relationship not found for " + first_entity + " and " + second_entity);
	return Ref<FlecsPair>();
}

TypedArray<FlecsPair> FlecsEntity::get_relationships() const {
	TypedArray<FlecsPair> result;
	for (const auto &pair : relationships) {
		if (pair.is_null()) {
			ERR_PRINT("FlecsEntity::get_relationships: pair is null, skipping.");
			continue;
		}
		result.append(pair);
	}
	return result;
}


