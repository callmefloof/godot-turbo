#pragma once
#include "scene/3d/physics/area_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/3d/physics/joints/joint_3d.h"
#include "scene/3d/physics/physics_body_3d.h"
#include "scene/3d/physics/soft_body_3d.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"

/**
 * @file physics3d_utility.h
 * @brief Utility for creating 3D physics entities in the Flecs ECS world.
 * 
 * This utility provides static methods to bridge Godot's 3D physics system (PhysicsServer3D)
 * with the Flecs ECS architecture. It creates ECS entities that represent physics objects
 * such as areas, bodies, joints, and soft bodies, with appropriate components attached.
 * 
 * @section physics3d_thread_safety Thread Safety
 * 
 * The Physics3DUtility methods are generally thread-safe for entity creation, as they:
 * - Call into PhysicsServer3D (which is thread-safe for most operations)
 * - Delegate to FlecsServer for entity creation
 * - Use NodeStorage for object tracking (protected by mutexes)
 * 
 * **Important Constraints:**
 * - Methods that accept Godot Node pointers (e.g., `create_area_with_object`) may need
 *   to access node properties. While the physics server calls are safe, accessing node
 *   properties should be done carefully in multi-threaded contexts.
 * - NodeStorage operations are protected by mutexes in the storage layer.
 * - Creating physics objects and immediately using them in the scene tree should be
 *   done from the main thread to avoid race conditions.
 * 
 * @section physics3d_usage Usage Examples
 * 
 * @subsection physics3d_example1 Creating a Basic 3D Physics Body
 * @code
 * // Create a physics body entity from scratch
 * RID world_id = FlecsServer::get_singleton()->create_world("MyWorld");
 * RID space_id = PhysicsServer3D::get_singleton()->space_create();
 * RID body_entity = Physics3DUtility::create_body(world_id, "PlayerBody", space_id);
 * @endcode
 * 
 * @subsection physics3d_example2 Converting a Godot RigidBody3D to ECS
 * @code
 * // Convert an existing RigidBody3D node to an ECS entity
 * RigidBody3D* rigid_body = get_node<RigidBody3D>("Player");
 * RID world_id = get_world_id(); // Your world RID
 * RID entity_rid = Physics3DUtility::create_rigid_body_with_object(world_id, rigid_body);
 * 
 * // The entity now has:
 * // - Body3DComponent (with the physics body RID)
 * // - ObjectInstanceComponent (linking back to the node)
 * // - Name set to the node's name
 * @endcode
 * 
 * @subsection physics3d_example3 Creating an Area3D Entity
 * @code
 * // Create an area for trigger detection
 * Area3D* trigger_area = get_node<Area3D>("TriggerZone");
 * RID world_id = get_world_id();
 * RID area_entity = Physics3DUtility::create_area_with_object(world_id, trigger_area);
 * 
 * // Query the entity later using Flecs
 * flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
 * flecs::entity e = FlecsServer::get_singleton()->_get_entity(world_id, area_entity);
 * const Area3DComponent* comp = e.get<Area3DComponent>();
 * if (comp) {
 *     RID area_rid = comp->area_id;
 *     // Use the area RID with PhysicsServer3D
 * }
 * @endcode
 * 
 * @subsection physics3d_example4 Creating a Soft Body
 * @code
 * // Create a soft body entity from an existing SoftBody3D node
 * SoftBody3D* cloth = get_node<SoftBody3D>("Cloth");
 * RID world_id = get_world_id();
 * RID soft_body_entity = Physics3DUtility::create_soft_body_with_object(world_id, cloth);
 * @endcode
 * 
 * @subsection physics3d_example5 Creating a Joint
 * @code
 * // Create a joint entity from an existing Joint3D node
 * HingeJoint3D* hinge = get_node<HingeJoint3D>("DoorHinge");
 * RID world_id = get_world_id();
 * RID joint_entity = Physics3DUtility::create_joint_with_object(world_id, hinge);
 * @endcode
 * 
 * @note This class should not be instantiated. All methods are static.
 * @note The created entities will have components from the all_components.h header.
 * @warning Do not delete the source Godot objects (Area3D, RigidBody3D, etc.) while
 *          the ECS entities reference them via ObjectInstanceComponent.
 */
class Physics3DUtility : public Object {
	GDCLASS(Physics3DUtility, Object)

public:
	Physics3DUtility() = default;
	~Physics3DUtility() = default;

	// ========================================================================
	// Area Creation
	// ========================================================================

