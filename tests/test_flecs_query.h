/**************************************************************************/
/*  test_flecs_query.h                                                   */
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

#ifndef TEST_FLECS_QUERY_H
#define TEST_FLECS_QUERY_H

#include "modules/godot_turbo/ecs/flecs_types/flecs_query.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "test_fixtures.h"
#include "tests/test_macros.h"

namespace TestFlecsQuery {

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

TEST_SUITE("[Modules][GodotTurbo][FlecsQuery]") {
	TEST_CASE("[FlecsQuery] Basic query initialization") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		CHECK(world_id.is_valid());

		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");

		query.init(world_id, components);

		// Verify initialization
		CHECK(query.get_world() == world_id);
		PackedStringArray returned_comps = query.get_required_components();
		CHECK(returned_comps.size() == 1);
		CHECK(returned_comps[0] == String("Position"));
	}

	TEST_CASE("[FlecsQuery] Query entities with single component") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create entities
		world->component<Position>();
		auto e1 = world->entity("Entity1").set<Position>({ 1.0f, 2.0f, 3.0f });
		auto e2 = world->entity("Entity2").set<Position>({ 4.0f, 5.0f, 6.0f });
		auto e3 = world->entity("Entity3"); // No Position component

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);

		// Get entities
		Array entities = query.get_entities();

		// Should return 2 entities (e1, e2)
		CHECK(entities.size() == 2);
	}

	TEST_CASE("[FlecsQuery] Query entities with multiple components") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<Position>();
		world->component<Velocity>();

		// Create entities with different component combinations
		auto e1 = world->entity("Entity1")
						  .set<Position>({ 1.0f, 2.0f, 3.0f })
						  .set<Velocity>({ 0.1f, 0.2f, 0.3f });
		auto e2 = world->entity("Entity2").set<Position>({ 4.0f, 5.0f, 6.0f });
		auto e3 = world->entity("Entity3")
						  .set<Position>({ 7.0f, 8.0f, 9.0f })
						  .set<Velocity>({ 0.4f, 0.5f, 0.6f });

		// Create query requiring both components
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		components.push_back("Velocity");
		query.init(world_id, components);

		// Get entities
		Array entities = query.get_entities();

		// Should return 2 entities (e1, e3) that have both components
		CHECK(entities.size() == 2);
	}

	TEST_CASE("[FlecsQuery] Get entity count") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create entities
		world->component<Position>();
		for (int i = 0; i < 10; i++) {
			world->entity().set<Position>({ (float)i, (float)i, (float)i });
		}

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);

		// Get entity count
		int count = query.get_entity_count();
		CHECK(count == 10);
	}

	TEST_CASE("[FlecsQuery] Get entities with components (full data)") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create entity
		world->component<Position>();
		auto e1 = world->entity("TestEntity").set<Position>({ 10.0f, 20.0f, 30.0f });

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);

		// Get entities with component data
		Array entities = query.get_entities_with_components();

		// Verify result structure
		CHECK(entities.size() == 1);
		
		Dictionary entity_data = entities[0];
		CHECK(entity_data.has("rid"));
		CHECK(entity_data.has("components"));
	}

	TEST_CASE("[FlecsQuery] Limited entity fetch") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create multiple entities
		world->component<Position>();
		for (int i = 0; i < 20; i++) {
			world->entity().set<Position>({ (float)i, (float)i, (float)i });
		}

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);

		// Get limited entities
		Array entities_batch1 = query.get_entities_limited(5, 0);
		Array entities_batch2 = query.get_entities_limited(5, 5);

		// Verify batching
		CHECK(entities_batch1.size() == 5);
		CHECK(entities_batch2.size() == 5);

		// Verify different entities (no overlap)
		bool entities_differ = false;
		if (entities_batch1.size() > 0 && entities_batch2.size() > 0) {
			RID first_rid = entities_batch1[0];
			RID second_rid = entities_batch2[0];
			entities_differ = (first_rid != second_rid);
		}
		CHECK(entities_differ);
	}

	TEST_CASE("[FlecsQuery] Caching strategy - NO_CACHE") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create entities
		world->component<Position>();
		auto e1 = world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create query with NO_CACHE strategy
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_caching_strategy(FlecsQuery::NO_CACHE);

		// First fetch
		Array entities1 = query.get_entities();
		CHECK(entities1.size() == 1);

		// Add new entity
		auto e2 = world->entity().set<Position>({ 4.0f, 5.0f, 6.0f });

		// Second fetch should reflect new entity immediately
		Array entities2 = query.get_entities();
		CHECK(entities2.size() == 2);
	}

	TEST_CASE("[FlecsQuery] Caching strategy - CACHE_ENTITIES") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();
		auto e1 = world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create query with CACHE_ENTITIES strategy
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_caching_strategy(FlecsQuery::CACHE_ENTITIES);

		// Verify strategy is set
		CHECK(query.get_caching_strategy() == FlecsQuery::CACHE_ENTITIES);

		// First fetch builds cache
		Array entities1 = query.get_entities();
		CHECK(entities1.size() == 1);

		// Cache should be clean after fetch
		CHECK_FALSE(query.is_cache_dirty());
	}

	TEST_CASE("[FlecsQuery] Force cache refresh") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();
		auto e1 = world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create query with caching
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_caching_strategy(FlecsQuery::CACHE_ENTITIES);

		// Build cache
		query.get_entities();

		// Force refresh
		query.force_cache_refresh();
		CHECK(query.is_cache_dirty());
	}

	TEST_CASE("[FlecsQuery] Instrumentation - basic metrics") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component and create entities
		world->component<Position>();
		for (int i = 0; i < 5; i++) {
			world->entity().set<Position>({ (float)i, (float)i, (float)i });
		}

		// Create query with instrumentation
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_instrumentation_enabled(true);

		// Perform fetch
		Array entities = query.get_entities();

		// Verify instrumentation
		CHECK(query.get_instrumentation_enabled());
		CHECK(query.get_total_fetches() >= 1);
		CHECK(query.get_last_fetch_entity_count() == 5);
		CHECK(query.get_total_entities_returned() >= 5);
	}

	TEST_CASE("[FlecsQuery] Instrumentation - get instrumentation data") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();
		world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create query with instrumentation
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_instrumentation_enabled(true);

		// Perform fetch
		query.get_entities();

		// Get instrumentation dictionary
		Dictionary instr_data = query.get_instrumentation_data();

		// Verify dictionary contains expected keys
		CHECK(instr_data.has("total_fetches"));
		CHECK(instr_data.has("total_entities_returned"));
		CHECK(instr_data.has("last_fetch_entity_count"));
		CHECK(instr_data.has("last_fetch_usec"));
	}

	TEST_CASE("[FlecsQuery] Reset instrumentation") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();
		world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create query with instrumentation
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_instrumentation_enabled(true);

		// Perform fetch to populate metrics
		query.get_entities();
		CHECK(query.get_total_fetches() > 0);

		// Reset instrumentation
		query.reset_instrumentation();

		// Verify reset
		CHECK(query.get_total_fetches() == 0);
		CHECK(query.get_total_entities_returned() == 0);
	}

	TEST_CASE("[FlecsQuery] Reset with new components") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<Position>();
		world->component<Velocity>();

		// Create entities
		auto e1 = world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });
		auto e2 = world->entity()
						  .set<Position>({ 4.0f, 5.0f, 6.0f })
						  .set<Velocity>({ 0.1f, 0.2f, 0.3f });

		// Create query for Position only
		FlecsQuery query;
		PackedStringArray components1;
		components1.push_back("Position");
		query.init(world_id, components1);

		Array entities1 = query.get_entities();
		CHECK(entities1.size() == 2);

		// Reset query for Position + Velocity
		PackedStringArray components2;
		components2.push_back("Position");
		components2.push_back("Velocity");
		query.reset(world_id, components2);

		Array entities2 = query.get_entities();
		CHECK(entities2.size() == 1);
	}

	TEST_CASE("[FlecsQuery] Set required components after init") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<Position>();
		world->component<Health>();

		// Create entities
		world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });
		world->entity()
				.set<Position>({ 4.0f, 5.0f, 6.0f })
				.set<Health>({ 100 });

		// Create query initially for Position
		FlecsQuery query;
		PackedStringArray components1;
		components1.push_back("Position");
		query.init(world_id, components1);

		// Change required components
		PackedStringArray components2;
		components2.push_back("Position");
		components2.push_back("Health");
		query.set_required_components(components2);

		// Verify components are updated
		PackedStringArray returned_comps = query.get_required_components();
		CHECK(returned_comps.size() == 2);
	}

	TEST_CASE("[FlecsQuery] Filter by name pattern") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();

		// Create named entities
		world->entity("Player1").set<Position>({ 1.0f, 2.0f, 3.0f });
		world->entity("Player2").set<Position>({ 4.0f, 5.0f, 6.0f });
		world->entity("Enemy1").set<Position>({ 7.0f, 8.0f, 9.0f });

		// Create query with name filter
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);
		query.set_filter_name_pattern("Player*");

		// Verify filter is set
		CHECK(query.get_filter_name_pattern() == String("Player*"));

		// Clear filter
		query.clear_filter();
		CHECK(query.get_filter_name_pattern() == String(""));
	}

	TEST_CASE("[FlecsQuery] Copy constructor") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component
		world->component<Position>();
		world->entity().set<Position>({ 1.0f, 2.0f, 3.0f });

		// Create original query
		FlecsQuery query1;
		PackedStringArray components;
		components.push_back("Position");
		query1.init(world_id, components);
		query1.set_instrumentation_enabled(true);

		// Copy construct
		FlecsQuery query2(query1);

		// Verify copy
		CHECK(query2.get_world() == world_id);
		CHECK(query2.get_instrumentation_enabled());
		
		PackedStringArray comps = query2.get_required_components();
		CHECK(comps.size() == 1);
		CHECK(comps[0] == String("Position"));
	}

	TEST_CASE("[FlecsQuery] Query with no matching entities") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register component but don't create any entities
		world->component<Position>();

		// Create query
		FlecsQuery query;
		PackedStringArray components;
		components.push_back("Position");
		query.init(world_id, components);

		// Get entities - should be empty
		Array entities = query.get_entities();
		CHECK(entities.size() == 0);

		// Entity count should be 0
		int count = query.get_entity_count();
		CHECK(count == 0);
	}

	TEST_CASE("[FlecsQuery] Multiple queries on same world") {
		REQUIRE_FLECS_SERVER();
		FlecsServerFixture fixture;
		RID world_id = fixture.create_world();
		flecs::world *world = fixture.get_world();
		REQUIRE(world != nullptr);

		// Register components
		world->component<Position>();
		world->component<Velocity>();
		world->component<Health>();

		// Create entities with different components
		world->entity()
				.set<Position>({ 1.0f, 2.0f, 3.0f })
				.set<Velocity>({ 0.1f, 0.2f, 0.3f });
		world->entity()
				.set<Position>({ 4.0f, 5.0f, 6.0f })
				.set<Health>({ 100 });
		world->entity()
				.set<Position>({ 7.0f, 8.0f, 9.0f })
				.set<Velocity>({ 0.4f, 0.5f, 0.6f })
				.set<Health>({ 200 });

		// Create multiple queries
		FlecsQuery query_pos;
		PackedStringArray comps_pos;
		comps_pos.push_back("Position");
		query_pos.init(world_id, comps_pos);

		FlecsQuery query_pos_vel;
		PackedStringArray comps_pos_vel;
		comps_pos_vel.push_back("Position");
		comps_pos_vel.push_back("Velocity");
		query_pos_vel.init(world_id, comps_pos_vel);

		FlecsQuery query_all;
		PackedStringArray comps_all;
		comps_all.push_back("Position");
		comps_all.push_back("Velocity");
		comps_all.push_back("Health");
		query_all.init(world_id, comps_all);

		// Verify each query returns correct entities
		CHECK(query_pos.get_entity_count() == 3);
		CHECK(query_pos_vel.get_entity_count() == 2);
		CHECK(query_all.get_entity_count() == 1);
	}
}

} // namespace TestFlecsQuery

#endif // TEST_FLECS_QUERY_H