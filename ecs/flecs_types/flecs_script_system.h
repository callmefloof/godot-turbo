/**
 * @file flecs_script_system.h
 * @brief GDScript-accessible ECS system with flexible dispatch modes and instrumentation
 * 
 * Provides a high-performance bridge between Flecs ECS and GDScript, allowing game logic
 * to process entities with callbacks while maintaining near-native performance through
 * batching, multi-threading, and change-only observation modes.
 * 
 * @author Floof
 * @date 21-7-2025
 */

#ifndef FLECS_SCRIPT_SYSTEM_H
#define FLECS_SCRIPT_SYSTEM_H
#include "core/templates/rid.h"
#include "core/typedefs.h"
#include "core/variant/callable.h"
#include "core/variant/typed_dictionary.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include <atomic>
#include <cstdint>
#include <mutex>




/**
 * @class FlecsScriptSystem
 * @brief High-performance GDScript-accessible ECS system with advanced features
 * 
 * FlecsScriptSystem bridges Flecs ECS with GDScript, providing multiple dispatch modes,
 * performance instrumentation, and flexible execution strategies. It can operate as:
 * - Per-entity dispatch: Call GDScript for each matching entity
 * - Batch dispatch: Accumulate entities and send in batches
 * - Change-only observers: React only to component changes (OnAdd/OnSet/OnRemove)
 * - Task systems: Execute without entity iteration
 * 
 * @section Features
 * - **Dispatch Modes**: Per-entity or batched for reduced GDScript call overhead
 * - **Multi-threading**: Optional parallel entity processing (batched automatically)
 * - **Change Observers**: React to component changes instead of polling every frame
 * - **Instrumentation**: Detailed performance metrics (timings, counts, distributions)
 * - **Batching Control**: Configurable chunk sizes and flush intervals
 * - **Deferred Calls**: Optional call_deferred() for thread-safe operation
 * 
 * @section Performance
 * - Batch mode: ~10-100x fewer GDScript calls vs per-entity
 * - Multi-threaded: Distributes entity processing across CPU cores
 * - Change-only: Processes only changed entities, not all entities every frame
 * 
 * @section Usage
 * @code
 * // GDScript example
 * var system_rid = FlecsServer.add_script_system(
 *     world_rid,
 *     PackedStringArray(["Transform", "Velocity"]),
 *     update_movement
 * )
 * 
 * func update_movement(entities: Array):
 *     for entity_data in entities:
 *         var rid = entity_data["rid"]
 *         var transform = entity_data["components"]["Transform"]
 *         var velocity = entity_data["components"]["Velocity"]
 *         # Process entity...
 * 
 * // Enable batch mode for better performance
 * FlecsServer.set_script_system_dispatch_mode(system_rid, 1) # BATCH
 * FlecsServer.set_script_system_batch_chunk_size(system_rid, 100)
 * @endcode
 * 
 * @note Thread-safety: Multi-threaded mode automatically batches and uses mutex protection
 * @warning GDScript callbacks from worker threads require use_deferred_calls = true
 */
class FlecsScriptSystem {
    /**
     * @struct PendingEntityUpdate
     * @brief Retained for potential future change-observer usage
     * @private
     */
    struct PendingEntityUpdate {
        RID rid; ///< Entity RID
        TypedDictionary<StringName, Dictionary> comps; ///< Component data
    };
    
    // ========================================================================
    // MEMBER VARIABLES
    // ========================================================================
    
    Callable callback; ///< GDScript callback function to invoke with entity data
    PackedStringArray required_components; ///< Component names to query for
    RID world_id; ///< Associated Flecs world RID
    flecs::world *world = nullptr; ///< Pointer to Flecs world instance
    
    // System entities
    flecs::entity script_system; ///< Main system entity handle
    flecs::entity batch_flush_system; ///< Runs after update to flush batches (PostUpdate)
    flecs::entity reset_system; ///< Per-frame auto-reset system (PreUpdate)
    
