#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"

namespace godot_turbo::components::navigation {

using namespace godot_turbo::components;

struct NavAgent2DComponent {
	RID agent_id;
};
struct NavLink2DComponent {
	RID link_id;
};
struct NavMap2DComponent {
	RID map_id;
};
struct NavObstacle2DComponent {
	RID obstacle_id;
};
struct NavRegion2DComponent {
	RID region_id;
};
struct SourceGeometryParser2DComponent {
	RID source_geometry_parser_id;
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

class Navigation2DComponents
		: public MultiComponentModule<Navigation2DComponents, Navigation2DBaseComponents> {
};

} // namespace godot_turbo::components::navigation
