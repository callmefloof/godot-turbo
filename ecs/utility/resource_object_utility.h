#pragma once

#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "core/io/resource.h"
#include "core/os/mutex.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"

/**
 * @file resource_object_utility.h
 * @brief Thread-safe utility for creating ECS entities from Godot Resource objects.
 * 
 * ResourceObjectUtility bridges Godot's Resource system with the Flecs ECS by creating
 * entities that represent resources. This allows resources (materials, meshes, textures,
 * scripts, etc.) to be tracked and queried within the ECS.
 * 
 * @note Thread-safe: Uses internal mutex for concurrent access
 */

/**
 * @class ResourceObjectUtility
 * @brief Utility class for converting Godot Resources into ECS entities.
 * 
 * ## Purpose
 * 
 * ResourceObjectUtility creates ECS entities that represent Godot resources. This is
 * particularly useful for:
 * - **Script tracking**: Representing attached scripts as entities
 * - **Resource dependencies**: Building entity hierarchies based on resource relationships
 * - **ECS queries**: Finding all entities using a specific resource type
 * - **Resource lifecycle**: Tracking resource usage within the ECS
 * 
 * ## How It Works
 * 
 * When a resource is converted to an entity:
 * 1. A Flecs entity is created with the resource's name
 * 2. A `ResourceComponent` is attached with metadata:
 *    - `resource_id`: The RID of the resource
 *    - `resource_type`: The class name (e.g., "StandardMaterial3D")
 *    - `resource_name`: The resource's name
 *    - `is_script_type`: Whether this is a Script resource
 * 
 * ## Thread Safety
 * 
 * All public methods are protected by an internal mutex, making the utility
 * safe for concurrent access from multiple threads.
 * 
 * ## Usage Example
 * 
 * ```cpp
 * // C++ usage
 * Ref<Script> script = load_script("res://player.gd");
 * RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, script);
 * ```
 * 
 * ```gdscript
 * # GDScript usage
 * var material = StandardMaterial3D.new()
 * var entity_rid = ResourceObjectUtility.create_resource_entity(world_id, material)
 * ```
 * 
 * ## Entity Hierarchy
 * 
 * Resource entities are often used as children of node entities. For example,
 * when a node has an attached script, the script is converted to a resource
 * entity and made a child of the node entity:
 * 
 * ```
 * player_entity (Node)
 *   └── player_script_entity (Resource: player.gd)
 * ```
 * 
 * This allows queries like "find all entities with Script resources" or
 * "get the script attached to this node entity".
 * 
 * @note This is a static utility class - do not instantiate it directly.
 * @note Resources must have valid RIDs to be converted to entities.
 */
class ResourceObjectUtility : public Object {
	GDCLASS(ResourceObjectUtility, Object)

private:
	static Mutex mutex;  ///< Mutex for thread-safe operations

	/**
	 * @brief Internal implementation - creates a Flecs entity from a resource.
	 * 
	 * This is the core implementation that creates the entity and sets up
	 * the ResourceComponent. It's called by the public API methods.
	 * 
	 * @param world The Flecs world to create the entity in
	 * @param resource The resource to convert (must be valid)
	 * @return The created Flecs entity, or an invalid entity on error
	 * 
	 * @note Not thread-safe by itself - callers must hold the mutex
	 * @note Internal use only
	 */
	static flecs::entity _create_resource_entity(const flecs::world *world, const Ref<Resource>& resource) {
		if (!resource.is_valid()) {
			ERR_FAIL_V_MSG(flecs::entity(), "ResourceObjectUtility: Resource is invalid");
		}
		
		// Verify the resource has a valid RID
		const RID rid = resource->get_rid();
		if (!rid.is_valid()) {
			ERR_FAIL_V_MSG(flecs::entity(), "ResourceObjectUtility: Resource RID is invalid");
		}

		// Create the ResourceComponent
		ResourceComponent rc;
		rc.resource_id = rid;
		rc.resource_type = resource->get_class();
		rc.resource_name = resource->get_name();
		
		// Check if this resource is a Script
		const Ref<Script> scr = resource->get_script();
		rc.is_script_type = scr.is_valid();

		// Create entity with resource name and component
		String entity_name = resource->get_name();
		if (entity_name.is_empty()) {
			// Fallback to class name if resource has no name
			entity_name = resource->get_class();
		}
		
		return world->entity(entity_name.ascii().get_data())
			.set<ResourceComponent>(rc);
	}

public:
	/**
	 * @brief Default constructor.
	 * @note This class should not be instantiated - use static methods only.
	 */
	ResourceObjectUtility() = default;

