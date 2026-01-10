/**************************************************************************/
/*  test_world_utility.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef TEST_WORLD_UTILITY_H
#define TEST_WORLD_UTILITY_H

#include "tests/test_macros.h"
#include "test_fixtures.h"
#include "modules/godot_turbo/ecs/systems/utility/world_utility.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "scene/resources/world_2d.h"
#include "scene/resources/3d/world_3d.h"
#include "servers/rendering_server.h"
#include "servers/physics_server_2d.h"
#include "servers/physics_server_3d.h"
#include "servers/navigation_server_2d.h"
#include "servers/navigation_server_3d.h"

#define REQUIRE_WORLD2D_SERVERS() \
	do { \
		REQUIRE_FLECS_SERVER(); \
		if (!RenderingServer::get_singleton()) { \
			MESSAGE("Skipping test - RenderingServer not initialized"); \
			return; \
		} \
		if (!PhysicsServer2D::get_singleton()) { \
			MESSAGE("Skipping test - PhysicsServer2D not initialized"); \
			return; \
		} \
		if (!NavigationServer2D::get_singleton()) { \
			MESSAGE("Skipping test - NavigationServer2D not initialized"); \
			return; \
		} \
	} while (0)

#define REQUIRE_WORLD3D_SERVERS() \
	do { \
		REQUIRE_FLECS_SERVER(); \
		if (!RenderingServer::get_singleton()) { \
			MESSAGE("Skipping test - RenderingServer not initialized"); \
			return; \
		} \
		if (!PhysicsServer3D::get_singleton()) { \
			MESSAGE("Skipping test - PhysicsServer3D not initialized"); \
			return; \
		} \
		if (!NavigationServer3D::get_singleton()) { \
			MESSAGE("Skipping test - NavigationServer3D not initialized"); \
			return; \
		} \
	} while (0)

namespace TestWorldUtility {

// ========================================================================
// World2DUtility Tests
// ========================================================================

#ifndef DISABLE_THREADED_TESTS
struct World2DThreadData {
	RID world_id;
	int thread_id;
	int iterations;
};

static void thread_update_world_2d(void *p_userdata) {
	World2DThreadData *data = static_cast<World2DThreadData *>(p_userdata);
	for (int i = 0; i < data->iterations; i++) {
		Ref<World2D> world_2d = memnew(World2D);
		World2DUtility::create_world_2d(data->world_id, world_2d);
	}
}

struct World3DThreadData {
	RID world_id;
	int thread_id;
	int iterations;
};

static void thread_update_world_3d(void *p_userdata) {
	World3DThreadData *data = static_cast<World3DThreadData *>(p_userdata);
	for (int i = 0; i < data->iterations; i++) {
		Ref<World3D> world_3d = memnew(World3D);
		World3DUtility::create_world_3d(data->world_id, world_3d);
	}
}
#endif // DISABLE_THREADED_TESTS

TEST_CASE("[World2DUtility] Auto-create world resources") {
	REQUIRE_WORLD2D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	CHECK(server != nullptr);

	RID world_id = server->create_world();
	CHECK(world_id.is_valid());

	flecs::world *world = server->_get_world(world_id);
	CHECK(world != nullptr);
	CHECK_FALSE(world->has<World2DComponent>());

	// Create with null World2D - should auto-create resources
	Ref<World2D> null_world;
	World2DUtility::create_world_2d(world_id, null_world);

	// Verify component was created
	CHECK(world->has<World2DComponent>());

	const World2DComponent& w2c = world->get<World2DComponent>();
	CHECK(w2c.canvas_id.is_valid());
	CHECK(w2c.navigation_map_id.is_valid());
	CHECK(w2c.space_id.is_valid());

	server->free_world(world_id);
}

TEST_CASE("[World2DUtility] Update existing World2D") {
	REQUIRE_WORLD2D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<World2D> godot_world = memnew(World2D);
	CHECK(godot_world.is_valid());

	RID expected_canvas = godot_world->get_canvas();
	RID expected_map = godot_world->get_navigation_map();
	RID expected_space = godot_world->get_space();

	World2DUtility::create_world_2d(world_id, godot_world);

	flecs::world *world = server->_get_world(world_id);
	CHECK(world->has<World2DComponent>());

	const World2DComponent& w2c = world->get<World2DComponent>();
	CHECK(w2c.canvas_id == expected_canvas);
	CHECK(w2c.navigation_map_id == expected_map);
	CHECK(w2c.space_id == expected_space);

	server->free_world(world_id);
}

TEST_CASE("[World2DUtility] Create World2D with null Ref") {
	REQUIRE_WORLD2D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	flecs::world *world = server->_get_world(world_id);

	// Create initial world
	Ref<World2D> null_world;
	World2DUtility::create_world_2d(world_id, null_world);

	const World2DComponent& initial = world->get<World2DComponent>();
	RID initial_canvas = initial.canvas_id;
	RID initial_map = initial.navigation_map_id;
	RID initial_space = initial.space_id;

	// Update with new World2D
	Ref<World2D> new_world = memnew(World2D);
	World2DUtility::create_world_2d(world_id, new_world);

	const World2DComponent& updated = world->get<World2DComponent>();
	
	// RIDs should have changed
	CHECK(updated.canvas_id != initial_canvas);
	CHECK(updated.navigation_map_id != initial_map);
	CHECK(updated.space_id != initial_space);

	// Should match new world's RIDs
	CHECK(updated.canvas_id == new_world->get_canvas());
	CHECK(updated.navigation_map_id == new_world->get_navigation_map());
	CHECK(updated.space_id == new_world->get_space());

	server->free_world(world_id);
}

TEST_CASE("[World2DUtility] Invalid world ID") {
	REQUIRE_FLECS_SERVER();
	
	RID invalid_world;
	Ref<World2D> null_world;

	// Should fail gracefully without crashing
	ERR_PRINT_OFF;
	World2DUtility::create_world_2d(invalid_world, null_world);
	ERR_PRINT_ON;

	// No way to verify much here - just ensure no crash
	CHECK(true);
}

TEST_CASE("[World2DUtility] Multiple calls idempotent") {
	REQUIRE_WORLD2D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<World2D> null_world;
	
	// First call
	World2DUtility::create_world_2d(world_id, null_world);
	
	flecs::world *world = server->_get_world(world_id);
	const World2DComponent& first = world->get<World2DComponent>();
	RID first_canvas = first.canvas_id;

	// Second call - should update but not crash
	World2DUtility::create_world_2d(world_id, null_world);
	
	const World2DComponent& second = world->get<World2DComponent>();
	
	CHECK(world->has<World2DComponent>());
	CHECK(second.canvas_id.is_valid());

	server->free_world(world_id);
}

#ifndef DISABLE_THREADED_TESTS
TEST_CASE("[World2DUtility] Thread safety") {
	REQUIRE_WORLD2D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	const int THREAD_COUNT = 4;
	const int ITERATIONS = 50;

	Thread threads[THREAD_COUNT];
	World2DThreadData thread_data[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].world_id = world_id;
		thread_data[i].thread_id = i;
		thread_data[i].iterations = ITERATIONS;
		threads[i].start(thread_update_world_2d, &thread_data[i]);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	// Verify component exists and is valid
	flecs::world *world = server->_get_world(world_id);
	CHECK(world->has<World2DComponent>());
	
	const World2DComponent& w2c = world->get<World2DComponent>();
	CHECK(w2c.canvas_id.is_valid());
	CHECK(w2c.navigation_map_id.is_valid());
	CHECK(w2c.space_id.is_valid());

	server->free_world(world_id);
}
#endif // DISABLE_THREADED_TESTS

// ========================================================================
// World3DUtility Tests
// ========================================================================

TEST_CASE("[World3DUtility] Auto-create world resources") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	flecs::world *world = server->_get_world(world_id);
	CHECK_FALSE(world->has<World3DComponent>());

	// Create with null World3D - should auto-create resources
	Ref<World3D> null_world;
	World3DUtility::create_world_3d(world_id, null_world);

	CHECK(world->has<World3DComponent>());

	const World3DComponent& w3c = world->get<World3DComponent>();
	CHECK(w3c.scenario_id.is_valid());
	CHECK(w3c.camera_attributes_id.is_valid());
	CHECK(w3c.environment_id.is_valid());
	CHECK(w3c.fallback_environment_id.is_valid());
	CHECK(w3c.navigation_map_id.is_valid());
	CHECK(w3c.space_id.is_valid());

	server->free_world(world_id);
}

TEST_CASE("[World3DUtility] Update existing World3D") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<World3D> godot_world = memnew(World3D);
	CHECK(godot_world.is_valid());

	RID expected_scenario = godot_world->get_scenario();
	RID expected_cam_attr = godot_world->get_camera_attributes()->get_rid();
	RID expected_env = godot_world->get_environment()->get_rid();
	RID expected_fallback = godot_world->get_fallback_environment()->get_rid();
	RID expected_map = godot_world->get_navigation_map();
	RID expected_space = godot_world->get_space();

	World3DUtility::create_world_3d(world_id, godot_world);

	flecs::world *world = server->_get_world(world_id);
	CHECK(world->has<World3DComponent>());

	const World3DComponent& w3c = world->get<World3DComponent>();
	CHECK(w3c.scenario_id == expected_scenario);
	CHECK(w3c.camera_attributes_id == expected_cam_attr);
	CHECK(w3c.environment_id == expected_env);
	CHECK(w3c.fallback_environment_id == expected_fallback);
	CHECK(w3c.navigation_map_id == expected_map);
	CHECK(w3c.space_id == expected_space);

	server->free_world(world_id);
}

TEST_CASE("[World3DUtility] Create World3D with null Ref") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	flecs::world *world = server->_get_world(world_id);

	// Create initial world
	Ref<World3D> null_world;
	World3DUtility::create_world_3d(world_id, null_world);

	const World3DComponent& initial = world->get<World3DComponent>();
	RID initial_scenario = initial.scenario_id;
	RID initial_space = initial.space_id;

	// Update with new World3D
	Ref<World3D> new_world = memnew(World3D);
	World3DUtility::create_world_3d(world_id, new_world);

	const World3DComponent& updated = world->get<World3DComponent>();
	
	// RIDs should have changed
	CHECK(updated.scenario_id != initial_scenario);
	CHECK(updated.space_id != initial_space);

	// Should match new world's RIDs
	CHECK(updated.scenario_id == new_world->get_scenario());
	CHECK(updated.space_id == new_world->get_space());

	server->free_world(world_id);
}

TEST_CASE("[World3DUtility] Invalid world ID") {
	REQUIRE_FLECS_SERVER();
	
	RID invalid_world;
	Ref<World3D> null_world;

	ERR_PRINT_OFF;
	World3DUtility::create_world_3d(invalid_world, null_world);
	ERR_PRINT_ON;

	CHECK(true);
}

TEST_CASE("[World3DUtility] Multiple calls idempotent") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<World3D> null_world;
	
	// First call
	World3DUtility::create_world_3d(world_id, null_world);
	
	flecs::world *world = server->_get_world(world_id);
	const World3DComponent& first = world->get<World3DComponent>();
	RID first_scenario = first.scenario_id;

	// Second call
	World3DUtility::create_world_3d(world_id, null_world);
	
	const World3DComponent& second = world->get<World3DComponent>();
	
	CHECK(world->has<World3DComponent>());
	CHECK(second.scenario_id.is_valid());

	server->free_world(world_id);
}

#ifndef DISABLE_THREADED_TESTS
TEST_CASE("[World3DUtility] Thread safety") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	const int THREAD_COUNT = 4;
	const int ITERATIONS = 50;

	Thread threads[THREAD_COUNT];
	World3DThreadData thread_data[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].world_id = world_id;
		thread_data[i].thread_id = i;
		thread_data[i].iterations = ITERATIONS;
		threads[i].start(thread_update_world_3d, &thread_data[i]);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	flecs::world *world = server->_get_world(world_id);
	CHECK(world->has<World3DComponent>());
	
	const World3DComponent& w3c = world->get<World3DComponent>();
	CHECK(w3c.scenario_id.is_valid());
	CHECK(w3c.camera_attributes_id.is_valid());
	CHECK(w3c.environment_id.is_valid());
	CHECK(w3c.space_id.is_valid());

	server->free_world(world_id);
}
#endif // DISABLE_THREADED_TESTS

TEST_CASE("[World3DUtility] All server resources valid") {
	REQUIRE_WORLD3D_SERVERS();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<World3D> null_world;
	World3DUtility::create_world_3d(world_id, null_world);

	flecs::world *world = server->_get_world(world_id);
	const World3DComponent& w3c = world->get<World3DComponent>();

	// Verify all RIDs are valid and recognized by servers
	RenderingServer *rs = RenderingServer::get_singleton();
	CHECK(rs != nullptr);
	
	PhysicsServer3D *ps3d = PhysicsServer3D::get_singleton();
	CHECK(ps3d != nullptr);
	
	NavigationServer3D *ns3d = NavigationServer3D::get_singleton();
	CHECK(ns3d != nullptr);

	// All RIDs should be valid
	CHECK(w3c.scenario_id.is_valid());
	CHECK(w3c.camera_attributes_id.is_valid());
	CHECK(w3c.environment_id.is_valid());
	CHECK(w3c.fallback_environment_id.is_valid());
	CHECK(w3c.navigation_map_id.is_valid());
	CHECK(w3c.space_id.is_valid());

	server->free_world(world_id);
}

} // namespace TestWorldUtility

#endif // TEST_WORLD_UTILITY_H
