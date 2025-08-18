#pragma once
#include "core/math/transform_3d.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"
#include "core/variant/dictionary.h"


struct Transform3DComponent : CompBase {
	Transform3D transform;

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
		if (entity.has<Transform3DComponent>()) {
			const Transform3DComponent &transform_component = entity.get<Transform3DComponent>();
			dict.set("transform", transform_component.transform);
		} else {
			ERR_PRINT("Transform3DComponent::to_dict: entity does not have Transform3DComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Transform3DComponent>()) {
			Transform3DComponent &transform_component = entity.get_mut<Transform3DComponent>();
			transform_component.transform = dict["transform"];
		} else {
			ERR_PRINT("Transform3DComponent::from_dict: entity does not have Transform3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Transform3DComponent";
	}
};
REGISTER_COMPONENT(Transform3DComponent);