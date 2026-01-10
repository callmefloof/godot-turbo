#pragma once
#include "modules/godot_turbo/ecs/flecs_types/flecs_phases.h"
#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"

/**
 * @class PipelineManager
 * @brief Manages the Flecs ECS pipeline and system execution order
 * 
 * PipelineManager provides high-level control over the Flecs pipeline, which determines
 * the order and phases in which ECS systems execute. It handles system registration,
 * custom phase creation, and system lookup.
 * 
 * @section Architecture
 * The pipeline manager maintains:
 * - A reference to the Flecs pipeline entity
 * - A collection of registered systems with their execution phases
 * - The associated Flecs world RID
 * 
 * @section Phases
 * Flecs provides built-in phases:
 * - `flecs::OnLoad` - Pre-frame initialization
 * - `flecs::PostLoad` - After loading
 * - `flecs::PreUpdate` - Before main update
 * - `flecs::OnUpdate` - Main game logic (default)
 * - `flecs::OnValidate` - Validation/constraints
 * - `flecs::PostUpdate` - After main update
 * - `flecs::PreStore` - Before storage
 * - `flecs::OnStore` - Store/serialize
 * - `flecs::PostFrame` - End of frame cleanup
 * - `flecs::OnPhysicsUpdate` - Physics simulation
 * 
 * Custom phases can be created with dependencies to control execution order.
 * 
 * @section Usage
 * ```cpp
 * // Create a pipeline manager for a world
 * RID world_rid = flecs_server->create_world();
 * PipelineManager pipeline(world_rid);
 * 
 * // Add a system to the default (OnUpdate) phase
 * flecs::system my_system = world->system<Transform3DComponent>()
 *     .iter([](flecs::iter& it, Transform3DComponent* transforms) {
 *         // System logic
 *     });
 * pipeline.add_to_pipeline(my_system);
 * 
 * // Add a system to a specific phase
 * flecs::system physics_system = world->system<RigidBodyComponent>()
 *     .iter([](flecs::iter& it, RigidBodyComponent* bodies) {
 *         // Physics logic
 *     });
 * pipeline.add_to_pipeline(physics_system, flecs::OnPhysicsUpdate);
 * 
 * // Create a custom phase
 * flecs::entity custom_phase = pipeline.create_custom_phase("CustomLogic", "OnUpdate");
 * pipeline.add_to_pipeline(another_system, custom_phase);
 * 
 * // Retrieve a system by name
 * flecs::system* found = pipeline.try_get_system("MySystem");
 * if (found) {
 *     found->enable(); // or disable()
 * }
 * ```
 * 
 * @note Systems added without an explicit phase use `flecs::OnUpdate` by default
 * @note System names must be unique for lookup to work correctly
 * @warning The pipeline manager does not own the Flecs world; ensure the world
 *          outlives the pipeline manager
 */
class PipelineManager {
    private:
        /**
         * @struct FlecsSystemContainer
         * @brief Internal storage for a system and its associated phase
         */
        struct FlecsSystemContainer{
            flecs::system system;              ///< The Flecs system entity
            flecs::entity_t relationship = 0ULL; ///< The phase/relationship this system belongs to
        };
        
        flecs::entity pipeline;                ///< The Flecs pipeline entity
        RID world_rid;                         ///< RID of the associated Flecs world
        Vector<FlecsSystemContainer> systems;  ///< Collection of registered systems
        
    public:
        /**
         * @brief Default constructor - creates an uninitialized manager
         * 
         * Call `set_world()` to initialize after construction.
         */
        PipelineManager() = default;
        
        /**
         * @brief Constructs a pipeline manager for a specific world
         * 
         * Retrieves the world's default pipeline and prepares the manager.
         * 
         * @param p_world_rid RID of the Flecs world to manage
         */
        PipelineManager(const RID& p_world_rid);
        
        /**
         * @brief Destructor - systems are cleaned up by Flecs
         */
        virtual ~PipelineManager() = default;
        
        /**
         * @brief Copy constructor - duplicates pipeline state
         * 
         * @param rhs The pipeline manager to copy from
         */
        PipelineManager(const PipelineManager& rhs);
        
        /**
         * @brief Copy assignment operator
         * 
         * @param rhs The pipeline manager to copy from
         * @return Reference to this manager
         */
        PipelineManager& operator=(const PipelineManager& rhs);
        
        /**
         * @brief Move constructor - transfers ownership
         * 
         * @param rhs The pipeline manager to move from
         */
        PipelineManager(PipelineManager&& rhs) noexcept;
        
        /**
         * @brief Move assignment operator
         * 
         * @param rhs The pipeline manager to move from
         * @return Reference to this manager
         */
        PipelineManager& operator=(PipelineManager&& rhs) noexcept;
        
        /**
         * @brief Finds a system by name
         * 
         * Searches the registered systems for one with the given name.
         * 
         * @param name The name of the system to find
         * @return Pointer to the system if found, nullptr otherwise
         * 
         * @note System names are assigned when creating the system with `.name()`
         */
        flecs::system* try_get_system(const String &name);
        
        /**
         * @brief Adds a system to the pipeline with default phase
         * 
         * Registers the system to run during the `flecs::OnUpdate` phase.
         * 
         * @param system The Flecs system to add
         * 
         * @warning System must have a name assigned or an error will be printed
         */
        void add_to_pipeline(flecs::system system);
        
        /**
         * @brief Adds a system to the pipeline with a specific phase
         * 
         * Registers the system to run during the specified execution phase.
         * The phase determines when the system executes relative to other systems.
         * 
         * @param system The Flecs system to add
         * @param phase The execution phase (e.g., flecs::OnUpdate, flecs::OnPhysicsUpdate)
         * 
         * @warning System must have a name assigned or registration will fail
         * @note The system is automatically added to the phase relationship
         */
        void add_to_pipeline(flecs::system system, flecs::entity_t phase);
        
        /**
         * @brief Creates a custom execution phase
         * 
         * Defines a new phase that can be used to organize system execution.
         * Optionally specify a dependency to control execution order.
         * 
         * @param phase_name Unique name for the custom phase
         * @param depends_on Name of phase this phase should run after (optional)
         * @return The created phase entity
         * 
         * @example
         * ```cpp
         * // Create a phase that runs after OnUpdate
         * auto late_update = pipeline.create_custom_phase("LateUpdate", "OnUpdate");
         * 
         * // Create an independent phase
         * auto custom = pipeline.create_custom_phase("CustomPhase");
         * ```
         */
        flecs::entity create_custom_phase(const String &phase_name, const String &depends_on = "");
        
        /**
         * @brief Sets or changes the associated Flecs world
         * 
         * Updates the world RID and retrieves the new world's pipeline.
         * Existing registered systems remain in the systems vector but may not
         * be valid if they were from a different world.
         * 
         * @param p_world_rid RID of the new Flecs world
         * 
         * @note Consider clearing the systems vector when changing worlds
         */
        void set_world(const RID& p_world_rid);
        
        /**
         * @brief Gets the associated world RID
         * 
         * @return RID of the Flecs world this manager is managing
         */
        RID get_world();
};
