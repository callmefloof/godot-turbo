#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "core/templates/rid.h"

namespace godot_turbo::components::physics {

using namespace godot_turbo::components;

struct Area2DComponent {
	RID area_id;
};
struct Body2DComponent {
	RID body_id;
};
struct Joint2DComponent {
	RID joint_id;
};
struct SoftBody2DComponent {
	RID soft_body_id;
};
struct Space2DComponent {
	RID space_id;
};

struct Physics2DBaseComponents {
	flecs::component<Area2DComponent> area;
	flecs::component<Body2DComponent> body;
	flecs::component<Joint2DComponent> joint;
	flecs::component<SoftBody2DComponent> soft_body;
	flecs::component<Space2DComponent> space;

	Physics2DBaseComponents(flecs::world &world) :
			area(world.component<Area2DComponent>("Area2DComponent")),
			body(world.component<Body2DComponent>("Body2DComponent")),
			joint(world.component<Joint2DComponent>("Joint2DComponent")),
			soft_body(world.component<SoftBody2DComponent>("SoftBody2DComponent")),
			space(world.component<Space2DComponent>("Space2DComponent")) {}
};

class Physics2DComponents
		: public MultiComponentModule<Physics2DComponents, Physics2DBaseComponents> {
};

} // namespace godot_turbo::components::physics
