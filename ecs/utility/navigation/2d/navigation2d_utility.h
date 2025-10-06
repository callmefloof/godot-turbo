#pragma once
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include <scene/2d/navigation/navigation_agent_2d.h>
#include <scene/2d/navigation/navigation_link_2d.h>
#include <scene/2d/navigation/navigation_obstacle_2d.h>
#include <scene/2d/navigation/navigation_region_2d.h>

class Navigation2DUtility: public Object {
	GDCLASS(Navigation2DUtility, Object)

	// This class is a utility for creating navigation entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent navigation components in the ECS world,
	// ensuring that the necessary properties are set correctly.
public:
	Navigation2DUtility() = default; // Prevent instantiation
	~Navigation2DUtility() = default;
	static RID create_nav_agent_with_id(const RID &world_id, const RID &agent, const String &name);
	static RID create_nav_agent(const RID &world_id, const String &name);
	static RID create_nav_link(const RID &world_id, const String &name);
	static RID create_nav_link_with_id(const RID &world_id, const RID &link, const String &name);
	static RID create_nav_obstacle(const RID &world_id, const String &name);
	static RID create_nav_obstacle_with_id(const RID &world_id, const RID &obstacle, const String &name);
	static RID create_nav_region_with_id(const RID &world_id, const RID &region, const String &name);
	static RID create_nav_region(const RID &world_id, const String &name);
	static RID create_sg_parser_with_id(const RID &world_id, const RID &source_geometry_parser, const String &name);
	static RID create_source_geometry_parser(const RID &world_id, const String &name);
	static RID create_nav_agent_with_object(const RID &world_id, NavigationAgent2D *nav_agent);
	static RID create_nav_link_with_object(const RID &world_id, NavigationLink2D *nav_link);
	static RID create_nav_obstacle_with_object(const RID &world_id, NavigationObstacle2D *nav_obstacle);
	static RID create_nav_region_with_object(const RID &world_id, NavigationRegion2D *nav_region);
	static RID create_sg_parser_with_callable(const RID &world_id, const Callable &callable, const String &name);

	static void _bind_methods();
};
