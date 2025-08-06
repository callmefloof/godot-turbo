#ifndef SCRIPT_COMPONENT_REGISTRY_H
#define SCRIPT_COMPONENT_REGISTRY_H

#include "core/object/ref_counted.h"
#include "core/templates/a_hash_map.h"
#include "core/string/string_name.h"
#include "core/variant/variant.h"
#include "core/object/object.h"
#include "core/variant/dictionary.h"

class ScriptComponentRegistry : public RefCounted {
	GDCLASS(ScriptComponentRegistry, RefCounted);



	static ScriptComponentRegistry* singleton;

public:
	struct FieldDef {
		Variant::Type type = Variant::NIL;
		Variant default_value;

		FieldDef() = default;
		explicit FieldDef(Variant::Type t, Variant def = Variant()) : type(t), default_value(def) {}
	};
private:
	AHashMap<StringName, AHashMap<StringName, FieldDef>> component_schemas;
public:
	ScriptComponentRegistry() { singleton = this; }
	~ScriptComponentRegistry() { singleton = nullptr; }

	static ScriptComponentRegistry* get_singleton() { return singleton; }

	void register_component_type(const StringName& name, const AHashMap<StringName, FieldDef>& fields);
	const AHashMap<StringName, FieldDef>* get_schema(const StringName& name) const;

	// optional: automatic population
	AHashMap<StringName, Variant> create_field_map(const StringName& name) const;
	void register_component_type_from_dict(const StringName& name, const Dictionary& def);
	static void register_singleton();
	static void _bind_methods(); // if exposing to script
};

#endif // SCRIPT_COMPONENT_REGISTRY_H
