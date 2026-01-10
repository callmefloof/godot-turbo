#pragma once

#include "core/object/script_language.h"
#include "core/templates/hash_map.h"
#include "core/templates/rid.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_phases.h"

/**
 * @class GDScriptRunnerSystem
 * @brief ECS system that executes script methods on entities with GameScriptComponent
 * 
 * This system bridges the gap between traditional Godot scripting and the ECS architecture.
 * It searches for entities with GameScriptComponent and executes their virtual methods
 * (_flecs_process, _flecs_physics_process) similar to how Node processes work.
 * 
 * @section Features
 * - **Method caching**: Checks method existence once per script type for performance
 * - **Multi-phase support**: Separate _flecs_process and _flecs_physics_process
 * - **GDScript & C# compatible**: Supports both naming conventions
 * - **Entity-aware**: Scripts receive entity RID to query components
 * 
 * @section Virtual Methods
 * Scripts attached to converted nodes can implement:
 * - `_flecs_process(entity_rid: RID, delta: float)` - Called every frame
 * - `_flecs_physics_process(entity_rid: RID, delta: float)` - Called at physics rate
 * - `_FlecsProcess(entityRid: Rid, delta: float)` - C# variant (process)
 * - `_FlecsPhysicsProcess(entityRid: Rid, delta: float)` - C# variant (physics)
 * 
 * @section Usage
 * ```cpp
 * // C++ - Create and initialize the system
 * GDScriptRunnerSystem runner;
 * runner.init(world_rid, world);
 * 
 * // Then call progress_world on each frame
 * // The system will automatically execute during OnUpdate/OnPhysicsUpdate
 * ```
 * 
 * ```gdscript
 * # GDScript - Script on converted node
 * extends Node
 * 
 * func _flecs_process(entity_rid: RID, delta: float) -> void:
 *     # Access components via FlecsServer
 *     var transform = FlecsServer.get_component_by_name(world, entity_rid, "Transform3DComponent")
 *     transform["position"] += Vector3.RIGHT * delta
 *     FlecsServer.set_component(world, entity_rid, "Transform3DComponent", transform)
 * 
 * func _flecs_physics_process(entity_rid: RID, delta: float) -> void:
 *     # Physics updates here
 *     pass
 * ```
 * 
 * @note This system is designed for converted scene nodes that need script behavior
 * @warning Scripts must be thread-safe if multi-threaded ECS is enabled
 */
class GDScriptRunnerSystem {
public:
    /**
     * @struct ScriptMethodCache
     * @brief Caches which virtual methods a script type has to avoid repeated reflection
     */
    struct ScriptMethodCache {
        bool has_process = false;              ///< Script has _flecs_process method
        bool has_physics_process = false;      ///< Script has _flecs_physics_process method
        bool checked = false;                  ///< Cache has been populated
        
        ScriptMethodCache() = default;
    };

private:
    flecs::entity process_system;              ///< System running during OnUpdate phase
    flecs::entity physics_process_system;      ///< System running during OnPhysicsUpdate phase
    flecs::world* world = nullptr;             ///< Pointer to Flecs world
    RID world_rid;                             ///< RID of the Flecs world
    bool physics_process_suspended_by_process = false; ///< Tracks auto-suspend when phases overlap
    
    // Method cache: maps script instance_type to method availability
    HashMap<StringName, ScriptMethodCache> method_cache;
    
    // Method name constants (checked in order for GDScript, then C# conventions)
    static constexpr const char* PROCESS_METHOD_GDSCRIPT = "_flecs_process";
    static constexpr const char* PHYSICS_PROCESS_METHOD_GDSCRIPT = "_flecs_physics_process";
    static constexpr const char* PROCESS_METHOD_CSHARP = "_FlecsProcess";
    static constexpr const char* PHYSICS_PROCESS_METHOD_CSHARP = "_FlecsPhysicsProcess";
    
