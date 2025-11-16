#pragma once

#include "core/object/ref_counted.h"
#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "servers/rendering_server.h"
#include "core/os/mutex.h"

/**
 * @file ref_storage.h
 * @brief Thread-safe storage container for Godot RefCounted resources with associated RIDs.
 * 
 * RefStorage manages the lifetime of RefCounted resources (e.g., Materials, Meshes, Textures)
 * alongside their corresponding RenderingServer RIDs. It ensures resources remain alive
 * while in use by the ECS and properly releases both the RefCounted object and the GPU resource.
 * 
 * @note Thread-safe: All public methods are protected by a mutex.
 */

/**
 * @struct RefContainer
 * @brief Container that pairs a RefCounted resource with its RenderingServer RID.
 * 
 * This structure holds:
 * - The GPU/server-side resource handle (RID)
 * - A strong reference to the RefCounted object (keeps it alive)
 * - The class name for debugging and reflection purposes
 * 
 * The container ensures that both the GPU resource and the Godot object
 * have synchronized lifetimes.
 */
struct RefContainer {
	RID rid;                  ///< RenderingServer handle (GPU resource identifier)
	Ref<Resource> resource;   ///< Strong reference keeps the resource alive
	String class_name;        ///< Class name for debugging / reflection (e.g., "StandardMaterial3D")

	/**
	 * @brief Equality comparison based on RID and class name.
	 * @param other The container to compare with
	 * @return true if both RID and class_name match
	 */
	bool operator==(const RefContainer &other) const {
		return rid == other.rid && class_name == other.class_name;
	}
};

/**
 * @class RefStorage
 * @brief Thread-safe storage pool for RefCounted resources with associated RIDs.
 * 
 * RefStorage manages the lifetime of Godot resources that have both:
 * 1. A RefCounted Godot object (e.g., Material, Mesh, Texture)
 * 2. A server-side RID (e.g., RenderingServer RID for GPU resources)
 * 
 * ## Purpose
 * 
 * In Godot's ECS integration, resources are often created through server APIs
 * (RenderingServer, PhysicsServer, etc.) which return RIDs. To prevent these
 * resources from being freed prematurely, RefStorage:
 * - Holds strong references to the Godot Resource objects
 * - Associates them with their server RIDs
 * - Ensures proper cleanup of both the Godot object and server resource
 * 
 * ## Thread Safety
 * 
 * All public methods are protected by an internal mutex, making RefStorage
 * safe for concurrent access from multiple threads.
 * 
 * ## Usage Example
 * 
 * ```cpp
 * RefStorage storage;
 * 
 * // Add a material to storage
 * Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
 * RID mat_rid = RS::get_singleton()->material_create();
 * storage.add(mat, mat_rid);
 * 
 * // Check if stored
 * if (storage.has(mat_rid)) {
 *     RefContainer* container = storage.get(mat_rid);
 *     print_line("Stored material: " + container->class_name);
 * }
 * 
 * // Release when done
 * storage.release(mat_rid);  // Frees GPU resource and unrefs object
 * ```
 * 
 * ## Lifecycle Management
 * 
 * When a resource is released:
 * 1. The server-side RID is freed (e.g., `RS::get_singleton()->free(rid)`)
 * 2. The Ref<Resource> is unreferenced (may trigger deletion if ref count = 0)
 * 3. The container is removed from the pool
 * 
 * @warning The destructor calls `release_all()`, which frees all stored resources.
 *          Ensure RefStorage outlives any systems that might access the stored RIDs.
 */
class RefStorage {
private:
	Vector<RefContainer> resource_pool;  ///< Internal storage vector
	mutable Mutex mutex;                  ///< Mutex for thread-safe operations

public:
	/**
	 * @brief Default constructor.
	 */
	RefStorage() = default;

	/**
	 * @brief Destructor - releases all stored resources.
	 * 
	 * Automatically calls `release_all()` to ensure proper cleanup
	 * of both GPU resources and RefCounted objects.
	 */
	~RefStorage() {
		release_all();
	}

	// Prevent copying (mutex is not copyable)
	RefStorage(const RefStorage&) = delete;
	RefStorage& operator=(const RefStorage&) = delete;

	// Allow moving
	RefStorage(RefStorage&& other) noexcept {
		MutexLock lock(other.mutex);
		resource_pool = std::move(other.resource_pool);
	}

	RefStorage& operator=(RefStorage&& other) noexcept {
		if (this != &other) {
			MutexLock lock1(mutex);
			MutexLock lock2(other.mutex);
			resource_pool = std::move(other.resource_pool);
		}
		return *this;
	}

