/**************************************************************************/
/*  test_flecs_variant.h                                                 */
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

#ifndef TEST_FLECS_VARIANT_H
#define TEST_FLECS_VARIANT_H

#include "modules/godot_turbo/ecs/flecs_types/flecs_variant.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "test_fixtures.h"
#include "tests/test_macros.h"

namespace TestFlecsVariant {

using namespace TestFixtures;

// Test component for variant tests
struct TestComponent {
	int value;
};

TEST_SUITE("[Modules][GodotTurbo][FlecsVariant]") {
	TEST_CASE("[FlecsWorldVariant] Default constructor") {
		FlecsWorldVariant world_var;
		flecs::world &world = world_var.get_world();

		// Verify world is created and valid
		CHECK(world.is_valid());
	}

	TEST_CASE("[FlecsWorldVariant] Move constructor from world") {
		flecs::world original_world;
		original_world.entity("TestEntity");

		// Move into variant
		FlecsWorldVariant world_var(std::move(original_world));
		flecs::world &world = world_var.get_world();

		// Verify world is valid and has the entity
		CHECK(world.is_valid());
		auto lookup = world.lookup("TestEntity");
		CHECK(lookup.is_valid());
	}

	TEST_CASE("[FlecsWorldVariant] Copy constructor from world") {
		flecs::world original_world;
		original_world.entity("TestEntity");

		// Copy into variant
		FlecsWorldVariant world_var(original_world);
		flecs::world &world = world_var.get_world();

		// Verify world is valid
		CHECK(world.is_valid());
		
		// Both worlds should reference the same underlying world
		auto lookup1 = original_world.lookup("TestEntity");
		auto lookup2 = world.lookup("TestEntity");
		CHECK(lookup1.is_valid());
		CHECK(lookup2.is_valid());
	}

	TEST_CASE("[FlecsWorldVariant] Copy constructor from variant") {
		FlecsWorldVariant original_var;
		original_var.get_world().entity("TestEntity");

		// Copy variant
		FlecsWorldVariant copy_var(original_var);
		flecs::world &copy_world = copy_var.get_world();

		// Verify both variants reference same world
		CHECK(copy_world.is_valid());
		auto lookup = copy_world.lookup("TestEntity");
		CHECK(lookup.is_valid());
	}

	TEST_CASE("[FlecsWorldVariant] Move constructor from variant") {
		FlecsWorldVariant original_var;
		original_var.get_world().entity("TestEntity");

		// Move variant
		FlecsWorldVariant moved_var(std::move(original_var));
		flecs::world &moved_world = moved_var.get_world();

		// Verify moved variant has the world
		CHECK(moved_world.is_valid());
		auto lookup = moved_world.lookup("TestEntity");
		CHECK(lookup.is_valid());
	}

	TEST_CASE("[FlecsWorldVariant] World operations") {
		FlecsWorldVariant world_var;
		flecs::world &world = world_var.get_world();

		// Create component
		world.component<TestComponent>();

		// Create entity with component
		auto entity = world.entity("TestEntity")
							.set<TestComponent>({ 42 });

		// Verify entity exists and has component
		CHECK(entity.is_valid());
		CHECK(entity.has<TestComponent>());
		
		const TestComponent *comp = entity.get<TestComponent>();
		CHECK(comp != nullptr);
		CHECK(comp->value == 42);
	}

	TEST_CASE("[FlecsEntityVariant] Construct from entity") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant entity_var(entity);

		// Verify entity is stored correctly
		flecs::entity retrieved = entity_var.get_entity();
		CHECK(retrieved.is_valid());
		CHECK(retrieved == entity);
	}

	TEST_CASE("[FlecsEntityVariant] Copy constructor") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant original_var(entity);
		FlecsEntityVariant copy_var(original_var);

		// Verify copy has same entity
		flecs::entity original_entity = original_var.get_entity();
		flecs::entity copy_entity = copy_var.get_entity();
		
