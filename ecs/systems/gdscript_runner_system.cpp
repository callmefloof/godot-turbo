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
	
	ready_system = world->system<GameScriptComponent>()
		.kind(flecs::PreUpdate)
		.each([this](flecs::entity e, GameScriptComponent& script_comp) {
			if (script_comp.ready_called) {
				return;
			}

			ScriptMethodCache local_cache;
			ScriptMethodCache* cache = &local_cache;
			const StringName cache_key = get_cache_key(script_comp);
			if (!cache_key.is_empty()) {
				cache = &get_or_create_cache(cache_key);
			}

			populate_method_cache(e, script_comp, *cache);
			if (!cache->has_ready) {
				script_comp.ready_called = true;
				return;
			}

			FlecsServer* server = FlecsServer::get_singleton();
			ERR_FAIL_NULL(server);

			RID entity_rid = server->_get_or_create_rid_for_entity(world_rid, e);
			const StringName method_name = cache->ready_method.is_empty() ?
					StringName(READY_METHOD_GDSCRIPT) : cache->ready_method;
			if (execute_script_method(e, entity_rid, &script_comp, method_name, 0.0f, false)) {
				script_comp.ready_called = true;
			}
		});

	// Create process system (runs during OnUpdate phase)
	process_system = world->system<GameScriptComponent>()
		.kind(flecs::OnUpdate)
		.each([this](flecs::entity e, GameScriptComponent& script_comp) {
			ScriptMethodCache local_cache;
			ScriptMethodCache* cache = &local_cache;
			const StringName cache_key = get_cache_key(script_comp);
			if (!cache_key.is_empty()) {
				cache = &get_or_create_cache(cache_key);
			}
			populate_method_cache(e, script_comp, *cache);

			// Execute if method exists
			if (cache->has_process) {
				FlecsServer* server = FlecsServer::get_singleton();
				ERR_FAIL_NULL(server);
				
				RID entity_rid = server->_get_or_create_rid_for_entity(world_rid, e);
				float delta = static_cast<float>(world->delta_time());
				const StringName method_name = cache->process_method.is_empty() ?
						StringName(PROCESS_METHOD_GDSCRIPT) : cache->process_method;
				
				execute_script_method(
					e,
					entity_rid,
					&script_comp,
					method_name,
					delta
				);
			}
		});
	
	// Create physics process system (runs during OnPhysicsUpdate phase)
	physics_process_system = world->system<GameScriptComponent>()
		.kind(flecs::OnPhysicsUpdate)
		.each([this](flecs::entity e, GameScriptComponent& script_comp) {
			ScriptMethodCache local_cache;
			ScriptMethodCache* cache = &local_cache;
			const StringName cache_key = get_cache_key(script_comp);
			if (!cache_key.is_empty()) {
				cache = &get_or_create_cache(cache_key);
			}
			populate_method_cache(e, script_comp, *cache);

			// Execute if method exists
			if (cache->has_physics_process) {
				FlecsServer* server = FlecsServer::get_singleton();
				ERR_FAIL_NULL(server);
				
				RID entity_rid = server->_get_or_create_rid_for_entity(world_rid, e);
				
				// For physics, use fixed physics delta
				// Note: Flecs doesn't have built-in physics timing, so we use delta_time
				// In a real implementation, you might want to track physics ticks separately
				float delta = static_cast<float>(world->delta_time());
				const StringName method_name = cache->physics_process_method.is_empty() ?
						StringName(PHYSICS_PROCESS_METHOD_GDSCRIPT) : cache->physics_process_method;
				
				execute_script_method(
					e,
					entity_rid,
					&script_comp,
					method_name,
					delta
				);
			}
		});
}

bool GDScriptRunnerSystem::check_and_cache_method(const StringName& instance_type,
                                                    const char* gdscript_name,
                                                    const char* csharp_name,
                                                    StringName* r_method_name) {
	// Try to find the class in ClassDB
	if (!ClassDB::class_exists(instance_type)) {
		// Not a ClassDB class, might be a GDScript class
		// We'll try to check via Script later
		return false;
	}
	
	// Check for GDScript naming convention first
	const StringName gdscript_method(gdscript_name);
	if (ClassDB::has_method(instance_type, gdscript_method, true)) {
		if (r_method_name) {
			*r_method_name = gdscript_method;
		}
		return true;
	}
	
	// Check for C# naming convention
	const StringName csharp_method(csharp_name);
	if (ClassDB::has_method(instance_type, csharp_method, true)) {
		if (r_method_name) {
			*r_method_name = csharp_method;
		}
		return true;
	}
	
	return false;
}

StringName GDScriptRunnerSystem::get_cache_key(const GameScriptComponent& script_comp) const {
	if (!script_comp.script_path.is_empty()) {
		return StringName(script_comp.script_path);
	}
	return script_comp.instance_type;
}

Node* GDScriptRunnerSystem::get_node_for_entity(flecs::entity entity) const {
	if (!entity.has<SceneNodeComponent>()) {
		return nullptr;
	}

	FlecsServer* server = FlecsServer::get_singleton();
	if (!server) {
		return nullptr;
	}

	const SceneNodeComponent& node_comp = entity.get<SceneNodeComponent>();
	return server->get_node_from_node_storage(node_comp.node_id, world_rid);
}

