#pragma once
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../component_module_base.h"
#include "../../../../core/templates/rid.h"
#include "../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../servers/navigation_server_3d.h"

struct NavAgent3DComponent {
	RID agent_id;
	~NavAgent3DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(agent_id);
		}
	}
};

#define NAV_AGENT_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, agent_id,NavAgent3DComponent)\

#define NAV_AGENT_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, agent_id, NavAgent3DComponentRef)\

#define NAV_AGENT_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, agent_id))\

#define NAV_AGENT_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(agent_id)\

class NavAgent3DComponentRef : public FlecsComponent<NavAgent3DComponent> {
GDCLASS(NavAgent3DComponentRef, FlecsComponent<NavAgent3DComponent>)
public : RID get_agent_id() const {
		auto typed = get_typed_data<NavAgent3DComponent>();
		if (typed) {
			return typed->agent_id;
		}
		return default_value<RID>();
	}
	void set_agent_id(RID value) const {
		auto typed = get_typed_data<NavAgent3DComponent>();
		if (typed) {
			typed->agent_id = value;
		}
	}
	static Ref<NavAgent3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavAgent3DComponentRef> class_ref = Ref<NavAgent3DComponentRef>(memnew(NavAgent3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavAgent3DComponent>({});
		class_ref->set_data(&entity->get_mut<NavAgent3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavAgent3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavAgent3DComponent>();
			NavAgent3DComponent *copied = memnew(NavAgent3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavAgent3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavAgent3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "agent_id",
					&NavAgent3DComponentRef::get_agent_id);
			ClassDB::bind_method("set_"
								 "agent_id",
					&NavAgent3DComponentRef::set_agent_id);
			::ClassDB::add_property(NavAgent3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "agent_id"), _scs_create("set_"
																																				  "agent_id"),
					_scs_create("get_"
								"agent_id"));
		} while (0);
		ClassDB::bind_static_method(NavAgent3DComponentRef::get_class_static(), "create_component", &NavAgent3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavAgent3DComponentRef::get_type_name);
	}
};
;

struct NavLink3DComponent {
	RID link_id;
	~NavLink3DComponent() {
		if (link_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(link_id);
		}
	}
};

#define NAV_LINK_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, link_id,NavLink3DComponent)\

#define NAV_LINK_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, link_id, NavLink3DComponentRef)\

#define NAV_LINK_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, link_id))\

#define NAV_LINK_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(link_id)\

class NavLink3DComponentRef : public FlecsComponent<NavLink3DComponent> {
GDCLASS(NavLink3DComponentRef, FlecsComponent<NavLink3DComponent>)
public : RID get_link_id() const {
		auto typed = get_typed_data<NavLink3DComponent>();
		if (typed) {
			return typed->link_id;
		}
		return default_value<RID>();
	}
	void set_link_id(RID value) const {
		auto typed = get_typed_data<NavLink3DComponent>();
		if (typed) {
			typed->link_id = value;
		}
	}
	static Ref<NavLink3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavLink3DComponentRef> class_ref = Ref<NavLink3DComponentRef>(memnew(NavLink3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavLink3DComponent>({});
		class_ref->set_data(&entity->get_mut<NavLink3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavLink3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavLink3DComponent>();
			NavLink3DComponent *copied = memnew(NavLink3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavLink3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavLink3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "link_id",
					&NavLink3DComponentRef::get_link_id);
			ClassDB::bind_method("set_"
								 "link_id",
					&NavLink3DComponentRef::set_link_id);
			::ClassDB::add_property(NavLink3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "link_id"), _scs_create("set_"
																																				"link_id"),
					_scs_create("get_"
								"link_id"));
		} while (0);
		ClassDB::bind_static_method(NavLink3DComponentRef::get_class_static(), "create_component", &NavLink3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavLink3DComponentRef::get_type_name);
	}
};
;

struct NavObstacle3DComponent {
	RID obstacle_id;
	~NavObstacle3DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(obstacle_id);
		}
	}
};

#define NAV_OBSTACLE_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, obstacle_id,NavObstacle3DComponent)\

#define NAV_OBSTACLE_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, obstacle_id, NavObstacle3DComponentRef)\

#define NAV_OBSTACLE_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, obstacle_id))\

#define NAV_OBSTACLE_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(obstacle_id)\

