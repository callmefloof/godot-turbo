#pragma once
#include "core/object/object_id.h"
#include "core/variant/variant.h"
#include "ecs/components/component_registry.h"

#include "core/variant/dictionary.h"
#include <cstdint>


struct SceneNodeComponent : CompBase {
	ObjectID node_id; // Unique identifier for the node
	StringName class_name; // Class name of the node

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("node_id", node_id);
		dict.set("class_name", class_name);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		node_id = ObjectID(dict["node_id"].operator int64_t());
		class_name = dict["class_name"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<SceneNodeComponent>()) {
			const SceneNodeComponent &node = entity.get<SceneNodeComponent>();
			dict.set("node_id", node.node_id);
			dict.set("class_name", node.class_name);
		} else {
			ERR_PRINT("SceneNodeComponent::to_dict: entity does not have SceneNodeComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SceneNodeComponent>()) {
			SceneNodeComponent &node = entity.get_mut<SceneNodeComponent>();
			node.node_id = ObjectID(dict["node_id"].operator int64_t());
			node.class_name = dict["class_name"];
		} else {
			ERR_PRINT("SceneNodeComponent::from_dict: entity does not have SceneNodeComponent");
		}
	}

	StringName get_type_name() const override {
		return "SceneNodeComponent";
	}
};
REGISTER_COMPONENT(SceneNodeComponent);
