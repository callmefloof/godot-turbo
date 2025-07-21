#pragma once
#include "../../object_id_storage.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/templates/rid.h"
#include "../../../../modules/godot_turbo/ecs/components/physics/2d/2d_physics_components.h"
#include "../../../../modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "../../../../scene/2d/physics/area_2d.h"
#include "../../../../scene/2d/physics/joints/joint_2d.h"
#include "../../../../scene/2d/physics/rigid_body_2d.h"
#include "../../../../servers/physics_server_2d.h"
#include "../../../../scene/2d/physics/physics_body_2d.h"
#include "../../../../scene/2d/physics/collision_object_2d.h"


class Physics2DUtility {
private:
	Physics2DUtility() = default; // Prevent instantiation
	Physics2DUtility(const Physics2DUtility &) = delete; // Prevent copy
	Physics2DUtility &operator=(const Physics2DUtility &) = delete; // Prevent assignment
	Physics2DUtility(Physics2DUtility &&) = delete; // Prevent move
	Physics2DUtility &operator=(Physics2DUtility &&) = delete; // Prevent move assignment

public:
	static flecs::entity create_area(const flecs::world &world, const String &name, const RID &space_id) {
		const RID area_id = PhysicsServer2D::get_singleton()->area_create();
		PhysicsServer2D::get_singleton()->area_set_space(area_id, space_id);
		return world.entity().set<Area2DComponent>({ area_id }).set_name(name.ascii().get_data());
	}

	static flecs::entity CreateBody(const flecs::world &world, const String &name, const RID &space_id) {
		const RID body_id = PhysicsServer2D::get_singleton()->body_create();
		PhysicsServer2D::get_singleton()->body_set_space(body_id, space_id);
		return world.entity().set<Body2DComponent>({ body_id }).set_name(name.ascii().get_data());
	}

	static flecs::entity create_joint(const flecs::world &world, const String &name, const RID &space_id) {
		const RID joint_id = PhysicsServer2D::get_singleton()->joint_create();
		//no way to find to space?
		//PhysicsServer2D::get_singleton()->joint(joint_id, space_id);
		return world.entity().set<Joint2DComponent>({ joint_id }).set_name(name.ascii().get_data());
	}

	/// Create an Area2D entity from a Godot Area2D object
	static flecs::entity create_area(const flecs::world &world, Area2D *area_2d) {
		if (area_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID area_id = area_2d->get_rid();
		if (!area_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<Area2DComponent>({ area_id })
							  .set_name(String(area_2d->get_name()).ascii().get_data());
		ObjectIDStorage::add(dynamic_cast<Object*>(area_2d), area_id);
		return entity;
	}

	/// Create a RigidBody2D entity from a Godot RigidBody2D object
	static flecs::entity create_rigid_body(const flecs::world &world, RigidBody2D *rigid_body) {
		if (rigid_body == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID body_id = rigid_body->get_rid();
		if (!body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<Body2DComponent>({ body_id })
							  .set_name(String(rigid_body->get_name()).ascii().get_data());
		ObjectIDStorage::add(dynamic_cast<Object*>(rigid_body), body_id);
		return entity;
	}

	static flecs::entity create_physics_body(const flecs::world& world, PhysicsBody2D* physics_body) {
		if (physics_body == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID body_id = physics_body->get_rid();
		if (!body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<Body2DComponent>({ body_id })
							  .set_name(String(physics_body->get_name()).ascii().get_data());
		ObjectIDStorage::add(dynamic_cast<Object*>(physics_body), body_id);
		return entity;
	}
	static flecs::entity create_joint(const flecs::world &world, Joint2D *joint_2d) {
		if (joint_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID joint_id = joint_2d->get_rid();
		if (!joint_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<Joint2DComponent>({ joint_id })
							  .set_name(String(joint_2d->get_name()).ascii().get_data());
		ObjectIDStorage::add(dynamic_cast<Object*>(joint_2d), joint_id);
		return entity;
	}
};