class NavObstacle3DComponentRef : public FlecsComponent<NavObstacle3DComponent> {
GDCLASS(NavObstacle3DComponentRef, FlecsComponent<NavObstacle3DComponent>)
public : RID get_obstacle_id() const {
		auto typed = get_typed_data<NavObstacle3DComponent>();
		if (typed) {
			return typed->obstacle_id;
		}
		return default_value<RID>();
	}
	void set_obstacle_id(RID value) const {
		auto typed = get_typed_data<NavObstacle3DComponent>();
		if (typed) {
			typed->obstacle_id = value;
		}
	}
	static Ref<NavObstacle3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavObstacle3DComponentRef> class_ref = Ref<NavObstacle3DComponentRef>(memnew(NavObstacle3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavObstacle3DComponent>({});
		class_ref->set_data(&entity->get_mut<NavObstacle3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavObstacle3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavObstacle3DComponent>();
			NavObstacle3DComponent *copied = memnew(NavObstacle3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavObstacle3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavObstacle3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "obstacle_id",
					&NavObstacle3DComponentRef::get_obstacle_id);
			ClassDB::bind_method("set_"
								 "obstacle_id",
					&NavObstacle3DComponentRef::set_obstacle_id);
			::ClassDB::add_property(NavObstacle3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "obstacle_id"), _scs_create("set_"
																																						"obstacle_id"),
					_scs_create("get_"
								"obstacle_id"));
		} while (0);
		ClassDB::bind_static_method(NavObstacle3DComponentRef::get_class_static(), "create_component", &NavObstacle3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavObstacle3DComponentRef::get_type_name);
	}
};
;

struct NavRegion3DComponent {
	RID region_id;
	~NavRegion3DComponent() {
		if (region_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(region_id);
		}
	}
};

#define NAV_REGION_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, region_id,NavRegion3DComponent)\

#define NAV_REGION_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, region_id, NavRegion3DComponentRef)\

#define NAV_REGION_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, region_id))\

#define NAV_REGION_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(region_id)\

class NavRegion3DComponentRef : public FlecsComponent<NavRegion3DComponent> {
GDCLASS(NavRegion3DComponentRef, FlecsComponent<NavRegion3DComponent>)
public : RID get_region_id() const {
		auto typed = get_typed_data<NavRegion3DComponent>();
		if (typed) {
			return typed->region_id;
		}
		return default_value<RID>();
	}
	void set_region_id(RID value) const {
		auto typed = get_typed_data<NavRegion3DComponent>();
		if (typed) {
			typed->region_id = value;
		}
	}
	static Ref<NavRegion3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavRegion3DComponentRef> class_ref = Ref<NavRegion3DComponentRef>(memnew(NavRegion3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavRegion3DComponent>({});
		class_ref->set_data(&entity->get_mut<NavRegion3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavRegion3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavRegion3DComponent>();
			NavRegion3DComponent *copied = memnew(NavRegion3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavRegion3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavRegion3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "region_id",
					&NavRegion3DComponentRef::get_region_id);
			ClassDB::bind_method("set_"
								 "region_id",
					&NavRegion3DComponentRef::set_region_id);
			::ClassDB::add_property(NavRegion3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "region_id"), _scs_create("set_"
																																					"region_id"),
					_scs_create("get_"
								"region_id"));
		} while (0);
		ClassDB::bind_static_method(NavRegion3DComponentRef::get_class_static(), "create_component", &NavRegion3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavRegion3DComponentRef::get_type_name);
	}
};
;

struct SourceGeometryParser3DComponent {
	RID source_geometry_parser_id;
	~SourceGeometryParser3DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer3D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};

#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, source_geometry_parser_id,SourceGeometryParser3DComponent)\

#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, source_geometry_parser_id, SourceGeometryParser3DComponentRef)\

#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, source_geometry_parser_id))\

#define SOURCE_GEOMETRY_PARSER_3D_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(source_geometry_parser_id)\

class SourceGeometryParser3DComponentRef : public FlecsComponent<SourceGeometryParser3DComponent> {
GDCLASS(SourceGeometryParser3DComponentRef, FlecsComponent<SourceGeometryParser3DComponent>)
public : RID get_source_geometry_parser_id() const {
		auto typed = get_typed_data<SourceGeometryParser3DComponent>();
		if (typed) {
			return typed->source_geometry_parser_id;
		}
		return default_value<RID>();
	}
	void set_source_geometry_parser_id(RID value) const {
		auto typed = get_typed_data<SourceGeometryParser3DComponent>();
		if (typed) {
			typed->source_geometry_parser_id = value;
		}
	}
	static Ref<SourceGeometryParser3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SourceGeometryParser3DComponentRef> class_ref = Ref<SourceGeometryParser3DComponentRef>(memnew(SourceGeometryParser3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SourceGeometryParser3DComponent>({});
		class_ref->set_data(&entity->get_mut<SourceGeometryParser3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SourceGeometryParser3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SourceGeometryParser3DComponent>();
			SourceGeometryParser3DComponent *copied = memnew(SourceGeometryParser3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SourceGeometryParser3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SourceGeometryParser3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "source_geometry_parser_id",
					&SourceGeometryParser3DComponentRef::get_source_geometry_parser_id);
			ClassDB::bind_method("set_"
								 "source_geometry_parser_id",
					&SourceGeometryParser3DComponentRef::set_source_geometry_parser_id);
			::ClassDB::add_property(SourceGeometryParser3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "source_geometry_parser_id"), _scs_create("set_"
																																											   "source_geometry_parser_id"),
					_scs_create("get_"
								"source_geometry_parser_id"));
		} while (0);
		ClassDB::bind_static_method(SourceGeometryParser3DComponentRef::get_class_static(), "create_component", &SourceGeometryParser3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SourceGeometryParser3DComponentRef::get_type_name);
	}
};
;

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