	/**
	 * @brief Adds a resource to the storage pool.
	 * 
	 * Stores the resource with its associated RID. The resource will be kept alive
	 * (via strong reference) until explicitly released.
	 * 
	 * @tparam T Resource type (must derive from Resource)
	 * @param p_resource The resource to store (must be valid)
	 * @param p_rid The server RID associated with this resource (must be valid)
	 * @return true if successfully added, false if resource is null or RID is invalid
	 * 
	 * @note Thread-safe
	 * @note If the same RID is added multiple times, duplicates will be stored.
	 *       Use `has()` to check for existence first if needed.
	 * 
	 * @example
	 * ```cpp
	 * Ref<Mesh> mesh = memnew(ArrayMesh);
	 * RID mesh_rid = RS::get_singleton()->mesh_create();
	 * storage.add(mesh, mesh_rid);
	 * ```
	 */
	template <typename T = Resource>
	bool add(Ref<T> p_resource, const RID &p_rid) {
		if (p_resource.is_null() || !p_rid.is_valid()) {
			return false;
		}

		MutexLock lock(mutex);

		RefContainer container;
		container.rid = p_rid;
		container.class_name = p_resource->get_class();
		container.resource = p_resource;

		resource_pool.append(container);
		return true;
	}

	/**
	 * @brief Removes and frees a resource by its RID.
	 * 
	 * This method:
	 * 1. Frees the server-side resource (e.g., GPU resource via RenderingServer)
	 * 2. Unreferences the RefCounted object
	 * 3. Removes the container from the pool
	 * 
	 * @param p_rid The RID of the resource to release
	 * @return true if the resource was found and released, false otherwise
	 * 
	 * @note Thread-safe
	 * @warning After calling this, the RID is no longer valid and should not be used.
	 * @warning If the resource's reference count reaches 0, it will be deleted.
	 * 
	 * @example
	 * ```cpp
	 * storage.release(material_rid);  // Material is freed from GPU and unreferenced
	 * ```
	 */
	bool release(const RID &p_rid) {
		MutexLock lock(mutex);

		for (auto it = resource_pool.begin(); it != resource_pool.end(); ++it) {
			if (it->rid != p_rid) {
				continue;
			}

			// Release GPU/server resource first (if still valid)
			if (p_rid.is_valid()) {
				RS::get_singleton()->free(p_rid);
			}

			// Then unref the Resource (may trigger deletion)
			if (it->resource.is_valid()) {
				it->resource.unref();
			}

			resource_pool.erase(*it);
			return true;
		}
		return false;
	}

	/**
	 * @brief Removes and frees all stored resources.
	 * 
	 * This method iterates through all containers and:
	 * 1. Frees each server-side RID
	 * 2. Unreferences each RefCounted object
	 * 3. Clears the storage pool
	 * 
	 * @note Thread-safe
	 * @note Called automatically by the destructor
	 * @warning All RIDs become invalid after this call
	 * @warning Resources may be deleted if no other references exist
	 * 
	 * @example
	 * ```cpp
	 * // Clean up all resources at once (e.g., when shutting down a world)
	 * storage.release_all();
	 * ```
	 */
	void release_all() {
		MutexLock lock(mutex);

		for (RefContainer &container : resource_pool) {
			if (container.rid.is_valid()) {
				RS::get_singleton()->free(container.rid);
			}
			if (container.resource.is_valid()) {
				container.resource.unref();
			}
		}
		resource_pool.clear();
	}

	/**
	 * @brief Checks if a RID exists in the storage.
	 * 
	 * @param p_rid The RID to search for
	 * @return true if the RID is found in storage, false otherwise
	 * 
	 * @note Thread-safe
	 * 
	 * @example
	 * ```cpp
	 * if (storage.has(material_rid)) {
	 *     print_line("Material is still in storage");
	 * }
	 * ```
	 */
	bool has(const RID &p_rid) const {
		MutexLock lock(mutex);

		for (const RefContainer &container : resource_pool) {
			if (container.rid == p_rid) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Retrieves a container by RID.
	 * 
	 * Returns a pointer to the container holding the resource. The pointer
	 * is valid only while the mutex is held (i.e., within the same call context).
	 * 
	 * @param p_rid The RID to search for
	 * @return Pointer to RefContainer if found, nullptr otherwise
	 * 
	 * @note Thread-safe
	 * @warning The returned pointer is NOT safe to use across function calls.
	 *          It may become invalid if other threads modify the storage.
	 *          Copy needed data immediately.
	 * 
	 * @example
	 * ```cpp
	 * if (RefContainer* container = storage.get(material_rid)) {
	 *     String class_name = container->class_name;  // Copy data immediately
	 *     Ref<Resource> res = container->resource;    // Strong ref is safe
	 *     // Don't hold onto the pointer beyond this scope!
	 * }
	 * ```
	 */
	RefContainer* get(const RID &p_rid) {
		MutexLock lock(mutex);

		for (auto it = resource_pool.begin(); it != resource_pool.end(); ++it) {
			if (it->rid == p_rid) {
				return &*it;
			}
		}
		return nullptr;
	}

	/**
	 * @brief Gets the number of resources currently stored.
	 * 
	 * @return The count of stored resources
	 * @note Thread-safe
	 * 
	 * @example
	 * ```cpp
	 * print_line("Total resources in storage: " + itos(storage.size()));
	 * ```
	 */
	int size() const {
		MutexLock lock(mutex);
		return resource_pool.size();
	}

	/**
	 * @brief Checks if the storage is empty.
	 * 
	 * @return true if no resources are stored, false otherwise
	 * @note Thread-safe
	 */
	bool is_empty() const {
		MutexLock lock(mutex);
		return resource_pool.is_empty();
	}
};