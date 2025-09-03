#include "register_types.h"

#include "../../core/object/class_db.h"
#include "core/os/memory.h"
#include "ecs/utility/world_utility.h"
#include "ecs/utility/scene_object_utility.h"
#include "ecs/utility/resource_object_utility.h"

#include "ecs/utility/navigation/2d/navigation2d_utility.h"
#include "ecs/utility/navigation/3d/navigation3d_utility.h"
#include "ecs/utility/physics/2d/physics2d_utility.h"
#include "ecs/utility/physics/3d/physics3d_utility.h"

#include "ecs/utility/rendering/2d/render_utility_2d.h"
#include "ecs/utility/rendering/3d/render_utility_3d.h"
#include "ecs/systems/commands/command.h"
#include "ecs/flecs_types/flecs_server.h"
#include "ecs/systems/demo/bad_apple_system.h"



void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<FlecsServer>();
	FlecsServer* p_fs = memnew(FlecsServer);
	Engine::get_singleton()->add_singleton(Engine::Singleton("FlecsServer", p_fs));
	ClassDB::register_runtime_class<RenderUtility2D>();
	ClassDB::register_runtime_class<RenderUtility3D>();
	ClassDB::register_runtime_class<Physics3DUtility>();
	ClassDB::register_runtime_class<Physics2DUtility>();
	ClassDB::register_runtime_class<Navigation2DUtility>();
	ClassDB::register_runtime_class<Navigation3DUtility>();
	ClassDB::register_runtime_class<World3DUtility>();
	ClassDB::register_runtime_class<World2DUtility>();
	ClassDB::register_runtime_class<SceneObjectUtility>();
	ClassDB::register_runtime_class<ResourceObjectUtility>();
	ClassDB::register_class<CommandHandler>();
	ClassDB::register_runtime_class<BadAppleSystem>();

}

void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
   // Nothing to do here in this example.
}


