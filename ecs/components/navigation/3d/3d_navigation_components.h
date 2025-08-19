#pragma once
#include "core/templates/rid.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

struct NavAgent3DComponent : CompBase {
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
		if (entity.has<NavAgent3DComponent>()) {
			const NavAgent3DComponent &agent_component = entity.get<NavAgent3DComponent>();
			dict.set("agent_id", agent_component.agent_id);
		} else {
			ERR_PRINT("NavAgent3DComponent::to_dict: entity does not have NavAgent3DComponent");
			dict.set("agent_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavAgent3DComponent>()) {
			NavAgent3DComponent &agent_component = entity.get_mut<NavAgent3DComponent>();
			agent_component.agent_id = dict["agent_id"];
		} else {
			ERR_PRINT("NavAgent3DComponent::from_dict: entity does not have NavAgent3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavAgent3DComponent";
	}
};
REGISTER_COMPONENT(NavAgent3DComponent);

struct NavLink3DComponent : CompBase {
	RID link_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("link_id", link_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		link_id = dict["link_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<NavLink3DComponent>()) {
			const NavLink3DComponent &link_component = entity.get<NavLink3DComponent>();
			dict.set("link_id", link_component.link_id);
		} else {
			ERR_PRINT("NavLink3DComponent::to_dict: entity does not have NavLink3DComponent");
			dict.set("link_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavLink3DComponent>()) {
			NavLink3DComponent &link_component = entity.get_mut<NavLink3DComponent>();
			link_component.link_id = dict["link_id"];
		} else {
			ERR_PRINT("NavLink3DComponent::from_dict: entity does not have NavLink3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavLink3DComponent";
	}
};
REGISTER_COMPONENT(NavLink3DComponent);

struct NavObstacle3DComponent : CompBase {
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
		if (entity.has<NavObstacle3DComponent>()) {
			const NavObstacle3DComponent &obstacle_component = entity.get<NavObstacle3DComponent>();
			dict.set("obstacle_id", obstacle_component.obstacle_id);
		} else {
			ERR_PRINT("NavObstacle3DComponent::to_dict: entity does not have NavObstacle3DComponent");
			dict.set("obstacle_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavObstacle3DComponent>()) {
			NavObstacle3DComponent &obstacle_component = entity.get_mut<NavObstacle3DComponent>();
			obstacle_component.obstacle_id = dict["obstacle_id"];
		} else {
			ERR_PRINT("NavObstacle3DComponent::from_dict: entity does not have NavObstacle3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavObstacle3DComponent";
	}
};
REGISTER_COMPONENT(NavObstacle3DComponent);

struct NavRegion3DComponent : CompBase {
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
		if (entity.has<NavRegion3DComponent>()) {
			const NavRegion3DComponent &region_component = entity.get<NavRegion3DComponent>();
			dict.set("region_id", region_component.region_id);
		} else {
			ERR_PRINT("NavRegion3DComponent::to_dict: entity does not have NavRegion3DComponent");
			dict.set("region_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<NavRegion3DComponent>()) {
			NavRegion3DComponent &region_component = entity.get_mut<NavRegion3DComponent>();
			region_component.region_id = dict["region_id"];
		} else {
			ERR_PRINT("NavRegion3DComponent::from_dict: entity does not have NavRegion3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "NavRegion3DComponent";
	}
};
REGISTER_COMPONENT(NavRegion3DComponent);

struct SourceGeometryParser3DComponent : CompBase {
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
		if (entity.has<SourceGeometryParser3DComponent>()) {
			const SourceGeometryParser3DComponent &source_geometry_parser_component = entity.get<SourceGeometryParser3DComponent>();
			dict.set("source_geometry_parser_id", source_geometry_parser_component.source_geometry_parser_id);
		} else {
			ERR_PRINT("SourceGeometryParser3DComponent::to_dict: entity does not have SourceGeometryParser3DComponent");
			dict.set("source_geometry_parser_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SourceGeometryParser3DComponent>()) {
			SourceGeometryParser3DComponent &source_geometry_parser_component = entity.get_mut<SourceGeometryParser3DComponent>();
			source_geometry_parser_component.source_geometry_parser_id = dict["source_geometry_parser_id"];
		} else {
			ERR_PRINT("SourceGeometryParser3DComponent::from_dict: entity does not have SourceGeometryParser3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "SourceGeometryParser3DComponent";
	}
};
REGISTER_COMPONENT(SourceGeometryParser3DComponent);

struct Navigation3DBaseComponents {
	flecs::component<NavAgent3DComponent> agent;
	flecs::component<NavLink3DComponent> link;
	flecs::component<NavObstacle3DComponent> obstacle;
	flecs::component<NavRegion3DComponent> region;
	flecs::component<SourceGeometryParser3DComponent> source_geometry_parser;

	explicit Navigation3DBaseComponents(flecs::world &world) :
			agent(world.component<NavAgent3DComponent>("NavAgent3DComponent")),
			link(world.component<NavLink3DComponent>("NavLink3DComponent")),
			obstacle(world.component<NavObstacle3DComponent>("NavObstacle3DComponent")),
			region(world.component<NavRegion3DComponent>("NavRegion3DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser3DComponent>("SourceGeometryParser3DComponent")) {
				ComponentRegistry::bind_to_world("NavAgent3DComponent", agent.id());
				ComponentRegistry::bind_to_world("NavLink3DComponent", link.id());
				ComponentRegistry::bind_to_world("NavObstacle3DComponent", obstacle.id());
				ComponentRegistry::bind_to_world("NavRegion3DComponent", region.id());
				ComponentRegistry::bind_to_world("SourceGeometryParser3DComponent", source_geometry_parser.id());
			}

};
