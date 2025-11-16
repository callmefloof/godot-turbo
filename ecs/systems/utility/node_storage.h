#pragma once

#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "core/os/mutex.h"
#include "scene/main/node.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/2d/physics/rigid_body_2d.h"
#include "scene/main/window.h"
#include "core/variant/variant.h"

/**
 * @file node_storage.h
 * @brief Thread-safe storage container for Godot scene nodes with lifecycle management.
 * 
 * NodeStorage manages inactive/pooled Godot nodes by:
 * - Storing nodes with their ObjectIDs
 * - Making nodes "inert" (disabled processing, invisible, physics frozen)
 * - Moving nodes to a dedicated storage parent in the scene tree
 * - Providing thread-safe access and lifecycle management
 * 
 * This is useful for object pooling, temporary node storage, or ECS-managed nodes
 * that should remain in memory but not actively participate in the scene.
 * 
 * @note Thread-safe: All public methods are protected by a mutex.
 */

/**
 * @struct NodeContainer
 * @brief Container that pairs a Node pointer with its ObjectID.
 * 
 * Stores:
 * - ObjectID for safe reference tracking
 * - Raw Node pointer (must be managed carefully - see NodeStorage docs)
 * - Class name for debugging and type identification
 */
struct NodeContainer {
	ObjectID id;           ///< Unique identifier for the node
	Node* node;            ///< Raw pointer to the node (managed by scene tree)
	String class_name;     ///< Class name for debugging / reflection (e.g., "RigidBody3D")

	/**
	 * @brief Equality comparison based on ObjectID and class name.
	 * @param other The container to compare with
	 * @return true if both ObjectID and class_name match
	 */
	bool operator==(const NodeContainer &other) const {
		return id == other.id && class_name == other.class_name;
	}
};

/**
 * @class NodeStorage
 * @brief Thread-safe storage pool for inactive Godot scene nodes.
 * 
 * ## Purpose
 * 
 * NodeStorage manages Godot nodes that need to be kept alive but inactive.
 * It's particularly useful for:
 * - **Object pooling**: Store inactive enemies, projectiles, effects
 * - **ECS integration**: Hold nodes managed by the ECS but not actively in scene
 * - **Temporary storage**: Park nodes during scene transitions
 * - **Performance optimization**: Reuse nodes instead of repeated allocation/deallocation
 * 
 * ## How It Works
 * 
 * When a node is added to storage:
 * 1. **Made Inert**: All processing disabled, visibility hidden, physics frozen
 * 2. **Reparented**: Moved to a dedicated `/root/NodeStorage` node
 * 3. **Tracked**: ObjectID and pointer stored for later retrieval
 * 
 * This ensures the node:
 * - Doesn't consume CPU (no process, physics, or input callbacks)
 * - Doesn't render (invisible)
 * - Doesn't interact with physics (frozen, no collisions)
 * - Remains in memory (strong reference via scene tree parent)
 * 
 * ## Thread Safety
 * 
 * All public methods are mutex-protected, allowing safe concurrent access
 * from multiple threads.
 * 
 * ## Usage Example
 * 
 * ```cpp
 * NodeStorage storage;
 * 
 * // Create and store a node
 * RigidBody3D* enemy = memnew(RigidBody3D);
 * get_tree()->get_root()->add_child(enemy);
 * storage.add(enemy, enemy->get_instance_id());
 * // Enemy is now inert and parented to /root/NodeStorage
 * 
 * // Retrieve later
 * if (NodeContainer* container = storage.try_get(enemy_id)) {
 *     RigidBody3D* enemy = Object::cast_to<RigidBody3D>(container->node);
 *     // Re-enable and use the enemy
 * }
 * 
 * // Release when done
 * storage.release(enemy_id);  // Queues node for deletion
 * ```
 * 
 * ## Node Lifecycle
 * 
 * - **add()**: Disables node and reparents to storage
 * - **release()**: Calls `queue_free()` on the node (deleted next frame)
 * - **release_all()**: Queues all stored nodes for deletion
 * - **Destructor**: Automatically calls `release_all()`
 * 
 * @warning Nodes are freed asynchronously via `queue_free()`.
 *          Don't access node pointers after calling `release()`.
 * 
 * @warning This class stores raw pointers. Nodes must remain valid
 *          (i.e., not freed externally) while in storage.
 */
