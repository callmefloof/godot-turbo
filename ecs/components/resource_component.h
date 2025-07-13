#pragma once
#include "single_component_module.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"

template <typename T = Resource>
struct ResourceComponent {
	RID resource_id; // Unique identifier for the resource
	String resource_type; // Type of the resource (e.g., "Mesh", "Texture", etc.)
	String name; // Name of the resource
	Ref<T> resource_ref; // Reference to the actual resource object
	ScriptInstance *script_instance; // Script associated with the resource, if any
	Ref<Script> script_ref; // Reference to the script resource, if applicable
	bool is_script_type; // Flag to indicate if the resource is a GDScript type
};
template <typename T = Resource>
using ResourceComponentModule = SingleComponentModule<ResourceComponent<T>>; // Specialization for ResourceComponent

