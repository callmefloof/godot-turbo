#pragma once

#include "tests/test_macros.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "core/object/class_db.h"

namespace TestPipelineManager {

/**
 * @brief Test fixture for PipelineManager tests
 */
class PipelineManagerTestFixture {
public:
	FlecsServer* server = nullptr;
	RID world_rid;
	flecs::world* world = nullptr;
	
	void setup() {
		// Get or create FlecsServer singleton
		server = FlecsServer::get_singleton();
		if (!server) {
			// In test context, create a temporary server
			server = memnew(FlecsServer);
		}
		
		// Create a test world
		world_rid = server->create_world();
		world = server->_get_world(world_rid);
		REQUIRE(world != nullptr);
	}
	
	void teardown() {
		// Clean up the test world
		if (world_rid.is_valid() && server) {
			server->remove_world(world_rid);
		}
	}
};

// Helper component for test systems
struct TestComponent {
	int value = 0;
};

/**
 * @test Constructor and world association
 */
TEST_CASE("[PipelineManager] Constructor initializes with valid world") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	CHECK(manager.get_world() == fixture.world_rid);
	
	fixture.teardown();
}

/**
 * @test Default constructor
 */
TEST_CASE("[PipelineManager] Default constructor creates uninitialized manager") {
	PipelineManager manager;
	
	// Default RID should be invalid
	CHECK(!manager.get_world().is_valid());
}

/**
 * @test set_world changes world association
 */
TEST_CASE("[PipelineManager] set_world updates world association") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager;
	CHECK(!manager.get_world().is_valid());
	
	manager.set_world(fixture.world_rid);
	CHECK(manager.get_world() == fixture.world_rid);
	
	fixture.teardown();
}

/**
 * @test Copy constructor
 */
TEST_CASE("[PipelineManager] Copy constructor duplicates state") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager original(fixture.world_rid);
	
	// Register a test component
	fixture.world->component<TestComponent>();
	
	// Create a simple system
	flecs::system test_system = fixture.world->system<TestComponent>()
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {
			// Simple test system
			for (auto i : it) {
				comps[i].value++;
			}
		});
	
	original.add_to_pipeline(test_system);
	
	// Copy
	PipelineManager copy(original);
	
	CHECK(copy.get_world() == original.get_world());
	
	fixture.teardown();
}

/**
 * @test Move constructor
 */
TEST_CASE("[PipelineManager] Move constructor transfers ownership") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager original(fixture.world_rid);
	RID original_world = original.get_world();
	
	PipelineManager moved(std::move(original));
	
	CHECK(moved.get_world() == original_world);
	
	fixture.teardown();
}

/**
 * @test Copy assignment operator
 */
TEST_CASE("[PipelineManager] Copy assignment operator duplicates state") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager original(fixture.world_rid);
	PipelineManager copy;
	
	copy = original;
	
	CHECK(copy.get_world() == original.get_world());
	
	fixture.teardown();
}

/**
 * @test Move assignment operator
 */
TEST_CASE("[PipelineManager] Move assignment operator transfers ownership") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager original(fixture.world_rid);
	RID original_world = original.get_world();
	
	PipelineManager moved;
	moved = std::move(original);
	
	CHECK(moved.get_world() == original_world);
	
	fixture.teardown();
}

/**
 * @test Self-assignment (copy)
 */
TEST_CASE("[PipelineManager] Self-assignment (copy) is safe") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	RID original_world = manager.get_world();
	
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
	manager = manager;
	#pragma GCC diagnostic pop
	
	CHECK(manager.get_world() == original_world);
	
	fixture.teardown();
}

/**
 * @test Self-assignment (move)
 */
TEST_CASE("[PipelineManager] Self-assignment (move) is safe") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	RID original_world = manager.get_world();
	
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wself-move"
	manager = std::move(manager);
	#pragma GCC diagnostic pop
	
	CHECK(manager.get_world() == original_world);
	
	fixture.teardown();
}

/**
 * @test Adding system with default phase
 */
TEST_CASE("[PipelineManager] add_to_pipeline with default phase") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create a named system
	flecs::system test_system = fixture.world->system<TestComponent>("TestSystem")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value++;
			}
		});
	
	// Add to pipeline (default phase)
	manager.add_to_pipeline(test_system);
	
	// Verify system can be found
	flecs::system* found = manager.try_get_system("TestSystem");
	CHECK(found != nullptr);
	if (found) {
		CHECK(found->name() == std::string("TestSystem"));
	}
	
	fixture.teardown();
}

/**
 * @test Adding system with specific phase
 */
TEST_CASE("[PipelineManager] add_to_pipeline with specific phase") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create a named system for OnPhysicsUpdate phase
	flecs::system physics_system = fixture.world->system<TestComponent>("PhysicsSystem")
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value += 2;
			}
		});
	
	// Add to pipeline with specific phase
	manager.add_to_pipeline(physics_system, flecs::OnPhysicsUpdate);
	
	// Verify system can be found
	flecs::system* found = manager.try_get_system("PhysicsSystem");
	CHECK(found != nullptr);
	
	fixture.teardown();
}

/**
 * @test Adding multiple systems
 */
TEST_CASE("[PipelineManager] add_to_pipeline with multiple systems") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create multiple systems
	flecs::system system1 = fixture.world->system<TestComponent>("System1")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {});
	
	flecs::system system2 = fixture.world->system<TestComponent>("System2")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {});
	
	flecs::system system3 = fixture.world->system<TestComponent>("System3")
		.kind(flecs::OnPhysicsUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {});
	
	// Add all systems
	manager.add_to_pipeline(system1);
	manager.add_to_pipeline(system2);
	manager.add_to_pipeline(system3, flecs::OnPhysicsUpdate);
	
	// Verify all can be found
	CHECK(manager.try_get_system("System1") != nullptr);
	CHECK(manager.try_get_system("System2") != nullptr);
	CHECK(manager.try_get_system("System3") != nullptr);
	
	fixture.teardown();
}

