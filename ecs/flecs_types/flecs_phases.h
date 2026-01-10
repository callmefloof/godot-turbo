#pragma once

#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"

// Flecs upstream doesn't define an OnPhysicsUpdate phase in this version.
// Alias it to OnUpdate so existing code/tests compile and run consistently.
namespace flecs {
inline const flecs::entity_t OnPhysicsUpdate = flecs::OnUpdate;
} // namespace flecs