    /**
     * @brief Checks if a script has a specific method and caches the result
     * 
     * @param instance_type The script's class/type name
     * @param gdscript_name Method name in GDScript convention
     * @param csharp_name Method name in C# convention
     * @return true if the script has the method
     */
    bool check_and_cache_method(const StringName& instance_type, 
                                 const char* gdscript_name, 
                                 const char* csharp_name);
    
    /**
     * @brief Gets or creates cache entry for a script type
     * 
     * @param instance_type The script's class/type name
     * @return Reference to the cache entry
     */
    ScriptMethodCache& get_or_create_cache(const StringName& instance_type);
    
    /**
     * @brief Executes a script method on an entity
     * 
     * @param entity The Flecs entity
     * @param entity_rid The RID of the entity (passed to script)
     * @param script_comp The GameScriptComponent with script info
     * @param method_name The method to call
     * @param delta Delta time to pass to the method
     */
    void execute_script_method(flecs::entity entity,
                               const RID& entity_rid,
                               const GameScriptComponent* script_comp,
                               const StringName& method_name,
                               float delta);
    
    /**
     * @brief System callback for processing phase
     * 
     * Called every frame during the OnUpdate phase.
     * Iterates entities with GameScriptComponent and calls _flecs_process.
     */
    void process_system_callback(flecs::iter& it);
    
    /**
     * @brief System callback for physics processing phase
     * 
     * Called at physics rate during the OnPhysicsUpdate phase.
     * Iterates entities with GameScriptComponent and calls _flecs_physics_process.
     */
    void physics_process_system_callback(flecs::iter& it);

public:
    GDScriptRunnerSystem() = default;
    ~GDScriptRunnerSystem();
    
    // Non-copyable, non-movable (contains Flecs handles)
    GDScriptRunnerSystem(const GDScriptRunnerSystem &other) = delete;
    GDScriptRunnerSystem &operator=(const GDScriptRunnerSystem &other) = delete;
    GDScriptRunnerSystem(GDScriptRunnerSystem &&other) noexcept = delete;
    GDScriptRunnerSystem &operator=(GDScriptRunnerSystem &&other) noexcept = delete;
    
    /**
     * @brief Initializes the script runner system
     * 
     * Creates two Flecs systems:
     * - Process system: Runs during OnUpdate phase
     * - Physics process system: Runs during OnPhysicsUpdate phase
     * 
     * Both systems query for entities with GameScriptComponent and execute
     * the appropriate virtual methods if they exist.
     * 
     * @param p_world_rid RID of the Flecs world
     * @param p_world Pointer to the Flecs world
     */
    void init(const RID& p_world_rid, flecs::world* p_world);
    
    /**
     * @brief Clears the method cache
     * 
     * Call this if scripts are reloaded or modified at runtime.
     * Forces method existence checks to be re-evaluated.
     */
    void clear_cache();
    
    /**
     * @brief Gets the number of cached script types
     * 
     * @return Number of entries in the method cache
     */
    int get_cache_size() const { return method_cache.size(); }
    
    /**
     * @brief Checks if a script type is cached
     * 
     * @param instance_type The script's class/type name
     * @return true if the script type has been cached
     */
    bool is_cached(const StringName& instance_type) const {
        return method_cache.has(instance_type);
    }
    
    /**
     * @brief Enables or disables the process system
     * 
     * @param enabled If false, _flecs_process won't be called
     */
    void set_process_enabled(bool enabled);
    
    /**
     * @brief Enables or disables the physics process system
     * 
     * @param enabled If false, _flecs_physics_process won't be called
     */
    void set_physics_process_enabled(bool enabled);
    
    /**
     * @brief Checks if process system is enabled
     * 
     * @return true if process system is enabled
     */
    bool is_process_enabled() const;
    
    /**
     * @brief Checks if physics process system is enabled
     * 
     * @return true if physics process system is enabled
     */
    bool is_physics_process_enabled() const;
};
