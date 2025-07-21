//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDCOMPONENTS_H
#define WORLDCOMPONENTS_H
#include "single_component_module.h"
#include "../../../../core/templates/rid.h"
#include "../../../../core/os/memory.h"
#include "../../../../servers/rendering_server.h"
#include "../../../../servers/physics_server_2d.h"
#include "../../../../servers/physics_server_3d.h"
#include "../../../../servers/navigation_server_2d.h"
#include "../../../../servers/navigation_server_3d.h"
#include "component_proxy.h"
#include "../flecs_types/flecs_component.h"

struct World2DComponent {
	RID canvas_id;
	RID navigation_map_id;
	RID space_id;
	bool is_valid() const { return canvas_id.is_valid() && navigation_map_id.is_valid() && space_id.is_valid(); }
	bool is_null() const { return canvas_id.is_null() && navigation_map_id.is_null() && space_id.is_null(); }
	virtual ~World2DComponent() {
		RenderingServer::get_singleton()->free(canvas_id);
		NavigationServer2D::get_singleton()->free(navigation_map_id);
		PhysicsServer2D::get_singleton()->free(space_id);
	}
};

#define WORLD_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, canvas_id,World2DComponent)\
DEFINE_PROPERTY(RID, navigation_map_id,World2DComponent)\
DEFINE_PROPERTY(RID, space_id,World2DComponent)\

#define WORLD_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, canvas_id, World2DComponentRef)\
BIND_PROPERTY(RID, navigation_map_id, World2DComponentRef)\
BIND_PROPERTY(RID, space_id, World2DComponentRef)\

class World2DComponentRef : public FlecsComponent<World2DComponent> {
GDCLASS(World2DComponentRef, FlecsComponent<World2DComponent>)
public : RID get_canvas_id() const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			return typed->canvas_id;
		}
		return default_value<RID>();
	}
	void set_canvas_id(RID value) const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			typed->canvas_id = value;
		}
	}
	RID get_navigation_map_id() const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			return typed->navigation_map_id;
		}
		return default_value<RID>();
	}
	void set_navigation_map_id(RID value) const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			typed->navigation_map_id = value;
		}
	}
	RID get_space_id() const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			return typed->space_id;
		}
		return default_value<RID>();
	}
	void set_space_id(RID value) const {
		auto typed = get_typed_data<World2DComponent>();
		if (typed) {
			typed->space_id = value;
		}
	}
	static Ref<World2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<World2DComponentRef> class_ref = Ref<World2DComponentRef>(memnew(World2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<World2DComponent>({});
		class_ref->set_data(&entity->get_mut<World2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<World2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<World2DComponent>();
			World2DComponent *copied = memnew(World2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(World2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "World2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "canvas_id",
					&World2DComponentRef::get_canvas_id);
			ClassDB::bind_method("set_"
								 "canvas_id",
					&World2DComponentRef::set_canvas_id);
			::ClassDB::add_property(World2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "canvas_id"), _scs_create("set_"
																																				"canvas_id"),
					_scs_create("get_"
								"canvas_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "navigation_map_id",
					&World2DComponentRef::get_navigation_map_id);
			ClassDB::bind_method("set_"
								 "navigation_map_id",
					&World2DComponentRef::set_navigation_map_id);
			::ClassDB::add_property(World2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "navigation_map_id"), _scs_create("set_"
																																						"navigation_map_id"),
					_scs_create("get_"
								"navigation_map_id"));
		}
		while (0) {
			do {
				ClassDB::bind_method("get_"
									 "space_id",
						&World2DComponentRef::get_space_id);
				ClassDB::bind_method("set_"
									 "space_id",
						&World2DComponentRef::set_space_id);
				::ClassDB::add_property(World2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "space_id"), _scs_create("set_"
																																				   "space_id"),
						_scs_create("get_"
									"space_id"));
			} while (0);
		}
		ClassDB::bind_static_method(World2DComponentRef::get_class_static(), "create_component", &World2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &World2DComponentRef::get_type_name);
	}
};
;

using World2DComponentModule = SingleComponentModule<World2DComponent>;

struct World3DComponent {
	RID camera_attributes_id;
	RID environment_id;
	RID fallback_environment_id;
	RID navigation_map_id;
	RID scenario_id;
	RID space_id;
	bool is_valid() const {
		return camera_attributes_id.is_valid() &&
		environment_id.is_valid() &&
		fallback_environment_id.is_valid() &&
		navigation_map_id.is_valid() &&
		scenario_id.is_valid() &&
		space_id.is_valid();
	}
	bool is_null() const {
		return camera_attributes_id.is_null() &&
		environment_id.is_null() &&
		fallback_environment_id.is_null() &&
		navigation_map_id.is_null() &&
		scenario_id.is_null() &&
		space_id.is_null();
	}
	~World3DComponent() {
		RenderingServer::get_singleton()->free(camera_attributes_id);
		RenderingServer::get_singleton()->free(environment_id);
		RenderingServer::get_singleton()->free(fallback_environment_id);
		NavigationServer3D::get_singleton()->free(navigation_map_id);
		RenderingServer::get_singleton()->free(scenario_id);
		PhysicsServer3D::get_singleton()->free(space_id);
	}
};

