#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "servers/navigation_server_3d.h"

struct NavAgent3DComponent : ScriptVisibleComponent{
	RID agent_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["agent_id"] = agent_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "agent_id", agent_id, Variant::RID);
	}
	~NavAgent3DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(agent_id);
		}
	}
};
struct NavLink3DComponent : ScriptVisibleComponent {
	RID link_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["link_id"] = link_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "link_id", link_id, Variant::RID);

	}
	~NavLink3DComponent() {
		if (link_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(link_id);
		}
	}
};
struct NavObstacle3DComponent  : ScriptVisibleComponent{
	RID obstacle_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["obstacle_id"] = obstacle_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "obstacle_id", obstacle_id, Variant::RID);
	}
	~NavObstacle3DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(obstacle_id);
		}
	}
};
struct NavRegion3DComponent : ScriptVisibleComponent {
	RID region_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["region_id"] = region_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "region_id", region_id, Variant::RID);

	}
	~NavRegion3DComponent() {
		if (region_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(region_id);
		}
	}
};
struct SourceGeometryParser3DComponent  : ScriptVisibleComponent{
	RID source_geometry_parser_id;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["source_geometry_parser_id"] = source_geometry_parser_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "source_geometry_parser_id", source_geometry_parser_id, Variant::RID);

	}
	~SourceGeometryParser3DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(source_geometry_parser_id);
		}
	}
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

