#pragma once
#include "../flecs_types/flecs_component.h"
#include "component_proxy.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/templates/rid.h"
#include "single_component_module.h"

struct ResourceComponent {
	RID resource_id; // Unique identifier for the resource
	StringName resource_type; // Type of the resource (e.g., "Mesh", "Texture", etc.)
	StringName name; // Name of the resource
	bool is_script_type; // Flag to indicate if the resource is a GDScript type
};

#define RESOURCE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, resource_id,ResourceComponent)\
DEFINE_PROPERTY(StringName, resource_type,ResourceComponent)\
DEFINE_PROPERTY(StringName, name,ResourceComponent)\
DEFINE_PROPERTY(bool, is_script_type,ResourceComponent)\

#define RESOURCE_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, resource_id, ResourceComponentRef)\
BIND_PROPERTY(StringName, resource_type, ResourceComponentRef)\
BIND_PROPERTY(StringName, name, ResourceComponentRef)\
BIND_PROPERTY(bool, is_script_type, ResourceComponentRef)\

class ResourceComponentRef : public FlecsComponent<ResourceComponent> {
GDCLASS(ResourceComponentRef, FlecsComponent<ResourceComponent>)
public : RID get_resource_id() const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			return typed->resource_id;
		}
		return default_value<RID>();
	}
	void set_resource_id(RID value) const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			typed->resource_id = value;
		}
	}
	StringName get_resource_type() const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			return typed->resource_type;
		}
		return default_value<StringName>();
	}
	void set_resource_type(StringName value) const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			typed->resource_type = value;
		}
	}
	StringName get_name() const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			return typed->name;
		}
		return default_value<StringName>();
	}
	void set_name(StringName value) const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			typed->name = value;
		}
	}
	bool get_is_script_type() const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			return typed->is_script_type;
		}
		return default_value<bool>();
	}
	void set_is_script_type(bool value) const {
		auto typed = get_typed_data<ResourceComponent>();
		if (typed) {
			typed->is_script_type = value;
		}
	}
	static Ref<ResourceComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<ResourceComponentRef> class_ref = Ref<ResourceComponentRef>(memnew(ResourceComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<ResourceComponent>({});
		class_ref->set_data(&entity->get_mut<ResourceComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<ResourceComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<ResourceComponent>();
			ResourceComponent *copied = memnew(ResourceComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(ResourceComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "ResourceComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "resource_id",
					&ResourceComponentRef::get_resource_id);
			ClassDB::bind_method("set_"
								 "resource_id",
					&ResourceComponentRef::set_resource_id);
			::ClassDB::add_property(ResourceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "resource_id"), _scs_create("set_"
																																				   "resource_id"),
					_scs_create("get_"
								"resource_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "resource_type",
					&ResourceComponentRef::get_resource_type);
			ClassDB::bind_method("set_"
								 "resource_type",
					&ResourceComponentRef::set_resource_type);
			::ClassDB::add_property(ResourceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<StringName>::value, "resource_type"), _scs_create("set_"
																																							"resource_type"),
					_scs_create("get_"
								"resource_type"));
		}
		while (0) {
			do {
				ClassDB::bind_method("get_"
									 "name",
						&ResourceComponentRef::get_name);
				ClassDB::bind_method("set_"
									 "name",
						&ResourceComponentRef::set_name);
				::ClassDB::add_property(ResourceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<StringName>::value, "name"), _scs_create("set_"
																																					   "name"),
						_scs_create("get_"
									"name"));
			} while (0) do {
				ClassDB::bind_method("get_"
									 "is_script_type",
						&ResourceComponentRef::get_is_script_type);
				ClassDB::bind_method("set_"
									 "is_script_type",
						&ResourceComponentRef::set_is_script_type);
				::ClassDB::add_property(ResourceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<bool>::value, "is_script_type"), _scs_create("set_"
																																						   "is_script_type"),
						_scs_create("get_"
									"is_script_type"));
			}
		}
		while (0)
			;
		ClassDB::bind_static_method(ResourceComponentRef::get_class_static(), "create_component", &ResourceComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &ResourceComponentRef::get_type_name);
	}
};
;

using ResourceComponentModule = SingleComponentModule<ResourceComponent>; // Specialization for ResourceComponent

