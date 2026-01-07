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
#include "modules/godot_turbo/runtime/flecs_runtime_debugger.h"

// Network module
#include "modules/godot_turbo/network/network_server.h"

#ifdef TOOLS_ENABLED
#include "modules/godot_turbo/editor/flecs_editor_plugin.h"
#include "modules/godot_turbo/editor/flecs_profiler_plugin.h"
#include "modules/godot_turbo/editor/network_editor_plugin.h"
#endif


FlecsRuntimeDebugger *p_runtime_debugger = nullptr;

void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Register FlecsServer singleton
		ClassDB::register_class<FlecsServer>();
		p_fs = memnew(FlecsServer);
		Engine::get_singleton()->add_singleton(Engine::Singleton("FlecsServer", p_fs));

		// Register NetworkServer singleton
		ClassDB::register_class<NetworkServer>();
		p_network_server = memnew(NetworkServer);
		Engine::get_singleton()->add_singleton(Engine::Singleton("NetworkServer", p_network_server));
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

		// Initialize runtime debugger
		p_runtime_debugger = memnew(FlecsRuntimeDebugger);
		p_runtime_debugger->initialize();
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(FlecsWorldEditorPlugin);
		EditorPlugins::add_by_type<FlecsWorldEditorPlugin>();
		
		GDREGISTER_CLASS(FlecsProfilerPlugin);
		EditorPlugins::add_by_type<FlecsProfilerPlugin>();
		
		// Network editor plugin
		GDREGISTER_CLASS(NetworkEditorPlugin);
		EditorPlugins::add_by_type<NetworkEditorPlugin>();
	}
#endif
}

void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (p_runtime_debugger) {
			p_runtime_debugger->shutdown();
			memdelete(p_runtime_debugger);
			p_runtime_debugger = nullptr;
		}
		
		// Clean up NetworkServer
		if (p_network_server) {
			memdelete(p_network_server);
			p_network_server = nullptr;
		}
		
		memdelete(p_fs);
	}
}