    // Dispatch configuration
    int dispatch_mode = 0; ///< 0 = per-entity, 1 = batch (enum defined publicly below)
    
    // Batching support
    Array batch_accumulator; ///< Accumulates entity data for batch dispatch
    bool batch_dirty = false; ///< Flag indicating batch has new data
    mutable std::mutex batch_mtx; ///< Protects batch_accumulator & batch_dirty (multi-threaded)
    int batch_flush_chunk_size = 0; ///< 0 = send all at once; >0 = send in chunks
    uint64_t min_flush_interval_usec = 0; ///< Minimum time between flushes (0 = no limit)
    uint64_t last_flush_time_usec = 0; ///< Last flush timestamp in microseconds
    
    // Change-only mode (uses observers instead of per-frame systems)
    bool change_only = false; ///< If true, use observers; if false, use regular systems
    flecs::entity change_observer; ///< OnSet observer handle
    flecs::entity change_observer_add; ///< OnAdd observer handle (optional)
    flecs::entity change_observer_remove; ///< OnRemove observer handle (optional)
    
    // Instrumentation counters
    bool instrumentation_enabled = false; ///< Enable performance tracking
    uint64_t last_frame_entity_count = 0; ///< Entities processed last frame
    uint64_t last_frame_batch_size = 0; ///< Size of last dispatched batch
    uint64_t last_frame_dispatch_usec = 0; ///< Last dispatch duration (microseconds)
    uint64_t total_entities_processed = 0; ///< Lifetime entity count
    uint64_t total_callbacks_invoked = 0; ///< Lifetime callback count
    uint64_t frame_dispatch_invocations = 0; ///< Callback invocations this frame
    uint64_t frame_dispatch_accum_usec = 0; ///< Total dispatch time this frame
    uint64_t frame_dispatch_min_usec = UINT64_MAX; ///< Minimum dispatch time this frame
    uint64_t frame_dispatch_max_usec = 0; ///< Maximum dispatch time this frame
    
    // Detailed timing
    bool detailed_timing_enabled = false; ///< Collect per-dispatch samples
    Vector<uint64_t> frame_dispatch_samples; ///< Per-invocation timings (if detailed_timing_enabled)
    int max_sample_count = 1024; ///< Maximum samples per frame (prevents unbounded growth)
    mutable std::mutex instr_mtx; ///< Protects instrumentation counters (multi-threaded)
    
    // Event counters (change-only mode)
    uint64_t last_frame_onadd = 0; ///< OnAdd events last frame
    uint64_t last_frame_onset = 0; ///< OnSet events last frame
    uint64_t last_frame_onremove = 0; ///< OnRemove events last frame
    uint64_t total_onadd = 0; ///< Lifetime OnAdd events
    uint64_t total_onset = 0; ///< Lifetime OnSet events
    uint64_t total_onremove = 0; ///< Lifetime OnRemove events
    
    // Configuration flags
    bool auto_reset_per_frame = false; ///< Auto-reset per-frame counters each frame
    bool observe_add_and_set = true; ///< In change-only mode: observe OnAdd & OnSet
    bool observe_remove = false; ///< In change-only mode: observe OnRemove
    bool is_paused = false; ///< System paused flag
    bool multi_threaded = false; ///< Request Flecs to schedule multi-threaded
    bool use_deferred_calls = false; ///< Use call_deferred() instead of immediate call()
    
    // Identity
    static std::atomic_uint32_t global_system_index; ///< Global counter for system IDs
    uint32_t id = ++global_system_index; ///< Unique system ID
    uint32_t depends_on_system_id = 0; ///< Optional dependency on another system
    String system_name; ///< Human-readable system name

    /**
     * @brief Build or rebuild the Flecs system based on current configuration
     * @private
     * 
     * Creates appropriate Flecs systems/observers based on:
     * - change_only: observers vs regular systems
     * - required_components.size(): task vs entity iteration
     * - dispatch_mode: per-entity vs batch
     * - multi_threaded: single vs multi-threaded
     * 
     * Also creates auxiliary systems (batch flush, auto-reset) as needed.
     */
    void build_system();
private:
    // ========================================================================
    // PRIVATE HELPER METHODS
    // ========================================================================
    
