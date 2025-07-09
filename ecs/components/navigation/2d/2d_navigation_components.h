#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "servers/navigation_server_2d.h"
struct NavAgent2DComponent {
	RID agent_id;
	~NavAgent2DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(agent_id);
		}
	}
};
struct NavLink2DComponent {
	RID link_id;
	~NavLink2DComponent() {
		if (link_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(link_id);
		}
	}
};
struct NavMap2DComponent {
	RID map_id;
	~NavMap2DComponent() {
		if (map_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(map_id);
		}
	}
};
struct NavObstacle2DComponent {
	RID obstacle_id;
	~NavObstacle2DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(obstacle_id);
		}
	}
};
struct NavRegion2DComponent {
	RID region_id;
	~NavRegion2DComponent() {
		if (region_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(region_id);
		}
	}
};
struct SourceGeometryParser2DComponent {
	RID source_geometry_parser_id;
	~SourceGeometryParser2DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};

struct Navigation2DBaseComponents {
	flecs::component<NavAgent2DComponent> agent;
	flecs::component<NavLink2DComponent> link;
	flecs::component<NavMap2DComponent> map;
	flecs::component<NavObstacle2DComponent> obstacle;
	flecs::component<NavRegion2DComponent> region;
	flecs::component<SourceGeometryParser2DComponent> source_geometry_parser;

	Navigation2DBaseComponents(flecs::world &world) :
			agent(world.component<NavAgent2DComponent>("NavAgent2DComponent")),
			link(world.component<NavLink2DComponent>("NavLink2DComponent")),
			map(world.component<NavMap2DComponent>("NavMap2DComponent")),
			obstacle(world.component<NavObstacle2DComponent>("NavObstacle2DComponent")),
			region(world.component<NavRegion2DComponent>("NavRegion2DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser2DComponent>("SourceGeometryParser2DComponent")) {}
};

using Navigation2DComponentModule = MultiComponentModule<Navigation2DBaseComponents>;
