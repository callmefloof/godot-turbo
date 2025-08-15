#include "flecs_system.h"
#include "ecs/flecs_types/flecs_world.h"

flecs::world* FlecsSystem::_get_world() const {
    return world;
}

FlecsWorld* FlecsSystem::get_world() const {
    return flecs_world_ref;
}

void FlecsSystem::_set_world(flecs::world *p_world) {
    world = p_world;
}

void FlecsSystem::set_world(FlecsWorld *p_world) {
    if (!p_world) {
        ERR_PRINT("FlecsSystem::set_world: p_world is null");
        return;
    }
    world = p_world->get_world_ref();
    flecs_world_ref = p_world;
}

void FlecsSystem::_bind_methods(){
    ClassDB::bind_method(D_METHOD("set_world", "p_world"), &FlecsSystem::set_world);
    ClassDB::bind_method(D_METHOD("get_world"), &FlecsSystem::get_world); 
}