#pragma once
#include "ecs/flecs_types/flecs_world.h"
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../../components/navigation/2d/2d_navigation_components.h"
#include "../../object_id_storage.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "servers/navigation_server_2d.h"
#include <scene/2d/navigation_agent_2d.h>
#include <scene/2d/navigation_link_2d.h>
#include <scene/2d/navigation_obstacle_2d.h>
#include <scene/2d/navigation_region_2d.h>

class Navigation2DUtility: public Object {
	GDCLASS(Navigation2DUtility, Object)

	// This class is a utility for creating navigation entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent navigation components in the ECS world,
	// ensuring that the necessary properties are set correctly.
public:
	Navigation2DUtility() = default; // Prevent instantiation
	~Navigation2DUtility() = default;
	static flecs::entity _create_nav_2d_agent(const flecs::world &world, const RID &agent, const String &name);
	static flecs::entity _create_nav_2d_agent(const flecs::world &world, const String &name);
	static flecs::entity _create_nav_2d_link(const flecs::world &world, const String &name);
	static flecs::entity _create_nav_2d_link(const flecs::world &world, const RID &link, const String &name);
	static flecs::entity _create_nav_2d_obstacle(const flecs::world &world, const RID &obstacle, const String &name);
	static flecs::entity _create_nav_2d_obstacle(const flecs::world &world, const String &name);
	static flecs::entity _create_nav_2d_region(const flecs::world &world, const RID &region, const String &name);
	static flecs::entity _create_nav_2d_region(const flecs::world &world, const String &name);
	static flecs::entity _create_source_geometry_parser_2d(const flecs::world &world, const RID &source_geometry_parser, const String &name);
	static flecs::entity _create_source_geometry_parser_2d(const flecs::world &world, const String &name);
	static flecs::entity _create_nav_2d_agent(const flecs::world &world, NavigationAgent2D *nav_agent);
	static flecs::entity _create_nav_2d_link(const flecs::world &world, NavigationLink2D *nav_link);
	static flecs::entity _create_nav_2d_obstacle(const flecs::world &world, NavigationObstacle2D *nav_obstacle);
	static flecs::entity _create_nav_2d_region(const flecs::world &world, NavigationRegion2D *nav_region);
	static flecs::entity _create_source_geometry_parser_2d(const flecs::world &world, const Callable &callable, const String &name);
	
	static Ref<FlecsEntity> create_nav_agent_with_object(FlecsWorld *world, NavigationAgent2D *nav_agent);
	static Ref<FlecsEntity> create_nav_link_with_object(FlecsWorld *world, NavigationLink2D *nav_link);
	static Ref<FlecsEntity> create_nav_obstacle_with_object(FlecsWorld *world, NavigationObstacle2D *nav_obstacle);
	static Ref<FlecsEntity> create_nav_region_with_object(FlecsWorld *world, NavigationRegion2D *nav_region);
	static Ref<FlecsEntity> create_source_geometry_parser(FlecsWorld *world,
			const Callable &callable, const String &name);
	static Ref<FlecsEntity> create_nav_agent(FlecsWorld *world, const RID &agent, const String &name);
	static Ref<FlecsEntity> create_nav_link(FlecsWorld *world, const RID &link, const String &name);
	static Ref<FlecsEntity> create_nav_obstacle(FlecsWorld *world, const RID &obstacle, const String &name);
	static Ref<FlecsEntity> create_nav_region(FlecsWorld *world, const RID &region, const String &name);
	
	static void _bind_methods();
};
