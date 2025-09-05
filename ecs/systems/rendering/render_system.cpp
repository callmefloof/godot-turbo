#include "render_system.h"
#include "core/string/string_name.h"
#include "flecs.h"
#include "../../flecs_types/flecs_server.h"
#include "servers/display_server.h"
#include <functional>
#include <optional>

Vector2i RenderSystem::get_window_size() const {
	return DisplayServer::get_singleton()->window_get_size();
}

flecs::world *RenderSystem::resolve_world() const {
    RID rid = world_rid.load();
    if (!rid.is_valid()) {
        return nullptr;
    }
    return FlecsServer::get_singleton()->_get_world(rid);
}


void RenderSystem::set_world(RID p_world_rid) {
    world_rid.store(p_world_rid);
}