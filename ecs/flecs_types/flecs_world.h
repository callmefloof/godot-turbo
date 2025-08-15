#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/a_hash_map.h"
#include "core/templates/hash_map.h"
#include "flecs_component_base.h"
#include "flecs_script_system.h"
#include "modules/godot_turbo/ecs/components/navigation/2d/2d_navigation_components.h"
#include <functional>
#include <typeinfo>
#include "ecs/systems/commands/command.h"
#include "ecs/systems/pipeline_manager.h"
#include "ecs/systems/rendering/mulitmesh_render_system.h"
#include "ecs/systems/rendering/occlusion/occlusion_system.h"
#include "ecs/systems/rendering/mesh_render_system.h"
#include "flecs_pair.h"

class ScriptVisibleComponentRef;


class FlecsEntity;

struct ComponentTypeInfo {
	std::function<Ref<FlecsComponentBase>()> creator;
	std::function<void(const flecs::entity&, Ref<FlecsComponentBase>)> apply;
	flecs::entity_t component_type;
};

struct SingletonComponentTypeInfo {
	std::function<Ref<FlecsComponentBase>()> creator;
	std::function<void(const flecs::world&, Ref<FlecsComponentBase>)> apply;
	flecs::entity_t component_type;
};

class FlecsWorld : public Resource {
	GDCLASS(FlecsWorld, Resource);
public:

private:

	flecs::world world;
	Vector<Ref<FlecsComponentBase>> components;
	Vector<Ref<FlecsPair>> relationships;
	ecs_entity_t OnPhysics = ecs_new_w_id(world, EcsPhase);
	ecs_entity_t OnCollisions = ecs_new_w_id(world, EcsPhase);
	Vector<Ref<FlecsScriptSystem>> script_systems;
	AHashMap<flecs::entity,Ref<FlecsEntity>> entities;
	CommandQueue system_command_queue;
	MultiMeshRenderSystem multi_mesh_render_system;
	MeshRenderSystem mesh_render_system;
	OcclusionSystem occlusion_system;
	PipelineManager pipeline_manager;
	/* data */
protected:
	static void _bind_methods();
public:
	static HashMap<StringName, ComponentTypeInfo> component_registry;
	static HashMap<StringName, SingletonComponentTypeInfo> singleton_component_registry;

	FlecsWorld();
	~FlecsWorld();
	void init_world();
	bool progress(const double delta);
	void register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const;
	void add_script_system(const Array &component_types, const Callable &callable);
	void set_component(const Ref<FlecsComponentBase> &comp_ref);
	Ref<FlecsComponentBase> get_component(const StringName &component_type) const;
	bool has_component(const StringName &component_type) const;
	PackedStringArray get_component_types() const;
	Ref<FlecsEntity> create_entity();
	Ref<FlecsEntity> create_entity_e(const flecs::entity &e);
	Ref<FlecsEntity> create_entity_n(const StringName &p_name);
	Ref<FlecsEntity> create_entity_nc(const StringName &p_name, const Ref<FlecsComponentBase> &p_comp);
	Ref<FlecsComponentBase> get_component_by_name(const StringName &component_type);


	// Accessor for the underlying Flecs world
	flecs::world* get_world_ref();
	Ref<FlecsEntity> add_entity(const flecs::entity &e);
	void init_render_system();
	void set_log_level(const int level);
	void add_relationship(FlecsPair *pair);
	void remove_relationship(const StringName &first_entity, const StringName &second_entity);
	Ref<FlecsPair> get_relationship(const StringName &first_entity, const StringName &second_entity) const;
	TypedArray<FlecsPair> get_relationships() const;

};



