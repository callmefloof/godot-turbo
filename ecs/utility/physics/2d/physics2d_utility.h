#pragma once
#include "ecs/utility/ref_storage.h"
#include "ecs/components/object_instance_component.h"
#include "ecs/utility/node_storage.h"
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
	static RID create_area(const RID &world_id, const String &name, const RID &space_id);
	static RID create_body(const RID &world_id, const String &name, const RID &space_id);
	static RID create_joint(const RID &world_id, const String &name, const RID &space_id);
	/// Create an Area2D entity from a Godot Area2D object
	static RID create_area_with_object(const RID &world_id, Area2D *area_2d);
	/// Create a RigidBody2D entity from a Godot RigidBody2D object
	static RID create_rigid_body_with_object(const RID &world_id, RigidBody2D *rigid_body);
	static RID create_physics_body_with_object(const RID &world_id, PhysicsBody2D* physics_body);
	static RID create_joint_with_object(const RID &world_id, Joint2D *joint_2d);
	
	static void _bind_methods();
};
