#pragma once
#include "../flecs_types/flecs_component.h"
#include "component_proxy.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "single_component_module.h"
#include "ecs/flecs_types/flecs_world.h"

struct ResourceComponent {
	RID resource_id; // Unique identifier for the resource
	StringName resource_type; // Type of the resource (e.g., "Mesh", "Texture", etc.)
	StringName resource_name; // Name of the resource
	bool is_script_type; // Flag to indicate if the resource is a GDScript type
};

class ResourceComponentRef : public FlecsComponent<ResourceComponent> {
	#define RESOURCE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, resource_id,ResourceComponent)\
	DEFINE_PROPERTY(StringName, resource_type,ResourceComponent)\
	DEFINE_PROPERTY(StringName, resource_name,ResourceComponent)\
	DEFINE_PROPERTY(bool, is_script_type,ResourceComponent)\

	#define RESOURCE_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, resource_id, ResourceComponentRef)\
	BIND_PROPERTY(StringName, resource_type, ResourceComponentRef)\
	BIND_PROPERTY(StringName, resource_name, ResourceComponentRef)\
	BIND_PROPERTY(bool, is_script_type, ResourceComponentRef)\

	DEFINE_COMPONENT_PROXY(ResourceComponent, RESOURCE_COMPONENT_PROPERTIES, RESOURCE_COMPONENT_BINDINGS);
};

using ResourceComponentModule = SingleComponentModule<ResourceComponent>; // Specialization for ResourceComponent

