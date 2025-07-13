#pragma once

#include "../thirdparty/flecs/distr/flecs.h"
#include "scene/main/node.h"
#include "servers/rendering_server.h"
#include "components/scene_node_component.h"
#include "components/rendering/rendering_components.h"
#include "components/physics/3d/3d_physics_components.h"
#include "components/physics/2d/2d_physics_components.h"
#include "components/navigation/3d/3d_navigation_components.h"
#include "components/navigation/2d/2d_navigation_components.h"
#include "components/transform_3d_component.h"
#include "components/transform_2d_component.h"
#include "utility/scene_object_utility.h"
#include "utility/rendering/render_utility_2d.h"
#include "utility/rendering/render_utility_3d.h"
#include "utility/physics/3d/physics3d_utility.h"
#include "utility/physics/2d/physics2d_utility.h"
#include "utility/navigation/3d/navigation3d_utility.h"
#include "utility/navigation/2d/navigation2d_utility.h"
#include "systems/rendering/mesh_render_system.h"
#include "systems/rendering/mulitmesh_render_system.h"
#include "../ecs/utility/scene_object_utility.h"



class World : public RefCounted {
	GDCLASS(World, RefCounted)
private:
	flecs::world world;
	ecs_entity_t OnPhysics = ecs_new_w_id(world, EcsPhase);
	ecs_entity_t OnCollisions = ecs_new_w_id(world, EcsPhase);
	/* data */
protected:
	static void _bind_methods();

public:
	World(/* args */);
	~World() override;
	void init_world();
	void progress();
	// Accessor for the underlying Flecs world
	flecs::world *get_world();
	// Register a system with the world
	template <typename System>
	void register_system(const char *name = nullptr, flecs::entity_t phase = flecs::OnUpdate) {
		System().register_system(world, name, phase);
	}

};


