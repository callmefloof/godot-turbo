//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPT_VISIBLE_COMPONENT_H
#define SCRIPT_VISIBLE_COMPONENT_H
#include "../../../../core/object/class_db.h"
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/os/memory.h"
#include "../../../../core/string/string_name.h"
#include "../../../../core/templates/oa_hash_map.h"
#include "../../../../core/variant/variant.h"
#include "../flecs_types/flecs_component.h"
#include "../../../../core/error/error_macros.h"
#include "script_component_registry.h"

struct ScriptVisibleComponent {
	StringName name;
	OAHashMap<StringName, Variant> fields;
};

//making an exception for templating here
class ScriptVisibleComponentRef : public FlecsComponent<ScriptVisibleComponent> {
	GDCLASS(ScriptVisibleComponentRef, FlecsComponent<ScriptVisibleComponent>)
private:
	void append_bytes(PackedByteArray &array, const void *data, size_t size) const;
	public:
	ScriptVisibleComponentRef() = default;
	~ScriptVisibleComponentRef() override = default;
	Variant get_field_value(const StringName& field_name) const;
	void set(const StringName& field_name, const Variant& value) const;
	static void _bind_methods();
	bool is_dynamic() const override;
	static Ref<ScriptVisibleComponentRef> create_component(const StringName& name);
	Ref<FlecsComponentBase> clone() const override;
	void apply_to_entity(flecs::entity &e) const override;
};

inline void append_bytes(PackedByteArray &ba, const void *data, const size_t size) {
	const int old_size = ba.size();
	ba.resize(old_size + size);
	memcpy(ba.ptrw() + old_size, data, size);
}


inline Variant ScriptVisibleComponentRef::get_field_value(const StringName &field_name) const {
	const auto component = static_cast<ScriptVisibleComponent *>(data);
	if (component == nullptr) {
		ERR_PRINT("ScriptVisibleComponentRef::get: data is null");
		return Variant();
	}
	if (component->fields.has(field_name)) {
		if (const Variant *value = component->fields.lookup_ptr(field_name)) {
			return *value;
		}
	}
	ERR_PRINT("Field type not found. Returning empty variant.");
	return Variant();
}
inline void ScriptVisibleComponentRef::set(const StringName &field_name, const Variant &value) const {
	if (const auto component = static_cast<ScriptVisibleComponent *>(data); component && component->fields.has(field_name)) {
		*component->fields.lookup_ptr(field_name) = value;
	}
	ERR_PRINT("Field type not found.");
}

inline void ScriptVisibleComponentRef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_field_value", "field_name"), &ScriptVisibleComponentRef::get_field_value);
	ClassDB::bind_method(D_METHOD("set", "field_name", "value"), &ScriptVisibleComponentRef::set);

	ClassDB::bind_static_method(ScriptVisibleComponentRef::get_class_static(), "create_component", &ScriptVisibleComponentRef::create_component);
}
inline bool ScriptVisibleComponentRef::is_dynamic() const {
	return true;
}

inline Ref<ScriptVisibleComponentRef> ScriptVisibleComponentRef::create_component(const StringName &name) {
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
inline Ref<FlecsComponentBase> ScriptVisibleComponentRef::clone() const {
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
inline void ScriptVisibleComponentRef::apply_to_entity(flecs::entity &e) const {
	if (data) {
		e.set<ScriptVisibleComponent>(*static_cast<ScriptVisibleComponent *>(data));
	}
}


#endif //SCRIPT_VISIBLE_COMPONENT_H
