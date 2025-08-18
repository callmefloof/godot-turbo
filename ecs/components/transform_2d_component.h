#pragma once
#include "core/math/transform_2d.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"
#include "core/variant/dictionary.h"

struct Transform2DComponent : CompBase {
	Transform2D transform;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("transform", transform);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		transform = dict["transform"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Transform2DComponent>()) {
			const Transform2DComponent &transform_component = entity.get<Transform2DComponent>();
			dict.set("transform", transform_component.transform);
		} else {
			ERR_PRINT("Transform2DComponent::to_dict: entity does not have Transform2DComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Transform2DComponent>()) {
			Transform2DComponent &transform_component = entity.get_mut<Transform2DComponent>();
			transform_component.transform = dict["transform"];
		} else {
			ERR_PRINT("Transform2DComponent::from_dict: entity does not have Transform2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Transform2DComponent";
	}
};
REGISTER_COMPONENT(Transform2DComponent);

