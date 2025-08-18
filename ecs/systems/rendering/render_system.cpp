#include "render_system.h"
#include "servers/display_server.h"

Vector2i RenderSystem::get_window_size() const {
	return DisplayServer::get_singleton()->window_get_size();
}


flecs::world* RenderSystem::get_world() const {
    return world;
}


void RenderSystem::set_world(flecs::world *p_world) {
    world = p_world;
}