#pragma once

#include "../thirdparty/flecs/distr/flecs.h"
#include "scene/main/node.h"
#include "servers/rendering_server.h"

class World : public Node
{
GDCLASS(World,Node)
private:
    flecs::world world;
	ecs_entity_t OnPhysics = ecs_new_w_id(world, EcsPhase);
	ecs_entity_t OnCollisions = ecs_new_w_id(world, EcsPhase);
    /* data */
protected:
    static void _bind_methods();
public:
    World(/* args */);
    ~World();
    void _ready();
    void _process(const double delta);
	void _physics_process(const double delta);
	// Accessor for the underlying Flecs world
	flecs::world &get_world() { return world; }
	// Register a system with the world
	template <typename System>
	void register_system(const char *name = nullptr, flecs::entity phase = flecs::OnUpdate) {
		System().register_system(world, name, phase);
	}
	// Get the RenderingServer singleton
	RenderingServer *get_rendering_server() {
		return RenderingServer::get_singleton();
	}

};


