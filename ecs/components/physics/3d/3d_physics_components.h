#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "servers/physics_server_3d.h"


	struct Area3DComponent : ScriptVisibleComponent {
		RID area_id;
		Dictionary to_dict() const override{
			Dictionary dict;
			dict["area_id"] = area_id;
			return dict;
		}
		void from_dict(Dictionary dict) override {
			SET_SCRIPT_COMPONENT_VALUE(dict, "area_id", area_id, Variant::RID);
		}
		~Area3DComponent() {
			if (area_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(area_id);
			}
		}
	};
	struct Body3DComponent : ScriptVisibleComponent {
		RID body_id;
		Dictionary to_dict() const override{
			Dictionary dict;
			dict["body_id"] = body_id;
			return dict;
		}
		void from_dict(Dictionary dict) override {
			SET_SCRIPT_COMPONENT_VALUE(dict, "body_id", body_id, Variant::RID);
		}
		~Body3DComponent() {
			if (body_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(body_id);
			}
		}
	};
	struct Joint3DComponent : ScriptVisibleComponent{
		RID joint_id;
		Dictionary to_dict() const override{
			Dictionary dict;
			dict["joint_id"] = joint_id;
		}
		void from_dict(Dictionary dict) override {
			SET_SCRIPT_COMPONENT_VALUE(dict, "joint_id", joint_id, Variant::RID);
		}
		~Joint3DComponent() {
			if (joint_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(joint_id);
			}
		}
	};
	struct SoftBody3DComponent : ScriptVisibleComponent {
		RID soft_body_id;
		Dictionary to_dict() const override{
			Dictionary dict;
			dict["soft_body_id"] = soft_body_id;
			return dict;
		}
		void from_dict(Dictionary dict) override {
			SET_SCRIPT_COMPONENT_VALUE(dict, "soft_body_id", soft_body_id, Variant::RID);
		}
		~SoftBody3DComponent() {
			if (soft_body_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(soft_body_id);
			}
		}
	};

	struct Physics3DBaseComponents {
		flecs::component<Area3DComponent> area;
		flecs::component<Body3DComponent> body;
		flecs::component<Joint3DComponent> joint;
		flecs::component<SoftBody3DComponent> soft_body;

		explicit Physics3DBaseComponents(const flecs::world &world) :
				area(world.component<Area3DComponent>("Area3DComponent")),
				body(world.component<Body3DComponent>("Body3DComponent")),
				joint(world.component<Joint3DComponent>("Joint3DComponent")),
				soft_body(world.component<SoftBody3DComponent>("SoftBody3DComponent")) {}
	};
using Physics3DComponentModule = MultiComponentModule<Physics3DBaseComponents>;

