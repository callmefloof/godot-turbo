#pragma once

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/rid.h"
#include "core/os/mutex.h"
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/world_2d.h"
#include "servers/navigation_server_2d.h"
#include "servers/navigation_server_3d.h"
#include "servers/physics_server_2d.h"
#include "servers/physics_server_3d.h"
#include "servers/rendering_server.h"
#include "scene/resources/camera_attributes.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

/**
 * @file world_utility.h
 * @brief Thread-safe utilities for creating 2D/3D world components in the ECS.
 * 
 * This file provides utilities to set up World2DComponent and World3DComponent
 * on Flecs worlds, which hold the server-side resources (canvas, scenario, space,
 * navigation maps) needed for 2D and 3D scenes.
 * 
 * @note Thread-safe: All public methods are protected by mutexes
 */

/**
 * @class World2DUtility
 * @brief Thread-safe utility for creating 2D world components in the ECS.
 * 
 * ## Purpose
 * 
 * World2DUtility sets up the World2DComponent on a Flecs world, which contains:
 * - **Canvas RID**: RenderingServer canvas for 2D rendering
 * - **Navigation Map RID**: NavigationServer2D map for pathfinding
 * - **Space RID**: PhysicsServer2D space for collision/physics
 * 
 * These server-side resources are required for 2D scenes to function properly.
 * 
 * ## Usage Modes
 * 
 * 1. **Auto-create mode**: Pass an invalid/null World2D → utility creates new server resources
 * 2. **Use existing mode**: Pass a valid World2D → utility uses its existing resources
 * 3. **Update mode**: If component already exists → updates RIDs without creating new entity
 * 
 * ## Thread Safety
 * 
 * All public methods are protected by an internal mutex, making the utility
 * safe for concurrent access from multiple threads.
 * 
 * @example
 * ```cpp
 * // C++ example: Auto-create world resources
 * RID world_id = FlecsServer::get_singleton()->create_world();
 * World2DUtility::create_world_2d(world_id, Ref<World2D>());  // Creates new resources
 * ```
 * 
 * @example
 * ```gdscript
 * # GDScript example: Use existing World2D
 * var world_id = FlecsServer.create_world()
 * var world_2d = get_viewport().find_world_2d()
 * World2DUtility.create_world_2d(world_id, world_2d)
 * ```
 * 
 * @note This is a static utility class - do not instantiate it directly.
 */
class World2DUtility : public Object {
	GDCLASS(World2DUtility, Object)

private:
	static Mutex mutex;  ///< Mutex for thread-safe operations

	/**
	 * @brief Internal implementation - creates World2DComponent with new server resources.
	 * 
	 * Creates fresh server-side resources:
	 * - RenderingServer canvas
	 * - NavigationServer2D map
	 * - PhysicsServer2D space
	 * 
	 * @param world The Flecs world to set the component on
	 * @note Not thread-safe by itself - callers must hold the mutex
	 * @note Internal use only
	 */
	static void _create_world_2d(const flecs::world *world) {
		if (world->has<World2DComponent>()) {
			return;
		}

		World2DComponent w2c;
		w2c.canvas_id = RS::get_singleton()->canvas_create();
		w2c.navigation_map_id = NavigationServer2D::get_singleton()->map_create();
		w2c.space_id = PhysicsServer2D::get_singleton()->space_create();
		world->set<World2DComponent>(w2c);
	}