	/**
	 * @brief Destructor.
	 */
	~ResourceObjectUtility() = default;

	// Prevent copying (static utility class - should not be instantiated)
	ResourceObjectUtility(const ResourceObjectUtility &) = delete;

	/**
	 * @brief Creates an ECS entity from a Godot Resource.
	 * 
	 * This is the main public API for converting resources to entities.
	 * It validates the world and resource, then creates a Flecs entity
	 * with a ResourceComponent containing the resource's metadata.
	 * 
	 * @param world_id The RID of the Flecs world to create the entity in
	 * @param resource The Resource to convert (must be valid with a valid RID)
	 * @return RID of the created entity, or invalid RID on error
	 * 
	 * @note Thread-safe
	 * @note The resource must have a valid RID (most resources do automatically)
	 * 
	 * ## Return Values
	 * - **Valid RID**: Entity successfully created
	 * - **Invalid RID**: Error occurred (world invalid, resource invalid, or resource has no RID)
	 * 
	 * ## Script Resources
	 * 
	 * When the resource is a Script (or has a script attached), the
	 * `is_script_type` flag is set to true in the ResourceComponent.
	 * This allows queries to distinguish script entities from other resources.
	 * 
	 * @example
	 * ```cpp
	 * // C++ example: Create entity from material
	 * Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	 * material->set_name("PlayerMaterial");
	 * RID material_entity = ResourceObjectUtility::create_resource_entity(world_id, material);
	 * 
	 * // Create entity from script
	 * Ref<Script> script = ResourceLoader::load("res://player.gd");
	 * RID script_entity = ResourceObjectUtility::create_resource_entity(world_id, script);
	 * ```
	 * 
	 * @example
	 * ```gdscript
	 * # GDScript example
	 * var texture = load("res://textures/player.png")
	 * var entity_rid = ResourceObjectUtility.create_resource_entity(world_id, texture)
	 * 
	 * if entity_rid.is_valid():
	 *     print("Created entity for texture")
	 * else:
	 *     print("Failed to create entity")
	 * ```
	 */
	static RID create_resource_entity(const RID& world_id, const Ref<Resource>& resource) {
		MutexLock lock(mutex);

		// Validate world ID
		if (!world_id.is_valid()) {
			ERR_FAIL_COND_V_MSG(!world_id.is_valid(), RID(), 
				"ResourceObjectUtility: World RID is invalid");
		}

		// Get the Flecs world
		flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
		if (!world) {
			ERR_FAIL_COND_V_MSG(!world, RID(), 
				"ResourceObjectUtility: Failed to get Flecs world from RID");
		}

		// Create the entity
		flecs::entity entity = _create_resource_entity(world, resource);
		if (!entity.is_valid()) {
			return RID();
		}

		// Convert to RID and return
		RID entity_rid = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, entity);
		return entity_rid;
	}

	/**
	 * @brief Binds methods for GDScript/engine reflection.
	 * 
	 * Exposes the utility to GDScript and the Godot editor, allowing
	 * resources to be converted to entities from scripts.
	 * 
	 * @note Called automatically during engine initialization
	 * @note Internal use only
	 */
	static void _bind_methods() {
		ClassDB::bind_static_method(
			get_class_static(), 
			D_METHOD("create_resource_entity", "world_id", "resource"),
			&ResourceObjectUtility::create_resource_entity
		);
	}
};