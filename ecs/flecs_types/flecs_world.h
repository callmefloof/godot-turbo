#pragma once

#include "../../thirdparty/flecs/distr/flecs.h"
#include "../components/navigation/2d/2d_navigation_components.h"
#include "../components/navigation/3d/3d_navigation_components.h"
#include "../components/rendering/rendering_components.h"
#include "../components/scene_node_component.h"
#include "../systems/rendering/mulitmesh_render_system.h"
#include "../utility/navigation/2d/navigation2d_utility.h"
#include "../utility/navigation/3d/navigation3d_utility.h"
#include "../utility/rendering/render_utility_3d.h"
#include "../utility/scene_object_utility.h"
#include "flecs_component.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "servers/rendering_server.h"


#include <typeinfo>
#include <functional>

struct ComponentTypeInfo {
	std::function<Ref<FlecsComponentBase>()> creator;
	std::function<void(flecs::entity, Ref<FlecsComponentBase>)> apply;
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
	FlecsWorld(/* args */);
	~FlecsWorld() override;
	void init_world();
	bool progress() const;
	template <typename T>
	void register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const;

	void set(const StringName &component_type, const Ref<FlecsComponentBase> &data) override;
	Ref<FlecsEntity> entity() const;
	Ref<FlecsEntity> entity(const StringName &name);
	Ref<FlecsEntity> entity(const StringName &name, const StringName &component_type);
	Ref<FlecsEntity> entity(const StringName &name, const StringName &component_type, const Dictionary &data);
	Ref<FlecsEntity> entity(const StringName &name, const StringName &component_type, const Ref<FlecsComponentBase> &data);

	void remove(const String &component_type) override;
	void remove_all_components() override;
	Ref<FlecsComponentBase> get(const String &component_type) override;
	PackedStringArray get_component_types() override;
	void set(const StringName &component_type) override;

	// Accessor for the underlying Flecs world
	flecs::world *get_world();
	// Register a system with the world
	template <typename System>
	void register_system(const char *name = nullptr, flecs::entity_t phase = flecs::OnUpdate) {
		System().register_system(world, name, phase);
	}
};


template <typename T>
void FlecsWorld::register_component_type(const StringName& type_name, const Ref<ScriptVisibleComponentRef>& script_visible_component_ref ) const {
	if (!script_visible_component_ref.is_valid() || script_visible_component_ref.is_null) {
		ERR_PRINT("component is not valid.");
		return;
	}
	const char* ctype_name = String(type_name).ascii().get_data();
	ecs_component_desc_t desc = {0};
	desc.entity = world.entity(ctype_name);
	desc.type.size = sizeof(T);
	desc.type.alignment = alignof(T);
	flecs::entity_t comp = ecs_component_init(world, &desc);

	component_registry[type_name] = {
		[]() -> Ref<T> {
			return memnew(FlecsComponent<T>);
		},
		[](flecs::entity e, const Ref<FlecsComponentBase> &comp) {
			if (auto real = Object::cast_to<FlecsComponent<T>>(comp.ptr()))
				real->apply_to_entity(e);
			else
				ERR_PRINT("Component type mismatch!");
		}
	};
}


