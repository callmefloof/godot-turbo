//
// Created by Floof on 28-7-2025.
//

#ifndef VISIBILITY_COMPONENT_H
#define VISIBILITY_COMPONENT_H
#include "ecs/components/component_registry.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

struct VisibilityComponent : CompBase {
	bool visible = true;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict["visible"] = visible;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		visible = dict["visible"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<VisibilityComponent>()) {
			const VisibilityComponent &visibility_component = entity.get<VisibilityComponent>();
			dict.set("visible", visibility_component.visible);
		} else {
			ERR_PRINT("VisibilityComponent::to_dict: entity does not have VisibilityComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<VisibilityComponent>()) {
			VisibilityComponent &visibility_component = entity.get_mut<VisibilityComponent>();
			visibility_component.visible = dict["visible"];
		} else {
			ERR_PRINT("VisibilityComponent::from_dict: entity does not have VisibilityComponent");
		}
	}

	StringName get_type_name() const override {
		return "VisibilityComponent";
	}
};
REGISTER_COMPONENT(VisibilityComponent);

#endif //VISIBILITY_COMPONENT_H