		CHECK(original_entity.is_valid());
		CHECK(copy_entity.is_valid());
		CHECK(original_entity == copy_entity);
	}

	TEST_CASE("[FlecsEntityVariant] Move constructor from entity") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant entity_var(std::move(entity));

		// Verify entity is stored
		flecs::entity retrieved = entity_var.get_entity();
		CHECK(retrieved.is_valid());
	}

	TEST_CASE("[FlecsEntityVariant] Move constructor from variant") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant original_var(entity);
		FlecsEntityVariant moved_var(std::move(original_var));

		// Verify moved variant has entity
		flecs::entity moved_entity = moved_var.get_entity();
		CHECK(moved_entity.is_valid());
	}

	TEST_CASE("[FlecsEntityVariant] is_valid() for valid entity") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant entity_var(entity);

		// Should be valid
		CHECK(entity_var.is_valid());
	}

	TEST_CASE("[FlecsEntityVariant] is_valid() for deleted entity") {
		flecs::world world;
		flecs::entity entity = world.entity("TestEntity");

		FlecsEntityVariant entity_var(entity);

		// Delete the entity
		entity.destruct();

		// Should no longer be valid
		CHECK_FALSE(entity_var.is_valid());
	}

	TEST_CASE("[FlecsEntityVariant] Entity with components") {
		flecs::world world;
		world.component<TestComponent>();
		
		flecs::entity entity = world.entity("TestEntity")
							.set<TestComponent>({ 123 });

		FlecsEntityVariant entity_var(entity);

		// Get entity and verify component
		flecs::entity retrieved = entity_var.get_entity();
		CHECK(retrieved.has<TestComponent>());
		
		const TestComponent *comp = retrieved.get<TestComponent>();
		CHECK(comp != nullptr);
		CHECK(comp->value == 123);
	}

	TEST_CASE("[FlecsSystemVariant] Construct from system") {
		flecs::world world;
		world.component<TestComponent>();

		bool system_ran = false;
		flecs::system sys = world.system<TestComponent>()
			.each([&](TestComponent &tc) {
				system_ran = true;
			});

		FlecsSystemVariant sys_var(sys);

		// Verify system is stored
		flecs::system retrieved = sys_var.get_system();
		CHECK(retrieved.is_valid());
		CHECK(retrieved == sys);
	}

	TEST_CASE("[FlecsSystemVariant] is_valid() for valid system") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::system sys = world.system<TestComponent>()
			.each([](TestComponent &tc) {});

		FlecsSystemVariant sys_var(sys);

		// Should be valid
		CHECK(sys_var.is_valid());
	}

	TEST_CASE("[FlecsSystemVariant] System execution") {
		flecs::world world;
		world.component<TestComponent>();

		int execution_count = 0;
		flecs::system sys = world.system<TestComponent>()
			.each([&](TestComponent &tc) {
				execution_count++;
			});

		FlecsSystemVariant sys_var(sys);

		// Create entities with component
		world.entity().set<TestComponent>({ 1 });
		world.entity().set<TestComponent>({ 2 });
		world.entity().set<TestComponent>({ 3 });

		// Run system
		world.progress(0.0f);

		// Verify system executed for all entities
		CHECK(execution_count == 3);
	}

	TEST_CASE("[FlecsTypeIDVariant] Construct from type ID") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::entity_t type_id = world.component<TestComponent>().id();
		FlecsTypeIDVariant type_var(type_id);

		// Verify type ID is stored
		flecs::entity_t retrieved = type_var.get_type();
		CHECK(retrieved == type_id);
		CHECK(retrieved != 0);
	}

	TEST_CASE("[FlecsTypeIDVariant] Copy constructor") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::entity_t type_id = world.component<TestComponent>().id();
		FlecsTypeIDVariant original_var(type_id);
		FlecsTypeIDVariant copy_var(original_var);

		// Verify copy has same type ID
		CHECK(copy_var.get_type() == original_var.get_type());
		CHECK(copy_var.get_type() == type_id);
	}

	TEST_CASE("[FlecsTypeIDVariant] Move constructor") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::entity_t type_id = world.component<TestComponent>().id();
		FlecsTypeIDVariant original_var(type_id);
		FlecsTypeIDVariant moved_var(std::move(original_var));

		// Verify moved variant has type ID
		CHECK(moved_var.get_type() == type_id);
		CHECK(moved_var.get_type() != 0);
	}

	TEST_CASE("[FlecsTypeIDVariant] is_valid() for valid type") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::entity_t type_id = world.component<TestComponent>().id();
		FlecsTypeIDVariant type_var(type_id);

		// Should be valid (non-zero)
		CHECK(type_var.is_valid());
	}

	TEST_CASE("[FlecsTypeIDVariant] is_valid() for zero type") {
		FlecsTypeIDVariant type_var(0);

		// Should be invalid (zero)
		CHECK_FALSE(type_var.is_valid());
	}

	TEST_CASE("[FlecsTypeIDVariant] Using type ID for component operations") {
		flecs::world world;
		world.component<TestComponent>();

		flecs::entity_t type_id = world.component<TestComponent>().id();
		FlecsTypeIDVariant type_var(type_id);

		// Create entity and add component using type ID
		flecs::entity entity = world.entity("TestEntity");
		entity.add(type_var.get_type());

		// Verify entity has the component
		CHECK(entity.has(type_var.get_type()));
	}

	TEST_CASE("[FlecsVariant] Multiple component types") {
		flecs::world world;
		
		// Define multiple component types
		struct Position { float x, y; };
		struct Velocity { float dx, dy; };
		struct Health { int value; };

		world.component<Position>();
		world.component<Velocity>();
		world.component<Health>();

		// Store type IDs in variants
		FlecsTypeIDVariant pos_type(world.component<Position>().id());
		FlecsTypeIDVariant vel_type(world.component<Velocity>().id());
		FlecsTypeIDVariant health_type(world.component<Health>().id());

		// All should be valid and different
		CHECK(pos_type.is_valid());
		CHECK(vel_type.is_valid());
		CHECK(health_type.is_valid());
		
		CHECK(pos_type.get_type() != vel_type.get_type());
		CHECK(vel_type.get_type() != health_type.get_type());
		CHECK(pos_type.get_type() != health_type.get_type());
	}

	TEST_CASE("[FlecsVariant] World-Entity-System integration") {
		// Create world variant
		FlecsWorldVariant world_var;
		flecs::world &world = world_var.get_world();

		world.component<TestComponent>();

		// Create entity variant
		flecs::entity entity = world.entity("IntegrationTest")
								.set<TestComponent>({ 999 });
		FlecsEntityVariant entity_var(entity);

		// Create system variant
		int execution_count = 0;
		flecs::system sys = world.system<TestComponent>()
			.each([&](TestComponent &tc) {
				execution_count++;
				CHECK(tc.value == 999);
			});
		FlecsSystemVariant sys_var(sys);

		// Verify all variants are valid
		CHECK(world_var.get_world().is_valid());
		CHECK(entity_var.is_valid());
		CHECK(sys_var.is_valid());

		// Run world progress
		world.progress(0.0f);

		// Verify system executed
		CHECK(execution_count == 1);
	}

	TEST_CASE("[FlecsVariant] Storage in RID_Owner compatibility") {
		// This test verifies variants can be used with RID_Owner
		// by checking they are copyable/movable
		
		FlecsWorldVariant world_var1;
		FlecsWorldVariant world_var2 = world_var1; // Copy
		FlecsWorldVariant world_var3 = std::move(world_var2); // Move

		CHECK(world_var1.get_world().is_valid());
		CHECK(world_var3.get_world().is_valid());

		flecs::world temp_world;
		flecs::entity entity = temp_world.entity("Test");
		
		FlecsEntityVariant entity_var1(entity);
		FlecsEntityVariant entity_var2 = entity_var1; // Copy
		FlecsEntityVariant entity_var3 = std::move(entity_var2); // Move

		CHECK(entity_var1.is_valid());
		CHECK(entity_var3.is_valid());
	}
}

} // namespace TestFlecsVariant

#endif // TEST_FLECS_VARIANT_H