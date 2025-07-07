#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"

namespace godot_turbo::components::navigation {

using namespace godot_turbo::components;

struct NavAgent3DComponent {
	RID agent_id = RID();
};
struct NavLink3DComponent {
	RID link_id = RID();
};
struct NavMap3DComponent {
	RID map_id = RID();
};
struct NavObstacle3DComponent {
	RID obstacle_id = RID();
};
struct NavRegion3DComponent {
	RID region_id = RID();
};
struct SourceGeometryParser3DComponent {
	RID source_geometry_parser_id = RID();
};

struct Navigation3DBaseComponents {
	flecs::component<NavAgent3DComponent> agent;
	flecs::component<NavLink3DComponent> link;
	flecs::component<NavMap3DComponent> map;
	flecs::component<NavObstacle3DComponent> obstacle;
	flecs::component<NavRegion3DComponent> region;
	flecs::component<SourceGeometryParser3DComponent> source_geometry_parser;

	Navigation3DBaseComponents(flecs::world &world) :
			agent(world.component<NavAgent3DComponent>("NavAgent3DComponent")),
			link(world.component<NavLink3DComponent>("NavLink3DComponent")),
			map(world.component<NavMap3DComponent>("NavMap3DComponent")),
			obstacle(world.component<NavObstacle3DComponent>("NavObstacle3DComponent")),
			region(world.component<NavRegion3DComponent>("NavRegion3DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser3DComponent>("SourceGeometryParser3DComponent")) {}
};

class Navigation3DComponents
		: public MultiComponentModule<Navigation3DComponents, Navigation3DBaseComponents> {
	// Inherits everything
};

} // namespace godot_turbo::components::navigation
