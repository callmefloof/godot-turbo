#pragma once
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "scene/3d/navigation/navigation_agent_3d.h"
#include "scene/3d/navigation/navigation_link_3d.h"
#include "scene/3d/navigation/navigation_obstacle_3d.h"
#include "scene/3d/navigation/navigation_region_3d.h"

/**
 * @file navigation3d_utility.h
 * @brief Utility for creating 3D navigation entities in the Flecs ECS world.
 * 
 * This utility provides static methods to bridge Godot's 3D navigation system (NavigationServer3D)
 * with the Flecs ECS architecture. It creates ECS entities that represent navigation objects
 * such as agents, links, obstacles, regions, and source geometry parsers, with appropriate
 * components attached.
 * 
 * @section navigation3d_thread_safety Thread Safety
 * 
 * The Navigation3DUtility methods are generally thread-safe for entity creation, as they:
 * - Call into NavigationServer3D (which is thread-safe for most operations)
 * - Delegate to FlecsServer for entity creation
 * - Use NodeStorage for object tracking (protected by mutexes)
 * 
 * **Important Constraints:**
 * - Methods that accept Godot Node pointers (e.g., `create_nav_agent_with_object`) may need
 *   to access node properties. While the navigation server calls are safe, accessing node
 *   properties should be done carefully in multi-threaded contexts.
 * - NodeStorage operations are protected by mutexes in the storage layer.
 * - Navigation map updates and queries should typically be done from the main thread
 *   or synchronized with the physics frame for consistency.
 * 
 * @section navigation3d_usage Usage Examples
 * 
 * @subsection navigation3d_example1 Creating a Navigation Agent
 * @code
 * // Create a navigation agent entity from scratch
 * RID world_id = FlecsServer::get_singleton()->create_world("MyWorld");
 * RID agent_entity = Navigation3DUtility::create_nav_agent(world_id, "PlayerAgent");
 * 
 * // Configure the agent via NavigationServer3D
 * flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
 * flecs::entity e = FlecsServer::get_singleton()->_get_entity(world_id, agent_entity);
 * const NavAgent3DComponent* comp = e.get<NavAgent3DComponent>();
 * if (comp) {
 *     NavigationServer3D::get_singleton()->agent_set_map(comp->agent_id, map_rid);
 *     NavigationServer3D::get_singleton()->agent_set_radius(comp->agent_id, 0.5);
 *     NavigationServer3D::get_singleton()->agent_set_height(comp->agent_id, 2.0);
 * }
 * @endcode
 * 
 * @subsection navigation3d_example2 Converting a Godot NavigationAgent3D to ECS
 * @code
 * // Convert an existing NavigationAgent3D node to an ECS entity
 * NavigationAgent3D* nav_agent = get_node<NavigationAgent3D>("NPCAgent");
 * RID world_id = get_world_id(); // Your world RID
 * RID entity_rid = Navigation3DUtility::create_nav_agent_with_object(world_id, nav_agent);
 * 
 * // The entity now has:
 * // - NavAgent3DComponent (with the navigation agent RID)
 * // - ObjectInstanceComponent (linking back to the node)
 * // - Name set to the node's name
 * @endcode
 * 
 * @subsection navigation3d_example3 Creating a Navigation Region
 * @code
 * // Create a navigation region from an existing NavigationRegion3D
 * NavigationRegion3D* nav_region = get_node<NavigationRegion3D>("NavMeshRegion");
 * RID world_id = get_world_id();
 * RID region_entity = Navigation3DUtility::create_nav_region_with_object(world_id, nav_region);
 * 
 * // The region is now tracked in ECS and can be queried
 * @endcode
 * 
 * @subsection navigation3d_example4 Creating Navigation Links
 * @code
 * // Create a navigation link (e.g., for jumps, teleports, ladders)
 * NavigationLink3D* jump_link = get_node<NavigationLink3D>("JumpPoint");
 * RID world_id = get_world_id();
 * RID link_entity = Navigation3DUtility::create_nav_link_with_object(world_id, jump_link);
 * @endcode
 * 
 * @subsection navigation3d_example5 Creating Navigation Obstacles
 * @code
 * // Create dynamic obstacles for navigation avoidance
 * NavigationObstacle3D* moving_obstacle = get_node<NavigationObstacle3D>("MovingBox");
 * RID world_id = get_world_id();
 * RID obstacle_entity = Navigation3DUtility::create_nav_obstacle_with_object(world_id, moving_obstacle);
 * @endcode
 * 
 * @subsection navigation3d_example6 Creating Source Geometry Parsers
 * @code
 * // Create a source geometry parser with a custom callback
 * Callable parser_callback = callable_mp(this, &MyClass::parse_geometry);
 * RID world_id = get_world_id();
 * RID parser_entity = Navigation3DUtility::create_sgp_with_callable(
 *     world_id, 
 *     parser_callback, 
 *     "CustomGeometryParser"
 * );
 * @endcode
 * 
 * @note This class should not be instantiated. All methods are static.
 * @note The created entities will have components from the all_components.h header.
 * @warning Do not delete the source Godot objects (NavigationAgent3D, NavigationRegion3D, etc.)
 *          while the ECS entities reference them via ObjectInstanceComponent.
 */
