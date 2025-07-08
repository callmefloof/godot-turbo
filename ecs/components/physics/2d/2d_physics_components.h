#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "servers/physics_server_2d.h"
#include "core/templates/rid.h"

namespace godot_turbo::components::physics {

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
	struct Space2DComponent {
		RID space_id;
		~Space2DComponent() {
			if (space_id.is_valid()) {
				PhysicsServer2D::get_singleton()->free(space_id);
			}
		}
	};

	struct Physics2DBaseComponents {
		flecs::component<Area2DComponent> area;
		flecs::component<Body2DComponent> body;
		flecs::component<Joint2DComponent> joint;
		flecs::component<Space2DComponent> space;

		Physics2DBaseComponents(flecs::world &world) :
				area(world.component<Area2DComponent>("Area2DComponent")),
				body(world.component<Body2DComponent>("Body2DComponent")),
				joint(world.component<Joint2DComponent>("Joint2DComponent")),
				space(world.component<Space2DComponent>("Space2DComponent")) {}
	};

	class Physics2DComponents
			: public MultiComponentModule<Physics2DComponents, Physics2DBaseComponents> {
	};

} // namespace godot_turbo::components::physics
