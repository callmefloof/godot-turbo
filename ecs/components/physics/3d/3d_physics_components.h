#pragma once
#include "core/templates/rid.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "core/variant/dictionary.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

struct Area3DComponent : CompBase {
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
		if (entity.has<Area3DComponent>()) {
			const Area3DComponent &area_component = entity.get<Area3DComponent>();
			dict.set("area_id", area_component.area_id);
		} else {
			ERR_PRINT("Area3DComponent::to_dict: entity does not have Area3DComponent");
			dict.set("area_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Area3DComponent>()) {
			Area3DComponent &area_component = entity.get_mut<Area3DComponent>();
			area_component.area_id = dict["area_id"];
		} else {
			ERR_PRINT("Area3DComponent::from_dict: entity does not have Area3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Area3DComponent";
	}
};
REGISTER_COMPONENT(Area3DComponent);

struct Body3DComponent : CompBase {
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
        if (entity.has<Body3DComponent>()) {
            const Body3DComponent &body_component = entity.get<Body3DComponent>();
            dict.set("body_id", body_component.body_id);
        } else {
            ERR_PRINT("Body3DComponent::to_dict: entity does not have Body3DComponent");
            dict.set("body_id", RID());
        }
        return dict;
    }

    void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
        if (entity.has<Body3DComponent>()) {
            Body3DComponent &body_component = entity.get_mut<Body3DComponent>();
            body_component.body_id = dict["body_id"];
        } else {
            ERR_PRINT("Body3DComponent::from_dict: entity does not have Body3DComponent");
        }
    }

    StringName get_type_name() const override {
        return "Body3DComponent";
    }
};
REGISTER_COMPONENT(Body3DComponent);

struct Joint3DComponent : CompBase {
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
		if (entity.has<Joint3DComponent>()) {
			const Joint3DComponent &joint_component = entity.get<Joint3DComponent>();
			dict.set("joint_id", joint_component.joint_id);
		} else {
			ERR_PRINT("Joint3DComponent::to_dict: entity does not have Joint3DComponent");
			dict.set("joint_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Joint3DComponent>()) {
			Joint3DComponent &joint_component = entity.get_mut<Joint3DComponent>();
			joint_component.joint_id = dict["joint_id"];
		} else {
			ERR_PRINT("Joint3DComponent::from_dict: entity does not have Joint3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "Joint3DComponent";
	}
};
REGISTER_COMPONENT(Joint3DComponent);

struct SoftBody3DComponent : CompBase {
	RID soft_body_id;
	SoftBody3DComponent() = default;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("soft_body_id", soft_body_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		soft_body_id = dict["soft_body_id"];
	}

	StringName get_type_name() const override {
		return "SoftBody3DComponent";
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<SoftBody3DComponent>()) {
			const SoftBody3DComponent &soft_body_component = entity.get<SoftBody3DComponent>();
			dict.set("soft_body_id", soft_body_component.soft_body_id);
		} else {
			ERR_PRINT("SoftBody3DComponent::to_dict: entity does not have SoftBody3DComponent");
			dict.set("soft_body_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SoftBody3DComponent>()) {
			SoftBody3DComponent &soft_body_component = entity.get_mut<SoftBody3DComponent>();
			soft_body_component.soft_body_id = dict["soft_body_id"];
		} else {
			ERR_PRINT("SoftBody3DComponent::from_dict: entity does not have SoftBody3DComponent");
		}
	}

};
REGISTER_COMPONENT(SoftBody3DComponent);

struct Physics3DBaseComponents {
	flecs::component<Area3DComponent> area;
	flecs::component<Body3DComponent> body;
	flecs::component<Joint3DComponent> joint;
	flecs::component<SoftBody3DComponent> soft_body;

	explicit Physics3DBaseComponents(flecs::world &world) :
			area(world.component<Area3DComponent>("Area3DComponent")),
			body(world.component<Body3DComponent>("Body3DComponent")),
			joint(world.component<Joint3DComponent>("Joint3DComponent")),
			soft_body(world.component<SoftBody3DComponent>("SoftBody3DComponent")) {
				ComponentRegistry::bind_to_world("Area3DComponent", area.id());
				ComponentRegistry::bind_to_world("Body3DComponent", body.id());
				ComponentRegistry::bind_to_world("Joint3DComponent", joint.id());
				ComponentRegistry::bind_to_world("SoftBody3DComponent", soft_body.id());
			}
};
