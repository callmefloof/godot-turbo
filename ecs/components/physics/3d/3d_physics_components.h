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

class Area3DComponentRef : public FlecsComponent<Area3DComponent> {
GDCLASS(Area3DComponentRef, FlecsComponent<Area3DComponent>)
public : RID get_area_id() const {
		auto typed = get_typed_data<Area3DComponent>();
		if (typed) {
			return typed->area_id;
		}
		return default_value<RID>();
	}
	void set_area_id(RID value) const {
		auto typed = get_typed_data<Area3DComponent>();
		if (typed) {
			typed->area_id = value;
		}
	}
	static Ref<Area3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Area3DComponentRef> class_ref = Ref<Area3DComponentRef>(memnew(Area3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Area3DComponent>({});
		class_ref->set_data(&entity->get_mut<Area3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Area3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Area3DComponent>();
			Area3DComponent *copied = memnew(Area3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Area3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Area3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "area_id",
					&Area3DComponentRef::get_area_id);
			ClassDB::bind_method("set_"
								 "area_id",
					&Area3DComponentRef::set_area_id);
			::ClassDB::add_property(Area3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "area_id"), _scs_create("set_"
																																			 "area_id"),
					_scs_create("get_"
								"area_id"));
		} while (0);
		ClassDB::bind_static_method(Area3DComponentRef::get_class_static(), "create_component", &Area3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Area3DComponentRef::get_type_name);
	}
};
;

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

class Body3DComponentRef : public FlecsComponent<Body3DComponent> {
GDCLASS(Body3DComponentRef, FlecsComponent<Body3DComponent>)
public : RID get_body_id() const {
		auto typed = get_typed_data<Body3DComponent>();
		if (typed) {
			return typed->body_id;
		}
		return default_value<RID>();
	}
	void set_body_id(RID value) const {
		auto typed = get_typed_data<Body3DComponent>();
		if (typed) {
			typed->body_id = value;
		}
	}
	static Ref<Body3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Body3DComponentRef> class_ref = Ref<Body3DComponentRef>(memnew(Body3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Body3DComponent>({});
		class_ref->set_data(&entity->get_mut<Body3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Body3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Body3DComponent>();
			Body3DComponent *copied = memnew(Body3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Body3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Body3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "body_id",
					&Body3DComponentRef::get_body_id);
			ClassDB::bind_method("set_"
								 "body_id",
					&Body3DComponentRef::set_body_id);
			::ClassDB::add_property(Body3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "body_id"), _scs_create("set_"
																																			 "body_id"),
					_scs_create("get_"
								"body_id"));
		} while (0);
		ClassDB::bind_static_method(Body3DComponentRef::get_class_static(), "create_component", &Body3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Body3DComponentRef::get_type_name);
	}
};
;

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

class Joint3DComponentRef : public FlecsComponent<Joint3DComponent> {
GDCLASS(Joint3DComponentRef, FlecsComponent<Joint3DComponent>)
public : RID get_joint_id() const {
		auto typed = get_typed_data<Joint3DComponent>();
		if (typed) {
			return typed->joint_id;
		}
		return default_value<RID>();
	}
	void set_joint_id(RID value) const {
		auto typed = get_typed_data<Joint3DComponent>();
		if (typed) {
			typed->joint_id = value;
		}
	}
	static Ref<Joint3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Joint3DComponentRef> class_ref = Ref<Joint3DComponentRef>(memnew(Joint3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Joint3DComponent>({});
		class_ref->set_data(&entity->get_mut<Joint3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Joint3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Joint3DComponent>();
			Joint3DComponent *copied = memnew(Joint3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Joint3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Joint3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "joint_id",
					&Joint3DComponentRef::get_joint_id);
			ClassDB::bind_method("set_"
								 "joint_id",
					&Joint3DComponentRef::set_joint_id);
			::ClassDB::add_property(Joint3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "joint_id"), _scs_create("set_"
																																			   "joint_id"),
					_scs_create("get_"
								"joint_id"));
		} while (0);
		ClassDB::bind_static_method(Joint3DComponentRef::get_class_static(), "create_component", &Joint3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Joint3DComponentRef::get_type_name);
	}
};
;

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

class SoftBody3DComponentRef : public FlecsComponent<SoftBody3DComponent> {
GDCLASS(SoftBody3DComponentRef, FlecsComponent<SoftBody3DComponent>)
public : RID get_soft_body_id() const {
		auto typed = get_typed_data<SoftBody3DComponent>();
		if (typed) {
			return typed->soft_body_id;
		}
		return default_value<RID>();
	}
	void set_soft_body_id(RID value) const {
		auto typed = get_typed_data<SoftBody3DComponent>();
		if (typed) {
			typed->soft_body_id = value;
		}
	}
	static Ref<SoftBody3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SoftBody3DComponentRef> class_ref = Ref<SoftBody3DComponentRef>(memnew(SoftBody3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SoftBody3DComponent>({});
		class_ref->set_data(&entity->get_mut<SoftBody3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SoftBody3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SoftBody3DComponent>();
			SoftBody3DComponent *copied = memnew(SoftBody3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SoftBody3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SoftBody3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "soft_body_id",
					&SoftBody3DComponentRef::get_soft_body_id);
			ClassDB::bind_method("set_"
								 "soft_body_id",
					&SoftBody3DComponentRef::set_soft_body_id);
			::ClassDB::add_property(SoftBody3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "soft_body_id"), _scs_create("set_"
																																					  "soft_body_id"),
					_scs_create("get_"
								"soft_body_id"));
		} while (0);
		ClassDB::bind_static_method(SoftBody3DComponentRef::get_class_static(), "create_component", &SoftBody3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SoftBody3DComponentRef::get_type_name);
	}
};
;

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

