//
// Created by Floof on 16-7-2025.
//
#include "script_component_registry.h"
#include "core/object/class_db.h"
#include "core/config/engine.h"
#include "core/os/memory.h"

ScriptComponentRegistry* ScriptComponentRegistry::singleton = nullptr;

void ScriptComponentRegistry::register_component_type(const StringName& name, const AHashMap<StringName, FieldDef>& fields) {
	component_schemas.insert(name, fields);
}

const AHashMap<StringName, ScriptComponentRegistry::FieldDef>* ScriptComponentRegistry::get_schema(const StringName& name) const {
	return component_schemas.getptr(name);
}

AHashMap<StringName, Variant> ScriptComponentRegistry::create_field_map(const StringName &name) const {
	AHashMap<StringName, Variant> result;
	const AHashMap<StringName, FieldDef>* schema = get_schema(name);
	if (schema) {
		for (AHashMap<StringName,FieldDef>::ConstIterator it = schema->begin(); it != schema->end(); ++it) {
			result.insert(it->key,it->value.default_value);
		}
	}
	return result;
}

void ScriptComponentRegistry::register_component_type_from_dict(const StringName &name, const Dictionary &def) {
	AHashMap<StringName, FieldDef> schema;
	for (const Variant *k = def.next(nullptr); k; k = def.next(k)) {
		StringName key = *k;
		Variant value = def[*k];

		if (value.get_type() == Variant::INT) {
			schema.insert(key, FieldDef(static_cast<Variant::Type>(static_cast<int>(value)), Variant()));
		} else if (value.get_type() == Variant::DICTIONARY) {
			Dictionary vdict = value;
			const auto type = static_cast<Variant::Type>(static_cast<int>(vdict.get("type", Variant::NIL)));
			const Variant def_val = vdict.get("default", Variant());
			schema.insert(key, FieldDef(type, def_val));
		}
	}
	register_component_type(name, schema);
}
void ScriptComponentRegistry::register_singleton() {
	// In some global init point (e.g., module or autoload)
	ScriptComponentRegistry *reg = memnew(ScriptComponentRegistry);
	const auto obj = dynamic_cast<Object*>(singleton);
	Engine::get_singleton()->add_singleton(Engine::Singleton("ScriptComponentRegistry", obj));
}

void ScriptComponentRegistry::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_component_type", "name", "fields"), &ScriptComponentRegistry::register_component_type_from_dict);}