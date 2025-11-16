/**************************************************************************/
/*  test_ref_storage.h                                                    */
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

#pragma once

#include "modules/godot_turbo/ecs/systems/utility/ref_storage.h"
#include "test_fixtures.h"

#include "core/io/resource.h"
#include "core/os/thread.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "servers/rendering_server.h"
#include "tests/test_macros.h"

namespace TestRefStorage {

TEST_CASE("[RefStorage] Constructor and basic properties") {
	RefStorage storage;

	CHECK_MESSAGE(
			storage.size() == 0,
			"New RefStorage should be empty.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"New RefStorage should report as empty.");
}

TEST_CASE("[RefStorage] Add and retrieve single resource") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	
	// Create a material with a valid server RID
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	RID material_rid = RS::get_singleton()->material_create();

	bool added = storage.add(material, material_rid);

	CHECK_MESSAGE(
			added,
			"add() should return true for valid resource and RID.");
	CHECK_MESSAGE(
			storage.has(material_rid),
			"Storage should contain the added resource.");
	CHECK_MESSAGE(
			storage.size() == 1,
			"Storage size should be 1 after adding one resource.");
	CHECK_MESSAGE(
			!storage.is_empty(),
			"Storage should not be empty after adding a resource.");

	RefContainer *container = storage.get(material_rid);
	CHECK_MESSAGE(
			container != nullptr,
			"Retrieved container should not be null.");
	CHECK_MESSAGE(
			container->resource == material,
			"Retrieved resource should be the same as added resource.");
	CHECK_MESSAGE(
			container->rid == material_rid,
			"Retrieved RID should match the added RID.");
	CHECK_MESSAGE(
			container->class_name == "StandardMaterial3D",
			"Container should store correct class name.");
}

TEST_CASE("[RefStorage] Add multiple resources") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	
	// Create multiple materials with server RIDs
	Ref<StandardMaterial3D> mat1 = memnew(StandardMaterial3D);
	Ref<StandardMaterial3D> mat2 = memnew(StandardMaterial3D);
	Ref<StandardMaterial3D> mat3 = memnew(StandardMaterial3D);
	
	RID rid1 = RS::get_singleton()->material_create();
	RID rid2 = RS::get_singleton()->material_create();
	RID rid3 = RS::get_singleton()->material_create();

	bool added1 = storage.add(mat1, rid1);
	bool added2 = storage.add(mat2, rid2);
	bool added3 = storage.add(mat3, rid3);

	CHECK_MESSAGE(
			added1,
			"First add() call should succeed.");
	CHECK_MESSAGE(
			added2,
			"Second add() call should succeed.");
	CHECK_MESSAGE(
			added3,
			"Third add() call should succeed.");
	CHECK_MESSAGE(
			storage.size() == 3,
			"Storage should contain 3 resources.");
	CHECK_MESSAGE(
			storage.has(rid1),
			"Storage should contain first added resource.");
	CHECK_MESSAGE(
			storage.has(rid2),
			"Storage should contain second added resource.");
	CHECK_MESSAGE(
			storage.has(rid3),
			"Storage should contain third added resource.");

	RefContainer *container1 = storage.get(rid1);
	RefContainer *container2 = storage.get(rid2);
	RefContainer *container3 = storage.get(rid3);

	CHECK_MESSAGE(
			container1 != nullptr,
			"First retrieved container should be valid.");
	CHECK_MESSAGE(
			container2 != nullptr,
			"Second retrieved container should be valid.");
	CHECK_MESSAGE(
			container3 != nullptr,
			"Third retrieved container should be valid.");
	CHECK_MESSAGE(
			container1->resource == mat1,
			"First retrieved resource should match the added resource.");
	CHECK_MESSAGE(
			container2->resource == mat2,
			"Second retrieved resource should match the added resource.");
	CHECK_MESSAGE(
			container3->resource == mat3,
			"Third retrieved resource should match the added resource.");
}

TEST_CASE("[RefStorage] Release single resource") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	RID material_rid = RS::get_singleton()->material_create();
	storage.add(material, material_rid);

	CHECK_MESSAGE(
			storage.has(material_rid),
			"Storage should contain the resource before release.");

	bool released = storage.release(material_rid);

	CHECK_MESSAGE(
			released,
			"release() should return true for existing resource.");
	CHECK_MESSAGE(
			!storage.has(material_rid),
			"Storage should not contain the resource after release.");
	CHECK_MESSAGE(
			storage.size() == 0,
			"Storage should be empty after releasing the only resource.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should report as empty after releasing all resources.");
}

