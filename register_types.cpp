#include "register_types.h"

#include "../../core/object/class_db.h"
#include "core/os/memory.h"
#include "modules/godot_turbo/ecs/systems/utility/world_utility.h"
#include "modules/godot_turbo/ecs/systems/utility/scene_object_utility.h"
#include "modules/godot_turbo/ecs/systems/utility/resource_object_utility.h"

#include "modules/godot_turbo/ecs/systems/utility/navigation2d_utility.h"
#include "modules/godot_turbo/ecs/systems/utility/navigation3d_utility.h"
#include "modules/godot_turbo/ecs/systems/utility/physics2d_utility.h"
#include "modules/godot_turbo/ecs/systems/utility/physics3d_utility.h"

#include "modules/godot_turbo/ecs/systems/utility/render_utility_2d.h"
#include "modules/godot_turbo/ecs/systems/utility/render_utility_3d.h"
#include "modules/godot_turbo/ecs/systems/command.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

#include "modules/godot_turbo/ecs/systems/demo/bad_apple_system.h"
#include "modules/register_module_types.h"



void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ClassDB::register_class<FlecsServer>();
	p_fs = memnew(FlecsServer);
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
	memdelete(p_fs);


   // Nothing to do here in this example.
}
