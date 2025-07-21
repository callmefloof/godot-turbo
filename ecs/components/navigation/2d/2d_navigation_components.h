#pragma once
#include "../../../../core/templates/rid.h"
#include "../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../servers/navigation_server_2d.h"
#include "../../../../thirdparty/flecs/distr/flecs.h"
#include "../../../flecs_types/flecs_component.h"
#include "../../../flecs_types/flecs_entity.h"
#include "modules/godot_turbo/ecs/components/component_module_base.h"

class FlecsEntity;

struct NavAgent2DComponent {

	RID agent_id;
	~NavAgent2DComponent() {
		if (agent_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(agent_id);
		}
	}
};

#define NAV_AGENT_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, agent_id,NavAgent2DComponent)\


#define NAV_AGENT_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, agent_id, NavAgent2DComponentRef)\

class NavAgent2DComponentRef : public FlecsComponent<NavAgent2DComponent> {
GDCLASS(NavAgent2DComponentRef, FlecsComponent<NavAgent2DComponent>)
public : RID get_agent_id() const {
		auto typed = get_typed_data<NavAgent2DComponent>();
		if (typed) {
			return typed->agent_id;
		}
		return default_value<RID>();
	}
	void set_agent_id(RID value) const {
		auto typed = get_typed_data<NavAgent2DComponent>();
		if (typed) {
			typed->agent_id = value;
		}
	}
	static Ref<NavAgent2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavAgent2DComponentRef> class_ref = Ref<NavAgent2DComponentRef>(memnew(NavAgent2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavAgent2DComponent>({});
		class_ref->set_data(&entity->get_mut<NavAgent2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavAgent2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavAgent2DComponent>();
			NavAgent2DComponent *copied = memnew(NavAgent2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavAgent2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavAgent2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method(D_METHOD("get_"
										  "agent_id"),
					&NavAgent2DComponentRef::get_agent_id);
			ClassDB::bind_method(D_METHOD("set_"
										  "agent_id",
										 "value"),
					&NavAgent2DComponentRef::set_agent_id);
			::ClassDB::add_property(NavAgent2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "agent_id"), _scs_create("set_"
																																				  "agent_id"),
					_scs_create("get_"
								"agent_id"));
		} while (0);
		ClassDB::bind_static_method(NavAgent2DComponentRef::get_class_static(), "create_component", &NavAgent2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavAgent2DComponentRef::get_type_name);
	}
};
;

struct NavLink2DComponent {
	RID link_id;
	~NavLink2DComponent() {
		if (link_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(link_id);
		}
	}
};

#define NAV_LINK_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, link_id,NavLink2DComponent)\


#define NAV_LINK_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, link_id, NavLink2DComponentRef)\

class NavLink2DComponentRef : public FlecsComponent<NavLink2DComponent> {
GDCLASS(NavLink2DComponentRef, FlecsComponent<NavLink2DComponent>)
public : RID get_link_id() const {
		auto typed = get_typed_data<NavLink2DComponent>();
		if (typed) {
			return typed->link_id;
		}
		return default_value<RID>();
	}
	void set_link_id(RID value) const {
		auto typed = get_typed_data<NavLink2DComponent>();
		if (typed) {
			typed->link_id = value;
		}
	}
	static Ref<NavLink2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavLink2DComponentRef> class_ref = Ref<NavLink2DComponentRef>(memnew(NavLink2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavLink2DComponent>({});
		class_ref->set_data(&entity->get_mut<NavLink2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavLink2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavLink2DComponent>();
			NavLink2DComponent *copied = memnew(NavLink2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavLink2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavLink2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method(D_METHOD("get_"
										  "link_id"),
					&NavLink2DComponentRef::get_link_id);
			ClassDB::bind_method(D_METHOD("set_"
										  "link_id",
										 "value"),
					&NavLink2DComponentRef::set_link_id);
			::ClassDB::add_property(NavLink2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "link_id"), _scs_create("set_"
																																				"link_id"),
					_scs_create("get_"
								"link_id"));
		} while (0);
		ClassDB::bind_static_method(NavLink2DComponentRef::get_class_static(), "create_component", &NavLink2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavLink2DComponentRef::get_type_name);
	}
};
;

struct NavObstacle2DComponent {
	RID obstacle_id;
	~NavObstacle2DComponent() {
		if (obstacle_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(obstacle_id);
		}
	}
};

#define NAV_OBSTACLE_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, obstacle_id,NavObstacle2DComponent)\


#define NAV_OBSTACLE_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, obstacle_id, NavObstacle2DComponentRef)\

class NavObstacle2DComponentRef : public FlecsComponent<NavObstacle2DComponent> {
GDCLASS(NavObstacle2DComponentRef, FlecsComponent<NavObstacle2DComponent>)
public : RID get_obstacle_id() const {
		auto typed = get_typed_data<NavObstacle2DComponent>();
		if (typed) {
			return typed->obstacle_id;
		}
		return default_value<RID>();
	}
	void set_obstacle_id(RID value) const {
		auto typed = get_typed_data<NavObstacle2DComponent>();
		if (typed) {
			typed->obstacle_id = value;
		}
	}
	static Ref<NavObstacle2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavObstacle2DComponentRef> class_ref = Ref<NavObstacle2DComponentRef>(memnew(NavObstacle2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavObstacle2DComponent>({});
		class_ref->set_data(&entity->get_mut<NavObstacle2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavObstacle2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavObstacle2DComponent>();
			NavObstacle2DComponent *copied = memnew(NavObstacle2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavObstacle2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavObstacle2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method(D_METHOD("get_"
										  "obstacle_id"),
					&NavObstacle2DComponentRef::get_obstacle_id);
			ClassDB::bind_method(D_METHOD("set_"
										  "obstacle_id",
										 "value"),
					&NavObstacle2DComponentRef::set_obstacle_id);
			::ClassDB::add_property(NavObstacle2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "obstacle_id"), _scs_create("set_"
																																						"obstacle_id"),
					_scs_create("get_"
								"obstacle_id"));
		} while (0);
		ClassDB::bind_static_method(NavObstacle2DComponentRef::get_class_static(), "create_component", &NavObstacle2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavObstacle2DComponentRef::get_type_name);
	}
};
;

struct NavRegion2DComponent {
	RID region_id;
	~NavRegion2DComponent() {
		if (region_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(region_id);
		}
	}
};

#define NAV_REGION_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, region_id,NavRegion2DComponent)\

#define NAV_REGION_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, region_id, NavRegion2DComponentRef)\

class NavRegion2DComponentRef : public FlecsComponent<NavRegion2DComponent> {
GDCLASS(NavRegion2DComponentRef, FlecsComponent<NavRegion2DComponent>)
public : RID get_region_id() const {
		auto typed = get_typed_data<NavRegion2DComponent>();
		if (typed) {
			return typed->region_id;
		}
		return default_value<RID>();
	}
	void set_region_id(RID value) const {
		auto typed = get_typed_data<NavRegion2DComponent>();
		if (typed) {
			typed->region_id = value;
		}
	}
	static Ref<NavRegion2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<NavRegion2DComponentRef> class_ref = Ref<NavRegion2DComponentRef>(memnew(NavRegion2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<NavRegion2DComponent>({});
		class_ref->set_data(&entity->get_mut<NavRegion2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<NavRegion2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<NavRegion2DComponent>();
			NavRegion2DComponent *copied = memnew(NavRegion2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(NavRegion2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "NavRegion2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method(D_METHOD("get_"
										  "region_id"),
					&NavRegion2DComponentRef::get_region_id);
			ClassDB::bind_method(D_METHOD("set_"
										  "region_id",
										 "value"),
					&NavRegion2DComponentRef::set_region_id);
			::ClassDB::add_property(NavRegion2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "region_id"), _scs_create("set_"
																																					"region_id"),
					_scs_create("get_"
								"region_id"));
		} while (0);
		ClassDB::bind_static_method(NavRegion2DComponentRef::get_class_static(), "create_component", &NavRegion2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &NavRegion2DComponentRef::get_type_name);
	}
};
;

struct SourceGeometryParser2DComponent {
	RID source_geometry_parser_id;
	~SourceGeometryParser2DComponent() {
		if (source_geometry_parser_id.is_valid()) {
			NavigationServer2D::get_singleton()->free(source_geometry_parser_id);
		}
	}
};

#define SOURCE_GEOMETRY_PARSER_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, source_geometry_parser_id,SourceGeometryParser2DComponent)\

#define SOURCE_GEOMETRY_PARSER_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, source_geometry_parser_id, SourceGeometryParser2DComponentRef)\

class SourceGeometryParser2DComponentRef : public FlecsComponent<SourceGeometryParser2DComponent> {
GDCLASS(SourceGeometryParser2DComponentRef, FlecsComponent<SourceGeometryParser2DComponent>)
public : RID get_source_geometry_parser_id() const {
		auto typed = get_typed_data<SourceGeometryParser2DComponent>();
		if (typed) {
			return typed->source_geometry_parser_id;
		}
		return default_value<RID>();
	}
	void set_source_geometry_parser_id(RID value) const {
		auto typed = get_typed_data<SourceGeometryParser2DComponent>();
		if (typed) {
			typed->source_geometry_parser_id = value;
		}
	}
	static Ref<SourceGeometryParser2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SourceGeometryParser2DComponentRef> class_ref = Ref<SourceGeometryParser2DComponentRef>(memnew(SourceGeometryParser2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SourceGeometryParser2DComponent>({});
		class_ref->set_data(&entity->get_mut<SourceGeometryParser2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SourceGeometryParser2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SourceGeometryParser2DComponent>();
			SourceGeometryParser2DComponent *copied = memnew(SourceGeometryParser2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SourceGeometryParser2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SourceGeometryParser2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method(D_METHOD("get_"
										  "source_geometry_parser_id"),
					&SourceGeometryParser2DComponentRef::get_source_geometry_parser_id);
			ClassDB::bind_method(D_METHOD("set_"
										  "source_geometry_parser_id",
										 "value"),
					&SourceGeometryParser2DComponentRef::set_source_geometry_parser_id);
			::ClassDB::add_property(SourceGeometryParser2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "source_geometry_parser_id"), _scs_create("set_"
																																											   "source_geometry_parser_id"),
					_scs_create("get_"
								"source_geometry_parser_id"));
		} while (0);
		ClassDB::bind_static_method(SourceGeometryParser2DComponentRef::get_class_static(), "create_component", &SourceGeometryParser2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SourceGeometryParser2DComponentRef::get_type_name);
	}
};
;

struct Navigation2DBaseComponents {
	flecs::component<NavAgent2DComponent> agent;
	flecs::component<NavLink2DComponent> link;
	flecs::component<NavObstacle2DComponent> obstacle;
	flecs::component<NavRegion2DComponent> region;
	flecs::component<SourceGeometryParser2DComponent> source_geometry_parser;

	explicit Navigation2DBaseComponents(const flecs::world &world) :
			agent(world.component<NavAgent2DComponent>("NavAgent2DComponent")),
			link(world.component<NavLink2DComponent>("NavLink2DComponent")),
			obstacle(world.component<NavObstacle2DComponent>("NavObstacle2DComponent")),
			region(world.component<NavRegion2DComponent>("NavRegion2DComponent")),
			source_geometry_parser(world.component<SourceGeometryParser2DComponent>("SourceGeometryParser2DComponent")) {}
};

using Navigation2DComponentModule = MultiComponentModule<Navigation2DBaseComponents>;
#undef NAV_AGENT_2D_COMPONENT_BINDINGS