#pragma once
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "scene/2d/physics/area_2d.h"
#include "scene/2d/physics/joints/joint_2d.h"
#include "scene/2d/physics/rigid_body_2d.h"
#include "scene/2d/physics/physics_body_2d.h"

/**
 * @file physics2d_utility.h
 * @brief Utility for creating 2D physics entities in the Flecs ECS world.
 * 
 * This utility provides static methods to bridge Godot's 2D physics system (PhysicsServer2D)
 * with the Flecs ECS architecture. It creates ECS entities that represent physics objects
 * such as areas, bodies, and joints, with appropriate components attached.
 * 
 * @section physics2d_thread_safety Thread Safety
 * 
 * The Physics2DUtility methods are generally thread-safe for entity creation, as they:
 * - Call into PhysicsServer2D (which is thread-safe for most operations)
 * - Delegate to FlecsServer for entity creation
 * - Use NodeStorage for object tracking
 * 
 * **Important Constraints:**
 * - Methods that accept Godot Node pointers (e.g., `create_area_with_object`) may need
 *   to access node properties. While the physics server calls are safe, accessing node
 *   properties should be done carefully in multi-threaded contexts.
 * - NodeStorage operations are protected by mutexes in the storage layer.
 * - Creating physics objects and immediately using them in the scene tree should be
 *   done from the main thread to avoid race conditions.
 * 
 * @section physics2d_usage Usage Examples
 * 
 * @subsection physics2d_example1 Creating a Basic Physics Body
 * @code
 * // Create a physics body entity from scratch
 * RID world_id = FlecsServer::get_singleton()->create_world("MyWorld");
 * RID space_id = PhysicsServer2D::get_singleton()->space_create();
 * RID body_entity = Physics2DUtility::create_body(world_id, "PlayerBody", space_id);
 * @endcode
 * 
 * @subsection physics2d_example2 Converting a Godot RigidBody2D to ECS
 * @code
 * // Convert an existing RigidBody2D node to an ECS entity
 * RigidBody2D* rigid_body = get_node<RigidBody2D>("Player");
 * RID world_id = get_world_id(); // Your world RID
 * RID entity_rid = Physics2DUtility::create_rigid_body_with_object(world_id, rigid_body);
 * 
 * // The entity now has:
 * // - Body2DComponent (with the physics body RID)
 * // - ObjectInstanceComponent (linking back to the node)
 * // - Name set to the node's name
 * @endcode
 * 
 * @subsection physics2d_example3 Creating an Area2D Entity
 * @code
 * // Create an area for trigger detection
 * Area2D* trigger_area = get_node<Area2D>("TriggerZone");
 * RID world_id = get_world_id();
 * RID area_entity = Physics2DUtility::create_area_with_object(world_id, trigger_area);
 * 
 * // Query the entity later using Flecs
 * flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
 * flecs::entity e = FlecsServer::get_singleton()->_get_entity(world_id, area_entity);
 * const Area2DComponent* comp = e.get<Area2DComponent>();
 * if (comp) {
 *     RID area_rid = comp->area_id;
 *     // Use the area RID with PhysicsServer2D
 * }
 * @endcode
 * 
 * @subsection physics2d_example4 Creating a Joint
 * @code
 * // Create a joint entity from an existing Joint2D node
 * Joint2D* pin_joint = get_node<Joint2D>("PinJoint");
 * RID world_id = get_world_id();
 * RID joint_entity = Physics2DUtility::create_joint_with_object(world_id, pin_joint);
 * @endcode
 * 
 * @note This class should not be instantiated. All methods are static.
 * @note The created entities will have components from the all_components.h header.
 * @warning Do not delete the source Godot objects (Area2D, RigidBody2D, etc.) while
 *          the ECS entities reference them via ObjectInstanceComponent.
 */
class Physics2DUtility : public Object {
	GDCLASS(Physics2DUtility, Object)

public:
	Physics2DUtility() = default;
	~Physics2DUtility() = default;

	// ========================================================================
	// Area Creation
	// ========================================================================

