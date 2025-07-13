#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "servers/physics_server_2d.h"
#include "core/templates/rid.h"

struct Area2DComponent {
	RID area_id;
	~Area2DComponent() {
		if (area_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(area_id);
		}
	}
};
struct Body2DComponent {
	RID body_id;
	~Body2DComponent() {
		if (body_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(body_id);
		}
	}
};
struct Joint2DComponent {
	RID joint_id;
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
