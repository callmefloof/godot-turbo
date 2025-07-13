#pragma once
#include "single_component_module.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"

struct ResourceComponent : ScriptVisibleComponent {
	RID resource_id; // Unique identifier for the resource
	String resource_type; // Type of the resource (e.g., "Mesh", "Texture", etc.)
	String name; // Name of the resource
	Ref<Resource> resource_ref; // Reference to the actual resource object
	ScriptInstance *script_instance; // Script associated with the resource, if any
	Ref<Script> script_ref; // Reference to the script resource, if applicable
	bool is_script_type; // Flag to indicate if the resource is a GDScript type
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["resource_id"] = resource_id;
		dict["resource_type"] = resource_type;
		dict["name"] = name;
		dict["resource_ref"] = resource_ref;
		dict["script_instance"] = script_instance;
		dict["script_ref"] = script_ref;
		dict["is_script_type"] = is_script_type;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "resource_id", resource_id, Variant::RID);
		SET_SCRIPT_COMPONENT_VALUE(dict, "resource_type", resource_type, Variant::STRING);
		SET_SCRIPT_COMPONENT_VALUE(dict, "name", name, Variant::STRING);
		SET_SCRIPT_COMPONENT_VALUE(dict, "resource_ref", resource_ref, Variant::OBJECT);
		SET_SCRIPT_COMPONENT_VALUE(dict, "script_ref", script_ref, Variant::OBJECT);
		SET_SCRIPT_COMPONENT_VALUE(dict, "is_script_type", is_script_type, Variant::BOOL);
	}
};
using ResourceComponentModule = SingleComponentModule<ResourceComponent>; // Specialization for ResourceComponent

