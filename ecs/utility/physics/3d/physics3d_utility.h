#pragma once
#include "scene/3d/physics/area_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/3d/physics/joints/joint_3d.h"
#include "scene/3d/physics/physics_body_3d.h"
#include "scene/3d/physics/soft_body_3d.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"

class Physics3DUtility : public Object {
	GDCLASS(Physics3DUtility, Object)

	// This class is a utility for creating physics entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent physics components in the ECS world,
	// ensuring that the necessary properties are set correctly.

public:
	Physics3DUtility() = default;
	~Physics3DUtility() = default;
	static RID create_area(const RID &world_id, const String &name, const RID &space_id);
	static RID create_body(const RID &world_id, const String &name, const RID &space_id);
	static RID create_joint(const RID &world_id, const String &name, const RID &space_id);
	static RID create_soft_body(const RID &world_id, const String &name, const RID &space_id);
	/// Create an Area3D entity from a Godot Area3D object
	static RID create_area_with_object(const RID &world_id, Area3D *area_3d);
	/// Create a RigidBody3D entity from a Godot RigidBody3D object
	static RID create_rigid_body_with_object(const RID &world_id, RigidBody3D *body_3d);
	static RID create_physics_body_with_object(const RID &world_id, PhysicsBody3D *physics_body);
	static RID create_joint_with_object(const RID &world_id, Joint3D *joint_3d);
	static RID create_soft_body_with_object(const RID &world_id, SoftBody3D *soft_body_3d);

	static void _bind_methods();
};