	/**
	 * @brief Internal implementation - creates/updates World2DComponent from World2D.
	 * 
	 * If world_2d is valid, uses its existing RIDs.
	 * If world_2d is invalid, falls back to creating new resources.
	 * If component already exists, updates it instead of creating new.
	 * 
	 * @param world The Flecs world to set the component on
	 * @param world_2d The Godot World2D to get RIDs from (can be null)
	 * @note Not thread-safe by itself - callers must hold the mutex
	 * @note Internal use only
	 */
	static void _create_world_2d(const flecs::world *world, const Ref<World2D> &world_2d) {
		// If component exists, update it
		if (world->has<World2DComponent>()) {
			World2DComponent& mut_ref = world->get_mut<World2DComponent>();
			if (world_2d.is_valid() && !world_2d.is_null()) {
				mut_ref.navigation_map_id = world_2d->get_navigation_map();
				mut_ref.canvas_id = world_2d->get_canvas();
				mut_ref.space_id = world_2d->get_space();
			}
			world->modified<World2DComponent>();
			return;
		}

		// If World2D is invalid, create new resources
		if (!world_2d.is_valid() || world_2d.is_null()) {
			_create_world_2d(world);
			return;
		}

		// Use existing World2D resources
		World2DComponent w2c;
		w2c.navigation_map_id = world_2d->get_navigation_map();
		w2c.canvas_id = world_2d->get_canvas();
		w2c.space_id = world_2d->get_space();
		world->set<World2DComponent>(w2c);
	}

public:
	/**
	 * @brief Default constructor.
	 * @note This class should not be instantiated - use static methods only.
	 */
	World2DUtility() = default;

	/**
	 * @brief Destructor.
	 */
	~World2DUtility() = default;

	// Prevent copying (static utility class - should not be instantiated)
	World2DUtility(const World2DUtility &) = delete;

	/**
	 * @brief Creates or updates the World2DComponent on a Flecs world.
	 * 
	 * This method sets up the 2D world resources needed for:
	 * - 2D rendering (canvas)
	 * - 2D navigation (navigation map)
	 * - 2D physics (physics space)
	 * 
	 * ## Behavior
	 * 
	 * - **If world_2d is valid**: Uses its existing server RIDs
	 * - **If world_2d is null/invalid**: Creates new server resources
	 * - **If component already exists**: Updates RIDs instead of creating new
	 * 
	 * @param world_id The RID of the Flecs world to configure
	 * @param world_2d The Godot World2D to use (pass null/invalid to auto-create)
	 * 
	 * @note Thread-safe
	 * @note Safe to call multiple times - will update existing component
	 * @warning The world_id must be valid or the function will fail
	 * 
	 * ## Use Cases
	 * 
	 * 1. **Auto-create**: When you want the ECS to manage its own 2D world
	 *    ```cpp
	 *    World2DUtility::create_world_2d(world_id, Ref<World2D>());
	 *    ```
	 * 
	 * 2. **Use viewport world**: When you want to share the viewport's 2D world
	 *    ```cpp
	 *    Ref<World2D> viewport_world = viewport->find_world_2d();
	 *    World2DUtility::create_world_2d(world_id, viewport_world);
	 *    ```
	 * 
	 * 3. **Update existing**: When you need to change the world resources
	 *    ```cpp
	 *    // Component exists, this will update it
	 *    World2DUtility::create_world_2d(world_id, new_world_2d);
	 *    ```
	 * 
	 * @example
	 * ```gdscript
	 * # GDScript: Auto-create world
	 * var world_id = FlecsServer.create_world()
	 * World2DUtility.create_world_2d(world_id, null)
	 * 
	 * # Use viewport's world
	 * var world_2d = get_viewport().find_world_2d()
	 * World2DUtility.create_world_2d(world_id, world_2d)
	 * ```
	 */
	static void create_world_2d(const RID& world_id, const Ref<World2D> &world_2d) {
		MutexLock lock(mutex);

		if (!world_id.is_valid()) {
			ERR_FAIL_COND_MSG(!world_id.is_valid(), 
				"World2DUtility: World RID is invalid");
			return;
		}

		flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
		if (!world) {
			ERR_FAIL_MSG("World2DUtility: Failed to get Flecs world from RID");
			return;
		}

		_create_world_2d(world, world_2d);
	}

	/**
	 * @brief Binds methods for GDScript/engine reflection.
	 * 
	 * Exposes the utility to GDScript and the Godot editor.
	 * 
	 * @note Called automatically during engine initialization
	 * @note Internal use only
	 */
	static void _bind_methods() {
		ClassDB::bind_static_method(
			get_class_static(), 
			D_METHOD("create_world_2d", "world_id", "world_2d"),
			&World2DUtility::create_world_2d
		);
	}
};

