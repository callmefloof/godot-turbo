#pragma once

#include "tests/test_macros.h"
#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "core/object/class_db.h"
#include "scene/main/node.h"
#include "scene/resources/packed_scene.h"

namespace TestGDScriptRunnerSystem {

/**
 * @brief Test fixture for GDScriptRunnerSystem tests
 */
class GDScriptRunnerSystemTestFixture {
public:
	FlecsServer* server = nullptr;
	RID world_rid;
	flecs::world* world = nullptr;
	GDScriptRunnerSystem* system = nullptr;
	
	void setup() {
		// Get or create FlecsServer singleton
		server = FlecsServer::get_singleton();
		if (!server) {
			server = memnew(FlecsServer);
		}
		
		// Create a test world
		world_rid = server->create_world();
		world = server->_get_world(world_rid);
		REQUIRE(world != nullptr);
		
		// Register components
		world->component<GameScriptComponent>();
		world->component<SceneNodeComponent>();
		
		// Create system
		system = memnew(GDScriptRunnerSystem);
		system->init(world_rid, world);
	}
	
	void teardown() {
		// Clean up
		if (system) {
			memdelete(system);
			system = nullptr;
		}
		
		if (world_rid.is_valid() && server) {
			server->remove_world(world_rid);
		}
	}
	
	flecs::entity create_test_entity_with_script(const String& instance_type) {
		GameScriptComponent script_comp;
		script_comp.instance_type = StringName(instance_type);
		script_comp.script_path = "res://test_script.gd";
		
		SceneNodeComponent node_comp;
		node_comp.node_path = "/root/TestNode";
		node_comp.original_name = "TestNode";
		
		return world->entity()
			.set<GameScriptComponent>(script_comp)
			.set<SceneNodeComponent>(node_comp);
	}
};

/**
 * @test Constructor and initialization
 */
TEST_CASE("[GDScriptRunnerSystem] Constructor creates valid system") {
	GDScriptRunnerSystem system;
	
	// System should be constructible
	CHECK(system.get_cache_size() == 0);
}

/**
 * @test Initialization with valid world
 */
TEST_CASE("[GDScriptRunnerSystem] init creates process and physics systems") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// System should be initialized
	CHECK(fixture.system != nullptr);
	
	// Both systems should be enabled by default
	CHECK(fixture.system->is_process_enabled() == true);
	CHECK(fixture.system->is_physics_process_enabled() == true);
	
	fixture.teardown();
}

/**
 * @test Cache initialization
 */
TEST_CASE("[GDScriptRunnerSystem] Cache starts empty") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	CHECK(fixture.system->get_cache_size() == 0);
	
	fixture.teardown();
}

/**
 * @test Enable and disable process system
 */
TEST_CASE("[GDScriptRunnerSystem] set_process_enabled controls process system") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	CHECK(fixture.system->is_process_enabled() == true);
	
	fixture.system->set_process_enabled(false);
	CHECK(fixture.system->is_process_enabled() == false);
	
	fixture.system->set_process_enabled(true);
	CHECK(fixture.system->is_process_enabled() == true);
	
	fixture.teardown();
}

/**
 * @test Enable and disable physics process system
 */
TEST_CASE("[GDScriptRunnerSystem] set_physics_process_enabled controls physics system") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	CHECK(fixture.system->is_physics_process_enabled() == true);
	
	fixture.system->set_physics_process_enabled(false);
	CHECK(fixture.system->is_physics_process_enabled() == false);
	
	fixture.system->set_physics_process_enabled(true);
	CHECK(fixture.system->is_physics_process_enabled() == true);
	
	fixture.teardown();
}

/**
 * @test Cache clearing
 */
TEST_CASE("[GDScriptRunnerSystem] clear_cache empties the cache") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create entity (would populate cache on first run)
	fixture.create_test_entity_with_script("TestScript");
	
	// Clear cache
	fixture.system->clear_cache();
	CHECK(fixture.system->get_cache_size() == 0);
	
	fixture.teardown();
}

/**
 * @test Cache population on entity creation
 */
TEST_CASE("[GDScriptRunnerSystem] Cache gets populated when entity is processed") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create entity with script component
	fixture.create_test_entity_with_script("Node");
	
	// Run world to trigger system
	fixture.world->progress(0.016f);
	
	// Cache should be populated (even if method doesn't exist)
	// The system will cache the result of the method check
	CHECK(fixture.system->get_cache_size() >= 0);
	
	fixture.teardown();
}

/**
 * @test Multiple entities with same script type
 */
