#pragma once
#pragma once
#include "../../object_id_storage.h"
#include <core/string/ustring.h>
#include <core/templates/rid.h>
#include <modules/godot_turbo/ecs/components/physics/2d/2d_physics_components.h>
#include <modules/godot_turbo/thirdparty/flecs/distr/flecs.h>
#include <scene/2d/physics/area_2d.h>
#include <scene/2d/physics/joints/joint_2d.h>
#include <scene/2d/physics/rigid_body_2d.h>
#include <servers/physics_server_2d.h>

namespace godot_turbo::utility::physics {
using namespace godot_turbo::components::physics;


class Physics2DUtility {
private:
	Physics2DUtility() = default; // Prevent instantiation
	Physics2DUtility(const Physics2DUtility &) = delete; // Prevent copy
	Physics2DUtility &operator=(const Physics2DUtility &) = delete; // Prevent assignment
	Physics2DUtility(Physics2DUtility &&) = delete; // Prevent move
	Physics2DUtility &operator=(Physics2DUtility &&) = delete; // Prevent move assignment

public:
	static inline flecs::entity CreateArea2D(flecs::world &world, const String &name, const RID &space_id) {
		RID area_id = PhysicsServer2D::get_singleton()->area_create();
		PhysicsServer2D::get_singleton()->area_set_space(area_id, space_id);
		return world.entity(name).set<Area2DComponent>({ area_id });
	}

	static inline flecs::entity CreateBody2D(flecs::world &world, const String &name, const RID &space_id) {
		RID body_id = PhysicsServer2D::get_singleton()->body_create();
		PhysicsServer2D::get_singleton()->body_set_space(body_id, space_id);
		return world.entity(name).set<Body2DComponent>({ body_id });
	}

	static inline flecs::entity CreateJoint2D(flecs::world &world, const String &name, const RID &space_id) {
		RID joint_id = PhysicsServer2D::get_singleton()->joint_create();
		//PhysicsServer2D::get_singleton()->joint(joint_id, space_id);
		return world.entity(name).set<Joint2DComponent>({ joint_id });
	}

	/// Create an Area2D entity from a Godot Area2D object
	static inline flecs::entity CreateArea2D(flecs::world &world, Area2D *area_2d) {
		if (area_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto area_id = area_2d->get_rid();
		if (!area_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(area_2d->get_name())
							  .set<Area2DComponent>({ area_id });
		ObjectIDStorage::add(area_2d, area_id);
		return entity;
	}

	/// Create a RigidBody2D entity from a Godot RigidBody2D object
	static inline flecs::entity CreateRigidBody2D(flecs::world &world, RigidBody2D *body_2d) {
		if (body_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto body_id = body_2d->get_rid();
		if (!body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(body_2d->get_name())
							  .set<Body2DComponent>({ body_id });
		ObjectIDStorage::add(body_2d, body_id);
		return entity;
	}

	static inline flecs::entity CreateJoint2D(flecs::world &world, Joint2D *joint_2d) {
		if (joint_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto joint_id = joint_2d->get_rid();
		if (!joint_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(joint_2d->get_name())
							  .set<Joint2DComponent>({ joint_id });
		ObjectIDStorage::add(joint_2d, joint_id);
		return entity;
	}

	static inline flecs::entity CreateSpace2D(flecs::world &world, const String &name) {
		RID space_id = PhysicsServer2D::get_singleton()->space_create();
		return world.entity(name).set<Space2DComponent>({ space_id });
	}

	static inline flecs::entity CreateSpace2D(flecs::world &world, const RID &space_id, const String &name) {
		return world.entity(name).set<Space2DComponent>({ space_id });
	}
};
}; //namespace godot_turbo::utility::physics