class NodeStorage {
private:
	Vector<NodeContainer> object_pool;  ///< Internal storage vector
	Window* scene_node = nullptr;        ///< Cached reference to scene root
	mutable Mutex mutex;                 ///< Mutex for thread-safe operations

public:
	/**
	 * @brief Default constructor.
	 */
	NodeStorage() = default;

	/**
	 * @brief Destructor - releases all stored nodes.
	 * 
	 * Automatically calls `release_all()` to queue all nodes for deletion.
	 */
	~NodeStorage() {
		release_all();
	}

	// Prevent copying (mutex is not copyable, raw pointers are unsafe to copy)
	NodeStorage(const NodeStorage&) = delete;
	NodeStorage& operator=(const NodeStorage&) = delete;

	// Allow moving
	NodeStorage(NodeStorage&& other) noexcept {
		MutexLock lock(other.mutex);
		object_pool = std::move(other.object_pool);
		scene_node = other.scene_node;
		other.scene_node = nullptr;
	}

	NodeStorage& operator=(NodeStorage&& other) noexcept {
		if (this != &other) {
			MutexLock lock1(mutex);
			MutexLock lock2(other.mutex);
			object_pool = std::move(other.object_pool);
			scene_node = other.scene_node;
			other.scene_node = nullptr;
		}
		return *this;
	}

	/**
	 * @brief Makes a node "inert" - disables all processing and interaction.
	 * 
	 * Disables:
	 * - Process callbacks (`_process`, `_physics_process`)
	 * - Input processing (all input types)
	 * - Visibility (Node2D/Node3D)
	 * - Physics interactions (RigidBody2D/3D frozen with no collisions)
	 * 
	 * @param p_node The node to make inert (nullptr is safe, does nothing)
	 * 
	 * @note Thread-safe (though typically called internally)
	 * @note This is automatically called by `add()`, no need to call manually
	 * 
	 * @example
	 * ```cpp
	 * // Manually disable a node without storing it
	 * storage.make_inert(my_node);
	 * ```
	 */
	void make_inert(Node *p_node) {
		if (!p_node) {
			return;
		}

		// Disable all processing
		p_node->set_process(false);
		p_node->set_physics_process(false);
		p_node->set_process_input(false);
		p_node->set_process_unhandled_input(false);
		p_node->set_process_unhandled_key_input(false);

		// Hide visual nodes
		if (Node2D *n2d = Object::cast_to<Node2D>(p_node)) {
			n2d->set_visible(false);
		} else if (Node3D *n3d = Object::cast_to<Node3D>(p_node)) {
			n3d->set_visible(false);
		}

		// Freeze physics bodies completely
		if (RigidBody3D *rb = Object::cast_to<RigidBody3D>(p_node)) {
			rb->set_sleeping(true);
			rb->set_freeze_mode(RigidBody3D::FREEZE_MODE_KINEMATIC);
			rb->set_freeze_enabled(true);
			rb->set_collision_layer(0);  // No collision layers
			rb->set_collision_mask(0);   // No collision detection
		}
		if (RigidBody2D *rb = Object::cast_to<RigidBody2D>(p_node)) {
			rb->set_sleeping(true);
			rb->set_freeze_mode(RigidBody2D::FREEZE_MODE_KINEMATIC);
			rb->set_freeze_enabled(true);
			rb->set_collision_layer(0);
			rb->set_collision_mask(0);
		}
	}

