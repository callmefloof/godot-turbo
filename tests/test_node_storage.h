/**************************************************************************/
/*  test_node_storage.h                                                   */
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

#include "modules/godot_turbo/ecs/systems/utility/node_storage.h"

#include "core/os/thread.h"
#include "scene/main/node.h"
#include "tests/test_macros.h"

namespace TestNodeStorage {

// Helper node class for testing
class TestNode : public Node {
	GDCLASS(TestNode, Node);

public:
	int test_value = 0;
	TestNode(int p_value = 0) :
			test_value(p_value) {}
};

TEST_CASE("[NodeStorage] Constructor and basic properties") {
	NodeStorage storage;

	CHECK_MESSAGE(
			storage.size() == 0,
			"New NodeStorage should be empty.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"New NodeStorage should report as empty.");
}

TEST_CASE("[NodeStorage] Add and retrieve single node") {
	NodeStorage storage;
	Node *node = memnew(TestNode(42));
	ObjectID node_id = node->get_instance_id();

	bool added = storage.add(node, node_id);

	CHECK_MESSAGE(
			added,
			"add() should return true on success.");
	CHECK_MESSAGE(
			storage.has(node_id),
			"Storage should contain the added node.");
	CHECK_MESSAGE(
			storage.size() == 1,
			"Storage size should be 1 after adding one node.");
	CHECK_MESSAGE(
			!storage.is_empty(),
			"Storage should not be empty after adding a node.");

	NodeContainer *retrieved = storage.try_get(node_id);
	CHECK_MESSAGE(
			retrieved != nullptr,
			"Retrieved container should not be null.");
	CHECK_MESSAGE(
			retrieved->node == node,
			"Retrieved node should be the same as added node.");

	TestNode *typed_retrieved = Object::cast_to<TestNode>(retrieved->node);
	CHECK_MESSAGE(
			typed_retrieved != nullptr,
			"Retrieved node should be castable to TestNode.");
	CHECK_MESSAGE(
			typed_retrieved->test_value == 42,
			"Retrieved node should have the correct value.");

	// Cleanup
	storage.release(node_id);
}

TEST_CASE("[NodeStorage] Add multiple nodes") {
	NodeStorage storage;
	Node *node1 = memnew(TestNode(1));
	Node *node2 = memnew(TestNode(2));
	Node *node3 = memnew(TestNode(3));

	ObjectID id1 = node1->get_instance_id();
	ObjectID id2 = node2->get_instance_id();
	ObjectID id3 = node3->get_instance_id();
	
	storage.add(node1, id1);
	storage.add(node2, id2);
	storage.add(node3, id3);

	CHECK_MESSAGE(
			storage.size() == 3,
			"Storage should contain 3 nodes.");

	CHECK_MESSAGE(
			storage.has(id1),
			"Storage should contain first added node.");
	CHECK_MESSAGE(
			storage.has(id2),
			"Storage should contain second added node.");
	CHECK_MESSAGE(
			storage.has(id3),
			"Storage should contain third added node.");

	NodeContainer *container1 = storage.try_get(id1);
	NodeContainer *container2 = storage.try_get(id2);
	NodeContainer *container3 = storage.try_get(id3);
	
	TestNode *retrieved1 = container1 ? Object::cast_to<TestNode>(container1->node) : nullptr;
	TestNode *retrieved2 = container2 ? Object::cast_to<TestNode>(container2->node) : nullptr;
	TestNode *retrieved3 = container3 ? Object::cast_to<TestNode>(container3->node) : nullptr;

	CHECK_MESSAGE(
			retrieved1 != nullptr,
			"First retrieved node should not be null.");
	CHECK_MESSAGE(
			retrieved1->test_value == 1,
			"First retrieved node should have correct value.");
	CHECK_MESSAGE(
			retrieved2 != nullptr,
			"Second retrieved node should not be null.");
	CHECK_MESSAGE(
			retrieved2->test_value == 2,
			"Second retrieved node should have correct value.");
	CHECK_MESSAGE(
			retrieved3 != nullptr,
			"Third retrieved node should not be null.");
	CHECK_MESSAGE(
			retrieved3->test_value == 3,
			"Third retrieved node should have correct value.");

	// Cleanup
	storage.release_all();
}

TEST_CASE("[NodeStorage] Release single node") {
	NodeStorage storage;
	Node *node = memnew(TestNode(100));
	ObjectID id = node->get_instance_id();
	storage.add(node, id);

	CHECK_MESSAGE(
			storage.has(id),
			"Storage should contain the node before release.");

	storage.release(id);

	CHECK_MESSAGE(
			!storage.has(id),
			"Storage should not contain the node after release.");
	CHECK_MESSAGE(
			storage.size() == 0,
			"Storage should be empty after releasing the only node.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should report as empty after releasing all nodes.");

	// Node is queue_free'd, no need to memdelete
}

TEST_CASE("[NodeStorage] Release nonexistent node") {
	NodeStorage storage;
	ObjectID fake_id;

	// Should not crash
	bool released = storage.release(fake_id);

	CHECK_MESSAGE(
			!released,
			"Release should return false for nonexistent node.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should remain empty after attempting to release nonexistent node.");
}

TEST_CASE("[NodeStorage] Release all nodes") {
	NodeStorage storage;
	Node *node1 = memnew(TestNode(1));
	Node *node2 = memnew(TestNode(2));
	Node *node3 = memnew(TestNode(3));

	ObjectID id1 = node1->get_instance_id();
	ObjectID id2 = node2->get_instance_id();
	ObjectID id3 = node3->get_instance_id();
	
	storage.add(node1, id1);
	storage.add(node2, id2);
	storage.add(node3, id3);

	CHECK_MESSAGE(
			storage.size() == 3,
			"Storage should contain 3 nodes.");

	storage.release_all();

	CHECK_MESSAGE(
			storage.size() == 0,
			"Storage size should be 0 after release_all.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should be empty after release_all.");

	// Nodes are queue_free'd, no need to memdelete
}

TEST_CASE("[NodeStorage] Try get nonexistent node") {
	NodeStorage storage;
	ObjectID fake_id;

	NodeContainer *result = storage.try_get(fake_id);

	CHECK_MESSAGE(
			result == nullptr,
			"try_get() should return nullptr for nonexistent ID.");
}

TEST_CASE("[NodeStorage] Has with invalid ObjectID") {
	NodeStorage storage;
	ObjectID invalid_id;

	CHECK_MESSAGE(
			!storage.has(invalid_id),
			"has() should return false for invalid ObjectID.");
}

TEST_CASE("[NodeStorage] Add null node") {
	NodeStorage storage;
	Node *null_node = nullptr;
	ObjectID invalid_id;

	bool added = storage.add(null_node, invalid_id);

	CHECK_MESSAGE(
			!added,
			"Adding null node should return false.");
	CHECK_MESSAGE(
			storage.is_empty(),
			"Storage should remain empty after attempting to add null node.");
}

TEST_CASE("[NodeStorage] Get all IDs") {
	NodeStorage storage;
	Node *node1 = memnew(TestNode(1));
	Node *node2 = memnew(TestNode(2));
	Node *node3 = memnew(TestNode(3));

	ObjectID id1 = node1->get_instance_id();
	ObjectID id2 = node2->get_instance_id();
	ObjectID id3 = node3->get_instance_id();
	
	storage.add(node1, id1);
	storage.add(node2, id2);
	storage.add(node3, id3);

	Vector<ObjectID> ids = storage.get_all_ids();

	CHECK_MESSAGE(
			ids.size() == 3,
			"get_all_ids() should return 3 IDs.");

	bool has_all = ids.has(id1) && ids.has(id2) && ids.has(id3);
	CHECK_MESSAGE(
			has_all,
			"get_all_ids() should return all added node IDs.");

	// Cleanup
	storage.release_all();
}

TEST_CASE("[NodeStorage] Make inert") {
	NodeStorage storage;
	Node *node = memnew(TestNode(999));
	
	// Test make_inert directly on a node (not stored yet)
	storage.make_inert(node);
	
	CHECK_MESSAGE(
			!node->is_processing(),
			"Node should not be processing after make_inert.");
	CHECK_MESSAGE(
			!node->is_physics_processing(),
			"Node should not be physics processing after make_inert.");

	// Now add to storage (which also calls make_inert internally)
	ObjectID id = node->get_instance_id();
	storage.add(node, id);

	CHECK_MESSAGE(
			storage.has(id),
			"Storage should contain the node.");

	NodeContainer *container = storage.try_get(id);
	CHECK_MESSAGE(
			container != nullptr,
			"Container should not be null.");
	CHECK_MESSAGE(
			container->node == node,
			"Node should be retrievable from storage.");

	// Cleanup
	storage.release(id);
}

TEST_CASE("[NodeStorage] Move semantics") {
	NodeStorage storage1;
	Node *node1 = memnew(TestNode(1));
	Node *node2 = memnew(TestNode(2));

	ObjectID id1 = node1->get_instance_id();
	ObjectID id2 = node2->get_instance_id();
	storage1.add(node1, id1);
	storage1.add(node2, id2);

	CHECK_MESSAGE(
			storage1.size() == 2,
			"Original storage should contain 2 nodes.");

	// Move constructor
	NodeStorage storage2(std::move(storage1));

	CHECK_MESSAGE(
			storage2.size() == 2,
			"Moved-to storage should contain 2 nodes.");
	CHECK_MESSAGE(
			storage2.has(id1),
			"Moved-to storage should contain first original node.");
	CHECK_MESSAGE(
			storage2.has(id2),
			"Moved-to storage should contain second original node.");

	// Move assignment
	NodeStorage storage3;
	storage3 = std::move(storage2);

	CHECK_MESSAGE(
			storage3.size() == 2,
			"Move-assigned storage should contain 2 nodes.");
	CHECK_MESSAGE(
			storage3.has(id1),
			"Move-assigned storage should contain first original node.");
	CHECK_MESSAGE(
			storage3.has(id2),
			"Move-assigned storage should contain second original node.");

	// Cleanup
	storage3.release_all();
}

TEST_CASE("[NodeStorage] Stress test - many nodes") {
	NodeStorage storage;
	const int COUNT = 1000;
	Vector<ObjectID> ids;

	// Add many nodes
	for (int i = 0; i < COUNT; i++) {
		Node *node = memnew(TestNode(i));
		ObjectID id = node->get_instance_id();
		storage.add(node, id);
		ids.push_back(id);
	}

	CHECK_MESSAGE(
			storage.size() == COUNT,
			"Storage should contain all added nodes.");

	// Verify all nodes
	for (int i = 0; i < COUNT; i++) {
		CHECK(storage.has(ids[i]));
		NodeContainer *container = storage.try_get(ids[i]);
		if (container) {
			TestNode *node = Object::cast_to<TestNode>(container->node);
			CHECK(node != nullptr);
			CHECK(node->test_value == i);
		}
	}

	// Release half
	for (int i = 0; i < COUNT / 2; i++) {
		storage.release(ids[i]);
	}

	CHECK_MESSAGE(
			storage.size() == COUNT / 2,
			"Storage should contain half the nodes after releasing half.");

	// Verify remaining
	for (int i = COUNT / 2; i < COUNT; i++) {
		CHECK(storage.has(ids[i]));
	}

	// Cleanup
	storage.release_all();
}

// Thread-safety tests
#ifndef DISABLE_THREADED_TESTS

struct ThreadTestData {
	NodeStorage *storage;
	Vector<ObjectID> *ids;
	Mutex *mutex;
	int thread_id;
	int operations_per_thread;
};

static void thread_add_nodes(void *p_userdata) {
	ThreadTestData *data = static_cast<ThreadTestData *>(p_userdata);

	for (int i = 0; i < data->operations_per_thread; i++) {
		int value = data->thread_id * 10000 + i;
		Node *node = memnew(TestNode(value));
		ObjectID node_id = node->get_instance_id();

		data->storage->add(node, node_id);

		data->mutex->lock();
		data->ids->push_back(node_id);
		data->mutex->unlock();
	}
}

static void thread_read_nodes(void *p_userdata) {
	ThreadTestData *data = static_cast<ThreadTestData *>(p_userdata);

	for (int i = 0; i < data->operations_per_thread; i++) {
		data->mutex->lock();
		if (data->ids->size() > 0) {
			int idx = i % data->ids->size();
			ObjectID id = (*data->ids)[idx];
			data->mutex->unlock();

			// Read operation
			NodeContainer *container = data->storage->try_get(id);
			if (container && container->node) {
				TestNode *test_node = Object::cast_to<TestNode>(container->node);
				// Just access to test thread-safety, don't check value
				[[maybe_unused]] int value = test_node ? test_node->test_value : 0;
			}
		} else {
			data->mutex->unlock();
		}
	}
}

TEST_CASE("[NodeStorage] Thread-safety - concurrent adds") {
	NodeStorage storage;
	Vector<ObjectID> all_ids;
	Mutex id_mutex;

	const int THREAD_COUNT = 4;
	const int OPS_PER_THREAD = 100;

	Thread threads[THREAD_COUNT];
	ThreadTestData thread_data[THREAD_COUNT];

	// Start threads
	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].storage = &storage;
		thread_data[i].ids = &all_ids;
		thread_data[i].mutex = &id_mutex;
		thread_data[i].thread_id = i;
		thread_data[i].operations_per_thread = OPS_PER_THREAD;

		threads[i].start(thread_add_nodes, &thread_data[i]);
	}

