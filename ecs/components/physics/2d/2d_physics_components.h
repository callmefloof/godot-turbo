#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "../../../../../../core/templates/rid.h"
#include "../../../../../../core/os/memory.h"
#include "../../../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../../../servers/physics_server_2d.h"
#include "../../../flecs_types/flecs_component.h"


struct Area2DComponent {
	RID area_id;
	~Area2DComponent() {
		if (area_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(area_id);
		}
	}
};

#define AREA_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, area_id,Area2DComponent)\

#define AREA_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, area_id, Area2DComponentRef)\

#define AREA_2D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, area_id))\

#define AREA_2D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(area_id)\

class Area2DComponentRef : public FlecsComponent<Area2DComponent> {
GDCLASS(Area2DComponentRef, FlecsComponent<Area2DComponent>)
public : RID get_area_id() const {
		auto typed = get_typed_data<Area2DComponent>();
		if (typed) {
			return typed->area_id;
		}
		return default_value<RID>();
	}
	void set_area_id(RID value) const {
		auto typed = get_typed_data<Area2DComponent>();
		if (typed) {
			typed->area_id = value;
		}
	}
	static Ref<Area2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Area2DComponentRef> class_ref = Ref<Area2DComponentRef>(memnew(Area2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Area2DComponent>({});
		class_ref->set_data(&entity->get_mut<Area2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Area2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Area2DComponent>();
			Area2DComponent *copied = memnew(Area2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Area2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Area2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "area_id",
					&Area2DComponentRef::get_area_id);
			ClassDB::bind_method("set_"
								 "area_id",
					&Area2DComponentRef::set_area_id);
			::ClassDB::add_property(Area2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "area_id"), _scs_create("set_"
																																			 "area_id"),
					_scs_create("get_"
								"area_id"));
		} while (0);
		ClassDB::bind_static_method(Area2DComponentRef::get_class_static(), "create_component", &Area2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Area2DComponentRef::get_type_name);
	}
};
;

struct Body2DComponent {
	RID body_id;
	~Body2DComponent() {
		if (body_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(body_id);
		}
	}
};

#define BODY_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, body_id,Body2DComponent)\

#define BODY_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, body_id, Body2DComponentRef)\

#define BODY_2D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, body_id))\

#define BODY_2D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(body_id)\

class Body2DComponentRef : public FlecsComponent<Body2DComponent> {
GDCLASS(Body2DComponentRef, FlecsComponent<Body2DComponent>)
public : RID get_body_id() const {
		auto typed = get_typed_data<Body2DComponent>();
		if (typed) {
			return typed->body_id;
		}
		return default_value<RID>();
	}
	void set_body_id(RID value) const {
		auto typed = get_typed_data<Body2DComponent>();
		if (typed) {
			typed->body_id = value;
		}
	}
	static Ref<Body2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Body2DComponentRef> class_ref = Ref<Body2DComponentRef>(memnew(Body2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Body2DComponent>({});
		class_ref->set_data(&entity->get_mut<Body2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Body2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Body2DComponent>();
			Body2DComponent *copied = memnew(Body2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Body2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Body2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "body_id",
					&Body2DComponentRef::get_body_id);
			ClassDB::bind_method("set_"
								 "body_id",
					&Body2DComponentRef::set_body_id);
			::ClassDB::add_property(Body2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "body_id"), _scs_create("set_"
																																			 "body_id"),
					_scs_create("get_"
								"body_id"));
		} while (0);
		ClassDB::bind_static_method(Body2DComponentRef::get_class_static(), "create_component", &Body2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Body2DComponentRef::get_type_name);
	}
};
;

struct Joint2DComponent {
	RID joint_id;
	~Joint2DComponent() {
		if (joint_id.is_valid()) {
			PhysicsServer2D::get_singleton()->free(joint_id);
		}
	}
};

#define JOINT_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, joint_id,Joint2DComponent)\

#define JOINT_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, joint_id, Joint2DComponentRef)\

#define JOINT_2D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, joint_id))\

#define JOINT_2D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(joint_id)\

class Joint2DComponentRef : public FlecsComponent<Joint2DComponent> {
GDCLASS(Joint2DComponentRef, FlecsComponent<Joint2DComponent>)
public : RID get_joint_id() const {
		auto typed = get_typed_data<Joint2DComponent>();
		if (typed) {
			return typed->joint_id;
		}
		return default_value<RID>();
	}
	void set_joint_id(RID value) const {
		auto typed = get_typed_data<Joint2DComponent>();
		if (typed) {
			typed->joint_id = value;
		}
	}
	static Ref<Joint2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Joint2DComponentRef> class_ref = Ref<Joint2DComponentRef>(memnew(Joint2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Joint2DComponent>({});
		class_ref->set_data(&entity->get_mut<Joint2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Joint2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Joint2DComponent>();
			Joint2DComponent *copied = memnew(Joint2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Joint2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Joint2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "joint_id",
					&Joint2DComponentRef::get_joint_id);
			ClassDB::bind_method("set_"
								 "joint_id",
					&Joint2DComponentRef::set_joint_id);
			::ClassDB::add_property(Joint2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "joint_id"), _scs_create("set_"
																																			   "joint_id"),
					_scs_create("get_"
								"joint_id"));
		} while (0);
		ClassDB::bind_static_method(Joint2DComponentRef::get_class_static(), "create_component", &Joint2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Joint2DComponentRef::get_type_name);
	}
};
;

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
