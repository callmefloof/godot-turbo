/**************************************************************************/
/*  test_scene_object_utility.h                                           */
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

#ifndef TEST_SCENE_OBJECT_UTILITY_H
#define TEST_SCENE_OBJECT_UTILITY_H

#include "tests/test_macros.h"
#include "test_fixtures.h"
#include "modules/godot_turbo/ecs/systems/utility/scene_object_utility.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "scene/main/node.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/2d/mesh_instance_2d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/camera_3d.h"
#include "scene/2d/camera_2d.h"
#include "scene/3d/light_3d.h"
#include "scene/2d/light_2d.h"
#include "scene/3d/physics/area_3d.h"
#include "scene/2d/physics/area_2d.h"

namespace TestSceneObjectUtility {

TEST_CASE("[SceneObjectUtility] Singleton access") {
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	CHECK(util != nullptr);
	
	// Should return same instance
	SceneObjectUtility *util2 = SceneObjectUtility::get_singleton();
	CHECK(util == util2);
}

TEST_CASE("[SceneObjectUtility] Create entity from null node") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	
	TypedArray<RID> entities = util->create_entity(world_id, nullptr);
	CHECK(entities.size() == 0);
	
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from basic Node") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Node *node = memnew(Node);
	node->set_name("TestNode");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	RID entity_rid = entities[0];
	CHECK(entity_rid.is_valid());
	
	// Verify entity exists and has SceneNodeComponent
	flecs::entity entity = server->_get_entity(world_id, entity_rid);
	CHECK(entity.is_valid());
	CHECK(entity.has<SceneNodeComponent>());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from Node3D") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Node3D *node = memnew(Node3D);
	node->set_name("TestNode3D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	CHECK(entity.has<SceneNodeComponent>());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from Node2D") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Node2D *node = memnew(Node2D);
	node->set_name("TestNode2D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	CHECK(entity.has<SceneNodeComponent>());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from MeshInstance3D") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	MeshInstance3D *node = memnew(MeshInstance3D);
	node->set_name("TestMesh3D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from Camera3D") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Camera3D *node = memnew(Camera3D);
	node->set_name("TestCamera3D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from DirectionalLight3D") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	DirectionalLight3D *node = memnew(DirectionalLight3D);
	node->set_name("TestLight3D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entity from Area3D") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Area3D *node = memnew(Area3D);
	node->set_name("TestArea3D");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Recursive entity creation - simple hierarchy") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create hierarchy: Root -> Child1 -> GrandChild
	Node *root = memnew(Node);
	root->set_name("Root");
	
	Node *child1 = memnew(Node);
	child1->set_name("Child1");
	root->add_child(child1);
	
	Node *grandchild = memnew(Node);
	grandchild->set_name("GrandChild");
	child1->add_child(grandchild);
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array);
	
	// Should create entities for all 3 nodes
	CHECK(entities.size() >= 3);
	
	// Verify all entities are valid
	for (int i = 0; i < entities.size(); i++) {
		RID entity_rid = entities[i];
		CHECK(entity_rid.is_valid());
		
		flecs::entity entity = server->_get_entity(world_id, entity_rid);
		CHECK(entity.is_valid());
	}
	
	memdelete(root);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Recursive entity creation - max depth limit") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create deep hierarchy
	Node *root = memnew(Node);
	root->set_name("Root");
	
	Node *current = root;
	for (int i = 0; i < 10; i++) {
		Node *child = memnew(Node);
		child->set_name(vformat("Level%d", i));
		current->add_child(child);
		current = child;
	}
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	
	// Test with max_depth = 5 (should stop at depth 5)
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array, 0, 5);
	
	// Should create entities for root + 5 levels = 6 nodes
	CHECK(entities.size() == 6);
	
	memdelete(root);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Recursive entity creation - mixed node types") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create hierarchy with different node types
	Node3D *root = memnew(Node3D);
	root->set_name("Root3D");
	
	Camera3D *camera = memnew(Camera3D);
	camera->set_name("Camera");
	root->add_child(camera);
	
	DirectionalLight3D *light = memnew(DirectionalLight3D);
	light->set_name("Light");
	root->add_child(light);
	
	MeshInstance3D *mesh = memnew(MeshInstance3D);
	mesh->set_name("Mesh");
	root->add_child(mesh);
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array);
	
	// Should create entities for all 4 nodes (root + 3 children)
	CHECK(entities.size() >= 4);
	
	memdelete(root);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Child entity fix verification") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create parent with children to verify the bug fix
	Node *parent = memnew(Node);
	parent->set_name("Parent");
	
	Node *child1 = memnew(Node);
	child1->set_name("Child1");
	parent->add_child(child1);
	
	Node *child2 = memnew(Node);
	child2->set_name("Child2");
	parent->add_child(child2);
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, parent, empty_array);
	
	// Should have parent + 2 children = 3 entities minimum
	// This verifies the fix where child_entity_result was not being appended
	CHECK(entities.size() >= 3);
	
	memdelete(parent);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Create entities with accumulator") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Node *node1 = memnew(Node);
	node1->set_name("Node1");
	
	Node *node2 = memnew(Node);
	node2->set_name("Node2");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	
	// Create first entity
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, node1, empty_array);
	int first_count = entities.size();
	
	// Create second entity, accumulating into existing array
	entities = util->create_entities(world_id, node2, entities);
	
	// Should have accumulated both entities
	CHECK(entities.size() >= first_count + 1);
	
	memdelete(node1);
	memdelete(node2);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Invalid world ID") {
	Node *node = memnew(Node);
	node->set_name("Test");
	
	RID invalid_world;
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	
	TypedArray<RID> entities = util->create_entity(invalid_world, node);
	
	// Should return empty array on error
	CHECK(entities.size() == 0);
	
	memdelete(node);
}

