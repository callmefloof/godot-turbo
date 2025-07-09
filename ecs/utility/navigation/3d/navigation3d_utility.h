#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_3d.h"
#include "servers/navigation_server_3d.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "../../object_id_storage.h"
#include "../../../components/transform_3d_component.h"
#include "../../../components/navigation/3d/3d_navigation_components.h"
#include <scene/3d/navigation_agent_3d.h>
#include <scene/3d/navigation_link_3d.h>
#include <scene/3d/navigation_region_3d.h>
#include <scene/3d/navigation_obstacle_3d.h>


class Navigation3DUtility {
private:
	Navigation3DUtility() = default; // Prevent instantiation
	Navigation3DUtility(const Navigation3DUtility &) = delete; // Prevent copy
	Navigation3DUtility &operator=(const Navigation3DUtility &) = delete; // Prevent assignment
	Navigation3DUtility(Navigation3DUtility &&) = delete; // Prevent move
	Navigation3DUtility &operator=(Navigation3DUtility &&) = delete; // Prevent move assignment
public:
	static inline flecs::entity CreateNav3DAgent(flecs::world &world, const RID &agent, const String &name) {
		return world.entity().set<NavAgent3DComponent>({ agent }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DAgent(flecs::world &world, const String &name) {
		RID nav_agent_id = NavigationServer3D::get_singleton()->agent_create();
		return world.entity().set<NavAgent3DComponent>({ nav_agent_id }).set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateNav3DLink(flecs::world &world, const String &name) {
		RID nav_link_id = NavigationServer3D::get_singleton()->link_create();
		return world.entity().set<NavLink3DComponent>({ nav_link_id }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DLink(flecs::world &world, const RID &link, const String &name) {
		return world.entity().set<NavLink3DComponent>({ link }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DMap(flecs::world &world, const RID &map, const String &name) {
		return world.entity().set<NavMap3DComponent>({ map }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DMap(flecs::world &world, const String &name) {
		RID nav_map_id = NavigationServer3D::get_singleton()->map_create();
		return world.entity().set<NavMap3DComponent>({ nav_map_id }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DObstacle(flecs::world &world, const RID &obstacle, const String &name) {
		return world.entity().set<NavObstacle3DComponent>({ obstacle }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DObstacle(flecs::world &world, const String &name) {
		RID nav_obstacle_id = NavigationServer3D::get_singleton()->obstacle_create();
		return world.entity().set<NavObstacle3DComponent>({ nav_obstacle_id }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DRegion(flecs::world &world, const RID &region, const String &name) {
		return world.entity().set<NavRegion3DComponent>({ region }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateNav3DRegion(flecs::world &world, const String &name) {
		RID nav_region_id = NavigationServer3D::get_singleton()->region_create();
		return world.entity().set<NavRegion3DComponent>({ nav_region_id }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateSourceGeometryParser3D(flecs::world &world, const RID &source_geometry_parser, const String &name) {
		return world.entity().set<SourceGeometryParser3DComponent>({ source_geometry_parser }).set_name(name.ascii().get_data());
	}
	static inline flecs::entity CreateSourceGeometryParser3D(flecs::world &world, const String &name) {
		RID source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
		return world.entity().set<SourceGeometryParser3DComponent>({ source_geometry_parser_id }).set_name(name.ascii().get_data());;
	}
	static inline flecs::entity CreateNav3DAgent(flecs::world &world, NavigationAgent3D *nav_agent) {
		if (nav_agent == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto agent_id = nav_agent->get_rid();
		if (!agent_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<NavAgent3DComponent>({ agent_id })
							  .set_name(String(nav_agent->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_agent, agent_id);
		return entity;
	}

	static inline flecs::entity CreateNav3DLink(flecs::world &world, NavigationLink3D *nav_link) {
		if (nav_link == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto link_id = nav_link->get_rid();
		if (!link_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
								.set<NavLink3DComponent>({ link_id })
								.set_name(String(nav_link->get_name()).ascii().get_data());

		ObjectIDStorage::add(nav_link, link_id);
		return entity;
	}

	static inline flecs::entity CreateNav3DMap(flecs::world &world, NavigationRegion3D *nav_map) {
		if (nav_map == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto map_id = nav_map->get_navigation_map();
		if (!map_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(String(nav_map->get_name()).ascii().get_data())
							  .set<NavMap3DComponent>({ map_id })
							  .set_name(String(nav_map->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_map, map_id);
		return entity;
	}

	static inline flecs::entity CreateNav3DObstacle(flecs::world &world, NavigationObstacle3D *nav_obstacle) {
		if (nav_obstacle == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto obstacle_id = nav_obstacle->get_rid();
		if (!obstacle_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<NavObstacle3DComponent>({ obstacle_id })
							  .set_name(String(nav_obstacle->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_obstacle, obstacle_id);
		return entity;
	}

	static inline flecs::entity CreateNav3DRegion(flecs::world &world, NavigationRegion3D *nav_region) {
		if (nav_region == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto region_id = nav_region->get_rid();
		if (!region_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<NavRegion3DComponent>({ region_id })
							  .set_name(String(nav_region->get_name()).ascii().get_data());

		ObjectIDStorage::add(nav_region, region_id);
		return entity;
	}

	static inline flecs::entity CreateSourceGeometryParser3D(flecs::world &world, Callable &callable, const String &name) {
		auto source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
		if (!source_geometry_parser_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<SourceGeometryParser3DComponent>({ source_geometry_parser_id })
							  .set_name(name.ascii().get_data());
		NavigationServer3D::get_singleton()->source_geometry_parser_set_callback(
				source_geometry_parser_id,
				callable);
		return entity;
	}
};