TEST_CASE("[RefStorage] Release nonexistent resource") {
	RefStorage storage;
	RID fake_rid;

	bool released = storage.release(fake_rid);

	CHECK_MESSAGE(
			!released,
			"release() should return false for nonexistent resource.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should remain empty after attempting to release nonexistent resource.");
}

TEST_CASE("[RefStorage] Release all resources") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	
	// Create multiple resources
	Ref<StandardMaterial3D> mat1 = memnew(StandardMaterial3D);
	Ref<StandardMaterial3D> mat2 = memnew(StandardMaterial3D);
	Ref<StandardMaterial3D> mat3 = memnew(StandardMaterial3D);
	
	RID rid1 = RS::get_singleton()->material_create();
	RID rid2 = RS::get_singleton()->material_create();
	RID rid3 = RS::get_singleton()->material_create();
	
	storage.add(mat1, rid1);
	storage.add(mat2, rid2);
	storage.add(mat3, rid3);

	CHECK_MESSAGE(
			storage.size() == 3,
			"Storage should contain 3 resources before release_all.");

	storage.release_all();

	CHECK_MESSAGE(
			storage.size() == 0,
			"Storage should be empty after release_all.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should report as empty after release_all.");
	CHECK_MESSAGE(
			!storage.has(rid1),
			"Storage should not contain first previously added resource.");
	CHECK_MESSAGE(
			!storage.has(rid2),
			"Storage should not contain second previously added resource.");
	CHECK_MESSAGE(
			!storage.has(rid3),
			"Storage should not contain third previously added resource.");
}

TEST_CASE("[RefStorage] Get nonexistent resource") {
	RefStorage storage;
	RID fake_rid;

	RefContainer *result = storage.get(fake_rid);

	CHECK_MESSAGE(
			result == nullptr,
			"get() should return nullptr for nonexistent RID.");
}

TEST_CASE("[RefStorage] Has with invalid RID") {
	RefStorage storage;
	RID invalid_rid;

	CHECK_MESSAGE(
			!storage.has(invalid_rid),
			"has() should return false for invalid RID.");
}

TEST_CASE("[RefStorage] Resource reference counting") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	RID material_rid = RS::get_singleton()->material_create();

	// Resource should have 1 reference (our Ref)
	int initial_refcount = material->get_reference_count();

	storage.add(material, material_rid);

	// Storage should increase reference count
	CHECK_MESSAGE(
			material->get_reference_count() > initial_refcount,
			"Storage should increase resource reference count.");

	storage.release(material_rid);

	// Reference count should return to initial
	CHECK_MESSAGE(
			material->get_reference_count() == initial_refcount,
			"Reference count should return to initial after release.");
}

TEST_CASE("[RefStorage] Add null resource") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	Ref<Resource> null_ref;
	RID valid_rid = RS::get_singleton()->material_create();

	bool added = storage.add(null_ref, valid_rid);

	CHECK_MESSAGE(
			!added,
			"Adding null resource should return false.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should remain empty after attempting to add null resource.");
	
	// Clean up the unused RID
	RS::get_singleton()->free(valid_rid);
}

TEST_CASE("[RefStorage] Add with invalid RID") {
	RefStorage storage;
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	RID invalid_rid;

	bool added = storage.add(material, invalid_rid);

	CHECK_MESSAGE(
			!added,
			"Adding with invalid RID should return false.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should remain empty after attempting to add with invalid RID.");
}

