#pragma once

#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/variant/dictionary.h"
#include "flecs_entity.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"

#include <functional>
#include <typeinfo>

class FlecsComponentBase;
class FlecsEntity;

struct ComponentTypeInfo {
	std::function<Ref<FlecsComponentBase>()> creator;
	std::function<void(const flecs::entity&, Ref<FlecsComponentBase>)> apply;
};

class FlecsWorld : public FlecsEntity {
	GDCLASS(FlecsWorld, FlecsEntity)
private:
	flecs::world world;
	ecs_entity_t OnPhysics = ecs_new_w_id(world, EcsPhase);
	ecs_entity_t OnCollisions = ecs_new_w_id(world, EcsPhase);
	/* data */
protected:
	static void _bind_methods();

public:
	static HashMap<StringName, ComponentTypeInfo> component_registry;
	FlecsWorld();
	explicit FlecsWorld(const flecs::world &p_world) : world(p_world) {
		set_entity(p_world.entity());
	}
	~FlecsWorld() override;
	void init_world();
	bool progress() const;
	void register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const;

	void set(const Ref<FlecsComponentBase> &comp) override;
	Ref<FlecsEntity> entity() const;
	Ref<FlecsEntity> entity(const StringName &p_name) const;
	Ref<FlecsEntity> entity(const StringName &p_name, const Ref<FlecsComponentBase> &comp) const;

	void remove(const Ref<FlecsComponentBase> &comp) override;
	void remove(const StringName &component_type);
	void remove_all_components() override;

	// Accessor for the underlying Flecs world
	flecs::world *get_world();
	// Register a system with the world
	template <typename System>
	void register_system(const char *name = nullptr, flecs::entity_t phase = flecs::OnUpdate) {
		System().register_system(world, name, phase);
	}

};