	/**
	 * @brief Create a new Area3D entity with a freshly created physics area.
	 * 
	 * Creates a new physics area via PhysicsServer3D and wraps it in a Flecs entity
	 * with an Area3DComponent. The area is automatically assigned to the specified space.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity (used for debugging/identification)
	 * @param space_id The physics space RID to which this area belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The returned RID is for the ECS entity, not the physics area itself.
	 * @note To get the physics area RID, query the Area3DComponent from the entity.
	 */
	static RID create_area(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create an Area3D entity from an existing Godot Area3D node.
	 * 
	 * Converts an existing Area3D node into a Flecs entity, preserving its physics
	 * properties and creating a bidirectional link via ObjectInstanceComponent.
	 * The node is registered in NodeStorage for lifecycle management.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param area_3d Pointer to the Area3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if area_3d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note The node is added to NodeStorage and should not be freed while the entity exists
	 * @warning Ensure the Area3D node remains valid for the lifetime of the entity
	 */
	static RID create_area_with_object(const RID &world_id, Area3D *area_3d);

	// ========================================================================
	// Body Creation
	// ========================================================================

	/**
	 * @brief Create a new physics body entity with a freshly created physics body.
	 * 
	 * Creates a new physics body via PhysicsServer3D and wraps it in a Flecs entity
	 * with a Body3DComponent. The body is automatically assigned to the specified space.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @param space_id The physics space RID to which this body belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The physics body created is a generic body; configure it via PhysicsServer3D
	 * @note To get the physics body RID, query the Body3DComponent from the entity
	 */
	static RID create_body(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create a RigidBody3D entity from an existing Godot RigidBody3D node.
	 * 
	 * Converts an existing RigidBody3D node into a Flecs entity with a Body3DComponent
	 * and ObjectInstanceComponent. The node is registered in NodeStorage.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param body_3d Pointer to the RigidBody3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if body_3d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the RigidBody3D node remains valid for the lifetime of the entity
	 */
	static RID create_rigid_body_with_object(const RID &world_id, RigidBody3D *body_3d);

	/**
	 * @brief Create a PhysicsBody3D entity from an existing Godot PhysicsBody3D node.
	 * 
	 * Generic method to convert any PhysicsBody3D-derived node (StaticBody3D,
	 * CharacterBody3D, RigidBody3D, AnimatableBody3D, etc.) into a Flecs entity 
	 * with a Body3DComponent and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param physics_body Pointer to the PhysicsBody3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note This is the most flexible body creation method, accepting any PhysicsBody3D
	 * @note Fails with ERR_FAIL_V if physics_body is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the PhysicsBody3D node remains valid for the lifetime of the entity
	 */
	static RID create_physics_body_with_object(const RID &world_id, PhysicsBody3D *physics_body);

	// ========================================================================
	// Joint Creation
	// ========================================================================

	/**
	 * @brief Create a new joint entity with a freshly created physics joint.
	 * 
	 * Creates a new physics joint via PhysicsServer3D and wraps it in a Flecs entity
	 * with a Joint3DComponent. 
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @param space_id The physics space RID (currently unused by PhysicsServer3D for joints)
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note PhysicsServer3D does not provide a joint_set_space method, so space_id is
	 *       included for API consistency but not used
	 * @note Configure the joint via PhysicsServer3D after creation
	 * @note To get the physics joint RID, query the Joint3DComponent from the entity
	 */
	static RID create_joint(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create a Joint3D entity from an existing Godot Joint3D node.
	 * 
	 * Converts an existing Joint3D node (HingeJoint3D, PinJoint3D, SliderJoint3D,
	 * ConeTwistJoint3D, Generic6DOFJoint3D, etc.) into a Flecs entity with a 
	 * Joint3DComponent and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param joint_3d Pointer to the Joint3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if joint_3d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the Joint3D node remains valid for the lifetime of the entity
	 */
	static RID create_joint_with_object(const RID &world_id, Joint3D *joint_3d);

	// ========================================================================
	// Soft Body Creation
	// ========================================================================

	/**
	 * @brief Create a new soft body entity with a freshly created physics soft body.
	 * 
	 * Creates a new physics soft body via PhysicsServer3D and wraps it in a Flecs entity
	 * with a SoftBody3DComponent. The soft body is automatically assigned to the specified space.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @param space_id The physics space RID to which this soft body belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Soft bodies require additional configuration (mesh, simulation parameters, etc.)
	 *       via PhysicsServer3D after creation
	 * @note To get the physics soft body RID, query the SoftBody3DComponent from the entity
	 */
	static RID create_soft_body(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create a SoftBody3D entity from an existing Godot SoftBody3D node.
	 * 
	 * Converts an existing SoftBody3D node into a Flecs entity with a SoftBody3DComponent
	 * and ObjectInstanceComponent. The node is registered in NodeStorage.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param soft_body_3d Pointer to the SoftBody3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if soft_body_3d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note Soft bodies are useful for cloth, deformable objects, and advanced simulations
	 * @warning Ensure the SoftBody3D node remains valid for the lifetime of the entity
	 */
	static RID create_soft_body_with_object(const RID &world_id, SoftBody3D *soft_body_3d);

	// ========================================================================
	// GDScript Bindings
	// ========================================================================

	/**
	 * @brief Binds methods to GDScript/C# for use in scripts.
	 * @internal This method is called automatically during class registration.
	 */
	static void _bind_methods();
};