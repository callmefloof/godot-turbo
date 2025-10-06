//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDCOMPONENTS_H
#define WORLDCOMPONENTS_H
#include "core/templates/rid.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

#include "core/variant/dictionary.h"

struct World2DComponent : CompBase {
	RID canvas_id;
	RID navigation_map_id;
	RID space_id;
	bool is_valid() const { return canvas_id.is_valid() && navigation_map_id.is_valid() && space_id.is_valid(); }
	bool is_null() const { return canvas_id.is_null() && navigation_map_id.is_null() && space_id.is_null(); }
	World2DComponent() = default;
	~World2DComponent() = default;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict["canvas_id"] = canvas_id;
		dict["navigation_map_id"] = navigation_map_id;
		dict["space_id"] = space_id;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		canvas_id = dict["canvas_id"];
		navigation_map_id = dict["navigation_map_id"];
		space_id = dict["space_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<World2DComponent>()) {
			const World2DComponent &world_component = entity.get<World2DComponent>();
			dict["canvas_id"] = world_component.canvas_id;
			dict["navigation_map_id"] = world_component.navigation_map_id;
			dict["space_id"] = world_component.space_id;
		} else {
			ERR_PRINT("World2DComponent::to_dict: entity does not have World2DComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<World2DComponent>()) {
			World2DComponent &world_component = entity.get_mut<World2DComponent>();
			world_component.canvas_id = dict["canvas_id"];
			world_component.navigation_map_id = dict["navigation_map_id"];
			world_component.space_id = dict["space_id"];
		} else {
			ERR_PRINT("World2DComponent::from_dict: entity does not have World2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "World2DComponent";
	}
};
REGISTER_COMPONENT(World2DComponent);

struct World3DComponent : CompBase {
	RID camera_attributes_id;
	RID environment_id;
	RID fallback_environment_id;
	RID navigation_map_id;
	RID scenario_id;
	RID space_id;
	bool is_valid() const {
		return camera_attributes_id.is_valid() &&
		environment_id.is_valid() &&
		fallback_environment_id.is_valid() &&
		navigation_map_id.is_valid() &&
		scenario_id.is_valid() &&
		space_id.is_valid();
	}
	bool is_null() const {
		return camera_attributes_id.is_null() &&
		environment_id.is_null() &&
		fallback_environment_id.is_null() &&
		navigation_map_id.is_null() &&
		scenario_id.is_null() &&
		space_id.is_null();
	}
	World3DComponent()= default;
	~World3DComponent() = default;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict["camera_attributes_id"] = camera_attributes_id;
		dict["environment_id"] = environment_id;
		dict["fallback_environment_id"] = fallback_environment_id;
		dict["navigation_map_id"] = navigation_map_id;
		dict["scenario_id"] = scenario_id;
		dict["space_id"] = space_id;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		camera_attributes_id = dict["camera_attributes_id"];
		environment_id = dict["environment_id"];
		fallback_environment_id = dict["fallback_environment_id"];
		navigation_map_id = dict["navigation_map_id"];
		scenario_id = dict["scenario_id"];
		space_id = dict["space_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<World3DComponent>()) {
			const World3DComponent &world_component = entity.get<World3DComponent>();
			dict["camera_attributes_id"] = world_component.camera_attributes_id;
			dict["environment_id"] = world_component.environment_id;
			dict["fallback_environment_id"] = world_component.fallback_environment_id;
			dict["navigation_map_id"] = world_component.navigation_map_id;
			dict["scenario_id"] = world_component.scenario_id;
			dict["space_id"] = world_component.space_id;
		} else {
			ERR_PRINT("World3DComponent::to_dict: entity does not have World3DComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<World3DComponent>()) {
			World3DComponent &world_component = entity.get_mut<World3DComponent>();
			world_component.camera_attributes_id = dict["camera_attributes_id"];
			world_component.environment_id = dict["environment_id"];
			world_component.fallback_environment_id = dict["fallback_environment_id"];
			world_component.navigation_map_id = dict["navigation_map_id"];
			world_component.scenario_id = dict["scenario_id"];
			world_component.space_id = dict["space_id"];
		} else {
			ERR_PRINT("World3DComponent::from_dict: entity does not have World3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "World3DComponent";
	}
};
REGISTER_COMPONENT(World3DComponent);

#endif //WORLDCOMPONENTS_H