#define WORLD_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, camera_attributes_id,World3DComponent)\
DEFINE_PROPERTY(RID, environment_id,World3DComponent)\
DEFINE_PROPERTY(RID, fallback_environment_id,World3DComponent)\
DEFINE_PROPERTY(RID, navigation_map_id,World3DComponent)\
DEFINE_PROPERTY(RID, scenario_id,World3DComponent)\
DEFINE_PROPERTY(RID, space_id,World3DComponent)\

#define WORLD_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, camera_attributes_id, World3DComponentRef)\
BIND_PROPERTY(RID, environment_id, World3DComponentRef)\
BIND_PROPERTY(RID, fallback_environment_id, World3DComponentRef)\
BIND_PROPERTY(RID, navigation_map_id, World3DComponentRef)\
BIND_PROPERTY(RID, scenario_id, World3DComponentRef)\
BIND_PROPERTY(RID, space_id, World3DComponentRef)\

class World3DComponentRef : public FlecsComponent<World3DComponent> {
GDCLASS(World3DComponentRef, FlecsComponent<World3DComponent>)
public : RID get_camera_attributes_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->camera_attributes_id;
		}
		return default_value<RID>();
	}
	void set_camera_attributes_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->camera_attributes_id = value;
		}
	}
	RID get_environment_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->environment_id;
		}
		return default_value<RID>();
	}
	void set_environment_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->environment_id = value;
		}
	}
	RID get_fallback_environment_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->fallback_environment_id;
		}
		return default_value<RID>();
	}
	void set_fallback_environment_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->fallback_environment_id = value;
		}
	}
	RID get_navigation_map_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->navigation_map_id;
		}
		return default_value<RID>();
	}
	void set_navigation_map_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->navigation_map_id = value;
		}
	}
	RID get_scenario_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->scenario_id;
		}
		return default_value<RID>();
	}
	void set_scenario_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->scenario_id = value;
		}
	}
	RID get_space_id() const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			return typed->space_id;
		}
		return default_value<RID>();
	}
	void set_space_id(RID value) const {
		auto typed = get_typed_data<World3DComponent>();
		if (typed) {
			typed->space_id = value;
		}
	}
	static Ref<World3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<World3DComponentRef> class_ref = Ref<World3DComponentRef>(memnew(World3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<World3DComponent>({});
		class_ref->set_data(&entity->get_mut<World3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<World3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<World3DComponent>();
			World3DComponent *copied = memnew(World3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(World3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "World3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "camera_attributes_id",
					&World3DComponentRef::get_camera_attributes_id);
			ClassDB::bind_method("set_"
								 "camera_attributes_id",
					&World3DComponentRef::set_camera_attributes_id);
			::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "camera_attributes_id"), _scs_create("set_"
																																						   "camera_attributes_id"),
					_scs_create("get_"
								"camera_attributes_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "environment_id",
					&World3DComponentRef::get_environment_id);
			ClassDB::bind_method("set_"
								 "environment_id",
					&World3DComponentRef::set_environment_id);
			::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "environment_id"), _scs_create("set_"
																																					 "environment_id"),
					_scs_create("get_"
								"environment_id"));
		}
		while (0) {
			do {
				ClassDB::bind_method("get_"
									 "fallback_environment_id",
						&World3DComponentRef::get_fallback_environment_id);
				ClassDB::bind_method("set_"
									 "fallback_environment_id",
						&World3DComponentRef::set_fallback_environment_id);
				::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "fallback_environment_id"), _scs_create("set_"
																																								  "fallback_environment_id"),
						_scs_create("get_"
									"fallback_environment_id"));
			} while (0) do {
				ClassDB::bind_method("get_"
									 "navigation_map_id",
						&World3DComponentRef::get_navigation_map_id);
				ClassDB::bind_method("set_"
									 "navigation_map_id",
						&World3DComponentRef::set_navigation_map_id);
				::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "navigation_map_id"), _scs_create("set_"
																																							"navigation_map_id"),
						_scs_create("get_"
									"navigation_map_id"));
			}
		}
		while (0) {
			do {
				ClassDB::bind_method("get_"
									 "scenario_id",
						&World3DComponentRef::get_scenario_id);
				ClassDB::bind_method("set_"
									 "scenario_id",
						&World3DComponentRef::set_scenario_id);
				::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "scenario_id"), _scs_create("set_"
																																					  "scenario_id"),
						_scs_create("get_"
									"scenario_id"));
			} while (0) do {
				ClassDB::bind_method("get_"
									 "space_id",
						&World3DComponentRef::get_space_id);
				ClassDB::bind_method("set_"
									 "space_id",
						&World3DComponentRef::set_space_id);
				::ClassDB::add_property(World3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "space_id"), _scs_create("set_"
																																				   "space_id"),
						_scs_create("get_"
									"space_id"));
			}
		}
		while (0)
			;
		ClassDB::bind_static_method(World3DComponentRef::get_class_static(), "create_component", &World3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &World3DComponentRef::get_type_name);
	}
};
;

using World3DComponentModule = SingleComponentModule<World3DComponent>;
#endif //WORLDCOMPONENTS_H
