/**************************************************************************/
/*  test_resource_object_utility.h                                        */
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

#ifndef TEST_RESOURCE_OBJECT_UTILITY_H
#define TEST_RESOURCE_OBJECT_UTILITY_H

#include "tests/test_macros.h"
#include "test_fixtures.h"
#include "modules/godot_turbo/ecs/systems/utility/resource_object_utility.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "core/io/resource.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/texture.h"

namespace TestResourceObjectUtility {

#ifndef DISABLE_THREADED_TESTS
struct ResourceThreadData {
	RID world_id;
	int thread_id;
	int resources_per_thread;
	Vector<RID> *entity_rids;
	Mutex *result_mutex;
	std::atomic<int> *entities_created;
};

static void thread_create_resources(void *p_userdata) {
	ResourceThreadData *data = static_cast<ResourceThreadData *>(p_userdata);
	for (int i = 0; i < data->resources_per_thread; i++) {
		Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
		material->set_name(vformat("Material_T%d_%d", data->thread_id, i));

		RID entity_rid = ResourceObjectUtility::create_resource_entity(data->world_id, material);

		data->result_mutex->lock();
		int idx = data->thread_id * data->resources_per_thread + i;
		data->entity_rids->write[idx] = entity_rid;
		
		if (entity_rid.is_valid()) {
			data->entities_created->fetch_add(1);
		}
		data->result_mutex->unlock();
	}
}
#endif // DISABLE_THREADED_TESTS

TEST_CASE("[ResourceObjectUtility] Basic resource entity creation") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	CHECK(server != nullptr);

	RID world_id = server->create_world();
	CHECK(world_id.is_valid());

	// Create a simple resource
	Ref<Resource> resource = memnew(Resource);
	resource->set_name("TestResource");

	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, resource);
	CHECK(entity_rid.is_valid());

	// Verify entity exists
	flecs::world *world = server->_get_world(world_id);
	CHECK(world != nullptr);

	flecs::entity entity = server->_get_entity(world_id, entity_rid);
	CHECK(entity.is_valid());
	CHECK(entity.has<ResourceComponent>());

	// Verify component data
	const ResourceComponent& rc = entity.get<ResourceComponent>();
	CHECK(rc.resource_type == "Resource");
	CHECK(rc.resource_name == "TestResource");

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Invalid world ID") {
	Ref<Resource> resource = memnew(Resource);
	resource->set_name("Test");

	RID invalid_world;
	RID entity_rid = ResourceObjectUtility::create_resource_entity(invalid_world, resource);
	CHECK_FALSE(entity_rid.is_valid());
}

TEST_CASE("[ResourceObjectUtility] Null resource") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<Resource> null_resource;
	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, null_resource);
	CHECK_FALSE(entity_rid.is_valid());

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Resource without RID") {
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	// Base Resource class doesn't create a server-side RID automatically
	Ref<Resource> resource = memnew(Resource);
	resource->set_name("NoRID");

	// This should fail gracefully since Resource has no RID
	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, resource);
	CHECK_FALSE(entity_rid.is_valid());

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Material resource") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	material->set_name("TestMaterial");

	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, material);
	CHECK(entity_rid.is_valid());

	flecs::entity entity = server->_get_entity(world_id, entity_rid);
	CHECK(entity.is_valid());
	CHECK(entity.has<ResourceComponent>());

	const ResourceComponent& rc = entity.get<ResourceComponent>();
	CHECK(rc.resource_type == "StandardMaterial3D");
	CHECK(rc.resource_name == "TestMaterial");
	CHECK(rc.resource_id.is_valid());

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Resource with empty name") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	// Don't set name - should use class name as fallback

	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, material);
	CHECK(entity_rid.is_valid());

	flecs::entity entity = server->_get_entity(world_id, entity_rid);
	CHECK(entity.is_valid());

	const ResourceComponent& rc = entity.get<ResourceComponent>();
	CHECK(rc.resource_name.is_empty());
	CHECK(rc.resource_type == "StandardMaterial3D");

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Multiple resources with names") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<StandardMaterial3D> mat1 = memnew(StandardMaterial3D);
	mat1->set_name("Material1");

	Ref<StandardMaterial3D> mat2 = memnew(StandardMaterial3D);
	mat2->set_name("Material2");

	Ref<StandardMaterial3D> mat3 = memnew(StandardMaterial3D);
	mat3->set_name("Material3");

	RID entity1 = ResourceObjectUtility::create_resource_entity(world_id, mat1);
	RID entity2 = ResourceObjectUtility::create_resource_entity(world_id, mat2);
	RID entity3 = ResourceObjectUtility::create_resource_entity(world_id, mat3);

	CHECK(entity1.is_valid());
	CHECK(entity2.is_valid());
	CHECK(entity3.is_valid());

	// Verify all entities exist and have correct data
	flecs::entity e1 = server->_get_entity(world_id, entity1);
	flecs::entity e2 = server->_get_entity(world_id, entity2);
	flecs::entity e3 = server->_get_entity(world_id, entity3);

	const ResourceComponent& rc1 = e1.get<ResourceComponent>();
	const ResourceComponent& rc2 = e2.get<ResourceComponent>();
	const ResourceComponent& rc3 = e3.get<ResourceComponent>();
	
	CHECK(rc1.resource_name == "Material1");
	CHECK(rc2.resource_name == "Material2");
	CHECK(rc3.resource_name == "Material3");

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Resource with script") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	material->set_name("ScriptedMaterial");

	// Note: Testing actual script attachment would require loading a script file,
	// which is complex in unit tests. This tests the code path for when a script
	// is present, but we can't easily verify is_script_type flag without a real script.

	RID entity_rid = ResourceObjectUtility::create_resource_entity(world_id, material);
	CHECK(entity_rid.is_valid());

	flecs::entity entity = server->_get_entity(world_id, entity_rid);
	const ResourceComponent& rc = entity.get<ResourceComponent>();
	
	// Without a script attached, is_script_type should be false
	CHECK_FALSE(rc.is_script_type);

	server->free_world(world_id);
}

