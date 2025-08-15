#pragma once

#include "core/object/ref_counted.h"
#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "servers/rendering_server.h"

struct RefContainer {
	RID rid;                  // Rendering handle
	Ref<Resource> resource;   // Keeps the resource alive
	String class_name;        // For debugging / reflection

	bool operator==(const RefContainer &other) const {
		return rid == other.rid && class_name == other.class_name;
	}
};

class RefStorage {
private:
	static inline Vector<RefContainer> resource_pool;

public:
	~RefStorage() {
		release_all();
	}

	// Add resource + RID to storage
	template <typename T = Resource>
	static bool add(Ref<T> p_resource, const RID &p_rid) {
		if (p_resource.is_null() || !p_rid.is_valid()) {
			return false;
		}

		RefContainer container;
		container.rid = p_rid;
		container.class_name = p_resource->get_class();
		container.resource = p_resource;

		resource_pool.append(container);
		return true;
	}

	// Remove resource by RID
	static bool release(const RID &p_rid) {
		for (auto it = resource_pool.begin(); it != resource_pool.end(); ++it) {
			if (it->rid != p_rid) {
				continue;
			}

			// Release GPU resource first (if still valid)
			if (p_rid.is_valid()) {
				RS::get_singleton()->free(p_rid);
			}

			// Then unref the Resource
			if (it->resource.is_valid()) {
				it->resource.unref();
			}

			resource_pool.erase(*it);
			return true;
		}
		return false;
	}

	// Remove all resources
	static void release_all() {
		for (RefContainer &container : resource_pool) {
			if (container.rid.is_valid()) {
				RS::get_singleton()->free(container.rid);
			}
			container.resource.unref();
		}
		resource_pool.clear();
	}

	// Check if RID exists
	static bool has(const RID &p_rid) {
		for (const RefContainer &container : resource_pool) {
			if (container.rid == p_rid) {
				return true;
			}
		}
		return false;
	}

	// Get container by RID
	static RefContainer *get(const RID &p_rid) {
		for (auto it = resource_pool.begin(); it != resource_pool.end(); ++it) {
			if (it->rid == p_rid) {
				return &*it;
			}
		}
		return nullptr;
	}
};
