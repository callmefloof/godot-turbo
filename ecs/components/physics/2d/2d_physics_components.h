#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

struct Area2DComponent : CompBase {
	RID area_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("area_id", area_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		area_id = dict["area_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Area2DComponent>()) {
			const Area2DComponent &area_component = entity.get<Area2DComponent>();
			dict.set("area_id", area_component.area_id);
		} else {
			ERR_PRINT("Area2DComponent::to_dict: entity does not have Area2DComponent");
			dict.set("area_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Area2DComponent>()) {
			Area2DComponent &area_component = entity.get_mut<Area2DComponent>();
			area_component.area_id = dict["area_id"];
		} else {
			ERR_PRINT("Area2DComponent::from_dict: entity does not have Area2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Area2DComponent";
	}
};

REGISTER_COMPONENT(Area2DComponent);

struct Body2DComponent : CompBase {
	RID body_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("body_id", body_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		body_id = dict["body_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Body2DComponent>()) {
			const Body2DComponent &body_component = entity.get<Body2DComponent>();
			dict.set("body_id", body_component.body_id);
		} else {
			ERR_PRINT("Body2DComponent::to_dict: entity does not have Body2DComponent");
			dict.set("body_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Body2DComponent>()) {
			Body2DComponent &body_component = entity.get_mut<Body2DComponent>();
			body_component.body_id = dict["body_id"];
		} else {
			ERR_PRINT("Body2DComponent::from_dict: entity does not have Body2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Body2DComponent";
	}
};
REGISTER_COMPONENT(Body2DComponent);

struct Joint2DComponent : CompBase {
	RID joint_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("joint_id", joint_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		joint_id = dict["joint_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Joint2DComponent>()) {
			const Joint2DComponent &joint_component = entity.get<Joint2DComponent>();
			dict.set("joint_id", joint_component.joint_id);
		} else {
			ERR_PRINT("Joint2DComponent::to_dict: entity does not have Joint2DComponent");
			dict.set("joint_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Joint2DComponent>()) {
			Joint2DComponent &joint_component = entity.get_mut<Joint2DComponent>();
			joint_component.joint_id = dict["joint_id"];
		} else {
			ERR_PRINT("Joint2DComponent::from_dict: entity does not have Joint2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Joint2DComponent";
	}
};

REGISTER_COMPONENT(Joint2DComponent);

struct Physics2DBaseComponents {
	flecs::component<Area2DComponent> area;
	flecs::component<Body2DComponent> body;
	flecs::component<Joint2DComponent> joint;

	explicit Physics2DBaseComponents(flecs::world &world) :
			area(world.component<Area2DComponent>("Area2DComponent")),
			body(world.component<Body2DComponent>("Body2DComponent")),
			joint(world.component<Joint2DComponent>("Joint2DComponent")){
				ComponentRegistry::bind_to_world("Area2DComponent", area.id());
				ComponentRegistry::bind_to_world("Body2DComponent", body.id());
				ComponentRegistry::bind_to_world("Joint2DComponent", joint.id());
			}
};