    /** @brief Destruct all existing system/observer entities before rebuild */
    void cleanup_existing_systems();
    
    /** @brief Build observer-based system for change-only mode */
    void build_change_observer_system();
    
    /** @brief Build task system (no entity iteration) */
    void build_task_system();
    
    /** @brief Build entity iteration system (regular or multi-threaded) */
    void build_entity_iteration_system();
    
    /** @brief Build batch flush system (PostUpdate phase) */
    void build_batch_flush_system();
    
    /** @brief Build auto-reset instrumentation system (PreUpdate phase) */
    void build_auto_reset_system();
    
    /** @brief Convert component names to Flecs entity terms */
    Vector<flecs::entity> get_component_terms();
    
    /** @brief Serialize all required components from an entity */
    Dictionary serialize_entity_components(flecs::entity e);
    
    /** @brief Update instrumentation counters after dispatch */
    void update_instrumentation(uint64_t start_time);
    
    /** @brief Invoke callback with proper deferred/immediate handling */
    void dispatch_callback(const Array& data);
    
public:
public:
    // ========================================================================
    // PUBLIC TYPES
    // ========================================================================
    
    /**
     * @enum DispatchMode
     * @brief Controls how entities are dispatched to GDScript callback
     */
    enum DispatchMode {
        DISPATCH_PER_ENTITY = 0, ///< Call GDScript once per entity (simple, higher overhead)
        DISPATCH_BATCH = 1       ///< Accumulate entities and call GDScript with batches (faster)
    };
    // ========================================================================
    // CONFIGURATION METHODS
    // ========================================================================
    
    /** @brief Get current dispatch mode */
    DispatchMode get_dispatch_mode() const { return static_cast<DispatchMode>(dispatch_mode); }
    
    /** @brief Set dispatch mode (rebuilds system) */
    void set_dispatch_mode(DispatchMode p_mode);
    
    /** @brief Enable/disable change-only mode (rebuilds system) */
    void set_change_only(bool p_change_only);
    
    /** @brief Set whether to observe both OnAdd & OnSet (change-only mode) */
    void set_change_observe_add_and_set(bool p_both);
    
    /** @brief Get OnAdd & OnSet observation setting */
    bool get_change_observe_add_and_set() const { return observe_add_and_set; }
    
    /** @brief Enable/disable OnRemove observation (change-only mode) */
    void set_change_observe_remove(bool p_remove);
    
    /** @brief Get OnRemove observation setting */
    bool get_change_observe_remove() const { return observe_remove; }
    
    /** @brief Enable/disable multi-threaded execution (rebuilds system) */
    void set_multi_threaded(bool p_enable) { multi_threaded = p_enable; build_system(); }
    
    /** @brief Get multi-threaded setting */
    bool get_multi_threaded() const { return multi_threaded; }
    
    /** @brief Set batch chunk size (0 = send all at once, >0 = chunk size) */
    void set_batch_flush_chunk_size(int p_size) { batch_flush_chunk_size = p_size < 0 ? 0 : p_size; }
    
    /** @brief Get batch chunk size */
    int get_batch_flush_chunk_size() const { return batch_flush_chunk_size; }
    
    /** @brief Set minimum flush interval in milliseconds (0 = no limit) */
    void set_flush_min_interval_msec(double p_ms) { if (p_ms <= 0.0) { min_flush_interval_usec = 0; } else { min_flush_interval_usec = (uint64_t)(p_ms * 1000.0); } }
    
    /** @brief Get minimum flush interval in milliseconds */
    double get_flush_min_interval_msec() const { return min_flush_interval_usec == 0 ? 0.0 : (double)min_flush_interval_usec / 1000.0; }
    
    /** @brief Enable/disable deferred calls (true = call_deferred, false = immediate) */
    void set_use_deferred_calls(bool p_deferred) { use_deferred_calls = p_deferred; }
    
