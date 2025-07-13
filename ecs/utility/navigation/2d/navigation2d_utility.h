#pragma once
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

class Navigation2DUtility {
private:
	Navigation2DUtility() = default; // Prevent instantiation
	Navigation2DUtility(const Navigation2DUtility &) = delete; // Prevent copy
	Navigation2DUtility &operator=(const Navigation2DUtility &) = delete; // Prevent assignment
	Navigation2DUtility(Navigation2DUtility &&) = delete; // Prevent move
	Navigation2DUtility &operator=(Navigation2DUtility &&) = delete; // Prevent move assignment
public:
	static flecs::entity CreateNav2DAgent(const flecs::world &world, const RID &agent, const String &name) {
		return world.entity().set<NavAgent2DComponent>({ agent }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DAgent(const flecs::world &world, const String &name) {
		const RID nav_agent_id = NavigationServer2D::get_singleton()->agent_create();
		return world.entity().set<NavAgent2DComponent>({ nav_agent_id }).set_name(name.ascii().get_data());
	}

	static flecs::entity CreateNav2DLink(const flecs::world &world, const String &name) {
		const RID nav_link_id = NavigationServer2D::get_singleton()->link_create();
		return world.entity().set<NavLink2DComponent>({ nav_link_id }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DLink(const flecs::world &world, const RID &link, const String &name) {
		return world.entity().set<NavLink2DComponent>({ link }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DObstacle(const flecs::world &world, const RID &obstacle, const String &name) {
		return world.entity().set<NavObstacle2DComponent>({ obstacle }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DObstacle(const flecs::world &world, const String &name) {
		const RID nav_obstacle_id = NavigationServer2D::get_singleton()->obstacle_create();
		return world.entity().set<NavObstacle2DComponent>({ nav_obstacle_id }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DRegion(const flecs::world &world, const RID &region, const String &name) {
		return world.entity().set<NavRegion2DComponent>({ region }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DRegion(const flecs::world &world, const String &name) {
		const RID nav_region_id = NavigationServer2D::get_singleton()->region_create();
		return world.entity().set<NavRegion2DComponent>({ nav_region_id }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateSourceGeometryParser2D(const flecs::world &world, const RID &source_geometry_parser, const String &name) {
		return world.entity().set<SourceGeometryParser2DComponent>({ source_geometry_parser }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateSourceGeometryParser2D(const flecs::world &world, const String &name) {
		const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
		return world.entity().set<SourceGeometryParser2DComponent>({ source_geometry_parser_id }).set_name(name.ascii().get_data());
	}
	static flecs::entity CreateNav2DAgent(const flecs::world &world, NavigationAgent2D *nav_agent) {
		if (nav_agent == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID agent_id = nav_agent->get_rid();
		if (!agent_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<NavAgent2DComponent>({ agent_id })
							  .set_name(String(nav_agent->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_agent, agent_id);
		return entity;
	}

	static flecs::entity CreateNav2DLink(const flecs::world &world, NavigationLink2D *nav_link) {
		if (nav_link == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID link_id = nav_link->get_rid();
		if (!link_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<NavLink2DComponent>({ link_id })
							  .set_name(String(nav_link->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_link, link_id);
		return entity;
	}

	static flecs::entity CreateNav2DObstacle(const flecs::world &world, NavigationObstacle2D *nav_obstacle) {
		if (nav_obstacle == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID obstacle_id = nav_obstacle->get_rid();
		if (!obstacle_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<NavObstacle2DComponent>({ obstacle_id })
							  .set_name(String(nav_obstacle->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_obstacle, obstacle_id);
		return entity;
	}

	static flecs::entity CreateNav2DRegion(const flecs::world &world, NavigationRegion2D *nav_region) {
		if (nav_region == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID region_id = nav_region->get_rid();
		if (!region_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<NavRegion2DComponent>({ region_id })
							  .set_name(String(nav_region->get_name()).ascii().get_data());
		ObjectIDStorage::add(nav_region, region_id);
		return entity;
	}

	static flecs::entity CreateSourceGeometryParser2D(const flecs::world &world, const Callable &callable, const String &name) {
		const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
		if (!source_geometry_parser_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity()
							  .set<SourceGeometryParser2DComponent>({ source_geometry_parser_id })
							  .set_name(name.ascii().get_data());
		NavigationServer2D::get_singleton()->source_geometry_parser_set_callback(
				source_geometry_parser_id,
				callable);
		return entity;
	}
};
