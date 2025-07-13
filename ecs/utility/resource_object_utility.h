#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../components/resource_component.h"
#include <core/string/ustring.h>
#include <core/templates/rid.h>
#include <core/io/resource.h>

class ResourceObjectUtility {
private:
	ResourceObjectUtility() = default; // Prevent instantiation
	ResourceObjectUtility(const ResourceObjectUtility &) = delete; // Prevent copy
	ResourceObjectUtility &operator=(const ResourceObjectUtility &) = delete; // Prevent assignment
	ResourceObjectUtility(ResourceObjectUtility &&) = delete; // Prevent move
	ResourceObjectUtility &operator=(ResourceObjectUtility &&) = delete; // Prevent move assignment
public:
	template <typename T = Resource>
	static inline flecs::entity CreateResourceEntity(const flecs::world &world, const Ref<T> resource) {
		if (!resource.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		if (const RID rid = resource->get_rid(); !rid.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		Ref<Script> scr = resource->get_script();
		auto script_instance = resource->get_script_instance();
		if (!scr.is_valid()) {
			return world.entity(String(resource->get_name()).ascii().get_data()).set<ResourceComponent<T>>({ resource->get_rid(), resource->get_class(), resource->get_name(), resource, nullptr, nullptr, false });
		}
		return world.entity(String(resource->get_name()).ascii().get_data()).set<ResourceComponent<T>>({ resource->get_rid(), resource->get_class(), resource->get_name(), resource, script_instance, scr, true });
	}
};