	/**
	 * @brief Create a new Area2D entity with a freshly created physics area.
	 * 
	 * Creates a new physics area via PhysicsServer2D and wraps it in a Flecs entity
	 * with an Area2DComponent. The area is automatically assigned to the specified space.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity (used for debugging/identification)
	 * @param space_id The physics space RID to which this area belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The returned RID is for the ECS entity, not the physics area itself.
	 * @note To get the physics area RID, query the Area2DComponent from the entity.
	 */
	static RID create_area(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create an Area2D entity from an existing Godot Area2D node.
	 * 
	 * Converts an existing Area2D node into a Flecs entity, preserving its physics
	 * properties and creating a bidirectional link via ObjectInstanceComponent.
	 * The node is registered in NodeStorage for lifecycle management.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param area_2d Pointer to the Area2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if area_2d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note The node is added to NodeStorage and should not be freed while the entity exists
	 * @warning Ensure the Area2D node remains valid for the lifetime of the entity
	 */
	static RID create_area_with_object(const RID &world_id, Area2D *area_2d);

	// ========================================================================
	// Body Creation
	// ========================================================================

	/**
	 * @brief Create a new physics body entity with a freshly created physics body.
	 * 
	 * Creates a new physics body via PhysicsServer2D and wraps it in a Flecs entity
	 * with a Body2DComponent. The body is automatically assigned to the specified space.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @param space_id The physics space RID to which this body belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The physics body created is a generic body; configure it via PhysicsServer2D
	 * @note To get the physics body RID, query the Body2DComponent from the entity
	 */
	static RID create_body(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create a RigidBody2D entity from an existing Godot RigidBody2D node.
	 * 
	 * Converts an existing RigidBody2D node into a Flecs entity with a Body2DComponent
	 * and ObjectInstanceComponent. The node is registered in NodeStorage.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param rigid_body Pointer to the RigidBody2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if rigid_body is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the RigidBody2D node remains valid for the lifetime of the entity
	 */
	static RID create_rigid_body_with_object(const RID &world_id, RigidBody2D *rigid_body);

	/**
	 * @brief Create a PhysicsBody2D entity from an existing Godot PhysicsBody2D node.
	 * 
	 * Generic method to convert any PhysicsBody2D-derived node (StaticBody2D,
	 * CharacterBody2D, RigidBody2D, etc.) into a Flecs entity with a Body2DComponent
	 * and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param physics_body Pointer to the PhysicsBody2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note This is the most flexible body creation method, accepting any PhysicsBody2D
	 * @note Fails with ERR_FAIL_V if physics_body is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the PhysicsBody2D node remains valid for the lifetime of the entity
	 */
	static RID create_physics_body_with_object(const RID &world_id, PhysicsBody2D* physics_body);

	// ========================================================================
	// Joint Creation
	// ========================================================================

	/**
	 * @brief Create a new joint entity with a freshly created physics joint.
	 * 
	 * Creates a new physics joint via PhysicsServer2D and wraps it in a Flecs entity
	 * with a Joint2DComponent. 
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @param space_id The physics space RID (currently unused by PhysicsServer2D for joints)
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note PhysicsServer2D does not provide a joint_set_space method, so space_id is
	 *       included for API consistency but not used
	 * @note Configure the joint via PhysicsServer2D after creation
	 * @note To get the physics joint RID, query the Joint2DComponent from the entity
	 */
	static RID create_joint(const RID &world_id, const String &name, const RID &space_id);

	/**
	 * @brief Create a Joint2D entity from an existing Godot Joint2D node.
	 * 
	 * Converts an existing Joint2D node (PinJoint2D, GrooveJoint2D, DampedSpringJoint2D,
	 * etc.) into a Flecs entity with a Joint2DComponent and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param joint_2d Pointer to the Joint2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if joint_2d is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the Joint2D node remains valid for the lifetime of the entity
	 */
	static RID create_joint_with_object(const RID &world_id, Joint2D *joint_2d);

	// ========================================================================
	// GDScript Bindings
	// ========================================================================

	/**
	 * @brief Binds methods to GDScript/C# for use in scripts.
	 * @internal This method is called automatically during class registration.
	 */
	static void _bind_methods();
};