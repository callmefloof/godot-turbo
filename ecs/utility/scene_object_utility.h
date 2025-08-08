#pragma once

#include "core/object/object.h"
#include "core/templates/vector.h"
#include "ecs/flecs_types/flecs_world.h"

#include "core/variant/typed_array.h"
#include <cassert>


class SceneObjectUtility : public Object {
	GDCLASS(SceneObjectUtility, Object)
private:
	static inline SceneObjectUtility* instance = nullptr;
public:
	SceneObjectUtility() = default;
	~SceneObjectUtility() = default;
	TypedArray<FlecsEntity> create_entities_from_scene(FlecsWorld* world, SceneTree* tree );
	TypedArray<FlecsEntity> create_entities(FlecsWorld* world, const Node *base_node, const TypedArray<FlecsEntity> &entities,
			int current_depth = 0, const int max_depth = 10000);
	TypedArray<FlecsEntity> create_entity(FlecsWorld* world, Node* node);
	Ref<FlecsEntity> get_script(FlecsWorld *world, const Node *node, const FlecsEntity *node_entity);
	static void _bind_methods();
	static SceneObjectUtility* get_singleton();
};

