#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/a_hash_map.h"
#include "core/templates/hash_map.h"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "flecs_script_system.h"
#include "modules/godot_turbo/ecs/components/navigation/2d/2d_navigation_components.h"
#include <functional>
#include <typeinfo>
#include "ecs/systems/commands/command.h"
#include "ecs/systems/pipeline_manager.h"
#include "ecs/systems/rendering/mulitmesh_render_system.h"
#include "ecs/systems/rendering/occlusion/occlusion_system.h"
#include "ecs/systems/rendering/mesh_render_system.h"

class ScriptVisibleComponentRef;




struct ComponentTypeInfo {
	std::function<Ref<FlecsComponentBase>()> creator;
	std::function<void(const flecs::entity&, Ref<FlecsComponentBase>)> apply;
	flecs::entity_t component_type;
};

class FlecsWorld : public FlecsEntity {
	GDCLASS(FlecsWorld, FlecsEntity);
public:

private:
	flecs::world world;
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
	FlecsWorld();
	explicit FlecsWorld(const flecs::world &p_world) : world(p_world), pipeline_manager(PipelineManager(world)){
		set_entity(p_world.entity());
	}

	~FlecsWorld();
	void init_world();
	bool progress();
	void register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const;
	void add_script_system(const Array &component_types, const Callable &callable);
	void set_component(const Ref<FlecsComponentBase> &comp_ref) override;
	Ref<FlecsEntity> create_entity() const;
	Ref<FlecsEntity> create_entity_e(const flecs::entity &e) const;
	Ref<FlecsEntity> create_entity_n(const StringName &p_name) const;
	Ref<FlecsEntity> create_entity_nc(const StringName &p_name, const Ref<FlecsComponentBase> &p_comp) const;

	void remove(const Ref<FlecsComponentBase> &comp_ref) override;
	void remove_t(const StringName &component_type);
	void remove_all_components() override;

	// Accessor for the underlying Flecs world
	flecs::world& get_world();
	Ref<FlecsEntity> add_entity(const flecs::entity &e);
	void init_render_system();

};



