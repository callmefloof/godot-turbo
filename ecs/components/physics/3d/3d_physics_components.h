#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "../../../../core/templates/rid.h"
#include "../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../servers/physics_server_3d.h"
#include "../../../flecs_types/flecs_entity.h"

struct Area3DComponent {
	RID area_id;
	~Area3DComponent() {
		if (area_id.is_valid()) {
			PhysicsServer3D::get_singleton()->free(area_id);
		}
	}
};

#define AREA_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, area_id, Area3DComponent)\

#define AREA_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, area_id, Area3DComponentRef)\

#define AREA_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, area_id))\

#define AREA_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(area_id)\

DEFINE_COMPONENT_PROXY(Area3DComponentRef, Area3DComponent,
AREA_3D_COMPONENT_PROPERTIES,
AREA_3D_COMPONENT_BINDINGS);

struct Body3DComponent {
	RID body_id;
	~Body3DComponent() {
		if (body_id.is_valid()) {
			PhysicsServer3D::get_singleton()->free(body_id);
		}
	}
};

#define BODY_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, body_id, Body3DComponent)\

#define BODY_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, body_id, Body3DComponentRef)\

#define BODY_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, body_id))\

#define BODY_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(body_id)\

DEFINE_COMPONENT_PROXY(Body3DComponentRef, Body3DComponent,
BODY_3D_COMPONENT_PROPERTIES,
BODY_3D_COMPONENT_BINDINGS);

struct Joint3DComponent {
	RID joint_id;
	~Joint3DComponent() {
		if (joint_id.is_valid()) {
			PhysicsServer3D::get_singleton()->free(joint_id);
		}
	}
};

#define JOINT_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, joint_id,Joint3DComponent)\

#define JOINT_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, joint_id, Joint3DComponentRef)\

#define JOINT_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, joint_id))\

#define JOINT_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(joint_id)\

DEFINE_COMPONENT_PROXY(Joint3DComponentRef, Joint3DComponent,
JOINT_3D_COMPONENT_PROPERTIES,
JOINT_3D_COMPONENT_BINDINGS);

struct SoftBody3DComponent {
	RID soft_body_id;
	~SoftBody3DComponent() {
		if (soft_body_id.is_valid()) {
			PhysicsServer3D::get_singleton()->free(soft_body_id);
		}
	}
};

#define SOFT_BODY_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, soft_body_id,SoftBody3DComponent)\

#define SOFT_BODY_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, soft_body_id, SoftBody3DComponentRef)\

#define SOFT_BODY_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, soft_body_id))\

#define SOFT_BODY_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(soft_body_id)\

DEFINE_COMPONENT_PROXY(SoftBody3DComponentRef, SoftBody3DComponent,
SOFT_BODY_3D_COMPONENT_PROPERTIES,
SOFT_BODY_3D_COMPONENT_BINDINGS);

struct Physics3DBaseComponents {
	flecs::component<Area3DComponent> area;
	flecs::component<Body3DComponent> body;
	flecs::component<Joint3DComponent> joint;
	flecs::component<SoftBody3DComponent> soft_body;

	explicit Physics3DBaseComponents(const flecs::world &world) :
			area(world.component<Area3DComponent>("Area3DComponent")),
			body(world.component<Body3DComponent>("Body3DComponent")),
			joint(world.component<Joint3DComponent>("Joint3DComponent")),
			soft_body(world.component<SoftBody3DComponent>("SoftBody3DComponent")) {}
};
using Physics3DComponentModule = MultiComponentModule<Physics3DBaseComponents>;