    /** @brief Get deferred calls setting */
    bool get_use_deferred_calls() const { return use_deferred_calls; }
    
    /** @brief Check if system is in change-only mode */
    bool is_change_only() const { return change_only; }
    
    // ========================================================================
    // INSTRUMENTATION METHODS
    // ========================================================================
    
    /** @brief Enable/disable performance instrumentation */
    void set_instrumentation_enabled(bool p_enabled) { instrumentation_enabled = p_enabled; }
    
    /** @brief Get instrumentation enabled state */
    bool get_instrumentation_enabled() const { return instrumentation_enabled; }
    
    /** @brief Enable/disable detailed per-dispatch timing samples */
    void set_detailed_timing_enabled(bool p_enabled) { detailed_timing_enabled = p_enabled; }
    
    /** @brief Get detailed timing enabled state */
    bool get_detailed_timing_enabled() const { return detailed_timing_enabled; }
    
    /** @brief Enable/disable automatic per-frame counter reset */
    void set_auto_reset_per_frame(bool p_auto) { auto_reset_per_frame = p_auto; }
    
    /** @brief Get auto-reset setting */
    bool get_auto_reset_per_frame() const { return auto_reset_per_frame; }
    
    /** @brief Get entities processed in last frame */
    uint64_t get_last_frame_entity_count() const { return last_frame_entity_count; }
    
    /** @brief Get last batch size dispatched */
    uint64_t get_last_frame_batch_size() const { return last_frame_batch_size; }
    
    /** @brief Get last dispatch duration in microseconds */
    uint64_t get_last_frame_dispatch_usec() const { return last_frame_dispatch_usec; }
    
    /** @brief Get total entities processed (lifetime) */
    uint64_t get_total_entities_processed() const { return total_entities_processed; }
    
    /** @brief Get total callbacks invoked (lifetime) */
    uint64_t get_total_callbacks_invoked() const { return total_callbacks_invoked; }
    
    /** @brief Get callback invocations this frame */
    uint64_t get_frame_dispatch_invocations() const { return frame_dispatch_invocations; }
    
    /** @brief Get accumulated dispatch time this frame (microseconds) */
    uint64_t get_frame_dispatch_accum_usec() const { return frame_dispatch_accum_usec; }
    
    /** @brief Get minimum dispatch time this frame (microseconds) */
    uint64_t get_frame_dispatch_min_usec() const { return frame_dispatch_min_usec == UINT64_MAX ? 0 : frame_dispatch_min_usec; }
    
    /** @brief Get maximum dispatch time this frame (microseconds) */
    uint64_t get_frame_dispatch_max_usec() const { return frame_dispatch_max_usec; }
    
    /**
     * @brief Get median dispatch time this frame
     * @return Median time in microseconds (0 if no samples or detailed timing disabled)
     */
    double get_frame_dispatch_median_usec() const;
    
    /**
     * @brief Get percentile dispatch time this frame
     * @param p Percentile (0-100)
     * @return Percentile time in microseconds using nearest-rank method
     */
    double get_frame_dispatch_percentile_usec(double p) const;
    
    /**
     * @brief Get standard deviation of dispatch times this frame
     * @return Standard deviation in microseconds
     */
    double get_frame_dispatch_stddev_usec() const;
    
    /** @brief Get maximum sample count per frame */
    int get_max_sample_count() const { return max_sample_count; }
    
    /** @brief Set maximum sample count per frame */
    void set_max_sample_count(int p_cap) { max_sample_count = p_cap <= 0 ? 1 : p_cap; }
    
    /** @brief Get OnAdd events last frame (change-only mode) */
    uint64_t get_last_frame_onadd() const { return last_frame_onadd; }
    
    /** @brief Get OnSet events last frame (change-only mode) */
    uint64_t get_last_frame_onset() const { return last_frame_onset; }
    
    /** @brief Get OnRemove events last frame (change-only mode) */
    uint64_t get_last_frame_onremove() const { return last_frame_onremove; }
    