/**
 * @test System lookup with non-existent name
 */
TEST_CASE("[PipelineManager] try_get_system returns nullptr for non-existent system") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	flecs::system* found = manager.try_get_system("NonExistentSystem");
	CHECK(found == nullptr);
	
	fixture.teardown();
}

/**
 * @test Creating custom phase without dependency
 */
TEST_CASE("[PipelineManager] create_custom_phase without dependency") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	flecs::entity custom_phase = manager.create_custom_phase("CustomPhase");
	
	CHECK(custom_phase.is_valid());
	CHECK(custom_phase.name() == std::string("CustomPhase"));
	
	fixture.teardown();
}

/**
 * @test Creating custom phase with dependency
 */
TEST_CASE("[PipelineManager] create_custom_phase with dependency") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Create a custom phase that depends on OnUpdate
	flecs::entity custom_phase = manager.create_custom_phase("LateUpdate", "OnUpdate");
	
	CHECK(custom_phase.is_valid());
	CHECK(custom_phase.name() == std::string("LateUpdate"));
	
	fixture.teardown();
}

/**
 * @test Using custom phase with system
 */
TEST_CASE("[PipelineManager] Using custom phase with system") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Create custom phase
	flecs::entity custom_phase = manager.create_custom_phase("CustomLogic", "OnUpdate");
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create system for custom phase
	flecs::system custom_system = fixture.world->system<TestComponent>("CustomSystem")
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value += 10;
			}
		});
	
	// Add system to custom phase
	manager.add_to_pipeline(custom_system, custom_phase);
	
	// Verify system can be found
	flecs::system* found = manager.try_get_system("CustomSystem");
	CHECK(found != nullptr);
	
	fixture.teardown();
}

/**
 * @test System execution order (integration test)
 */
TEST_CASE("[PipelineManager] Systems execute in pipeline") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create an entity with the component
	flecs::entity entity = fixture.world->entity()
		.set<TestComponent>({0});
	
	// Create a system that increments the value
	flecs::system increment_system = fixture.world->system<TestComponent>("IncrementSystem")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value++;
			}
		});
	
	manager.add_to_pipeline(increment_system);
	
	// Run the world
	fixture.world->progress(0.016f); // 16ms tick
	
	// Verify the system ran
	const TestComponent* comp = entity.get<TestComponent>();
	CHECK(comp->value == 1);
	
	// Run again
	fixture.world->progress(0.016f);
	CHECK(entity.get<TestComponent>()->value == 2);
	
	fixture.teardown();
}

/**
 * @test Multiple systems execution order
 */
TEST_CASE("[PipelineManager] Multiple systems execute in order") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	// Register component
	fixture.world->component<TestComponent>();
	
	// Create an entity
	flecs::entity entity = fixture.world->entity()
		.set<TestComponent>({1});
	
	// System 1: Multiply by 2
	flecs::system system1 = fixture.world->system<TestComponent>("MultiplySystem")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value *= 2;
			}
		});
	
	// System 2: Add 3
	flecs::system system2 = fixture.world->system<TestComponent>("AddSystem")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {
			for (auto i : it) {
				comps[i].value += 3;
			}
		});
	
	manager.add_to_pipeline(system1);
	manager.add_to_pipeline(system2);
	
	// Run the world
	fixture.world->progress(0.016f);
	
	// Value should be: (1 * 2) + 3 = 5
	const TestComponent* comp = entity.get<TestComponent>();
	CHECK(comp->value == 5);
	
	fixture.teardown();
}

/**
 * @test Changing world updates pipeline
 */
TEST_CASE("[PipelineManager] set_world updates pipeline reference") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	// Create a second world
	RID world2_rid = fixture.server->create_world();
	
	PipelineManager manager(fixture.world_rid);
	CHECK(manager.get_world() == fixture.world_rid);
	
	// Change to second world
	manager.set_world(world2_rid);
	CHECK(manager.get_world() == world2_rid);
	
	// Cleanup
	fixture.server->remove_world(world2_rid);
	fixture.teardown();
}

/**
 * @test Invalid world RID handling
 */
TEST_CASE("[PipelineManager] Handles invalid world RID gracefully") {
	RID invalid_rid;
	
	// Should not crash
	PipelineManager manager(invalid_rid);
	
	// World should be invalid
	CHECK(!manager.get_world().is_valid());
}

/**
 * @test System name lookup case sensitivity
 */
TEST_CASE("[PipelineManager] try_get_system is case-sensitive") {
	PipelineManagerTestFixture fixture;
	fixture.setup();
	
	PipelineManager manager(fixture.world_rid);
	
	fixture.world->component<TestComponent>();
	
	flecs::system test_system = fixture.world->system<TestComponent>("TestSystem")
		.kind(flecs::OnUpdate)
		.iter([](flecs::iter& it, TestComponent* comps) {});
	
	manager.add_to_pipeline(test_system);
	
	// Exact match should work
	CHECK(manager.try_get_system("TestSystem") != nullptr);
	
	// Case mismatch should fail
	CHECK(manager.try_get_system("testsystem") == nullptr);
	CHECK(manager.try_get_system("TESTSYSTEM") == nullptr);
	
	fixture.teardown();
}

} // namespace TestPipelineManager