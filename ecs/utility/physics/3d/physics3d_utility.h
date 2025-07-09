#pragma once
#include <servers/physics_server_3d.h>
#include "../../../components/physics/3d/3d_physics_components.h"
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include <scene/3d/physics/area_3d.h>
#include <scene/3d/physics/rigid_body_3d.h>
#include <scene/3d/physics/joints/joint_3d.h>
#include <scene/3d/soft_body_3d.h>
#include <core/templates/rid.h>
#include <core/string/ustring.h>
#include "../../object_id_storage.h"
#include <scene/3d/soft_body_3d.h>



class Physics3DUtility {
private:
	Physics3DUtility() = default; // Prevent instantiation
	Physics3DUtility(const Physics3DUtility &) = delete; // Prevent copy
	Physics3DUtility &operator=(const Physics3DUtility &) = delete; // Prevent assignment
	Physics3DUtility(Physics3DUtility &&) = delete; // Prevent move
	Physics3DUtility &operator=(Physics3DUtility &&) = delete; // Prevent move assignment

public:
	static inline flecs::entity CreateArea3D(flecs::world &world, const String &name, const RID &space_id) {
		RID area_id = PhysicsServer3D::get_singleton()->area_create();
		PhysicsServer3D::get_singleton()->area_set_space(area_id, space_id);
		return world.entity().set<Area3DComponent>({ area_id }).set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateBody3D(flecs::world &world, const String &name, const RID &space_id) {
		RID body_id = PhysicsServer3D::get_singleton()->body_create();
		PhysicsServer3D::get_singleton()->body_set_space(body_id, space_id);
		return world.entity().set<Body3DComponent>({ body_id }).set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateJoint3D(flecs::world &world, const String &name, const RID &space_id) {
		RID joint_id = PhysicsServer3D::get_singleton()->joint_create();
		//PhysicsServer3D::get_singleton()->joint(joint_id, space_id);
		return world.entity().set<Joint3DComponent>({ joint_id }).set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateSoftBody3D(flecs::world &world, const String &name, const RID &space_id) {
		RID soft_body_id = PhysicsServer3D::get_singleton()->soft_body_create();
		PhysicsServer3D::get_singleton()->soft_body_set_space(soft_body_id, space_id);
		return world.entity().set<SoftBody3DComponent>({ soft_body_id }).set_name(name.ascii().get_data());
	}

	/// Create an Area3D entity from a Godot Area3D object
	static inline flecs::entity CreateArea3D(flecs::world &world, Area3D *area_3d) {
		if (area_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto area_id = area_3d->get_rid();
		if (!area_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<Area3DComponent>({ area_id })
							  .set_name(String(area_3d->get_name()).ascii().get_data());
		ObjectIDStorage::add(area_3d, area_id);
		return entity;
	}

	/// Create a RigidBody3D entity from a Godot RigidBody3D object
	static inline flecs::entity CreateRigidBody3D(flecs::world &world, RigidBody3D *body_3d) {
		if (body_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto body_id = body_3d->get_rid();
		if (!body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<Body3DComponent>({ body_id })
							  .set_name(String(body_3d->get_name()).ascii().get_data());
		ObjectIDStorage::add(body_3d, body_id);
		return entity;
	}

	
	static inline flecs::entity CreatePhysicsBody(flecs::world &world, PhysicsBody3D *physics_body) {
		if (physics_body == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto body_id = physics_body->get_rid();
		if (!body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<Body3DComponent>({ body_id })
							  .set_name(String(physics_body->get_name()).ascii().get_data());
		ObjectIDStorage::add(physics_body, body_id);
		return entity;
	}

	static inline flecs::entity CreateJoint3D(flecs::world &world, Joint3D *joint_3d) {
		if (joint_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto joint_id = joint_3d->get_rid();
		if (!joint_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<Joint3DComponent>({ joint_id })
							  .set_name(String(joint_3d->get_name()).ascii().get_data());
		ObjectIDStorage::add(joint_3d, joint_id);
		return entity;
	}

	static inline flecs::entity CreateSoftBody3D(flecs::world &world, SoftBody3D *soft_body_3d) {
		if (soft_body_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto soft_body_id = soft_body_3d->get_physics_rid();
		if (!soft_body_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<SoftBody3DComponent>({ soft_body_id })
							  .set_name(String(soft_body_3d->get_name()).ascii().get_data());
		ObjectIDStorage::add(soft_body_3d, soft_body_id);
		return entity;
	}

	static inline flecs::entity CreateSpace3D(flecs::world &world, const String &name) {
		RID space_id = PhysicsServer3D::get_singleton()->space_create();
		return world.entity().set<Space3DComponent>({ space_id }).set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateSpace3D(flecs::world &world, const RID& space_id, const String &name) {
		return world.entity().set<Space3DComponent>({ space_id }).set_name(name.ascii().get_data());
	}
};
