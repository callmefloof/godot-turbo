#include "gdscript_runner_system.h"

#include "core/object/class_db.h"
#include "core/object/script_language.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "scene/main/node.h"

GDScriptRunnerSystem::~GDScriptRunnerSystem() {
	// Flecs entities are automatically cleaned up when world is destroyed
	// Just clear our cache
	method_cache.clear();
}

void GDScriptRunnerSystem::init(const RID& p_world_rid, flecs::world* p_world) {
	ERR_FAIL_NULL(p_world);
	
	world_rid = p_world_rid;
	world = p_world;
	
	// Create process system (runs during OnUpdate phase)
	process_system = world->system<GameScriptComponent>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::entity e, GameScriptComponent& script_comp) {
			// Get or create cache for this script type
			ScriptMethodCache& cache = get_or_create_cache(script_comp.instance_type);
			
			// Check if script has _flecs_process method
			if (!cache.checked) {
				cache.has_process = check_and_cache_method(
					script_comp.instance_type,
					PROCESS_METHOD_GDSCRIPT,
					PROCESS_METHOD_CSHARP
				);
				cache.has_physics_process = check_and_cache_method(
					script_comp.instance_type,
					PHYSICS_PROCESS_METHOD_GDSCRIPT,
					PHYSICS_PROCESS_METHOD_CSHARP
				);
				cache.checked = true;
			}
			
			// Execute if method exists
			if (cache.has_process) {
				FlecsServer* server = FlecsServer::get_singleton();
				ERR_FAIL_NULL(server);
				
				RID entity_rid = server->_get_or_create_rid_for_entity(world_rid, e);
				float delta = static_cast<float>(world->delta_time());
				
				execute_script_method(
					e,
					entity_rid,
					&script_comp,
					PROCESS_METHOD_GDSCRIPT,
					delta
				);
			}
		});
	
	// Create physics process system (runs during OnPhysicsUpdate phase)
	physics_process_system = world->system<GameScriptComponent>()
		.kind(flecs::OnPhysicsUpdate)
		.each([this](flecs::entity e, GameScriptComponent& script_comp) {
			// Get or create cache for this script type
			ScriptMethodCache& cache = get_or_create_cache(script_comp.instance_type);
			
			// Check if script has _flecs_physics_process method
			if (!cache.checked) {
				cache.has_process = check_and_cache_method(
					script_comp.instance_type,
					PROCESS_METHOD_GDSCRIPT,
					PROCESS_METHOD_CSHARP
				);
				cache.has_physics_process = check_and_cache_method(
					script_comp.instance_type,
					PHYSICS_PROCESS_METHOD_GDSCRIPT,
					PHYSICS_PROCESS_METHOD_CSHARP
				);
				cache.checked = true;
			}
			
			// Execute if method exists
			if (cache.has_physics_process) {
				FlecsServer* server = FlecsServer::get_singleton();
				ERR_FAIL_NULL(server);
				
				RID entity_rid = server->_get_or_create_rid_for_entity(world_rid, e);
				
				// For physics, use fixed physics delta
				// Note: Flecs doesn't have built-in physics timing, so we use delta_time
				// In a real implementation, you might want to track physics ticks separately
				float delta = static_cast<float>(world->delta_time());
				
				execute_script_method(
					e,
					entity_rid,
					&script_comp,
					PHYSICS_PROCESS_METHOD_GDSCRIPT,
					delta
				);
			}
		});
}

bool GDScriptRunnerSystem::check_and_cache_method(const StringName& instance_type,
                                                    const char* gdscript_name,
                                                    const char* csharp_name) {
	// Try to find the class in ClassDB
	if (!ClassDB::class_exists(instance_type)) {
		// Not a ClassDB class, might be a GDScript class
		// We'll try to check via Script later
		return false;
	}
	
	// Check for GDScript naming convention first
	if (ClassDB::has_method(instance_type, StringName(gdscript_name), true)) {
		return true;
	}
	
	// Check for C# naming convention
	if (ClassDB::has_method(instance_type, StringName(csharp_name), true)) {
		return true;
	}
	
	return false;
}

GDScriptRunnerSystem::ScriptMethodCache& GDScriptRunnerSystem::get_or_create_cache(const StringName& instance_type) {
	if (!method_cache.has(instance_type)) {
		method_cache[instance_type] = ScriptMethodCache();
	}
	return method_cache[instance_type];
}