class Callable;
class Navigation3DUtility : public Object {
	GDCLASS(Navigation3DUtility, Object)

public:
	Navigation3DUtility() = default;
	~Navigation3DUtility() = default;

	// ========================================================================
	// Navigation Agent Creation
	// ========================================================================

	/**
	 * @brief Create a new navigation agent entity with a freshly created agent.
	 * 
	 * Creates a new navigation agent via NavigationServer3D and wraps it in a Flecs entity
	 * with a NavAgent3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity (used for debugging/identification)
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The returned RID is for the ECS entity, not the navigation agent itself.
	 * @note To get the navigation agent RID, query the NavAgent3DComponent from the entity.
	 * @note Configure the agent (radius, height, max speed, etc.) via NavigationServer3D after creation.
	 */
	static RID create_nav_agent(const RID &world_id, const String &name);

	/**
	 * @brief Create a navigation agent entity with an existing agent RID.
	 * 
	 * Wraps an existing navigation agent RID in a Flecs entity with a NavAgent3DComponent.
	 * Useful when you've already created the agent via NavigationServer3D directly.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param agent The existing navigation agent RID from NavigationServer3D
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Does not create a new navigation agent; uses the provided RID
	 * @note You are responsible for managing the lifecycle of the agent RID
	 */
	static RID create_nav_agent_with_id(const RID &world_id, const RID &agent, const String &name);

	/**
	 * @brief Create a navigation agent entity from an existing Godot NavigationAgent3D node.
	 * 
	 * Converts an existing NavigationAgent3D node into a Flecs entity, preserving its
	 * navigation properties and creating a bidirectional link via ObjectInstanceComponent.
	 * The node is registered in NodeStorage for lifecycle management.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param nav_agent Pointer to the NavigationAgent3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if nav_agent is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @note The node is added to NodeStorage and should not be freed while the entity exists
	 * @warning Ensure the NavigationAgent3D node remains valid for the lifetime of the entity
	 */
	static RID create_nav_agent_with_object(const RID &world_id, NavigationAgent3D *nav_agent);

	// ========================================================================
	// Navigation Link Creation
	// ========================================================================

	/**
	 * @brief Create a new navigation link entity with a freshly created link.
	 * 
	 * Creates a new navigation link via NavigationServer3D and wraps it in a Flecs entity
	 * with a NavLink3DComponent. Links allow agents to traverse non-standard connections
	 * such as jumps, teleports, ladders, or one-way passages.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure the link (start/end positions, bidirectional, etc.) via NavigationServer3D
	 * @note To get the navigation link RID, query the NavLink3DComponent from the entity
	 */
	static RID create_nav_link(const RID &world_id, const String &name);

	/**
	 * @brief Create a navigation link entity with an existing link RID.
	 * 
	 * Wraps an existing navigation link RID in a Flecs entity with a NavLink3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param link The existing navigation link RID from NavigationServer3D
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Does not create a new navigation link; uses the provided RID
	 */
	static RID create_nav_link_with_id(const RID &world_id, const RID &link, const String &name);

	/**
	 * @brief Create a navigation link entity from an existing Godot NavigationLink3D node.
	 * 
	 * Converts an existing NavigationLink3D node into a Flecs entity with a NavLink3DComponent
	 * and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param nav_link Pointer to the NavigationLink3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if nav_link is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the NavigationLink3D node remains valid for the lifetime of the entity
	 */
	static RID create_nav_link_with_object(const RID &world_id, NavigationLink3D *nav_link);

	// ========================================================================
	// Navigation Obstacle Creation
	// ========================================================================

	/**
	 * @brief Create a new navigation obstacle entity with a freshly created obstacle.
	 * 
	 * Creates a new navigation obstacle via NavigationServer3D and wraps it in a Flecs entity
	 * with a NavObstacle3DComponent. Obstacles are used for dynamic avoidance by navigation agents.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure the obstacle (position, radius, height, velocity) via NavigationServer3D
	 * @note Obstacles should be updated each frame for dynamic avoidance
	 * @note To get the navigation obstacle RID, query the NavObstacle3DComponent from the entity
	 */
	static RID create_nav_obstacle(const RID &world_id, const String &name);