void GDScriptRunnerSystem::populate_method_cache(flecs::entity entity,
                                                  GameScriptComponent& script_comp,
                                                  ScriptMethodCache& cache) {
	if (cache.checked) {
		return;
	}

	cache.has_ready = script_comp.has_ready;
	cache.has_process = script_comp.has_process;
	cache.has_physics_process = script_comp.has_physics_process;
	cache.ready_method = script_comp.ready_method;
	cache.process_method = script_comp.process_method;
	cache.physics_process_method = script_comp.physics_process_method;

	Node* node = get_node_for_entity(entity);
	if (node) {
		Ref<Script> script = node->get_script();
		if (script.is_valid()) {
			if (script_comp.script_path.is_empty()) {
				script_comp.script_path = script->get_path();
			}
			if (script_comp.instance_type.is_empty()) {
				script_comp.instance_type = script->get_global_name();
				if (script_comp.instance_type.is_empty()) {
					script_comp.instance_type = script->get_instance_base_type();
				}
				if (script_comp.instance_type.is_empty()) {
					script_comp.instance_type = node->get_class();
				}
			}

			const StringName gdscript_ready(READY_METHOD_GDSCRIPT);
			const StringName csharp_ready(READY_METHOD_CSHARP);
			if (script->has_method(gdscript_ready)) {
				cache.has_ready = true;
				cache.ready_method = gdscript_ready;
			} else if (script->has_method(csharp_ready)) {
				cache.has_ready = true;
				cache.ready_method = csharp_ready;
			}

			const StringName gdscript_process(PROCESS_METHOD_GDSCRIPT);
			const StringName csharp_process(PROCESS_METHOD_CSHARP);
			if (script->has_method(gdscript_process)) {
				cache.has_process = true;
				cache.process_method = gdscript_process;
			} else if (script->has_method(csharp_process)) {
				cache.has_process = true;
				cache.process_method = csharp_process;
			}

			const StringName gdscript_physics(PHYSICS_PROCESS_METHOD_GDSCRIPT);
			const StringName csharp_physics(PHYSICS_PROCESS_METHOD_CSHARP);
			if (script->has_method(gdscript_physics)) {
				cache.has_physics_process = true;
				cache.physics_process_method = gdscript_physics;
			} else if (script->has_method(csharp_physics)) {
				cache.has_physics_process = true;
				cache.physics_process_method = csharp_physics;
			}
		}
	}

	if (!script_comp.instance_type.is_empty()) {
		if (!cache.has_ready) {
			cache.has_ready = check_and_cache_method(
				script_comp.instance_type,
				READY_METHOD_GDSCRIPT,
				READY_METHOD_CSHARP,
				&cache.ready_method
			);
		}
		if (!cache.has_process) {
			cache.has_process = check_and_cache_method(
				script_comp.instance_type,
				PROCESS_METHOD_GDSCRIPT,
				PROCESS_METHOD_CSHARP,
				&cache.process_method
			);
		}
		if (!cache.has_physics_process) {
			cache.has_physics_process = check_and_cache_method(
				script_comp.instance_type,
				PHYSICS_PROCESS_METHOD_GDSCRIPT,
				PHYSICS_PROCESS_METHOD_CSHARP,
				&cache.physics_process_method
			);
		}
	}

	script_comp.has_ready = cache.has_ready;
	script_comp.has_process = cache.has_process;
	script_comp.has_physics_process = cache.has_physics_process;
	script_comp.ready_method = cache.ready_method;
	script_comp.process_method = cache.process_method;
	script_comp.physics_process_method = cache.physics_process_method;
	cache.checked = true;
}

GDScriptRunnerSystem::ScriptMethodCache& GDScriptRunnerSystem::get_or_create_cache(const StringName& instance_type) {
	if (!method_cache.has(instance_type)) {
		method_cache[instance_type] = ScriptMethodCache();
	}
	return method_cache[instance_type];
}

bool GDScriptRunnerSystem::execute_script_method(flecs::entity entity,
                                                  const RID& entity_rid,
                                                  const GameScriptComponent* script_comp,
                                                  const StringName& method_name,
                                                  float delta,
                                                  bool p_include_delta) {
	ERR_FAIL_NULL_V(script_comp, false);
	
	Node* node = get_node_for_entity(entity);
	if (!node) {
		// Node not in storage or has been freed
		return false;
	}
	
	// Get the script attached to the node
	Ref<Script> script = node->get_script();
	if (script.is_null()) {
		// No script attached
		return false;
	}
	
	// Determine which method name to use (GDScript or C# convention)
	StringName actual_method_name = method_name;
	
	// Check GDScript convention first
	if (!node->has_method(actual_method_name) &&
			actual_method_name == StringName(READY_METHOD_GDSCRIPT) &&
			node->has_method(StringName(READY_METHOD_CSHARP))) {
		actual_method_name = StringName(READY_METHOD_CSHARP);
	}
	if (!node->has_method(actual_method_name) &&
			!node->has_method(StringName(PROCESS_METHOD_GDSCRIPT)) &&
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
		return false;
	}
	
	// Prepare arguments: entity_rid, optional delta
	const Variant* args[2];
	Variant arg0 = entity_rid;
	Variant arg1 = delta;
	args[0] = &arg0;
	args[1] = &arg1;
	
	// Call the method
	Callable::CallError call_error;
	Variant result = node->callp(actual_method_name, args, p_include_delta ? 2 : 1, call_error);
	
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

	return true;
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
