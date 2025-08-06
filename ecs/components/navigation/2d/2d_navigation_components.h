#pragma once
#include "core/templates/rid.h"
#include "modules/godot_turbo/ecs/components/component_proxy.h"
#include "servers/navigation_server_2d.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/flecs_types/flecs_component.h"
#include "modules/godot_turbo/ecs/components/component_module_base.h"
#include "core/string/ustring.h"


class FlecsEntity;

struct NavAgent2DComponent {

	RID agent_id;
	~NavAgent2DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(agent_id);
		}
	}
};

class NavAgent2DComponentRef : public FlecsComponent<NavAgent2DComponent> {
	#define NAV_AGENT_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, agent_id,NavAgent2DComponent)\


	#define NAV_AGENT_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, agent_id, NavAgent2DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavAgent2DComponent,
	NAV_AGENT_2D_COMPONENT_PROPERTIES,
	NAV_AGENT_2D_COMPONENT_BINDINGS);
};

struct NavLink2DComponent {
	RID link_id;
	~NavLink2DComponent() {
		if (link_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(link_id);
		}
	}
};

class NavLink2DComponentRef : public FlecsComponent<NavLink2DComponent> {
	#define NAV_LINK_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, link_id,NavLink2DComponent)\


	#define NAV_LINK_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, link_id, NavLink2DComponentRef)\

	DEFINE_COMPONENT_PROXY( NavLink2DComponent,
	NAV_LINK_2D_COMPONENT_PROPERTIES,
	NAV_LINK_2D_COMPONENT_BINDINGS);
};


struct NavObstacle2DComponent {
	RID obstacle_id;
	~NavObstacle2DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(obstacle_id);
		}
	}
};
	
class NavObstacle2DComponentRef : public FlecsComponent<NavObstacle2DComponent> {
	#define NAV_OBSTACLE_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, obstacle_id,NavObstacle2DComponent)\


	#define NAV_OBSTACLE_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, obstacle_id, NavObstacle2DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavObstacle2DComponent,
	NAV_OBSTACLE_2D_COMPONENT_PROPERTIES,
	NAV_OBSTACLE_2D_COMPONENT_BINDINGS);
};

struct NavRegion2DComponent {
	RID region_id;
	~NavRegion2DComponent() {
		if (region_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(region_id);
		}
	}
};

class NavRegion2DComponentRef : public FlecsComponent<NavRegion2DComponent> {
	#define NAV_REGION_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, region_id,NavRegion2DComponent)\

	#define NAV_REGION_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, region_id, NavRegion2DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavRegion2DComponent,
	NAV_REGION_2D_COMPONENT_PROPERTIES,
	NAV_REGION_2D_COMPONENT_BINDINGS);
};

struct SourceGeometryParser2DComponent {
	RID source_geometry_parser_id;
	~SourceGeometryParser2DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};
	
class SourceGeometryParser2DComponentRef : public FlecsComponent<SourceGeometryParser2DComponent> {
	#define SOURCE_GEOMETRY_PARSER_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, source_geometry_parser_id,SourceGeometryParser2DComponent)\

	#define SOURCE_GEOMETRY_PARSER_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, source_geometry_parser_id, SourceGeometryParser2DComponentRef)\

	DEFINE_COMPONENT_PROXY(SourceGeometryParser2DComponent,
	SOURCE_GEOMETRY_PARSER_2D_COMPONENT_PROPERTIES,
	SOURCE_GEOMETRY_PARSER_2D_COMPONENT_BINDINGS);
};
;

struct Navigation2DBaseComponents {
	flecs::component<NavAgent2DComponent> agent;
	flecs::component<NavLink2DComponent> link;
	flecs::component<NavObstacle2DComponent> obstacle;
	flecs::component<NavRegion2DComponent> region;
	flecs::component<SourceGeometryParser2DComponent> source_geometry_parser;

	explicit Navigation2DBaseComponents(const flecs::world &world) :
			agent(world.component<NavAgent2DComponent>("NavAgent2DComponent")),
			link(world.component<NavLink2DComponent>("NavLink2DComponent")),
			obstacle(world.component<NavObstacle2DComponent>("NavObstacle2DComponent")),
			region(world.component<NavRegion2DComponent>("NavRegion2DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser2DComponent>("SourceGeometryParser2DComponent")) {}
};

using Navigation2DComponentModule = MultiComponentModule<Navigation2DBaseComponents>;
#undef NAV_AGENT_2D_COMPONENT_BINDINGS