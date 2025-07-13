#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "servers/physics_server_2d.h"

struct Area2DComponent : ScriptVisibleComponent {
	RID area_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["area_id"] = area_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "area_id", area_id, Variant::RID);
	}
	~Area2DComponent() {
		if (area_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(area_id);
		}
	}
};
struct Body2DComponent : ScriptVisibleComponent{
	RID body_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["body_id"] = body_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "body_id", body_id, Variant::RID);

	}
	~Body2DComponent() {
		if (body_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(body_id);
		}
	}
};
struct Joint2DComponent : ScriptVisibleComponent {
	RID joint_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["joint_id"] = joint_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "joint_id", joint_id, Variant::RID);
	}
	~Joint2DComponent() {
		if (joint_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(joint_id);
		}
	}
};

struct Physics2DBaseComponents {
	flecs::component<Area2DComponent> area;
	flecs::component<Body2DComponent> body;
	flecs::component<Joint2DComponent> joint;

	explicit Physics2DBaseComponents(const flecs::world &world) :
			area(world.component<Area2DComponent>("Area2DComponent")),
			body(world.component<Body2DComponent>("Body2DComponent")),
			joint(world.component<Joint2DComponent>("Joint2DComponent")){}
};

using Physics2DComponentModule =  MultiComponentModule<Physics2DBaseComponents>;