#ifndef DISABLE_THREADED_TESTS
TEST_CASE("[ResourceObjectUtility] Thread-safety - concurrent resource creation") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	const int THREAD_COUNT = 4;
	const int RESOURCES_PER_THREAD = 25;

	Vector<RID> entity_rids;
	entity_rids.resize(THREAD_COUNT * RESOURCES_PER_THREAD);

	Mutex result_mutex;
	std::atomic<int> entities_created(0);

	Thread threads[THREAD_COUNT];
	ResourceThreadData thread_data[THREAD_COUNT];

	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].world_id = world_id;
		thread_data[i].thread_id = i;
		thread_data[i].resources_per_thread = RESOURCES_PER_THREAD;
		thread_data[i].entity_rids = &entity_rids;
		thread_data[i].result_mutex = &result_mutex;
		thread_data[i].entities_created = &entities_created;
		threads[i].start(thread_create_resources, &thread_data[i]);
	}

	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	// Verify all entities were created
	CHECK(entities_created.load() == THREAD_COUNT * RESOURCES_PER_THREAD);

	// Verify all entities are valid and have correct components
	for (int i = 0; i < entity_rids.size(); i++) {
		CHECK(entity_rids[i].is_valid());
		
		flecs::entity entity = server->_get_entity(world_id, entity_rids[i]);
		CHECK(entity.is_valid());
		CHECK(entity.has<ResourceComponent>());
	}

	server->free_world(world_id);
}
#endif // DISABLE_THREADED_TESTS

TEST_CASE("[ResourceObjectUtility] Different resource types") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	// Test various resource types
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	material->set_name("Material");

	Ref<ArrayMesh> mesh = memnew(ArrayMesh);
	mesh->set_name("Mesh");

	RID mat_entity = ResourceObjectUtility::create_resource_entity(world_id, material);
	RID mesh_entity = ResourceObjectUtility::create_resource_entity(world_id, mesh);

	CHECK(mat_entity.is_valid());
	CHECK(mesh_entity.is_valid());

	flecs::entity mat_e = server->_get_entity(world_id, mat_entity);
	flecs::entity mesh_e = server->_get_entity(world_id, mesh_entity);

	const ResourceComponent& mat_rc = mat_e.get<ResourceComponent>();
	const ResourceComponent& mesh_rc = mesh_e.get<ResourceComponent>();
	
	CHECK(mat_rc.resource_type == "StandardMaterial3D");
	CHECK(mesh_rc.resource_type == "ArrayMesh");

	server->free_world(world_id);
}

TEST_CASE("[ResourceObjectUtility] Stress test - many resources") {
	REQUIRE_FLECS_SERVER();
	
	FlecsServer *server = FlecsServer::get_singleton();
	RID world_id = server->create_world();

	const int RESOURCE_COUNT = 500;
	Vector<RID> entity_rids;
	entity_rids.resize(RESOURCE_COUNT);

	for (int i = 0; i < RESOURCE_COUNT; i++) {
		Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
		material->set_name(vformat("Material_%d", i));

		entity_rids.write[i] = ResourceObjectUtility::create_resource_entity(world_id, material);
	}

	// Verify all entities
	int valid_count = 0;
	for (int i = 0; i < RESOURCE_COUNT; i++) {
		if (entity_rids[i].is_valid()) {
			flecs::entity entity = server->_get_entity(world_id, entity_rids[i]);
			if (entity.is_valid() && entity.has<ResourceComponent>()) {
				valid_count++;
			}
		}
	}

	CHECK(valid_count == RESOURCE_COUNT);

	server->free_world(world_id);
}

} // namespace TestResourceObjectUtility

#endif // TEST_RESOURCE_OBJECT_UTILITY_H