/**
 * @class World3DUtility
 * @brief Thread-safe utility for creating 3D world components in the ECS.
 * 
 * ## Purpose
 * 
 * World3DUtility sets up the World3DComponent on a Flecs world, which contains:
 * - **Scenario RID**: RenderingServer scenario for 3D rendering
 * - **Camera Attributes RID**: Camera settings (exposure, DOF, etc.)
 * - **Environment RID**: Scene environment (sky, ambient light, fog, etc.)
 * - **Fallback Environment RID**: Default environment when none specified
 * - **Navigation Map RID**: NavigationServer3D map for pathfinding
 * - **Space RID**: PhysicsServer3D space for collision/physics
 * 
 * These server-side resources are required for 3D scenes to function properly.
 * 
 * ## Usage Modes
 * 
 * 1. **Auto-create mode**: Pass an invalid/null World3D → utility creates new server resources
 * 2. **Use existing mode**: Pass a valid World3D → utility uses its existing resources
 * 3. **Update mode**: If component already exists → updates RIDs without creating new entity
 * 
 * ## Thread Safety
 * 
 * All public methods are protected by an internal mutex, making the utility
 * safe for concurrent access from multiple threads.
 * 
 * @example
 * ```cpp
 * // C++ example: Auto-create world resources
 * RID world_id = FlecsServer::get_singleton()->create_world();
 * World3DUtility::create_world_3d(world_id, Ref<World3D>());  // Creates new resources
 * ```
 * 
 * @example
 * ```gdscript
 * # GDScript example: Use existing World3D
 * var world_id = FlecsServer.create_world()
 * var world_3d = get_viewport().find_world_3d()
 * World3DUtility.create_world_3d(world_id, world_3d)
 * ```
 * 
 * @note This is a static utility class - do not instantiate it directly.
 */
class World3DUtility : public Object {
	GDCLASS(World3DUtility, Object)

private:
	static Mutex mutex;  ///< Mutex for thread-safe operations

	/**
	 * @brief Internal implementation - creates World3DComponent with new server resources.
	 * 
	 * Creates fresh server-side resources:
	 * - RenderingServer scenario, camera attributes, environments
	 * - NavigationServer3D map
	 * - PhysicsServer3D space
	 * 
	 * @param world The Flecs world to set the component on
	 * @note Not thread-safe by itself - callers must hold the mutex
	 * @note Internal use only
	 */
	static void _create_world_3d(const flecs::world *world) {
		if (world->has<World3DComponent>()) {
			return;
		}

		World3DComponent w3c;
		w3c.camera_attributes_id = RS::get_singleton()->camera_attributes_create();
		w3c.environment_id = RS::get_singleton()->environment_create();
		w3c.fallback_environment_id = RS::get_singleton()->environment_create();
		w3c.navigation_map_id = NavigationServer3D::get_singleton()->map_create();
		w3c.scenario_id = RS::get_singleton()->scenario_create();
		w3c.space_id = PhysicsServer3D::get_singleton()->space_create();
		world->set<World3DComponent>(w3c);
	}

	/**
	 * @brief Internal implementation - creates/updates World3DComponent from World3D.
	 * 
	 * If world_3d is valid, uses its existing RIDs.
	 * If world_3d is invalid, falls back to creating new resources.
	 * If component already exists, updates it instead of creating new.
	 * 
	 * @param world The Flecs world to set the component on
	 * @param world_3d The Godot World3D to get RIDs from (can be null)
	 * @note Not thread-safe by itself - callers must hold the mutex
	 * @note Internal use only
	 */
	static void _create_world_3d(const flecs::world *world, const Ref<World3D> &world_3d) {
		// If component exists, update it
		if (world->has<World3DComponent>()) {
			World3DComponent& mut_ref = world->get_mut<World3DComponent>();
			if (world_3d.is_valid() && !world_3d.is_null()) {
				mut_ref.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
				mut_ref.environment_id = world_3d->get_environment()->get_rid();
				mut_ref.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
				mut_ref.navigation_map_id = world_3d->get_navigation_map();
				mut_ref.scenario_id = world_3d->get_scenario();
				mut_ref.space_id = world_3d->get_space();
			}
			world->modified<World3DComponent>();
			return;
		}

		// If World3D is invalid, create new resources
		if (!world_3d.is_valid() || world_3d.is_null()) {
			_create_world_3d(world);
			return;
		}

		// Use existing World3D resources
		World3DComponent w3c;
		w3c.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
		w3c.environment_id = world_3d->get_environment()->get_rid();
		w3c.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
		w3c.navigation_map_id = world_3d->get_navigation_map();
		w3c.scenario_id = world_3d->get_scenario();
		w3c.space_id = world_3d->get_space();
		world->set<World3DComponent>(w3c);
	}

public:
	/**
	 * @brief Default constructor.
	 * @note This class should not be instantiated - use static methods only.
	 */
	World3DUtility() = default;