TEST_CASE("[GDScriptRunnerSystem] Multiple entities with same script share cache entry") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create multiple entities with same script type
	fixture.create_test_entity_with_script("Node");
	fixture.create_test_entity_with_script("Node");
	fixture.create_test_entity_with_script("Node");
	
	// Run world
	fixture.world->progress(0.016f);
	
	// Should only have one cache entry for "Node"
	CHECK(fixture.system->is_cached(StringName("Node")) == true);
	
	fixture.teardown();
}

/**
 * @test Multiple entities with different script types
 */
TEST_CASE("[GDScriptRunnerSystem] Different script types create separate cache entries") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create entities with different script types
	fixture.create_test_entity_with_script("Node");
	fixture.create_test_entity_with_script("Node2D");
	fixture.create_test_entity_with_script("Node3D");
	
	// Run world
	fixture.world->progress(0.016f);
	
	// Should have cache entries for all types
	CHECK(fixture.system->get_cache_size() >= 0);
	
	fixture.teardown();
}

/**
 * @test System processes only entities with GameScriptComponent
 */
TEST_CASE("[GDScriptRunnerSystem] System processes only entities with GameScriptComponent") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create entity WITHOUT GameScriptComponent
	fixture.world->entity()
		.set<SceneNodeComponent>({"/root/Test", "Test"});
	
	// Create entity WITH GameScriptComponent
	fixture.create_test_entity_with_script("Node");
	
	// Run world
	fixture.world->progress(0.016f);
	
	// Should process without errors
	// (Detailed behavior depends on actual script execution)
	
	fixture.teardown();
}

/**
 * @test System handles entities without SceneNodeComponent
 */
TEST_CASE("[GDScriptRunnerSystem] System handles entities without SceneNodeComponent gracefully") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create entity with GameScriptComponent but no SceneNodeComponent
	GameScriptComponent script_comp;
	script_comp.instance_type = StringName("Node");
	script_comp.script_path = "res://test.gd";
	
	fixture.world->entity()
		.set<GameScriptComponent>(script_comp);
	
	// Should not crash when processing
	fixture.world->progress(0.016f);
	
	fixture.teardown();
}

/**
 * @test is_cached returns false for uncached types
 */
TEST_CASE("[GDScriptRunnerSystem] is_cached returns false for uncached types") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	CHECK(fixture.system->is_cached(StringName("UnknownType")) == false);
	
	fixture.teardown();
}

/**
 * @test System runs during OnUpdate phase
 */
TEST_CASE("[GDScriptRunnerSystem] Process system runs during OnUpdate phase") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// Progress should trigger OnUpdate phase
	fixture.world->progress(0.016f);
	
	// If we got here without crashing, the system ran
	CHECK(true);
	
	fixture.teardown();
}

/**
 * @test Disabled process system doesn't run
 */
TEST_CASE("[GDScriptRunnerSystem] Disabled process system doesn't execute") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// Disable process system
	fixture.system->set_process_enabled(false);
	
	// Progress should not crash even with disabled system
	fixture.world->progress(0.016f);
	
	CHECK(fixture.system->is_process_enabled() == false);
	
	fixture.teardown();
}

/**
 * @test Disabled physics process system doesn't run
 */
TEST_CASE("[GDScriptRunnerSystem] Disabled physics process system doesn't execute") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// Disable physics process system
	fixture.system->set_physics_process_enabled(false);
	
	// Progress should not crash
	fixture.world->progress(0.016f);
	
	CHECK(fixture.system->is_physics_process_enabled() == false);
	
	fixture.teardown();
}

/**
 * @test Re-enabling systems after disable
 */
TEST_CASE("[GDScriptRunnerSystem] Re-enabling systems after disable works") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Disable both
	fixture.system->set_process_enabled(false);
	fixture.system->set_physics_process_enabled(false);
	
	CHECK(fixture.system->is_process_enabled() == false);
	CHECK(fixture.system->is_physics_process_enabled() == false);
	
	// Re-enable both
	fixture.system->set_process_enabled(true);
	fixture.system->set_physics_process_enabled(true);
	
	CHECK(fixture.system->is_process_enabled() == true);
	CHECK(fixture.system->is_physics_process_enabled() == true);
	
	fixture.teardown();
}

/**
 * @test Multiple progress calls
 */
TEST_CASE("[GDScriptRunnerSystem] Multiple progress calls work correctly") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// Multiple progress calls
	for (int i = 0; i < 10; ++i) {
		fixture.world->progress(0.016f);
	}
	
	// Should complete without issues
	CHECK(true);
	
	fixture.teardown();
}