	/**
	 * @brief Adds a node to storage, making it inert and reparenting it.
	 * 
	 * This method:
	 * 1. Validates the node and ObjectID
	 * 2. Makes the node inert (see `make_inert()`)
	 * 3. Creates or finds the `/root/NodeStorage` parent node
	 * 4. Reparents the node to storage (via deferred calls for thread safety)
	 * 5. Adds the container to the pool
	 * 
	 * @tparam T Node type (must derive from Node)
	 * @param p_node The node to store (must be valid and have a SceneTree)
	 * @param p_id The ObjectID for this node (must be valid)
	 * @return true if successfully added, false if node/ID is invalid
	 * 
	 * @note Thread-safe
	 * @note Uses deferred calls for reparenting (safe for threading)
	 * @warning The node must have an active SceneTree (be in the scene)
	 * 
	 * @example
	 * ```cpp
	 * // Store an enemy node
	 * Enemy* enemy = create_enemy();
	 * add_child(enemy);  // Must be in scene first
	 * storage.add(enemy, enemy->get_instance_id());
	 * // Enemy is now inactive and stored
	 * ```
	 */
	template <typename T = Node>
	bool add(T* p_node, const ObjectID &p_id) {
		if (!p_node || !p_id.is_valid()) {
			return false;
		}

		MutexLock lock(mutex);

		NodeContainer container;
		container.id = p_id;
		container.class_name = p_node->get_class();
		container.node = p_node;

		// Make the node inert before storing
		make_inert(p_node);

		// Only perform scene tree operations if node is in a tree
		// This allows testing without a SceneTree while maintaining compatibility
		if (p_node->is_inside_tree()) {
			// Cache scene root reference
			if (!scene_node) {
				scene_node = p_node->get_tree()->get_root();
			}

			// Find or create the storage parent node
			Node *storage_parent = scene_node->get_node_or_null(NodePath(String("/root/NodeStorage")));
			if (!storage_parent) {
				storage_parent = memnew(Node);
				storage_parent->set_name("NodeStorage");
				// Use deferred call for thread safety
				scene_node->call_deferred("add_child", storage_parent);
			}

			// Reparent to storage (deferred for thread safety)
			if (p_node->get_parent()) {
				p_node->get_parent()->call_deferred("remove_child", p_node);
			}
			storage_parent->call_deferred("add_child", p_node);
		}
		// If not in tree, just store the pointer - the container keeps it alive

		object_pool.append(container);
		return true;
	}

	/**
	 * @brief Removes and queues a node for deletion by ObjectID.
	 * 
	 * This method:
	 * 1. Finds the node by ObjectID
	 * 2. Calls `queue_free()` on it (deletion happens next frame)
	 * 3. Removes the container from the pool
	 * 
	 * @param p_id The ObjectID of the node to release
	 * @return true if the node was found and released, false otherwise
	 * 
	 * @note Thread-safe
	 * @warning The node pointer becomes invalid after this call.
	 *          Do not use it after calling `release()`.
	 * @note Deletion is deferred (happens next frame via `queue_free()`)
	 * 
	 * @example
	 * ```cpp
	 * // Release a stored enemy
	 * storage.release(enemy_id);
	 * // enemy_id is now invalid, don't use it
	 * ```
	 */
	bool release(const ObjectID &p_id) {
		MutexLock lock(mutex);

		for (auto it = object_pool.begin(); it != object_pool.end(); ++it) {
			if (it->id != p_id) {
				continue;
			}

			// Delete the node appropriately based on whether it's in a tree
			if (it->node) {
				if (it->node->is_inside_tree()) {
					// Queue for deletion if in scene tree (happens next frame)
					it->node->queue_free();
				} else {
					// Direct deletion for nodes not in tree (e.g., unit tests)
					memdelete(it->node);
				}
			}

			object_pool.erase(*it);
			return true;
		}
		return false;
	}

