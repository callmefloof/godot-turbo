#pragma once
#include "../../../core/object/ref_counted.h"
#include "../../../core/templates/rid.h"
#include "../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../servers/navigation_server_3d.h"
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "servers/navigation_server_3d.h"

struct NavAgent3DComponent {
	RID agent_id;
	~NavAgent3DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(agent_id);
		}
	}
};

class NavAgent3DComponentRef : public FlecsComponent<NavAgent3DComponent> {
	#define NAV_AGENT_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, agent_id,NavAgent3DComponent)\

	#define NAV_AGENT_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, agent_id, NavAgent3DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavAgent3DComponentRef, NavAgent3DComponent,
	NAV_AGENT_3D_COMPONENT_PROPERTIES,
	NAV_AGENT_3D_COMPONENT_BINDINGS);
};

struct NavLink3DComponent {
	RID link_id;
	~NavLink3DComponent() {
		if (link_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(link_id);
		}
	}
};

class NavLink3DComponentRef : public FlecsComponent<NavLink3DComponent> {
	#define NAV_LINK_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, link_id,NavLink3DComponent)\

	#define NAV_LINK_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, link_id, NavLink3DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavLink3DComponentRef, NavLink3DComponent,
	NAV_LINK_3D_COMPONENT_PROPERTIES,
	NAV_LINK_3D_COMPONENT_BINDINGS);
};

struct NavObstacle3DComponent {
	RID obstacle_id;
	~NavObstacle3DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(obstacle_id);
		}
	}
};

class NavObstacle3DComponentRef : public FlecsComponent<NavObstacle3DComponent> {
	#define NAV_OBSTACLE_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, obstacle_id,NavObstacle3DComponent)\

	#define NAV_OBSTACLE_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, obstacle_id, NavObstacle3DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavObstacle3DComponentRef, NavObstacle3DComponent,
	NAV_OBSTACLE_3D_COMPONENT_PROPERTIES,
	NAV_OBSTACLE_3D_COMPONENT_BINDINGS);
};

struct NavRegion3DComponent {
	RID region_id;
	~NavRegion3DComponent() {
		if (region_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(region_id);
		}
	}
};

class NavRegion3DComponentRef : public FlecsComponent<NavRegion3DComponent> {
	#define NAV_REGION_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, region_id,NavRegion3DComponent)\

	#define NAV_REGION_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, region_id, NavRegion3DComponentRef)\

	DEFINE_COMPONENT_PROXY(NavRegion3DComponentRef, NavRegion3DComponent,
	NAV_REGION_3D_COMPONENT_PROPERTIES,
	NAV_REGION_3D_COMPONENT_BINDINGS);
};

struct SourceGeometryParser3DComponent {
	RID source_geometry_parser_id;
	~SourceGeometryParser3DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};

class SourceGeometryParser3DComponentRef : public FlecsComponent<SourceGeometryParser3DComponent> {
	#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, source_geometry_parser_id,SourceGeometryParser3DComponent)\

	#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, source_geometry_parser_id, SourceGeometryParser3DComponentRef)\

	DEFINE_COMPONENT_PROXY(SourceGeometryParser3DComponentRef, SourceGeometryParser3DComponent,
	SOURCE_GEOMETRY_PARSER_3D_COMPONENT_PROPERTIES,
	SOURCE_GEOMETRY_PARSER_3D_COMPONENT_BINDINGS);
};


struct Navigation3DBaseComponents {
	flecs::component<NavAgent3DComponent> agent;
	flecs::component<NavLink3DComponent> link;
	flecs::component<NavObstacle3DComponent> obstacle;
	flecs::component<NavRegion3DComponent> region;
	flecs::component<SourceGeometryParser3DComponent> source_geometry_parser;

	explicit Navigation3DBaseComponents(const flecs::world &world) :
			agent(world.component<NavAgent3DComponent>("NavAgent3DComponent")),
			link(world.component<NavLink3DComponent>("NavLink3DComponent")),
			obstacle(world.component<NavObstacle3DComponent>("NavObstacle3DComponent")),
			region(world.component<NavRegion3DComponent>("NavRegion3DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser3DComponent>("SourceGeometryParser3DComponent")) {}
};
using Navigation3DComponentModule = MultiComponentModule<Navigation3DBaseComponents>;

