#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "servers/navigation_server_2d.h"
struct NavAgent2DComponent  : ScriptVisibleComponent{
	RID agent_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["agent_id"] = agent_id;
		return dict;

	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "agent_id", agent_id, Variant::RID);
	}
	~NavAgent2DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(agent_id);
		}
	}
};
struct NavLink2DComponent  : ScriptVisibleComponent{
	RID link_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["link_id"] = link_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "link_id", link_id, Variant::RID);
	}
	~NavLink2DComponent() {
		if (link_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(link_id);
		}
	}
};
struct NavObstacle2DComponent  : ScriptVisibleComponent{
	RID obstacle_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["obstacle_id"] = obstacle_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "obstacle_id", obstacle_id, Variant::RID);
	}
	~NavObstacle2DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(obstacle_id);
		}
	}
};
struct NavRegion2DComponent  : ScriptVisibleComponent {
	RID region_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["region_id"] = region_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "region_id", region_id, Variant::RID);
	}
	~NavRegion2DComponent() {
		if (region_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(region_id);
		}
	}
};
struct SourceGeometryParser2DComponent : ScriptVisibleComponent {
	RID source_geometry_parser_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["source_geometry_parser_id"] = source_geometry_parser_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "source_geometry_parser_id",source_geometry_parser_id, Variant::RID);

	}
	~SourceGeometryParser2DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};

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