	/**
	 * @brief Removes and queues all stored nodes for deletion.
	 * 
	 * Iterates through all containers and:
	 * 1. Calls `queue_free()` on each node
	 * 2. Clears node pointers
	 * 3. Clears the storage pool
	 * 
	 * @note Thread-safe
	 * @note Called automatically by the destructor
	 * @warning All stored node pointers become invalid after this call
	 * @note Deletion is deferred (happens next frame via `queue_free()`)
	 * 
	 * @example
	 * ```cpp
	 * // Clean up all stored nodes (e.g., on level exit)
	 * storage.release_all();
	 * ```
	 */
	void release_all() {
		MutexLock lock(mutex);

		for (NodeContainer &container : object_pool) {
			if (container.node) {
				if (container.node->is_inside_tree()) {
					// Queue for deletion if in scene tree
					container.node->queue_free();
				} else {
					// Direct deletion for nodes not in tree
					memdelete(container.node);
				}
			}
		}
		object_pool.clear();
	}

	/**
	 * @brief Checks if an ObjectID exists in storage.
	 * 
	 * @param p_id The ObjectID to search for
	 * @return true if the ObjectID is found in storage, false otherwise
	 * 
	 * @note Thread-safe
	 * 
	 * @example
	 * ```cpp
	 * if (storage.has(enemy_id)) {
	 *     print_line("Enemy is in storage");
	 * }
	 * ```
	 */
	bool has(const ObjectID &p_id) const {
		MutexLock lock(mutex);

		for (const NodeContainer &container : object_pool) {
			if (container.id == p_id) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Retrieves a container by ObjectID.
	 * 
	 * Returns a pointer to the container holding the node. The pointer
	 * is valid only while the mutex is held (i.e., within the same call context).
	 * 
	 * @param p_id The ObjectID to search for
	 * @return Pointer to NodeContainer if found, nullptr otherwise
	 * 
	 * @note Thread-safe
	 * @warning The returned pointer is NOT safe to use across function calls.
	 *          It may become invalid if other threads modify the storage.
	 * @warning The node pointer in the container may be dangling if the node
	 *          was freed externally. Always validate before use.
	 * 
	 * @example
	 * ```cpp
	 * if (NodeContainer* container = storage.try_get(enemy_id)) {
	 *     String class_name = container->class_name;  // Copy data immediately
	 *     Node* node = container->node;               // Use immediately
	 *     if (Object::cast_to<RigidBody3D>(node)) {
	 *         // Safe to use within this scope
	 *     }
	 *     // Don't hold onto the pointer beyond this scope!
	 * }
	 * ```
	 */
	NodeContainer* try_get(const ObjectID &p_id) {
		MutexLock lock(mutex);

		for (auto it = object_pool.begin(); it != object_pool.end(); ++it) {
			if (it->id == p_id) {
				return &*it;
			}
		}
		return nullptr;
	}

	/**
	 * @brief Gets the number of nodes currently stored.
	 * 
	 * @return The count of stored nodes
	 * @note Thread-safe
	 * 
	 * @example
	 * ```cpp
	 * print_line("Total nodes in storage: " + itos(storage.size()));
	 * ```
	 */
	int size() const {
		MutexLock lock(mutex);
		return object_pool.size();
	}

	/**
	 * @brief Checks if the storage is empty.
	 * 
	 * @return true if no nodes are stored, false otherwise
	 * @note Thread-safe
	 */
	bool is_empty() const {
		MutexLock lock(mutex);
		return object_pool.is_empty();
	}

	/**
	 * @brief Gets a read-only copy of all stored ObjectIDs.
	 * 
	 * Useful for iteration or bulk operations without holding the lock.
	 * 
	 * @return Vector containing all ObjectIDs currently in storage
	 * @note Thread-safe
	 * @note Returns a copy, so it's safe to iterate even if storage is modified
	 * 
	 * @example
	 * ```cpp
	 * Vector<ObjectID> ids = storage.get_all_ids();
	 * for (const ObjectID& id : ids) {
	 *     if (NodeContainer* container = storage.try_get(id)) {
	 *         // Process each stored node
	 *     }
	 * }
	 * ```
	 */
	Vector<ObjectID> get_all_ids() const {
		MutexLock lock(mutex);
		Vector<ObjectID> ids;
		ids.resize(object_pool.size());
		for (int i = 0; i < object_pool.size(); ++i) {
			ids.write[i] = object_pool[i].id;
		}
		return ids;
	}
};