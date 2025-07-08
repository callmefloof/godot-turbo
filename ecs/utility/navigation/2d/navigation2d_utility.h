#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../../components/navigation/2d/2d_navigation_components.h"
#include "../../../components/transform_2d_component.h"
#include "../../object_id_storage.h"
#include "core/math/transform_2d.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"
#include "servers/navigation_server_2d.h"
#include <scene/2d/navigation_agent_2d.h>
#include <scene/2d/navigation_link_2d.h>
#include <scene/2d/navigation_obstacle_2d.h>
#include <scene/2d/navigation_region_2d.h>

namespace godot_turbo::utility::navigation {

using namespace godot_turbo::components::navigation;
class Navigation2DUtility {
private:
	Navigation2DUtility() = default; // Prevent instantiation
	Navigation2DUtility(const Navigation2DUtility &) = delete; // Prevent copy
	Navigation2DUtility &operator=(const Navigation2DUtility &) = delete; // Prevent assignment
	Navigation2DUtility(Navigation2DUtility &&) = delete; // Prevent move
	Navigation2DUtility &operator=(Navigation2DUtility &&) = delete; // Prevent move assignment
public:
	static inline flecs::entity CreateNav2DAgent(flecs::world &world, const RID &agent, const String &name) {
		return world.entity(name).set<NavAgent2DComponent>({ agent });
	}
	static inline flecs::entity CreateNav2DAgent(flecs::world &world, const String &name) {
		RID nav_agent_id = NavigationServer2D::get_singleton()->agent_create();
		return world.entity(name).set<NavAgent2DComponent>({ nav_agent_id });
	}

	static inline flecs::entity CreateNav2DLink(flecs::world &world, const String &name) {
		RID nav_link_id = NavigationServer2D::get_singleton()->link_create();
		return world.entity(name).set<NavLink2DComponent>({ nav_link_id });
	}
	static inline flecs::entity CreateNav2DLink(flecs::world &world, const RID &link, const String &name) {
		return world.entity(name).set<NavLink2DComponent>({ link });
	}
	static inline flecs::entity CreateNav2DMap(flecs::world &world, const RID &map, const String &name) {
		return world.entity(name).set<NavMap2DComponent>({ map });
	}
	static inline flecs::entity CreateNav2DMap(flecs::world &world, const String &name) {
		RID nav_map_id = NavigationServer2D::get_singleton()->map_create();
		return world.entity(name).set<NavMap2DComponent>({ nav_map_id });
	}
	static inline flecs::entity CreateNav2DObstacle(flecs::world &world, const RID &obstacle, const String &name) {
		return world.entity(name).set<NavObstacle2DComponent>({ obstacle });
	}
	static inline flecs::entity CreateNav2DObstacle(flecs::world &world, const String &name) {
		RID nav_obstacle_id = NavigationServer2D::get_singleton()->obstacle_create();
		return world.entity(name).set<NavObstacle2DComponent>({ nav_obstacle_id });
	}
	static inline flecs::entity CreateNav2DRegion(flecs::world &world, const RID &region, const String &name) {
		return world.entity(name).set<NavRegion2DComponent>({ region });
	}
	static inline flecs::entity CreateNav2DRegion(flecs::world &world, const String &name) {
		RID nav_region_id = NavigationServer2D::get_singleton()->region_create();
		return world.entity(name).set<NavRegion2DComponent>({ nav_region_id });
	}
	static inline flecs::entity CreateSourceGeometryParser2D(flecs::world &world, const RID &source_geometry_parser, const String &name) {
		return world.entity(name).set<SourceGeometryParser2DComponent>({ source_geometry_parser });
	}
	static inline flecs::entity CreateSourceGeometryParser2D(flecs::world &world, const String &name) {
		RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
		return world.entity(name).set<SourceGeometryParser2DComponent>({ source_geometry_parser_id });
	}
	static inline flecs::entity CreateNav2DAgent(flecs::world &world, NavigationAgent2D *nav_agent) {
		if (nav_agent == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto agent_id = nav_agent->get_rid();
		if (!agent_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(nav_agent->get_name())
							  .set<NavAgent2DComponent>({ agent_id });
		ObjectIDStorage::add(nav_agent, agent_id);
		return entity;
	}

	static inline flecs::entity CreateNav2DLink(flecs::world &world, NavigationLink2D *nav_link) {
		if (nav_link == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto link_id = nav_link->get_rid();
		if (!link_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(nav_link->get_name())
							  .set<NavLink2DComponent>({ link_id });
		ObjectIDStorage::add(nav_link, link_id);
		return entity;
	}

	static inline flecs::entity CreateNav2DMap(flecs::world &world, NavigationRegion2D *nav_map) {
		if (nav_map == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto map_id = nav_map->get_navigation_map();
		if (!map_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(nav_map->get_name())
							  .set<NavMap2DComponent>({ map_id });
		ObjectIDStorage::add(nav_map, map_id);
		return entity;
	}

	static inline flecs::entity CreateNav2DObstacle(flecs::world &world, NavigationObstacle2D *nav_obstacle) {
		if (nav_obstacle == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto obstacle_id = nav_obstacle->get_rid();
		if (!obstacle_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(nav_obstacle->get_name())
							  .set<NavObstacle2DComponent>({ obstacle_id });
		ObjectIDStorage::add(nav_obstacle, obstacle_id);
		return entity;
	}

	static inline flecs::entity CreateNav2DRegion(flecs::world &world, NavigationRegion2D *nav_region) {
		if (nav_region == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto region_id = nav_region->get_rid();
		if (!region_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(nav_region->get_name())
							  .set<NavRegion2DComponent>({ region_id });
		ObjectIDStorage::add(nav_region, region_id);
		return entity;
	}

	static inline flecs::entity CreateSourceGeometryParser2D(flecs::world &world, Callable &callable, const String &name) {
		auto source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
		if (!source_geometry_parser_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(name)
							  .set<SourceGeometryParser2DComponent>({ source_geometry_parser_id });
		NavigationServer2D::get_singleton()->source_geometry_parser_set_callback(
				source_geometry_parser_id,
				callable);
		return entity;
	}
}; //namespace godot_turbo::utility::navigation
