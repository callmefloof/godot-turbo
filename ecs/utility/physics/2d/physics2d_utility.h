#pragma once
#include "ecs/flecs_types/flecs_world.h"
#include "ecs/utility/object_id_storage.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "ecs/components/physics/2d/2d_physics_components.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "scene/2d/physics/area_2d.h"
#include "scene/2d/physics/joints/joint_2d.h"
#include "scene/2d/physics/rigid_body_2d.h"
#include "servers/physics_server_2d.h"
#include "scene/2d/physics/physics_body_2d.h"
#include "scene/2d/physics/collision_object_2d.h"


class Physics2DUtility : public Object {
	GDCLASS(Physics2DUtility, Object)

	// This class is a utility for creating physics entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent physics components in the ECS world,
	// ensuring that the necessary properties are set correctly.	
public:
	Physics2DUtility() = default;
	~Physics2DUtility() = default;
	static flecs::entity _create_area(const flecs::world &world, const String &name, const RID &space_id);
	static flecs::entity _create_body(const flecs::world &world, const String &name, const RID &space_id);
	static flecs::entity _create_joint(const flecs::world &world, const String &name, const RID &space_id);
	/// Create an Area2D entity from a Godot Area2D object
	static flecs::entity _create_area(const flecs::world &world, Area2D *area_2d);
	/// Create a RigidBody2D entity from a Godot RigidBody2D object
	static flecs::entity _create_rigid_body(const flecs::world &world, RigidBody2D *rigid_body);
	static flecs::entity _create_physics_body(const flecs::world& world, PhysicsBody2D* physics_body);
	static flecs::entity _create_joint(const flecs::world &world, Joint2D *joint_2d);
	
	static Ref<FlecsEntity> create_area_with_object(FlecsWorld *world, Area2D *area_2d);
	static Ref<FlecsEntity> create_rigid_body_with_object(FlecsWorld *world, RigidBody2D *rigid_body);
	static Ref<FlecsEntity> create_physics_body_with_object(FlecsWorld *world, PhysicsBody2D *physics_body);
	static Ref<FlecsEntity> create_joint_with_object(FlecsWorld *world, Joint2D *joint_2d);
	static Ref<FlecsEntity> create_area(FlecsWorld *world, const String &name, const RID &space_id);
	static Ref<FlecsEntity> create_body(FlecsWorld *world, const String &name, const RID &space_id);
	static Ref<FlecsEntity> create_joint(FlecsWorld *world, const String &name, const RID &space_id);

	static void _bind_methods();
};
