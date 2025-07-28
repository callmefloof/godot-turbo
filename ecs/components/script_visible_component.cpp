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


void ScriptVisibleComponentRef::set_data(ScriptVisibleComponent &p_data) {
	FlecsComponent<ScriptVisibleComponent>::set_data(p_data);
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
	if (auto script_visible_component = owner.get_mut<ScriptVisibleComponent>(); script_visible_component.fields.has(field_name)) {
		if (const Variant *value = script_visible_component.fields.getptr(field_name)) {
			return *value;
		}
	}
	ERR_PRINT("Field type not found. Returning empty variant.");
	return Variant();
}
 void ScriptVisibleComponentRef::set_field(const StringName &field_name, const Variant &value) const {
	if (auto script_visible_component = owner.get_mut<ScriptVisibleComponent>(); script_visible_component.fields.has(field_name)) {
		const auto value_ptr = script_visible_component.fields.getptr(field_name);
		*value_ptr = value;
	}
	ERR_PRINT("Field type not found.");
}

 void ScriptVisibleComponentRef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_field_value", "field_name"), &ScriptVisibleComponentRef::get_field_value);
	ClassDB::bind_method(D_METHOD("set", "field_name", "value"), &ScriptVisibleComponentRef::set_field);
	ClassDB::bind_method(D_METHOD("get_virtual_component_type_hash"), &ScriptVisibleComponentRef::get_virtual_component_type_hash);


	ClassDB::bind_static_method(ScriptVisibleComponentRef::get_class_static(), "create_component", &ScriptVisibleComponentRef::create_component);
}
 bool ScriptVisibleComponentRef::is_dynamic() const {
	return true;
}

 Ref<ScriptVisibleComponentRef> ScriptVisibleComponentRef::create_component(const StringName &name, const Ref<FlecsEntity> &p_owner) {
	const auto ref = Ref<ScriptVisibleComponentRef>(memnew(ScriptVisibleComponentRef));
	ref->set_flecs_owner(p_owner->get_entity());
	ref->set_owner(p_owner);
	auto comp = ref->get_internal_owner().get_mut<ScriptVisibleComponent>();
	if (const auto *registry = ScriptComponentRegistry::get_singleton()) {
		comp.fields = registry->create_field_map(comp.name);
	}
	return ref;
}
 Ref<FlecsComponentBase> ScriptVisibleComponentRef::clone() const {
	Ref<ScriptVisibleComponentRef> new_ref;
	new_ref.instantiate();
	new_ref->set_data(get_data());
	return new_ref;
}

uint64_t ScriptVisibleComponentRef::get_virtual_component_type_hash() const {
	return owner.get_ref<ScriptVisibleComponent>()->get_virtual_component_type_hash();
}