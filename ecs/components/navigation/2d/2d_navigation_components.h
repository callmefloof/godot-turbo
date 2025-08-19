#pragma once
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

struct NavAgent2DComponent : CompBase {
	RID agent_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("agent_id", agent_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		agent_id = dict["agent_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<NavAgent2DComponent>()) {
			const NavAgent2DComponent &agent_component = entity.get<NavAgent2DComponent>();
			dict.set("agent_id", agent_component.agent_id);
		} else {
			ERR_PRINT("NavAgent2DComponent::to_dict: entity does not have NavAgent2DComponent");
			dict.set("agent_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavAgent2DComponent>()) {
			NavAgent2DComponent &agent_component = entity.get_mut<NavAgent2DComponent>();
			agent_component.agent_id = dict["agent_id"];
		} else {
			ERR_PRINT("NavAgent2DComponent::from_dict: entity does not have NavAgent2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavAgent2DComponent";
	}


};

REGISTER_COMPONENT(NavAgent2DComponent);

struct NavLink2DComponent : CompBase {
	RID link_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("link_id", link_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		link_id = dict["link_id"];
	}

	StringName get_type_name() const override {
		return "NavLink2DComponent";
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<NavLink2DComponent>()) {
			const NavLink2DComponent &link_component = entity.get<NavLink2DComponent>();
			dict.set("link_id", link_component.link_id);
		} else {
			ERR_PRINT("NavLink2DComponent::to_dict: entity does not have NavLink2DComponent");
			dict.set("link_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavLink2DComponent>()) {
			NavLink2DComponent &link_component = entity.get_mut<NavLink2DComponent>();
			link_component.link_id = dict["link_id"];
		} else {
			ERR_PRINT("NavLink2DComponent::from_dict: entity does not have NavLink2DComponent");
		}
	}

};

REGISTER_COMPONENT(NavLink2DComponent);

struct NavObstacle2DComponent : CompBase {
	RID obstacle_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("obstacle_id", obstacle_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		obstacle_id = dict["obstacle_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<NavObstacle2DComponent>()) {
			const NavObstacle2DComponent &obstacle_component = entity.get<NavObstacle2DComponent>();
			dict.set("obstacle_id", obstacle_component.obstacle_id);
		} else {
			ERR_PRINT("NavObstacle2DComponent::to_dict: entity does not have NavObstacle2DComponent");
			dict.set("obstacle_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavObstacle2DComponent>()) {
			NavObstacle2DComponent &obstacle_component = entity.get_mut<NavObstacle2DComponent>();
			obstacle_component.obstacle_id = dict["obstacle_id"];
		} else {
			ERR_PRINT("NavObstacle2DComponent::from_dict: entity does not have NavObstacle2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavObstacle2DComponent";
	}
};

REGISTER_COMPONENT(NavObstacle2DComponent);

struct NavRegion2DComponent : CompBase {
	RID region_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("region_id", region_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		region_id = dict["region_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<NavRegion2DComponent>()) {
			const NavRegion2DComponent &region_component = entity.get<NavRegion2DComponent>();
			dict.set("region_id", region_component.region_id);
		} else {
			ERR_PRINT("NavRegion2DComponent::to_dict: entity does not have NavRegion2DComponent");
			dict.set("region_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavRegion2DComponent>()) {
			NavRegion2DComponent &region_component = entity.get_mut<NavRegion2DComponent>();
			region_component.region_id = dict["region_id"];
		} else {
			ERR_PRINT("NavRegion2DComponent::from_dict: entity does not have NavRegion2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavRegion2DComponent";
	}
};

REGISTER_COMPONENT(NavRegion2DComponent);

struct SourceGeometryParser2DComponent : CompBase {
	RID source_geometry_parser_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("source_geometry_parser_id", source_geometry_parser_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		source_geometry_parser_id = dict["source_geometry_parser_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<SourceGeometryParser2DComponent>()) {
			const SourceGeometryParser2DComponent &source_geometry_parser_component = entity.get<SourceGeometryParser2DComponent>();
			dict.set("source_geometry_parser_id", source_geometry_parser_component.source_geometry_parser_id);
		} else {
			ERR_PRINT("SourceGeometryParser2DComponent::to_dict: entity does not have SourceGeometryParser2DComponent");
			dict.set("source_geometry_parser_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SourceGeometryParser2DComponent>()) {
			SourceGeometryParser2DComponent &source_geometry_parser_component = entity.get_mut<SourceGeometryParser2DComponent>();
			source_geometry_parser_component.source_geometry_parser_id = dict["source_geometry_parser_id"];
		} else {
			ERR_PRINT("SourceGeometryParser2DComponent::from_dict: entity does not have SourceGeometryParser2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "SourceGeometryParser2DComponent";
	}
};

REGISTER_COMPONENT(SourceGeometryParser2DComponent);

struct Navigation2DBaseComponents {
	flecs::component<NavAgent2DComponent> agent;
	flecs::component<NavLink2DComponent> link;
	flecs::component<NavObstacle2DComponent> obstacle;
	flecs::component<NavRegion2DComponent> region;
	flecs::component<SourceGeometryParser2DComponent> source_geometry_parser;

	explicit Navigation2DBaseComponents(flecs::world &world) :
			agent(world.component<NavAgent2DComponent>("NavAgent2DComponent")),
			link(world.component<NavLink2DComponent>("NavLink2DComponent")),
			obstacle(world.component<NavObstacle2DComponent>("NavObstacle2DComponent")),
			region(world.component<NavRegion2DComponent>("NavRegion2DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser2DComponent>("SourceGeometryParser2DComponent")) {
				ComponentRegistry::bind_to_world("NavAgent2DComponent", agent.id());
				ComponentRegistry::bind_to_world("NavLink2DComponent", link.id());
				ComponentRegistry::bind_to_world("NavObstacle2DComponent", obstacle.id());
				ComponentRegistry::bind_to_world("NavRegion2DComponent", region.id());
				ComponentRegistry::bind_to_world("SourceGeometryParser2DComponent", source_geometry_parser.id());
			}
};