/**************************************************************************/
/*  test_flecs_script_system.h                                           */
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

#ifndef TEST_FLECS_SCRIPT_SYSTEM_H
#define TEST_FLECS_SCRIPT_SYSTEM_H

#include "modules/godot_turbo/ecs/flecs_types/flecs_script_system.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "test_fixtures.h"
#include "tests/test_macros.h"

namespace TestFlecsScriptSystem {

using namespace TestFixtures;

// Test component structures
struct Position {
	float x;
	float y;
	float z;
};

struct Velocity {
	float dx;
	float dy;
	float dz;
};

struct Health {
	int value;
};

// Mock callable for testing
class MockCallable {
public:
	int call_count = 0;
	Array last_entities;
	
	void reset() {
		call_count = 0;
		last_entities.clear();
	}
	
	void operator()(const Array &entities) {
		call_count++;
		last_entities = entities;
	}
};

TEST_SUITE("[Modules][GodotTurbo][FlecsScriptSystem]") {
	TEST_CASE("[FlecsScriptSystem] Basic initialization") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		CHECK(world_id.is_valid());

		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();

		// Create script system
		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		
		Callable callback; // Empty callable for init test
		script_system.init(world_id, components, callback);

		// Verify initialization
		CHECK(script_system.get_world() == world_id);
		PackedStringArray returned_comps = script_system.get_required_components();
		CHECK(returned_comps.size() == 1);
		CHECK(returned_comps[0] == String("Position"));
	}

	TEST_CASE("[FlecsScriptSystem] Set and get dispatch mode") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be per-entity (0)
		CHECK(script_system.get_dispatch_mode() == FlecsScriptSystem::DISPATCH_PER_ENTITY);