	/**
	 * @brief Destructor.
	 */
	~World3DUtility() = default;

	// Prevent copying (static utility class - should not be instantiated)
	World3DUtility(const World3DUtility &) = delete;

	/**
	 * @brief Creates or updates the World3DComponent on a Flecs world.
	 * 
	 * This method sets up the 3D world resources needed for:
	 * - 3D rendering (scenario, camera attributes, environments)
	 * - 3D navigation (navigation map)
	 * - 3D physics (physics space)
	 * 
	 * ## Behavior
	 * 
	 * - **If world_3d is valid**: Uses its existing server RIDs
	 * - **If world_3d is null/invalid**: Creates new server resources
	 * - **If component already exists**: Updates RIDs instead of creating new
	 * 
	 * @param world_id The RID of the Flecs world to configure
	 * @param world_3d The Godot World3D to use (pass null/invalid to auto-create)
	 * 
	 * @note Thread-safe
	 * @note Safe to call multiple times - will update existing component
	 * @warning The world_id must be valid or the function will fail
	 * 
	 * ## Use Cases
	 * 
	 * 1. **Auto-create**: When you want the ECS to manage its own 3D world
	 *    ```cpp
	 *    World3DUtility::create_world_3d(world_id, Ref<World3D>());
	 *    ```
	 * 
	 * 2. **Use viewport world**: When you want to share the viewport's 3D world
	 *    ```cpp
	 *    Ref<World3D> viewport_world = viewport->find_world_3d();
	 *    World3DUtility::create_world_3d(world_id, viewport_world);
	 *    ```
	 * 
	 * 3. **Update existing**: When you need to change the world resources
	 *    ```cpp
	 *    // Component exists, this will update it
	 *    World3DUtility::create_world_3d(world_id, new_world_3d);
	 *    ```
	 * 
	 * @example
	 * ```gdscript
	 * # GDScript: Auto-create world
	 * var world_id = FlecsServer.create_world()
	 * World3DUtility.create_world_3d(world_id, null)
	 * 
	 * # Use viewport's world
	 * var world_3d = get_viewport().find_world_3d()
	 * World3DUtility.create_world_3d(world_id, world_3d)
	 * ```
	 */
	static void create_world_3d(const RID& world_id, const Ref<World3D> &world_3d) {
		MutexLock lock(mutex);

		if (!world_id.is_valid()) {
			ERR_FAIL_COND_MSG(!world_id.is_valid(), 
				"World3DUtility: World RID is invalid");
			return;
		}

		flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
		if (!world) {
			ERR_FAIL_MSG("World3DUtility: Failed to get Flecs world from RID");
			return;
		}

		_create_world_3d(world, world_3d);
	}

	/**
	 * @brief Binds methods for GDScript/engine reflection.
	 * 
	 * Exposes the utility to GDScript and the Godot editor.
	 * 
	 * @note Called automatically during engine initialization
	 * @note Internal use only
	 */
	static void _bind_methods() {
		ClassDB::bind_static_method(
			get_class_static(), 
			D_METHOD("create_world_3d", "world_id", "world_3d"),
			&World3DUtility::create_world_3d
		);
	}
};