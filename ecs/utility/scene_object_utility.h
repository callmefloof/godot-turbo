#pragma once

#include "core/object/object.h"
#include "scene/main/scene_tree.h"
#include "core/variant/typed_array.h"
#include <cassert>


class SceneObjectUtility : public Object {
	GDCLASS(SceneObjectUtility, Object)
private:
	static inline SceneObjectUtility* instance = nullptr;
public:
	SceneObjectUtility() = default;
	~SceneObjectUtility() = default;
	TypedArray<RID> create_entities_from_scene(const RID &world_id, SceneTree* tree );
	TypedArray<RID> create_entities(const RID &world_id, const Node *base_node, const TypedArray<RID> &entities,
			int current_depth = 0, const int max_depth = 10000);
	TypedArray<RID> create_entity(const RID &world_id, Node* node);
	RID get_node_script(const RID &world_id, const Node *node, const RID &node_entity);
	static void _bind_methods();
	static SceneObjectUtility* get_singleton();
};

