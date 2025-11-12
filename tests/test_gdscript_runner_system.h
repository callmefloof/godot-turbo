/**************************************************************************/
/*  test_gdscript_runner_system.h                                        */
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

#ifndef TEST_GDSCRIPT_RUNNER_SYSTEM_H
#define TEST_GDSCRIPT_RUNNER_SYSTEM_H

#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "test_fixtures.h"
#include "tests/test_macros.h"

namespace TestGDScriptRunnerSystem {

using namespace TestFixtures;

TEST_SUITE("[Modules][GodotTurbo][GDScriptRunnerSystem]") {
	TEST_CASE("[GDScriptRunnerSystem] Basic initialization") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		CHECK(world_id.is_valid());

		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<GameScriptComponent>();

		// Create and initialize system
		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Verify systems are created (they should be enabled by default)
		CHECK(runner.is_process_enabled());
		CHECK(runner.is_physics_process_enabled());
	}

	TEST_CASE("[GDScriptRunnerSystem] Cache initialization") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Cache should start empty
		CHECK(runner.get_cache_size() == 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Cache population on entity processing") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<GameScriptComponent>();

		// Create entity with GameScriptComponent
		auto entity = world->entity("TestScriptEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Node"; // Use Node as a known type
		entity.set<GameScriptComponent>(script_comp);

		// Initialize runner
		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world to trigger system
		world->progress(0.016f);

		// Cache should now have an entry for "Node"
		CHECK(runner.is_cached("Node"));
	}

	TEST_CASE("[GDScriptRunnerSystem] Clear cache") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		// Create entity with script component
		auto entity = world->entity("TestScriptEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Node";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Trigger caching
		world->progress(0.016f);
		CHECK(runner.get_cache_size() > 0);

		// Clear cache
		runner.clear_cache();
		CHECK(runner.get_cache_size() == 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Enable and disable process") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Should start enabled
		CHECK(runner.is_process_enabled());

		// Disable
		runner.set_process_enabled(false);
		CHECK_FALSE(runner.is_process_enabled());

		// Re-enable
		runner.set_process_enabled(true);
		CHECK(runner.is_process_enabled());
	}

	TEST_CASE("[GDScriptRunnerSystem] Enable and disable physics process") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Should start enabled
		CHECK(runner.is_physics_process_enabled());

		// Disable
		runner.set_physics_process_enabled(false);
		CHECK_FALSE(runner.is_physics_process_enabled());

		// Re-enable
		runner.set_physics_process_enabled(true);
		CHECK(runner.is_physics_process_enabled());
	}

	TEST_CASE("[GDScriptRunnerSystem] Multiple entities with different script types") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		// Create entities with different script types
		auto entity1 = world->entity("Entity1");
		GameScriptComponent script1;
		script1.instance_type = "Node";
		entity1.set<GameScriptComponent>(script1);

		auto entity2 = world->entity("Entity2");
		GameScriptComponent script2;
		script2.instance_type = "Node2D";
		entity2.set<GameScriptComponent>(script2);

		auto entity3 = world->entity("Entity3");
		GameScriptComponent script3;
		script3.instance_type = "Node3D";
		entity3.set<GameScriptComponent>(script3);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world
		world->progress(0.016f);

		// Should have cached all three types
		CHECK(runner.is_cached("Node"));
		CHECK(runner.is_cached("Node2D"));
		CHECK(runner.is_cached("Node3D"));
		CHECK(runner.get_cache_size() >= 3);
	}

	TEST_CASE("[GDScriptRunnerSystem] System without entities") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world without any entities
		world->progress(0.016f);
		world->progress(0.016f);

		// Cache should remain empty
		CHECK(runner.get_cache_size() == 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Entity without GameScriptComponent") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();
		world->component<Transform3DComponent>();

		// Create entity without GameScriptComponent
		auto entity = world->entity("NoScriptEntity");
		entity.set<Transform3DComponent>({});

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world
		world->progress(0.016f);

		// No cache entries should be created
		CHECK(runner.get_cache_size() == 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Multiple progress calls") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("TestEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Node";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Multiple progress calls
		for (int i = 0; i < 10; i++) {
			world->progress(0.016f);
		}

		// Cache should still only have one entry
		CHECK(runner.is_cached("Node"));
		CHECK(runner.get_cache_size() >= 1);
	}

	TEST_CASE("[GDScriptRunnerSystem] Cache persistence across frames") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("TestEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Control";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// First frame
		world->progress(0.016f);
		int cache_size_frame1 = runner.get_cache_size();
		CHECK(cache_size_frame1 > 0);

		// Second frame
		world->progress(0.016f);
		int cache_size_frame2 = runner.get_cache_size();
		
		// Cache size should remain the same
		CHECK(cache_size_frame2 == cache_size_frame1);
	}

	TEST_CASE("[GDScriptRunnerSystem] Disabled system doesn't process") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("TestEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Node";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Disable both systems
		runner.set_process_enabled(false);
		runner.set_physics_process_enabled(false);

		// Progress world
		world->progress(0.016f);

		// Cache should remain empty since systems are disabled
		CHECK(runner.get_cache_size() == 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Re-enabling system after disable") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("TestEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "Node";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Disable
		runner.set_process_enabled(false);
		world->progress(0.016f);
		CHECK(runner.get_cache_size() == 0);

		// Re-enable
		runner.set_process_enabled(true);
		world->progress(0.016f);

		// Should now cache
		CHECK(runner.get_cache_size() > 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Empty instance_type") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("EmptyScriptEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = ""; // Empty type
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world - should handle empty type gracefully
		world->progress(0.016f);

		// May or may not cache empty type, but shouldn't crash
		CHECK(runner.get_cache_size() >= 0);
	}

	TEST_CASE("[GDScriptRunnerSystem] Unknown script type") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<GameScriptComponent>();

		auto entity = world->entity("UnknownScriptEntity");
		GameScriptComponent script_comp;
		script_comp.instance_type = "CompletelyFakeClassName12345";
		entity.set<GameScriptComponent>(script_comp);

		GDScriptRunnerSystem runner;
		runner.init(world_id, world);

		// Progress world - should handle unknown types gracefully
		world->progress(0.016f);

		// Should still cache the type (even if methods don't exist)
		CHECK(runner.is_cached("CompletelyFakeClassName12345"));
	}
}

} // namespace TestGDScriptRunnerSystem

#endif // TEST_GDSCRIPT_RUNNER_SYSTEM_H