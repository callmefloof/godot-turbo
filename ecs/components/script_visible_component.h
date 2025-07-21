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
#include "../../../../core/typedefs.h"
#include "../../../../core/error/error_macros.h"
#include "script_component_registry.h"
class FlecsEntity;

struct ScriptVisibleComponent {
	StringName name;
	OAHashMap<StringName, Variant> fields;
};

//making an exception for templating here
class ScriptVisibleComponentRef : public FlecsComponent<ScriptVisibleComponent> {
	GDCLASS(ScriptVisibleComponentRef, FlecsComponent<ScriptVisibleComponent>);
public:
	void commit_to_entity(const Ref<FlecsEntity> &p_entity) const override;
	void set_data(ScriptVisibleComponent *p_data) override;
	void *get_data_ptr() const override;
	void clear_component() override;
	StringName get_type_name() const override;


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


template <>
inline void FlecsComponent<ScriptVisibleComponent>::byte_deserialize(const PackedByteArray &p_ba) {
	ERR_PRINT("Not implemented");
}


#endif //SCRIPT_VISIBLE_COMPONENT_H
