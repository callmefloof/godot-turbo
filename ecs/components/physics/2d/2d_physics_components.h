#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "../../../../../../core/templates/rid.h"
#include "../../../../../../core/os/memory.h"
#include "../../../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../../../servers/physics_server_2d.h"
#include "../../../flecs_types/flecs_component.h"
#include "../../../core/string/ustring.h"


struct Area2DComponent {
	RID area_id;
	~Area2DComponent() {
		if (area_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(area_id);
		}
	}
};

class Area2DComponentRef : public FlecsComponent<Area2DComponent> {
	#define AREA_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, area_id,Area2DComponent)\

	#define AREA_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, area_id, Area2DComponentRef)\

	DEFINE_COMPONENT_PROXY(Area2DComponent,
	AREA_2D_COMPONENT_PROPERTIES,
	AREA_2D_COMPONENT_BINDINGS);
};

struct Body2DComponent {
	RID body_id;
	~Body2DComponent() {
		if (body_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(body_id);
		}
	}
};

class Body2DComponentRef : public FlecsComponent<Body2DComponent> {
	#define BODY_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, body_id,Body2DComponent)\

	#define BODY_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, body_id, Body2DComponentRef)\

	DEFINE_COMPONENT_PROXY(Body2DComponent,
	BODY_2D_COMPONENT_PROPERTIES,
	BODY_2D_COMPONENT_BINDINGS);
};

struct Joint2DComponent {
	RID joint_id;
	~Joint2DComponent() {
		if (joint_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(joint_id);
		}
	}
};

class Joint2DComponentRef : public FlecsComponent<Joint2DComponent> {
	#define JOINT_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, joint_id,Joint2DComponent)\

	#define JOINT_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, joint_id, Joint2DComponentRef)\

	DEFINE_COMPONENT_PROXY(Joint2DComponent,
	JOINT_2D_COMPONENT_PROPERTIES,
	JOINT_2D_COMPONENT_BINDINGS);
};

struct Physics2DBaseComponents {
	flecs::component<Area2DComponent> area;
	flecs::component<Body2DComponent> body;
	flecs::component<Joint2DComponent> joint;

	explicit Physics2DBaseComponents(const flecs::world &world) :
			area(world.component<Area2DComponent>("Area2DComponent")),
			body(world.component<Body2DComponent>("Body2DComponent")),
			joint(world.component<Joint2DComponent>("Joint2DComponent")){}
};

using Physics2DComponentModule =  MultiComponentModule<Physics2DBaseComponents>;