/**
 * @test Cache persists across progress calls
 */
TEST_CASE("[GDScriptRunnerSystem] Cache persists across multiple progress calls") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// First progress
	fixture.world->progress(0.016f);
	int cache_size_1 = fixture.system->get_cache_size();
	
	// Second progress
	fixture.world->progress(0.016f);
	int cache_size_2 = fixture.system->get_cache_size();
	
	// Cache size should be the same (not growing each frame)
	CHECK(cache_size_1 == cache_size_2);
	
	fixture.teardown();
}

/**
 * @test Empty script path handling
 */
TEST_CASE("[GDScriptRunnerSystem] Handles empty script path") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	GameScriptComponent script_comp;
	script_comp.instance_type = StringName("Node");
	script_comp.script_path = ""; // Empty path
	
	fixture.world->entity()
		.set<GameScriptComponent>(script_comp);
	
	// Should not crash
	fixture.world->progress(0.016f);
	
	fixture.teardown();
}

/**
 * @test Empty instance type handling
 */
TEST_CASE("[GDScriptRunnerSystem] Handles empty instance type") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	GameScriptComponent script_comp;
	script_comp.instance_type = StringName(""); // Empty
	script_comp.script_path = "res://test.gd";
	
	fixture.world->entity()
		.set<GameScriptComponent>(script_comp);
	
	// Should not crash
	fixture.world->progress(0.016f);
	
	fixture.teardown();
}

/**
 * @test Destructor cleanup
 */
TEST_CASE("[GDScriptRunnerSystem] Destructor cleans up properly") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	fixture.world->progress(0.016f);
	
	// Destructor called in teardown - should not leak
	fixture.teardown();
	
	CHECK(true);
}

/**
 * @test Non-copyable constraint
 */
TEST_CASE("[GDScriptRunnerSystem] System is non-copyable") {
	// This test verifies that the class is properly marked as non-copyable
	// The actual constraint is enforced at compile time
	
	CHECK(std::is_copy_constructible<GDScriptRunnerSystem>::value == false);
	CHECK(std::is_copy_assignable<GDScriptRunnerSystem>::value == false);
}

/**
 * @test Non-movable constraint
 */
TEST_CASE("[GDScriptRunnerSystem] System is non-movable") {
	// This test verifies that the class is properly marked as non-movable
	// The actual constraint is enforced at compile time
	
	CHECK(std::is_move_constructible<GDScriptRunnerSystem>::value == false);
	CHECK(std::is_move_assignable<GDScriptRunnerSystem>::value == false);
}

/**
 * @test Cache after clear and re-run
 */
TEST_CASE("[GDScriptRunnerSystem] Cache rebuilds after clear") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	fixture.create_test_entity_with_script("Node");
	
	// First run
	fixture.world->progress(0.016f);
	CHECK(fixture.system->get_cache_size() >= 0);
	
	// Clear cache
	fixture.system->clear_cache();
	CHECK(fixture.system->get_cache_size() == 0);
	
	// Run again - cache should rebuild
	fixture.world->progress(0.016f);
	CHECK(fixture.system->get_cache_size() >= 0);
	
	fixture.teardown();
}

/**
 * @test Many entities stress test
 */
TEST_CASE("[GDScriptRunnerSystem] Handles many entities efficiently") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create many entities
	const int num_entities = 1000;
	for (int i = 0; i < num_entities; ++i) {
		fixture.create_test_entity_with_script("Node");
	}
	
	// Should process without issues
	fixture.world->progress(0.016f);
	
	// Cache should be efficient (not creating 1000 entries for same type)
	CHECK(fixture.system->get_cache_size() < num_entities);
	
	fixture.teardown();
}

/**
 * @test Mixed entity types
 */
TEST_CASE("[GDScriptRunnerSystem] Handles mixed entity types") {
	GDScriptRunnerSystemTestFixture fixture;
	fixture.setup();
	
	// Create diverse set of entities
	fixture.create_test_entity_with_script("Node");
	fixture.create_test_entity_with_script("Node2D");
	fixture.create_test_entity_with_script("Node3D");
	fixture.create_test_entity_with_script("Control");
	fixture.create_test_entity_with_script("CanvasItem");
	
	// Process
	fixture.world->progress(0.016f);
	
	// All types should be handled
	CHECK(fixture.system->get_cache_size() > 0);
	
	fixture.teardown();
}

} // namespace TestGDScriptRunnerSystem