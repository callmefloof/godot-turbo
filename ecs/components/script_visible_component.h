//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPT_VISIBLE_COMPONENT_H
#define SCRIPT_VISIBLE_COMPONENT_H
#include "../../../../core/error/error_macros.h"
#include "../../../../core/object/class_db.h"
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/string/string_name.h"
#include "../../../../core/templates/a_hash_map.h"
#include "../../../../core/typedefs.h"
#include "../../../../core/variant/variant.h"
#include "../flecs_types/flecs_component.h"
#include "type_id_generator.h"
#include "single_component_module.h"
class FlecsEntity;

struct ScriptVisibleComponent {
	StringName name;
	AHashMap<StringName, Variant> fields;
	uint64_t get_virtual_component_type_hash() const {
		return TypeIDGenerator::get_id_for_string(name);
	}
	ScriptVisibleComponent() = default;
	ScriptVisibleComponent(const ScriptVisibleComponent& rhs) {
		name = rhs.name;
		fields = rhs.fields;
	}
	ScriptVisibleComponent operator =(const ScriptVisibleComponent &rhs) {
		name = rhs.name;
		fields = rhs.fields;
		return *this;
	}
};

//making an exception for templating here
class ScriptVisibleComponentRef : public FlecsComponent<ScriptVisibleComponent> {
	GDCLASS(ScriptVisibleComponentRef, FlecsComponent<ScriptVisibleComponent>);
public:
	void set_data(ScriptVisibleComponent &p_data) override;
	void clear_component() override;
	StringName get_type_name() const override;


private:
	void append_bytes(PackedByteArray &array, const void *data, size_t size) const;
	public:
	ScriptVisibleComponentRef() = default;
	~ScriptVisibleComponentRef() override = default;
	Variant get_field_value(const StringName& field_name) const;
	void set_field(const StringName& field_name, const Variant& value) const;
	static void _bind_methods();
	bool is_dynamic() const override;
	static Ref<ScriptVisibleComponentRef> create_component(const StringName& name, const Ref<FlecsEntity> &p_owner);
	Ref<FlecsComponentBase> clone() const override;
	uint64_t get_virtual_component_type_hash() const;
};


// template <>
// inline void FlecsComponent<ScriptVisibleComponent>::byte_deserialize(const PackedByteArray &p_ba) {
// 	ERR_PRINT("Not implemented");
// }

using ScriptVisibleComponentModule = SingleComponentModule<ScriptVisibleComponent>;

#endif //SCRIPT_VISIBLE_COMPONENT_H
