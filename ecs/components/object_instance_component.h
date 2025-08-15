#pragma once
#include "core/object/ref_counted.h"
#include "single_component_module.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/flecs_types/flecs_world.h"
#include "component_proxy.h"

struct ObjectInstanceComponent {
    ObjectID instance_id; // Unique identifier for the object instance

    // Default constructor initializes instance_id to an invalid state
    ObjectInstanceComponent() = default;
    ~ObjectInstanceComponent() = default;

    // Constructor to initialize with a specific instance ID
    explicit ObjectInstanceComponent(ObjectID p_instance_id) : instance_id(p_instance_id) {}
};


class ObjectInstanceComponentRef : public FlecsComponent<ObjectInstanceComponent> {
    #define OBJECT_INSTANCE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(ObjectID, instance_id,ObjectInstanceComponent)\

	#define OBJECT_INSTANCE_COMPONENT_BINDINGS\
	BIND_PROPERTY(ObjectID, instance_id, ObjectInstanceComponentRef)\

	DEFINE_COMPONENT_PROXY(ObjectInstanceComponent,OBJECT_INSTANCE_COMPONENT_PROPERTIES,OBJECT_INSTANCE_COMPONENT_BINDINGS);
};

using ObjectInstanceComponentModule = SingleComponentModule<ObjectInstanceComponent>;