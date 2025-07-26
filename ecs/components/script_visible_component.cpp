//
// Created by Floof on 21-7-2025.
//
#include "script_visible_component.h"
#include "../../../../core/object/object.h"
#include "../../../../core/typedefs.h"
#include "../../../../core/variant/variant.h"
#include "../flecs_types/flecs_component_base.h"
#include "script_component_registry.h"

template class FlecsComponent<ScriptVisibleComponent>;

void ScriptVisibleComponentRef::commit_to_entity(const Ref<FlecsEntity> &p_entity) const {
	FlecsComponent<ScriptVisibleComponent>::commit_to_entity(p_entity);
}
void ScriptVisibleComponentRef::set_data(ScriptVisibleComponent *p_data) {
	FlecsComponent<ScriptVisibleComponent>::set_data(p_data);
}
void *ScriptVisibleComponentRef::get_data_ptr() const {
	return FlecsComponent<ScriptVisibleComponent>::get_data_ptr();
}
void ScriptVisibleComponentRef::clear_component() {
	FlecsComponent<ScriptVisibleComponent>::clear_component();
}
StringName ScriptVisibleComponentRef::get_type_name() const {
	return "ScriptVisibleComponentRef";
}
void ScriptVisibleComponentRef::append_bytes(PackedByteArray &ba, const void *p_data, const size_t size) const{
	const int old_size = ba.size();
	ba.resize(old_size + size);
	memcpy(ba.ptrw() + old_size, p_data, size);
}


 Variant ScriptVisibleComponentRef::get_field_value(const StringName &field_name) const {
	const auto script_visible_component = static_cast<ScriptVisibleComponent *>(FlecsComponentBase::data);
	if (script_visible_component == nullptr) {
		ERR_PRINT("ScriptVisibleComponentRef::get: data is null");
		return Variant();
	}
	if (script_visible_component->fields.has(field_name)) {
		if (const Variant *value = script_visible_component->fields.getptr(field_name)) {
			return *value;
		}
	}
	ERR_PRINT("Field type not found. Returning empty variant.");
	return Variant();
}
 void ScriptVisibleComponentRef::set(const StringName &field_name, const Variant &value) const {

 	const auto script_visible_component = static_cast<ScriptVisibleComponent *>(FlecsComponentBase::data);
	if (script_visible_component && script_visible_component->fields.has(field_name)) {
		*script_visible_component->fields.getptr(field_name) = value;
	}
	ERR_PRINT("Field type not found.");
}

 void ScriptVisibleComponentRef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_field_value", "field_name"), &ScriptVisibleComponentRef::get_field_value);
	ClassDB::bind_method(D_METHOD("set", "field_name", "value"), &ScriptVisibleComponentRef::set);

	ClassDB::bind_static_method(ScriptVisibleComponentRef::get_class_static(), "create_component", &ScriptVisibleComponentRef::create_component);
}
 bool ScriptVisibleComponentRef::is_dynamic() const {
	return true;
}

 Ref<ScriptVisibleComponentRef> ScriptVisibleComponentRef::create_component(const StringName &name) {
	const auto ref = Ref<ScriptVisibleComponentRef>(memnew(ScriptVisibleComponentRef));
	ref->data = memnew(ScriptVisibleComponent);

	// Example: this assumes you set the name somehow (maybe pass it into create_component)
	const auto comp = static_cast<ScriptVisibleComponent *>(ref->data);
	comp->name = name;

	// Auto-fill default values based on schema
	if (const auto *registry = ScriptComponentRegistry::get_singleton()) {
		comp->fields = registry->create_field_map(comp->name);
	}
	ref->component_type_hash = FlecsComponentBase::type_hash<ScriptVisibleComponent>();
	return ref;
}
 Ref<FlecsComponentBase> ScriptVisibleComponentRef::clone() const {
	Ref<ScriptVisibleComponentRef> new_ref;
	new_ref.instantiate();
	if (data) {
		// Step 1: cast void* to actual type
		const auto typed = static_cast<ScriptVisibleComponent *>(data);

		// Step 2: allocate memory and copy
		ScriptVisibleComponent *copied = memnew(ScriptVisibleComponent);
		*copied = *typed; // copy the struct contents

		new_ref->set_data(copied);
	}
	return new_ref;
}
 void ScriptVisibleComponentRef::apply_to_entity(flecs::entity &e) const {
	if (data) {
		e.set<ScriptVisibleComponent>(*static_cast<ScriptVisibleComponent *>(data));
	}
}