    /** @brief Get total OnAdd events (lifetime, change-only mode) */
    uint64_t get_total_onadd() const { return total_onadd; }
    
    /** @brief Get total OnSet events (lifetime, change-only mode) */
    uint64_t get_total_onset() const { return total_onset; }
    
    /** @brief Get total OnRemove events (lifetime, change-only mode) */
    uint64_t get_total_onremove() const { return total_onremove; }
    
    /**
     * @brief Get raw timing samples for this frame
     * @return Reference to vector of microsecond timings (internal use)
     * @private
     */
    const Vector<uint64_t> &_get_frame_dispatch_samples() const { return frame_dispatch_samples; }
    
    /** @brief Reset all instrumentation counters to zero */
    void reset_instrumentation();
    
    // ========================================================================
    // LIFECYCLE METHODS
    // ========================================================================
    
    /**
     * @brief Initialize system with world, components, and callback
     * @param world_id Flecs world RID
     * @param req_comps Required component names to query
     * @param p_callable GDScript callback function
     */
    void init(const RID &world_id, const PackedStringArray &req_comps, const Callable& p_callable);
    
    /**
     * @brief Reset and reinitialize system
     * @param world_id Flecs world RID
     * @param req_comps Required component names
     * @param p_callable GDScript callback
     */
    void reset(const RID &world_id, const PackedStringArray &req_comps, const Callable& p_callable);
    
    /** @brief Set required component names (rebuilds system) */
    void set_required_components(const PackedStringArray &req_comps);
    
    /** @brief Get required component names */
    PackedStringArray get_required_components() const;
    
    /** @brief Set callback function (rebuilds system) */
    void set_callback(const Callable& p_callback);
    
    /** @brief Get current callback */
    Callable get_callback() const;
    
    /** @brief Get required components (duplicate method, kept for compatibility) */
    PackedStringArray get_required_components();
    
    /**
     * @brief Get world pointer (internal use)
     * @return Raw pointer to flecs::world
     * @private
     */
    flecs::world* _get_world() const;
    
    /**
     * @brief Set world pointer (internal use)
     * @param p_world Raw pointer to flecs::world
     * @private
     */
    void _set_world(flecs::world *p_world);
    
    /** @brief Get world RID */
    RID get_world();
    
    /** @brief Set world RID */
    void set_world(const RID &world_id);
    
    // ========================================================================
    // PAUSE & DEPENDENCY METHODS
    // ========================================================================
    
    /** @brief Pause/unpause system execution */
    void set_is_paused(bool p_paused) { is_paused = p_paused; }
    
    /** @brief Check if system is paused */
    bool get_is_paused() const { return is_paused; }
    
    /** @brief Check if system has a dependency */
    bool get_depends_on_system() const { return depends_on_system_id != 0; }
    
    /** @brief Get unique system ID */
    uint32_t get_system_id() const { return id; }
    
    /** @brief Set system to depend on another system */
    void set_system_dependency(uint32_t p_system_id);
    
    /** @brief Get dependency system ID */
    uint32_t get_system_dependency_id() const { return depends_on_system_id; }
    
    /** @brief Set human-readable system name */
    void set_system_name(const String &p_name) { system_name = p_name; if(script_system.is_valid()) { script_system.set_name(p_name.ascii().get_data()); } }
    
    /** @brief Get system name */
    String get_system_name() const { return system_name; }
    
    // ========================================================================
    // CONSTRUCTORS & OPERATORS
    // ========================================================================
    
    /** @brief Default constructor */
    FlecsScriptSystem() = default;
    
    /** @brief Destructor - cleans up Flecs system entities */
    ~FlecsScriptSystem();
    
    /** @brief Copy constructor */
    FlecsScriptSystem(const FlecsScriptSystem &other);
    
    /** @brief Assignment operator */
    FlecsScriptSystem& operator=(const FlecsScriptSystem &other);
};



#endif //FLECS_SCRIPT_SYSTEM_H