		// Set to batch mode
		script_system.set_dispatch_mode(FlecsScriptSystem::DISPATCH_BATCH);
		CHECK(script_system.get_dispatch_mode() == FlecsScriptSystem::DISPATCH_BATCH);
	}

	TEST_CASE("[FlecsScriptSystem] Set and get required components") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();
		world->component<Velocity>();

		FlecsScriptSystem script_system;
		PackedStringArray initial_components;
		initial_components.push_back("Position");
		Callable callback;
		script_system.init(world_id, initial_components, callback);

		// Change required components
		PackedStringArray new_components;
		new_components.push_back("Position");
		new_components.push_back("Velocity");
		script_system.set_required_components(new_components);

		// Verify update
		PackedStringArray returned = script_system.get_required_components();
		CHECK(returned.size() == 2);
	}

	TEST_CASE("[FlecsScriptSystem] Change-only mode") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should not be change-only
		CHECK_FALSE(script_system.is_change_only());

		// Enable change-only mode
		script_system.set_change_only(true);
		CHECK(script_system.is_change_only());

		// Disable
		script_system.set_change_only(false);
		CHECK_FALSE(script_system.is_change_only());
	}

	TEST_CASE("[FlecsScriptSystem] Change observer flags") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Test observe_add_and_set flag
		CHECK(script_system.get_change_observe_add_and_set()); // Default true
		script_system.set_change_observe_add_and_set(false);
		CHECK_FALSE(script_system.get_change_observe_add_and_set());

		// Test observe_remove flag
		CHECK_FALSE(script_system.get_change_observe_remove()); // Default false
		script_system.set_change_observe_remove(true);
		CHECK(script_system.get_change_observe_remove());
	}

	TEST_CASE("[FlecsScriptSystem] Multi-threaded mode") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be false
		CHECK_FALSE(script_system.get_multi_threaded());

		// Enable multi-threaded
		script_system.set_multi_threaded(true);
		CHECK(script_system.get_multi_threaded());
	}

	TEST_CASE("[FlecsScriptSystem] Batch configuration") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Set batch chunk size
		script_system.set_batch_flush_chunk_size(100);
		CHECK(script_system.get_batch_flush_chunk_size() == 100);

		// Set flush interval
		script_system.set_flush_min_interval_msec(16.0);
		CHECK(script_system.get_flush_min_interval_msec() == 16.0);
	}

	TEST_CASE("[FlecsScriptSystem] Deferred calls flag") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be false
		CHECK_FALSE(script_system.get_use_deferred_calls());

		// Enable deferred calls
		script_system.set_use_deferred_calls(true);
		CHECK(script_system.get_use_deferred_calls());
	}

	TEST_CASE("[FlecsScriptSystem] Instrumentation enable/disable") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be false
		CHECK_FALSE(script_system.get_instrumentation_enabled());

		// Enable instrumentation
		script_system.set_instrumentation_enabled(true);
		CHECK(script_system.get_instrumentation_enabled());
	}

	TEST_CASE("[FlecsScriptSystem] Detailed timing") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be false
		CHECK_FALSE(script_system.get_detailed_timing_enabled());

		// Enable detailed timing
		script_system.set_detailed_timing_enabled(true);
		CHECK(script_system.get_detailed_timing_enabled());
	}

	TEST_CASE("[FlecsScriptSystem] Auto-reset per frame") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should be false
		CHECK_FALSE(script_system.get_auto_reset_per_frame());

		// Enable auto-reset
		script_system.set_auto_reset_per_frame(true);
		CHECK(script_system.get_auto_reset_per_frame());
	}

	TEST_CASE("[FlecsScriptSystem] Pause and resume") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default should not be paused
		CHECK_FALSE(script_system.get_is_paused());

		// Pause
		script_system.set_is_paused(true);
		CHECK(script_system.get_is_paused());

		// Resume
		script_system.set_is_paused(false);
		CHECK_FALSE(script_system.get_is_paused());
	}

	TEST_CASE("[FlecsScriptSystem] System name") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Set system name
		script_system.set_system_name("MovementSystem");
		CHECK(script_system.get_system_name() == String("MovementSystem"));
	}

	TEST_CASE("[FlecsScriptSystem] System ID assignment") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// System should have an ID assigned
		uint32_t system_id = script_system.get_system_id();
		CHECK(system_id > 0);
	}

	TEST_CASE("[FlecsScriptSystem] Instrumentation counters") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);
		script_system.set_instrumentation_enabled(true);

		// Initially all counters should be zero
		CHECK(script_system.get_last_frame_entity_count() == 0);
		CHECK(script_system.get_total_entities_processed() == 0);
		CHECK(script_system.get_total_callbacks_invoked() == 0);
		CHECK(script_system.get_frame_dispatch_invocations() == 0);
	}

	TEST_CASE("[FlecsScriptSystem] Reset instrumentation") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);
		script_system.set_instrumentation_enabled(true);

		// Reset instrumentation
		script_system.reset_instrumentation();

		// All counters should be zero after reset
		CHECK(script_system.get_total_entities_processed() == 0);
		CHECK(script_system.get_total_callbacks_invoked() == 0);
	}

	TEST_CASE("[FlecsScriptSystem] Event totals (change-only mode)") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Initially all event counters should be zero
		CHECK(script_system.get_last_frame_onadd() == 0);
		CHECK(script_system.get_last_frame_onset() == 0);
		CHECK(script_system.get_last_frame_onremove() == 0);
		CHECK(script_system.get_total_onadd() == 0);
		CHECK(script_system.get_total_onset() == 0);
		CHECK(script_system.get_total_onremove() == 0);
	}

	TEST_CASE("[FlecsScriptSystem] Max sample count") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Default max sample count
		int default_count = script_system.get_max_sample_count();
		CHECK(default_count > 0);

		// Set custom max sample count
		script_system.set_max_sample_count(2048);
		CHECK(script_system.get_max_sample_count() == 2048);
	}

	TEST_CASE("[FlecsScriptSystem] System dependency") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);

		// Initially should not have dependency
		CHECK_FALSE(script_system.get_depends_on_system());

		// Set dependency
		uint32_t dependency_id = 12345;
		script_system.set_system_dependency(dependency_id);
		CHECK(script_system.get_system_dependency_id() == dependency_id);
	}

	TEST_CASE("[FlecsScriptSystem] Reset system with new components") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();
		world->component<Velocity>();

		FlecsScriptSystem script_system;
		PackedStringArray components1;
		components1.push_back("Position");
		Callable callback;
		script_system.init(world_id, components1, callback);

		// Reset with different components
		PackedStringArray components2;
		components2.push_back("Position");
		components2.push_back("Velocity");
		script_system.reset(world_id, components2, callback);

		// Verify components updated
		PackedStringArray returned = script_system.get_required_components();
		CHECK(returned.size() == 2);
	}

	TEST_CASE("[FlecsScriptSystem] Copy constructor") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem original;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		original.init(world_id, components, callback);
		original.set_instrumentation_enabled(true);
		original.set_system_name("OriginalSystem");

		// Copy construct
		FlecsScriptSystem copy(original);

		// Verify copy has same properties
		CHECK(copy.get_world() == world_id);
		CHECK(copy.get_instrumentation_enabled());
		CHECK(copy.get_system_name() == String("OriginalSystem"));
	}

	TEST_CASE("[FlecsScriptSystem] Timing statistics") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();

		FlecsScriptSystem script_system;
		PackedStringArray components;
		components.push_back("Position");
		Callable callback;
		script_system.init(world_id, components, callback);
		script_system.set_instrumentation_enabled(true);

		// Timing stats should be initialized
		CHECK(script_system.get_frame_dispatch_min_usec() >= 0);
		CHECK(script_system.get_frame_dispatch_max_usec() >= 0);
		CHECK(script_system.get_frame_dispatch_accum_usec() >= 0);
	}

	TEST_CASE("[FlecsScriptSystem] Multiple systems on same world") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		world->component<Position>();
		world->component<Velocity>();

		// Create first system
		FlecsScriptSystem system1;
		PackedStringArray components1;
		components1.push_back("Position");
		Callable callback1;
		system1.init(world_id, components1, callback1);
		system1.set_system_name("PositionSystem");

		// Create second system
		FlecsScriptSystem system2;
		PackedStringArray components2;
		components2.push_back("Velocity");
		Callable callback2;
		system2.init(world_id, components2, callback2);
		system2.set_system_name("VelocitySystem");

		// Both should be valid and have different IDs
		CHECK(system1.get_world() == world_id);
		CHECK(system2.get_world() == world_id);
		CHECK(system1.get_system_id() != system2.get_system_id());
	}
}

} // namespace TestFlecsScriptSystem

#endif // TEST_FLECS_SCRIPT_SYSTEM_H