//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPT_VISIBLE_COMPONENT_H
#define SCRIPT_VISIBLE_COMPONENT_H
#include "../../../../core/error/error_macros.h"
#include "../../../../core/object/class_db.h"
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/os/memory.h"
#include "../../../../core/string/string_name.h"
#include "../../../../core/templates/oa_hash_map.h"
#include "../../../../core/variant/variant.h"
#include "../flecs_types/flecs_component.h"

struct ScriptVisibleComponent {
	StringName name;
	OAHashMap<StringName, Variant> fields;
};

//making an exception for templating here
class ScriptVisibleComponentRef : public FlecsComponent<ScriptVisibleComponent> {
	GDCLASS(ScriptVisibleComponentRef, FlecsComponent)
	public:
	ScriptVisibleComponentRef() = default;
	~ScriptVisibleComponentRef() override = default;
	Variant get(const StringName& field_name) const;
	void set(const StringName& field_name, const Variant& value) const;
	static void _bind_methods();

	static Ref<ScriptVisibleComponentRef> create_component();
};

inline Variant ScriptVisibleComponentRef::get(const StringName &field_name) const {
	if (data && data->fields.has(field_name)) {
		return data->fields.lookup_ptr(field_name);
	}
	ERR_PRINT("Field type not found. Returning empty variant.");
	return Variant();
}
inline void ScriptVisibleComponentRef::set(const StringName &field_name, const Variant &value) const {
	if (data && data->fields.has(field_name)) {
		*data->fields.lookup_ptr(field_name) = value;
	}
	ERR_PRINT("Field type not found.");
}

inline void ScriptVisibleComponentRef::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get", "field_name"), &ScriptVisibleComponentRef::get);
	ClassDB::bind_method(D_METHOD("set", "field_name", "value"), &ScriptVisibleComponentRef::set);

	ClassDB::bind_static_method(ScriptVisibleComponentRef::get_class_static(),"create_component",&ScriptVisibleComponentRef::create_component);\
}

inline Ref<ScriptVisibleComponentRef> ScriptVisibleComponentRef::create_component() {
	return Ref<ScriptVisibleComponentRef>(memnew(ScriptVisibleComponentRef));
}
#endif //SCRIPT_VISIBLE_COMPONENT_H
