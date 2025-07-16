#pragma once
#include "../flecs_types/flecs_component.h"
#include "component_proxy.h"
#include "../../../../core/object/object_id.h"
#include "../../../../core/os/memory.h"
#include "node_ref.h"
#include "../../../../scene/main/node.h"
#include "single_component_module.h"

struct SceneNodeComponent {
	ObjectID node_id; // Unique identifier for the node
	StringName class_name; // Class name of the node
	Ref<NodeRef> node_ref; // Reference to the node
};

#define SCENE_NODE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(ObjectID, node_id,SceneNodeComponent)\
DEFINE_PROPERTY(StringName, class_name,SceneNodeComponent)\
DEFINE_PROPERTY(Ref<NodeRef>, node_ref,SceneNodeComponent)\

#define SCENE_NODE_COMPONENT_BINDINGS\
BIND_PROPERTY(ObjectID, node_id, SceneNodeComponentRef)\
BIND_PROPERTY(StringName, class_name, SceneNodeComponentRef)\
BIND_PROPERTY(Ref<NodeRef>, node_ref, SceneNodeComponentRef)\

DEFINE_COMPONENT_PROXY(SceneNodeComponentRef, SceneNodeComponent,
SCENE_NODE_COMPONENT_PROPERTIES,
SCENE_NODE_COMPONENT_BINDINGS);

template<typename T = Node>
using SceneNodeComponentModule = SingleComponentModule<SceneNodeComponent>;
