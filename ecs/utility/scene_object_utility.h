#pragma once

#include "core/object/object.h"
#include "core/templates/vector.h"
#include "ecs/flecs_types/flecs_world.h"

#include "core/variant/typed_array.h"
#include <cassert>


class SceneObjectUtility : public Object {
	GDCLASS(SceneObjectUtility, Object)
	
public:
	SceneObjectUtility() = default;
	~SceneObjectUtility() = default;
	static Array  create_entities_from_scene(FlecsWorld* world, const SceneTree* tree );
	static Array  create_entities(FlecsWorld* world, const Node *base_node, Array &entities,
			int current_depth = 0, const int max_depth = 10000);
	static Array create_entity(FlecsWorld* world, Node* node);
	static Ref<FlecsEntity> get_script(FlecsWorld* world, const Node* node, const FlecsEntity* node_entity, Array &entities);
	static void _bind_methods();
};