	/**
	 * @brief Create a navigation obstacle entity with an existing obstacle RID.
	 * 
	 * Wraps an existing navigation obstacle RID in a Flecs entity with a NavObstacle3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param obstacle The existing navigation obstacle RID from NavigationServer3D
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Does not create a new navigation obstacle; uses the provided RID
	 */
	static RID create_nav_obstacle_with_id(const RID &world_id, const RID &obstacle, const String &name);

	/**
	 * @brief Create a navigation obstacle entity from an existing Godot NavigationObstacle3D node.
	 * 
	 * Converts an existing NavigationObstacle3D node into a Flecs entity with a 
	 * NavObstacle3DComponent and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param nav_obstacle Pointer to the NavigationObstacle3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if nav_obstacle is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the NavigationObstacle3D node remains valid for the lifetime of the entity
	 */
	static RID create_nav_obstacle_with_object(const RID &world_id, NavigationObstacle3D *nav_obstacle);

	// ========================================================================
	// Navigation Region Creation
	// ========================================================================

	/**
	 * @brief Create a new navigation region entity with a freshly created region.
	 * 
	 * Creates a new navigation region via NavigationServer3D and wraps it in a Flecs entity
	 * with a NavRegion3DComponent. Regions define the walkable navigation mesh areas.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure the region (navigation mesh, transform, layers) via NavigationServer3D
	 * @note To get the navigation region RID, query the NavRegion3DComponent from the entity
	 */
	static RID create_nav_region(const RID &world_id, const String &name);

	/**
	 * @brief Create a navigation region entity with an existing region RID.
	 * 
	 * Wraps an existing navigation region RID in a Flecs entity with a NavRegion3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param region The existing navigation region RID from NavigationServer3D
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Does not create a new navigation region; uses the provided RID
	 */
	static RID create_nav_region_with_id(const RID &world_id, const RID &region, const String &name);

	/**
	 * @brief Create a navigation region entity from an existing Godot NavigationRegion3D node.
	 * 
	 * Converts an existing NavigationRegion3D node into a Flecs entity with a 
	 * NavRegion3DComponent and ObjectInstanceComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param nav_region Pointer to the NavigationRegion3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Fails with ERR_FAIL_V if nav_region is null or its RID is invalid
	 * @note The entity's name is set to the node's name
	 * @warning Ensure the NavigationRegion3D node remains valid for the lifetime of the entity
	 */
	static RID create_nav_region_with_object(const RID &world_id, NavigationRegion3D *nav_region);

	// ========================================================================
	// Source Geometry Parser Creation
	// ========================================================================

	/**
	 * @brief Create a new source geometry parser entity with a freshly created parser.
	 * 
	 * Creates a new source geometry parser via NavigationServer3D and wraps it in a Flecs entity
	 * with a SourceGeometryParser3DComponent. Parsers extract geometry from scenes for navmesh baking.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure the parser via NavigationServer3D before use
	 * @note To get the parser RID, query the SourceGeometryParser3DComponent from the entity
	 */
	static RID create_source_geometry_parser(const RID &world_id, const String &name);

	/**
	 * @brief Create a source geometry parser entity with an existing parser RID.
	 * 
	 * Wraps an existing source geometry parser RID in a Flecs entity with a 
	 * SourceGeometryParser3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param source_geometry_parser The existing parser RID from NavigationServer3D
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Does not create a new parser; uses the provided RID
	 */
	static RID create_sgp_with_id(const RID &world_id, const RID &source_geometry_parser, const String &name);

	/**
	 * @brief Create a source geometry parser entity with a custom callback.
	 * 
	 * Creates a new source geometry parser via NavigationServer3D, sets a custom geometry
	 * parsing callback, and wraps it in a Flecs entity with a SourceGeometryParser3DComponent.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param callable The callback function to use for parsing geometry
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note The callable should match the signature expected by NavigationServer3D
	 * @note Fails with ERR_FAIL_V if parser creation fails
	 * @note Useful for custom navmesh generation logic
	 */
	static RID create_sgp_with_callable(const RID &world_id, const Callable &callable, const String &name);

	// ========================================================================
	// GDScript Bindings
	// ========================================================================

	/**
	 * @brief Binds methods to GDScript/C# for use in scripts.
	 * @internal This method is called automatically during class registration.
	 */
	static void _bind_methods();
};