void GDScriptRunnerSystem::execute_script_method(flecs::entity entity,
                                                  const RID& entity_rid,
                                                  const GameScriptComponent* script_comp,
                                                  const StringName& method_name,
                                                  float delta) {
	ERR_FAIL_NULL(script_comp);
	
	// Get the actual script instance from the entity
	// First, try to get the Node associated with this entity via SceneNodeComponent
	FlecsServer* server = FlecsServer::get_singleton();
	ERR_FAIL_NULL(server);
	
	// Check if entity has SceneNodeComponent
	if (!entity.has<SceneNodeComponent>()) {
		// No node associated, can't execute script
		return;
	}
	
	const SceneNodeComponent& node_comp = entity.get<SceneNodeComponent>();
	
	// Get the Node from storage
	Node* node = server->get_node_from_node_storage(node_comp.node_id, world_rid);
	if (!node) {
		// Node not in storage or has been freed
		return;
	}
	
	// Get the script attached to the node
	Ref<Script> script = node->get_script();
	if (script.is_null()) {
		// No script attached
		return;
	}
	
	// Determine which method name to use (GDScript or C# convention)
	StringName actual_method_name = method_name;
	
	// Check GDScript convention first
	if (!node->has_method(StringName(PROCESS_METHOD_GDSCRIPT)) && 
	    !node->has_method(StringName(PHYSICS_PROCESS_METHOD_GDSCRIPT))) {
		// Try C# convention
		if (method_name == StringName(PROCESS_METHOD_GDSCRIPT)) {
			actual_method_name = StringName(PROCESS_METHOD_CSHARP);
		} else if (method_name == StringName(PHYSICS_PROCESS_METHOD_GDSCRIPT)) {
			actual_method_name = StringName(PHYSICS_PROCESS_METHOD_CSHARP);
		}
	}
	
	// Check if the method exists
	if (!node->has_method(actual_method_name)) {
		return;
	}
	
	// Prepare arguments: entity_rid, delta
	const Variant* args[2];
	Variant arg0 = entity_rid;
	Variant arg1 = delta;
	args[0] = &arg0;
	args[1] = &arg1;
	
	// Call the method
	Callable::CallError call_error;
	Variant result = node->callp(actual_method_name, args, 2, call_error);
	
	// Check for errors
	if (call_error.error != Callable::CallError::CALL_OK) {
		String error_msg;
		switch (call_error.error) {
			case Callable::CallError::CALL_ERROR_INVALID_METHOD:
				error_msg = "Invalid method";
				break;
			case Callable::CallError::CALL_ERROR_INVALID_ARGUMENT:
				error_msg = vformat("Invalid argument at index %d", call_error.argument);
				break;
			case Callable::CallError::CALL_ERROR_TOO_MANY_ARGUMENTS:
				error_msg = vformat("Too many arguments (expected 2, got %d)", call_error.expected);
				break;
			case Callable::CallError::CALL_ERROR_TOO_FEW_ARGUMENTS:
				error_msg = vformat("Too few arguments (expected 2, got %d)", call_error.expected);
				break;
			case Callable::CallError::CALL_ERROR_INSTANCE_IS_NULL:
				error_msg = "Instance is null";
				break;
			case Callable::CallError::CALL_ERROR_METHOD_NOT_CONST:
				error_msg = "Method is not const";
				break;
			default:
				error_msg = "Unknown error";
				break;
		}
		
		ERR_PRINT(vformat("Error calling %s on entity %s: %s",
			actual_method_name,
			node->get_name(),
			error_msg));
	}
}

void GDScriptRunnerSystem::clear_cache() {
	method_cache.clear();
}

void GDScriptRunnerSystem::set_process_enabled(bool enabled) {
	if (process_system.is_valid()) {
		if (enabled) {
			process_system.enable();
		} else {
			process_system.disable();
		}
	}

	if (physics_process_system.is_valid() && flecs::OnPhysicsUpdate == flecs::OnUpdate) {
		if (!enabled) {
			if (physics_process_system.enabled()) {
				physics_process_system.disable();
				physics_process_suspended_by_process = true;
			}
		} else if (physics_process_suspended_by_process) {
			physics_process_system.enable();
			physics_process_suspended_by_process = false;
		}
	}
}

void GDScriptRunnerSystem::set_physics_process_enabled(bool enabled) {
	if (physics_process_system.is_valid()) {
		if (enabled) {
			physics_process_system.enable();
		} else {
			physics_process_system.disable();
		}
	}
	physics_process_suspended_by_process = false;
}

bool GDScriptRunnerSystem::is_process_enabled() const {
	if (process_system.is_valid()) {
		return process_system.enabled();
	}
	return false;
}

bool GDScriptRunnerSystem::is_physics_process_enabled() const {
	if (physics_process_system.is_valid()) {
		return physics_process_system.enabled();
	}
	return false;
}
