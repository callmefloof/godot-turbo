#pragma once
#include "ecs/components/component_registry.h"
#include "core/variant/dictionary.h"
#include "core/templates/rid.h"
#include "core/variant/variant.h"

struct ResourceComponent : CompBase {
	RID resource_id; // Unique identifier for the resource
	StringName resource_type; // Type of the resource (e.g., "Mesh", "Texture", etc.)
	StringName resource_name; // Name of the resource
	bool is_script_type = false; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("resource_id", resource_id);
		dict.set("resource_type", resource_type);
		dict.set("resource_name", resource_name);
		dict.set("is_script_type", is_script_type);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		resource_id = dict["resource_id"];
		resource_type = dict["resource_type"];
		resource_name = dict["resource_name"];
		is_script_type = dict["is_script_type"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ResourceComponent>()) {
			const ResourceComponent &resource = entity.get<ResourceComponent>();
			dict.set("resource_id", resource.resource_id);
			dict.set("resource_type", resource.resource_type);
			dict.set("resource_name", resource.resource_name);
			dict.set("is_script_type", resource.is_script_type);
		} else {
			ERR_PRINT("ResourceComponent::to_dict: entity does not have ResourceComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ResourceComponent>()) {
			ResourceComponent &resource = entity.get_mut<ResourceComponent>();
			resource.resource_id = dict["resource_id"];
			resource.resource_type = dict["resource_type"];
			resource.resource_name = dict["resource_name"];
			resource.is_script_type = dict["is_script_type"];
		} else {
			ERR_PRINT("ResourceComponent::from_dict: entity does not have ResourceComponent");
		}
	}

	StringName get_type_name() const override {
		return "ResourceComponent";
	}
};
REGISTER_COMPONENT(ResourceComponent);