	// Wait for completion
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	CHECK_MESSAGE(
			storage.size() == THREAD_COUNT * OPS_PER_THREAD,
			"Storage should contain all nodes added by all threads.");

	// Verify all nodes are accessible
	for (int i = 0; i < all_ids.size(); i++) {
		CHECK_MESSAGE(
				storage.has(all_ids[i]),
				"Storage should contain node added by thread.");
	}

	// Cleanup
	storage.release_all();
}

TEST_CASE("[NodeStorage] Thread-safety - concurrent reads and writes") {
	NodeStorage storage;
	Vector<ObjectID> all_ids;
	Mutex id_mutex;

	// Pre-populate with some nodes
	for (int i = 0; i < 50; i++) {
		Node *node = memnew(TestNode(i));
		ObjectID id = node->get_instance_id();
		storage.add(node, id);
		all_ids.push_back(id);
	}

	const int THREAD_COUNT = 4;
	const int OPS_PER_THREAD = 50;

	Thread threads[THREAD_COUNT];
	ThreadTestData thread_data[THREAD_COUNT];

	// Start half as writers, half as readers
	for (int i = 0; i < THREAD_COUNT; i++) {
		thread_data[i].storage = &storage;
		thread_data[i].ids = &all_ids;
		thread_data[i].mutex = &id_mutex;
		thread_data[i].thread_id = i;
		thread_data[i].operations_per_thread = OPS_PER_THREAD;

		if (i % 2 == 0) {
			threads[i].start(thread_add_nodes, &thread_data[i]);
		} else {
			threads[i].start(thread_read_nodes, &thread_data[i]);
		}
	}

	// Wait for completion
	for (int i = 0; i < THREAD_COUNT; i++) {
		threads[i].wait_to_finish();
	}

	// Storage should be in a valid state (no crashes, no corruption)
	CHECK_MESSAGE(
			storage.size() > 0,
			"Storage should contain nodes after concurrent operations.");

	// All IDs should still be accessible if they exist
	for (int i = 0; i < all_ids.size(); i++) {
		if (storage.has(all_ids[i])) {
			NodeContainer *container = storage.try_get(all_ids[i]);
			CHECK_MESSAGE(
					container != nullptr,
					"Container should be valid after concurrent access.");
		}
	}

	// Cleanup
	storage.release_all();
}

#endif // DISABLE_THREADED_TESTS

} // namespace TestNodeStorage