TEST_CASE("[SceneObjectUtility] Large hierarchy stress test") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create a wide hierarchy (1 root + 50 children)
	Node *root = memnew(Node);
	root->set_name("Root");
	
	for (int i = 0; i < 50; i++) {
		Node *child = memnew(Node);
		child->set_name(vformat("Child%d", i));
		root->add_child(child);
	}
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array);
	
	// Should create 51 entities (root + 50 children)
	CHECK(entities.size() >= 51);
	
	// Verify all are valid
	for (int i = 0; i < entities.size(); i++) {
		RID entity_rid = entities[i];
		CHECK(entity_rid.is_valid());
	}
	
	memdelete(root);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Deep hierarchy stress test") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create a deep hierarchy (100 levels)
	Node *root = memnew(Node);
	root->set_name("Root");
	
	Node *current = root;
	const int DEPTH = 100;
	
	for (int i = 0; i < DEPTH; i++) {
		Node *child = memnew(Node);
		child->set_name(vformat("Level%d", i));
		current->add_child(child);
		current = child;
	}
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array, 0, DEPTH + 10);
	
	// Should create DEPTH + 1 entities (root + all children)
	CHECK(entities.size() >= DEPTH + 1);
	
	memdelete(root);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Node class name stored") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	Node *node = memnew(Node);
	node->set_name("MyCustomNodeName");
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> entities = util->create_entity(world_id, node);
	
	CHECK(entities.size() >= 1);
	
	flecs::entity entity = server->_get_entity(world_id, entities[0]);
	CHECK(entity.is_valid());
	
	const SceneNodeComponent& snc = entity.get<SceneNodeComponent>();
	CHECK(snc.class_name == "Node");
	
	memdelete(node);
	server->free_world(world_id);
}

TEST_CASE("[SceneObjectUtility] Multiple node types in single conversion") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();
	
	// Create a complex scene with various node types
	Node *root = memnew(Node);
	root->set_name("SceneRoot");
	
	// Add 3D nodes
	Node3D *spatial = memnew(Node3D);
	spatial->set_name("Spatial");
	root->add_child(spatial);
	
	Camera3D *camera = memnew(Camera3D);
	camera->set_name("Camera");
	spatial->add_child(camera);
	
	// Add 2D nodes
	Node2D *node2d = memnew(Node2D);
	node2d->set_name("Node2D");
	root->add_child(node2d);
	
	Camera2D *camera2d = memnew(Camera2D);
	camera2d->set_name("Camera2D");
	node2d->add_child(camera2d);
	
	SceneObjectUtility *util = SceneObjectUtility::get_singleton();
	TypedArray<RID> empty_array;
	TypedArray<RID> entities = util->create_entities(world_id, root, empty_array);
	
	// Should have created entities for all nodes
	CHECK(entities.size() >= 5);
	
	// Verify all entities are valid
	for (int i = 0; i < entities.size(); i++) {
		flecs::entity entity = server->_get_entity(world_id, entities[i]);
		CHECK(entity.is_valid());
	}
	
	memdelete(root);
	server->free_world(world_id);
}

} // namespace TestSceneObjectUtility

#endif // TEST_SCENE_OBJECT_UTILITY_H