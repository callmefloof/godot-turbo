#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "ecs/flecs_types/flecs_world.h"
#include "../components/resource_component.h"
#include <core/string/ustring.h>
#include <core/templates/rid.h>
#include <core/io/resource.h>

class ResourceObjectUtility : Object {
	GDCLASS(ResourceObjectUtility, Object)

	// This class is a utility for creating resource entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent resources in the ECS world,
	// ensuring that the necessary properties are set correctly.

public:
	ResourceObjectUtility() = default; // Prevent instantiation
	ResourceObjectUtility(const ResourceObjectUtility &) = delete; // Prevent copy
	static flecs::entity _create_resource_entity(const flecs::world &world, const Ref<Resource>& resource) {
		if (!resource.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		if (const RID rid = resource->get_rid(); !rid.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		if (const Ref<Script> scr = resource->get_script(); !scr.is_valid()) {
			return world.entity(String(resource->get_name()).ascii().get_data()).set<ResourceComponent>({ resource->get_rid(), resource->get_class(), resource->get_name(), false });
		}
		return world.entity(String(resource->get_name()).ascii().get_data()).set<ResourceComponent>({ resource->get_rid(), resource->get_class(), resource->get_name(), true });
	}
	static Ref<FlecsEntity> create_resource_entity(const Ref<FlecsWorld>& world, const Ref<Resource>& resource) {
		if (!world.is_valid() || !world.is_null()) {
			ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), Ref<FlecsEntity>());
		}
		Ref<FlecsEntity> entity = memnew(FlecsEntity);
		entity->set_entity(_create_resource_entity(world->get_world(), resource));
		ResourceComponentRef::create_component(entity);
		return entity;
	}

	static void _bind_methods() {
		ClassDB::bind_static_method(get_class_static(), "create_resource_entity",
			&ResourceObjectUtility::create_resource_entity, "world", "resource");
	}
};