TEST_CASE("[RefStorage] Move semantics") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage1;
	
	Ref<StandardMaterial3D> mat1 = memnew(StandardMaterial3D);
	Ref<StandardMaterial3D> mat2 = memnew(StandardMaterial3D);
	RID rid1 = RS::get_singleton()->material_create();
	RID rid2 = RS::get_singleton()->material_create();
	
	storage1.add(mat1, rid1);
	storage1.add(mat2, rid2);

	CHECK_MESSAGE(
			storage1.size() == 2,
			"Original storage should contain 2 resources.");

	// Move constructor
	RefStorage storage2(std::move(storage1));

	CHECK_MESSAGE(
			storage2.size() == 2,
			"Moved-to storage should contain 2 resources.");
	CHECK_MESSAGE(
			storage2.has(rid1),
			"Moved-to storage should contain first original resource.");
	CHECK_MESSAGE(
			storage2.has(rid2),
			"Moved-to storage should contain second original resource.");

	// Move assignment
	RefStorage storage3;
	storage3 = std::move(storage2);

	CHECK_MESSAGE(
			storage3.size() == 2,
			"Move-assigned storage should contain 2 resources.");
	CHECK_MESSAGE(
			storage3.has(rid1),
			"Move-assigned storage should contain first original resource.");
	CHECK_MESSAGE(
			storage3.has(rid2),
			"Move-assigned storage should contain second original resource.");
}

TEST_CASE("[RefStorage] Stress test - many resources") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	const int COUNT = 1000;
	Vector<RID> rids;
	Vector<Ref<StandardMaterial3D>> materials;

	// Add many resources
	for (int i = 0; i < COUNT; i++) {
		Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
		RID rid = RS::get_singleton()->material_create();
		
		storage.add(mat, rid);
		rids.push_back(rid);
		materials.push_back(mat);
	}

	CHECK_MESSAGE(
			storage.size() == COUNT,
			"Storage should contain all added resources.");

	// Verify all resources
	for (int i = 0; i < COUNT; i++) {
		CHECK_MESSAGE(
				storage.has(rids[i]),
				(String("Storage should contain resource at index ") + itos(i)).utf8().get_data());
		
		RefContainer *container = storage.get(rids[i]);
		CHECK_MESSAGE(
				container != nullptr,
				(String("Container at index ") + itos(i) + " should not be null.").utf8().get_data());
		CHECK_MESSAGE(
				container->resource == materials[i],
				(String("Resource at index ") + itos(i) + " should match.").utf8().get_data());
	}

	// Release half
	for (int i = 0; i < COUNT / 2; i++) {
		storage.release(rids[i]);
	}

	CHECK_MESSAGE(
			storage.size() == COUNT / 2,
			"Storage should contain half the resources after releasing half.");

	// Verify remaining
	for (int i = COUNT / 2; i < COUNT; i++) {
		CHECK_MESSAGE(
				storage.has(rids[i]),
				(String("Storage should still contain resource at index ") + itos(i)).utf8().get_data());
	}
}

TEST_CASE("[RefStorage] Multiple resource types") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	
	// Test with different resource types
	Ref<StandardMaterial3D> material = memnew(StandardMaterial3D);
	Ref<ArrayMesh> mesh = memnew(ArrayMesh);
	
	RID material_rid = RS::get_singleton()->material_create();
	RID mesh_rid = RS::get_singleton()->mesh_create();
	
	storage.add(material, material_rid);
	storage.add(mesh, mesh_rid);
	
	CHECK_MESSAGE(
			storage.size() == 2,
			"Storage should contain both resource types.");
	
	RefContainer *mat_container = storage.get(material_rid);
	RefContainer *mesh_container = storage.get(mesh_rid);
	
	CHECK_MESSAGE(
			mat_container != nullptr,
			"Material container should not be null.");
	CHECK_MESSAGE(
			mat_container->class_name == "StandardMaterial3D",
			"Material container should have correct class name.");
	CHECK_MESSAGE(
			mesh_container != nullptr,
			"Mesh container should not be null.");
	CHECK_MESSAGE(
			mesh_container->class_name == "ArrayMesh",
			"Mesh container should have correct class name.");
}

// Thread-safety tests
#ifndef DISABLE_THREADED_TESTS

struct ThreadTestData {
	RefStorage *storage;
	Vector<RID> *rids;
	Vector<Ref<StandardMaterial3D>> *materials;
	Mutex *mutex;
	int thread_id;
	int operations_per_thread;
};

static void thread_add_resources(void *p_userdata) {
	ThreadTestData *data = static_cast<ThreadTestData *>(p_userdata);

	for (int i = 0; i < data->operations_per_thread; i++) {
		Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
		RID rid = RS::get_singleton()->material_create();
		
		data->storage->add(mat, rid);

		data->mutex->lock();
		data->rids->push_back(rid);
		data->materials->push_back(mat);
		data->mutex->unlock();
	}
}

