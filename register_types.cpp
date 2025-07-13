#include "register_types.h"

#include "core/object/class_db.h"
#include "ecs/flecs_types/flecs_world.h"

void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
    ClassDB::register_class<World>();
}

void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
   // Nothing to do here in this example.
}

