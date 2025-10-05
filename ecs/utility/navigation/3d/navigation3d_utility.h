#pragma once
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "scene/3d/navigation/navigation_agent_3d.h"
#include "scene/3d/navigation/navigation_link_3d.h"
#include "scene/3d/navigation/navigation_obstacle_3d.h"
#include "scene/3d/navigation/navigation_region_3d.h"
#include "servers/navigation_server_3d.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "components/navigation/3d/3d_navigation_components.h"
#include "ecs/utility/ref_storage.h"
#include "ecs/utility/node_storage.h"
#include "ecs/components/object_instance_component.h"

class Callable;
class Navigation3DUtility : public Object {
	GDCLASS(Navigation3DUtility, Object)

	// This class is a utility for creating navigation entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent navigation components in the ECS world,
	// ensuring that the necessary properties are set correctly.
public:
	Navigation3DUtility() = default;
	~Navigation3DUtility() = default;
	static RID create_nav_agent_with_id(const RID &world_id, const RID &agent, const String &name);
	static RID create_nav_agent(const RID &world_id, const String &name);
	static RID create_nav_link(const RID &world_id, const String &name);
	static RID create_nav_link_with_id(const RID &world_id, const RID &link, const String &name);
	static RID create_nav_obstacle_with_id(const RID &world_id, const RID &obstacle, const String &name);
	static RID create_nav_obstacle(const RID &world_id, const String &name);
	static RID create_nav_region_with_id(const RID &world_id, const RID &region, const String &name);
	static RID create_nav_region(const RID &world_id, const String &name);
	static RID create_sgp_with_id(const RID &world_id, const RID &source_geometry_parser, const String &name);
	static RID create_source_geometry_parser(const RID &world_id, const String &name);
	static RID create_nav_link_with_object(const RID &world_id, NavigationLink3D *nav_link);
	static RID create_nav_agent_with_object(const RID &world_id, NavigationAgent3D *nav_agent);
	static RID create_nav_obstacle_with_object(const RID &world_id, NavigationObstacle3D *nav_obstacle);
	static RID create_nav_region_with_object(const RID &world_id, NavigationRegion3D *nav_region);
	static RID create_sgp_with_callable(const RID &world_id, const Callable &callable, const String &name);
	
	static void _bind_methods();
};