static void thread_read_resources(void *p_userdata) {
	ThreadTestData *data = static_cast<ThreadTestData *>(p_userdata);

	for (int i = 0; i < data->operations_per_thread; i++) {
		data->mutex->lock();
		if (data->rids->size() > 0) {
			int idx = i % data->rids->size();
			RID rid = (*data->rids)[idx];
			data->mutex->unlock();

			// Read operation
			if (data->storage->has(rid)) {
				RefContainer *container = data->storage->get(rid);
				if (container != nullptr) {
					// Access container data
					String class_name = container->class_name;
					Ref<Resource> res = container->resource;
				}
			}
		} else {
			data->mutex->unlock();
		}

		// Small delay to increase contention
		OS::get_singleton()->delay_usec(1);
	}
}

TEST_CASE("[RefStorage] Thread-safety - concurrent adds") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	Vector<RID> all_rids;
	Vector<Ref<StandardMaterial3D>> all_materials;
	Mutex rid_mutex;

	const int THREAD_COUNT = 4;
	const int OPS_PER_THREAD = 100;

	Thread threads[THREAD_COUNT];
	ThreadTestData thread_data[THREAD_COUNT];

	// Start threads
	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].storage = &storage;
		thread_data[i].rids = &all_rids;
		thread_data[i].materials = &all_materials;
		thread_data[i].mutex = &rid_mutex;
		thread_data[i].thread_id = i;
		thread_data[i].operations_per_thread = OPS_PER_THREAD;

		threads[i].start(thread_add_resources, &thread_data[i]);
	}

	// Wait for completion
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	CHECK_MESSAGE(
			storage.size() == THREAD_COUNT * OPS_PER_THREAD,
			"Storage should contain all resources added by all threads.");

	// Verify all RIDs are valid and resources are correct
	for (int i = 0; i < all_rids.size(); i++) {
		CHECK_MESSAGE(
				storage.has(all_rids[i]),
				"Storage should contain resource added by thread.");

		RefContainer *container = storage.get(all_rids[i]);
		CHECK_MESSAGE(
				container != nullptr,
				"Container should be valid.");
		CHECK_MESSAGE(
				container->resource == all_materials[i],
				"Resource should match.");
	}
}

TEST_CASE("[RefStorage] Thread-safety - concurrent reads and writes") {
	REQUIRE_RENDERING_SERVER();
	
	RefStorage storage;
	Vector<RID> all_rids;
	Vector<Ref<StandardMaterial3D>> all_materials;
	Mutex rid_mutex;

	// Pre-populate with some resources
	for (int i = 0; i < 50; i++) {
		Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
		RID rid = RS::get_singleton()->material_create();
		storage.add(mat, rid);
		all_rids.push_back(rid);
		all_materials.push_back(mat);
	}

	const int THREAD_COUNT = 4;
	const int OPS_PER_THREAD = 50;

	Thread threads[THREAD_COUNT];
	ThreadTestData thread_data[THREAD_COUNT];

	// Start half as writers, half as readers
	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].storage = &storage;
		thread_data[i].rids = &all_rids;
		thread_data[i].materials = &all_materials;
		thread_data[i].mutex = &rid_mutex;
		thread_data[i].thread_id = i;
		thread_data[i].operations_per_thread = OPS_PER_THREAD;

		if (i % 2 == 0) {
			threads[i].start(thread_add_resources, &thread_data[i]);
		} else {
			threads[i].start(thread_read_resources, &thread_data[i]);
		}
	}

	// Wait for completion
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	// Storage should be in a valid state (no crashes, no corruption)
	CHECK_MESSAGE(
			storage.size() > 0,
			"Storage should contain resources after concurrent operations.");

	// All RIDs should still be valid
	for (int i = 0; i < all_rids.size(); i++) {
		if (storage.has(all_rids[i])) {
			RefContainer *container = storage.get(all_rids[i]);
			CHECK_MESSAGE(
					container != nullptr,
					"Container should be valid after concurrent access.");
			CHECK_MESSAGE(
					container->resource.is_valid(),
					"Resource should be valid after concurrent access.");
		}
	}
}

#endif // DISABLE_THREADED_TESTS

} // namespace TestRefStorage