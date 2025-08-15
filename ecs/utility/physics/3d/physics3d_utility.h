#pragma once
#include "ecs/flecs_types/flecs_world.h"
#include "servers/physics_server_3d.h"
#include "components/physics/3d/3d_physics_components.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "scene/3d/physics/area_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/3d/physics/joints/joint_3d.h"
#include "scene/3d/physics/physics_body_3d.h"
#include "scene/3d/soft_body_3d.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"
#include "ecs/utility/ref_storage.h"
#include "ecs/utility/node_storage.h"
#include "ecs/components/object_instance_component.h"

class Physics3DUtility : public Object {
	GDCLASS(Physics3DUtility, Object)

	// This class is a utility for creating physics entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent physics components in the ECS world,
	// ensuring that the necessary properties are set correctly.

public:
	Physics3DUtility() = default;
	~Physics3DUtility() = default;
	static flecs::entity _create_area(const flecs::world *world, const String &name, const RID &space_id);
	static flecs::entity _create_body_3d(const flecs::world *world, const String &name, const RID &space_id);
	static flecs::entity _create_joint(const flecs::world *world, const String &name, const RID &space_id);
	static flecs::entity _create_soft_body(const flecs::world *world, const String &name, const RID &space_id);
	/// Create an Area3D entity from a Godot Area3D object
	static flecs::entity _create_area(const flecs::world *world, Area3D *area_3d);
	/// Create a RigidBody3D entity from a Godot RigidBody3D object
	static flecs::entity _create_rigid_body(const flecs::world *world, RigidBody3D *body_3d);
	static flecs::entity _create_physics_body(const flecs::world *world, PhysicsBody3D *physics_body);
	static flecs::entity _create_joint(const flecs::world *world, Joint3D *joint_3d);
	static flecs::entity _create_soft_body(const flecs::world *world, SoftBody3D *soft_body_3d);

	static Ref<FlecsEntity> create_area_with_object(FlecsWorld *world, Area3D *area_3d);
	static Ref<FlecsEntity> create_rigid_body_with_object(FlecsWorld *world, RigidBody3D *rigid_body_3d);
	static Ref<FlecsEntity> create_physics_body_with_object(FlecsWorld *world, PhysicsBody3D *physics_body_3d);
	static Ref<FlecsEntity> create_joint_with_object(FlecsWorld *world, Joint3D *joint_3d);
	static Ref<FlecsEntity> create_soft_body_with_object(FlecsWorld *world, SoftBody3D *soft_body_3d);
	static Ref<FlecsEntity> create_area(FlecsWorld *world, const String &name, const RID &space_id);
	static Ref<FlecsEntity> create_body(FlecsWorld *world, const String &name, const RID &space_id);
	static Ref<FlecsEntity> create_joint(FlecsWorld *world, const String &name, const RID &space_id);
	static Ref<FlecsEntity> create_soft_body(FlecsWorld *world, const String &name, const RID &space_id);
	static void _bind_methods();
};
