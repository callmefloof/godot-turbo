#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "servers/physics_server_3d.h"

namespace godot_turbo::components::physics {

	struct Area3DComponent {
		RID area_id;
		~Area3DComponent() {
			if (area_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(area_id);
			}
		}
	};
	struct Body3DComponent {
		RID body_id;
		~Body3DComponent() {
			if (body_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(body_id);
			}
		}
	};
	struct Joint3DComponent {
		RID joint_id;
		~Joint3DComponent() {
			if (joint_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(joint_id);
			}
		}
	};
	struct SoftBody3DComponent {
		RID soft_body_id;
		~SoftBody3DComponent() {
			if (soft_body_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(soft_body_id);
			}
		}
	};
	struct Space3DComponent {
		RID space_id;
		~Space3DComponent() {
			if (space_id.is_valid()) {
				PhysicsServer3D::get_singleton()->free(space_id);
			}
		}
	};

	struct Physics3DBaseComponents {
		flecs::component<Area3DComponent> area;
		flecs::component<Body3DComponent> body;
		flecs::component<Joint3DComponent> joint;
		flecs::component<SoftBody3DComponent> soft_body;
		flecs::component<Space3DComponent> space;

		Physics3DBaseComponents(flecs::world &world) :
				area(world.component<Area3DComponent>("Area3DComponent")),
				body(world.component<Body3DComponent>("Body3DComponent")),
				joint(world.component<Joint3DComponent>("Joint3DComponent")),
				soft_body(world.component<SoftBody3DComponent>("SoftBody3DComponent")),
				space(world.component<Space3DComponent>("Space3DComponent")) {}
	};

	class Physics3DComponents
			: public MultiComponentModule<Physics3DComponents, Physics3DBaseComponents> {
		// Nothing else needed
	};

} // namespace godot_turbo::components::physics
