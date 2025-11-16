#pragma once
#include "core/templates/a_hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "core/object/object.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"
#include "core/templates/rid.h"
#include "core/templates/rid_owner.h"
#include "core/variant/variant.h"
#include "core/object/ref_counted.h"
#include "flecs_script_system.h"
#include "flecs_query.h"
#include <cstdint>
#include "modules/godot_turbo/ecs/systems/command.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "core/variant/callable.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_variant.h"
#include "modules/godot_turbo/ecs/systems/utility/node_storage.h"
#include "modules/godot_turbo/ecs/systems/utility/ref_storage.h"
#include <limits>
#include <atomic>


#define CHECK_ENTITY_VALIDITY_V(entity_id, world_id, default_value, func_name) \
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id); \
	if (!entity_variant) { \
		ERR_PRINT("FlecsServer:: " #func_name ": entity_id is not a valid entity"); \
		return default_value; \
	} \
	flecs::entity entity = entity_variant->get_entity();

#define CHECK_WORLD_VALIDITY_V(world_id, default_value, func_name) \
	FlecsWorldVariant* world_variant = flecs_world_owners.get_or_null(world_id); \
	if (!world_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": world_id is not a valid world"); \
		return default_value; \
	} \

#define CHECK_ENTITY_VALIDITY(entity_id, world_id, func_name) \
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id); \
	if (!entity_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": entity_id is not a valid entity"); \
		return; \
	} \


#define CHECK_WORLD_VALIDITY(world_id, func_name) \
	FlecsWorldVariant* world_variant = flecs_world_owners.get_or_null(world_id); \
	if (!world_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": world_id is not a valid world"); \
		return; \
	} \


#define CHECK_SYSTEM_VALIDITY_V(system_id, world_id, default_value, func_name) \
	FlecsSystemVariant* system_variant = flecs_variant_owners.get(world_id).system_owner.get_or_null(system_id); \
	if (!system_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": system_id is not a valid system"); \
		return default_value; \
	} \


#define CHECK_SYSTEM_VALIDITY(system_id, world_id, func_name) \
	FlecsSystemVariant* system_variant = flecs_variant_owners.get(world_id).system_owner.get_or_null(system_id); \
	if (!system_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": system_id is not a valid system"); \
		return; \
	} \


#define CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, default_value, func_name) \
	FlecsScriptSystem* script_system = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(script_system_id); \
	if (!script_system) { \
		ERR_PRINT("FlecsServer::" #func_name ": script_system_id is not a valid script system"); \
		return default_value; \
	} \

#define CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, func_name) \
	FlecsScriptSystem* script_system = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(script_system_id); \
	if (!script_system) { \
		ERR_PRINT("FlecsServer::" #func_name ": script_system_id is not a valid script system"); \
		return; \
	} \

#define CHECK_QUERY_VALIDITY_V(query_id, world_id, default_value, func_name) \
	FlecsQuery* query = flecs_variant_owners.get(world_id).query_owner.get_or_null(query_id); \
	if (!query) { \
		ERR_PRINT("FlecsServer::" #func_name ": query_id is not a valid query"); \
		return default_value; \
	} \

#define CHECK_QUERY_VALIDITY(query_id, world_id, func_name) \
	FlecsQuery* query = flecs_variant_owners.get(world_id).query_owner.get_or_null(query_id); \
	if (!query) { \
		ERR_PRINT("FlecsServer::" #func_name ": query_id is not a valid query"); \
		return; \
	} \

#define CHECK_TYPE_ID_VALIDITY_V(type_rid, world_id, default_value, func_name) \
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(type_rid); \
	if (!type_id_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": type_id is not a valid type ID"); \
		return default_value; \
	} \


#define CHECK_TYPE_ID_VALIDITY(type_id, world_id, func_name) \
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(type_id); \
	if (!type_id_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": type_id is not a valid type ID"); \
		return; \
	} \

constexpr static int MAX_WORLD_COUNT = std::numeric_limits<uint8_t>::max() + 1;
constexpr static int MAX_ENTITY_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_COMPONENT_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_SYSTEM_COUNT = std::numeric_limits<uint16_t>::max() + 1;
constexpr static int MAX_SCRIPT_SYSTEM_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_TYPE_ID_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_COMMAND_HANDLER_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_QUERY_COUNT = std::numeric_limits<uint32_t>::max() + 1;


constexpr static int WORLD_OWNER_CHUNK_SIZE = 65536;
constexpr static int ENTITY_OWNER_CHUNK_SIZE = 65536;
constexpr static int COMPONENT_OWNER_CHUNK_SIZE = 65536;
constexpr static int SYSTEM_OWNER_CHUNK_SIZE = 65536;
constexpr static int SCRIPT_SYSTEM_OWNER_CHUNK_SIZE = 65536;
constexpr static int TYPE_ID_OWNER_CHUNK_SIZE = 65536;
constexpr static int COMMAND_HANDLER_OWNER_CHUNK_SIZE = 65536;
constexpr static int QUERY_OWNER_CHUNK_SIZE = 65536;



/**
 * @class FlecsServer
 * @brief Central singleton server managing Flecs ECS worlds and their lifecycle
 * 
 * FlecsServer is the core interface between Godot and the Flecs ECS library.
 * It manages multiple independent ECS worlds, entities, components, systems,
 * and provides a GDScript-friendly API for all ECS operations.
 * 
 * @section Architecture
 * - **Singleton Pattern**: Single global instance accessible from GDScript
 * - **Multi-World Support**: Each world is an isolated ECS instance
 * - **RID-Based API**: All ECS objects (worlds, entities, systems) are referenced by RIDs
 * - **Thread-Safe**: Operations are mutex-protected for safe multi-threaded access
 * 
 * @section Key Features
 * - World creation and management
 * - Entity creation with components and hierarchies
 * - Component registration and manipulation
 * - Script systems (GDScript callbacks) and native systems
 * - Query support for efficient entity iteration
 * - Resource and Node storage integration
 * - Command handlers for extensible systems
 * - Pipeline management for system ordering
 * - Comprehensive instrumentation and profiling
 * 
 * @section Usage from GDScript
 * @code
 * # Get singleton
 * var flecs = FlecsServer.get_singleton()
 * 
 * # Create world
 * var world = flecs.create_world()
 * 
 * # Register component type
 * var transform_type = flecs.register_component_type(world, "Transform")
 * 
 * # Create entity with component
 * var entity = flecs.create_entity_with_name(world, "Player")
 * flecs.set_component(world, entity, "Transform", {"position": Vector3.ZERO})
 * 
 * # Add script system
 * var system = flecs.add_script_system(
 *     world,
 *     PackedStringArray(["Transform", "Velocity"]),
 *     update_movement
 * )
 * 
 * # Progress world each frame
 * func _process(delta):
 *     flecs.progress_world(world, delta)
 * @endcode
 * 
 * @section Performance
 * - RID_Owner provides fast O(1) lookups for ECS objects
 * - Chunk-based allocation reduces fragmentation
 * - Multi-threaded system execution support
 * - Batched operations for reduced overhead
 * 
 * @note All RIDs must be freed with appropriate free_*() methods to prevent leaks
 * @warning Always check RID validity before use (RIDs can become invalid after free)
 * 
 * @see FlecsScriptSystem, FlecsQuery, FlecsWorldVariant, FlecsEntityVariant
 */
class FlecsServer : public Object {
	GDCLASS(FlecsServer, Object);
	constexpr static char render_command_handler_name[] = "Rendering";
	constexpr static char command_handler_name[] = "CommandHandler";
	constexpr static char script_system_name[] = "ScriptSystem";

	std::atomic_uint16_t counter = 0;
	static FlecsServer *singleton;
	static void thread_func(void *p_udata);
	bool thread_exited = false;
	mutable bool exit_thread;
	Thread thread;
	Mutex mutex;

	double delta_time = 0.0;
	HashMap<RID, bool> regular_system_paused; // track pause state for non-script systems

protected:
	static void _bind_methods();

public:
	static FlecsServer *get_singleton();
	Error init();
	void lock();
	void unlock();
	void finish();
	FlecsServer();
	~FlecsServer();
	RID create_world();
	int8_t get_world_count() const;
	void init_world(const RID& world_id);

	bool progress_world(const RID& world_id, const double delta);
	RID add_script_system(const RID& world_id, const Array &component_types, const Callable &callable);
	RID create_entity(const RID& world_id);
	RID create_entity_with_name(const RID& world_id, const String &name);
	RID create_entity_with_name_and_comps(const RID& world_id, const String &name, const TypedArray<RID> &components_type_ids);
	RID lookup(const RID& world_id, const String &entity_name);
	flecs::world *_get_world(const RID &world_id);
	RID get_world_of_entity(const RID &entity_id);
	void set_log_level(const int level);
	
#ifndef DISABLE_DEPRECATED
	// DEPRECATED: Use create_runtime_component() instead. Will be removed in v2.0.0.
	// This method uses heap-allocated ScriptVisibleComponent with Dictionary, which is less efficient.
	RID register_component_type(const RID& world_id, const String &type_name, const Dictionary &script_visible_component_data);
#endif // DISABLE_DEPRECATED
	
	// Create a component type at runtime with typed fields using Flecs reflection API.
	// Supports all Godot Variant types (primitives, vectors, transforms, etc.)
	// Returns component type RID on success, invalid RID on failure.
	RID create_runtime_component(const RID& world_id, const String &component_name, const Dictionary &fields);
	Ref<CommandHandler> get_render_system_command_handler(const RID &world_id);
	PipelineManager* _get_pipeline_manager(const RID &world_id);
	void remove_all_components_from_entity(const RID &entity_id);
	bool has_component(const RID &entity_id,const String &component_type);
	PackedStringArray get_component_types_as_name(const RID &entity_id);
	TypedArray<RID> get_component_types_as_id(const RID &entity_id);

	// Debug helpers
	void debug_check_rid(const RID &rid);

	String get_entity_name(const RID &entity_id);
	void set_entity_name(const RID& entity_id, const String &p_name);
	void set_component(const RID& entity_id, const String& component_name, const Dictionary &comp_data);
	void remove_component_from_entity_with_id(const RID &entity_id, const RID &component_type_id);
	void remove_component_from_entity_with_name(const RID &entity_id,const String &component_type);
	Dictionary get_component_by_name(const RID &entity_id, const String &component_type);
	Dictionary get_component_by_id(const RID& entity_id, const RID& component_type_id);
	RID get_component_type_by_name(const RID& entity_id, const String &component_type);
	RID get_parent(const RID &entity_id);
	void set_parent(const RID& entity_id, const RID& parent_id);
	void add_component(const RID &entity_id, const RID &comp_rid);
	void add_child(const RID &parent_id, const RID &child_id);
	void remove_child(const RID& parent_id, const RID &child_id);
	TypedArray<RID> get_children(const RID &parent_id);
	RID get_child(const RID &parent_id, int index);
	void set_children(const RID &parent_id, const TypedArray<RID> &children);
	RID get_child_by_name(const RID &parent_id,const String &name);
	void remove_child_by_name(const RID &parent_id, const String &name);
	void remove_child_by_index(const RID &parent_id, int index);
	void remove_all_children(const RID &parent_id);
	void add_relationship(const RID& entity_id, const RID &relationship);
	void remove_relationship(const RID& entity_id, const RID &relationship);
	RID get_relationship(const RID& entity_id, const String& first_entity, const String& second_entity);
	TypedArray<RID> get_relationships(const RID& entity_id);
	RID _create_rid_for_entity(const RID& world_id, const flecs::entity &entity);
	RID _create_rid_for_system(const RID& world_id, const flecs::system &system);
	RID _get_rid_for_world(const flecs::world *world);
	RID _create_rid_for_type_id(const RID& world_id, const flecs::entity_t &type_id);
	RID _create_rid_for_script_system(const RID& world_id, const FlecsScriptSystem &system);
	void free_world(const RID& world_id);
	void free_system(const RID& world_id, const RID& system_id, const bool include_flecs_world);
	void free_script_system(const RID& world_id, const RID& script_system_id);
	void free_entity(const RID& world_id, const RID& entity_id, const bool include_flecs_world);
	flecs::entity _get_entity(const RID& entity_id, const RID& world_id);
	void free_type_id(const RID& world_id, const RID& type_id);
	void add_to_ref_storage(const Ref<Resource> &resource, const RID &world_id);
	void remove_from_ref_storage(const RID &resource_rid, const RID &world_id);
	void add_to_node_storage(Node *node, const RID &world_id);
	void remove_from_node_storage(const  int64_t node_id, const RID &world_id);
	Ref<Resource> get_resource_from_ref_storage(const RID &resource_id, const RID &world_id);
	Node *get_node_from_node_storage(const int64_t node_id, const RID &world_id);
	RID _get_or_create_rid_for_entity(const RID &world_id, const flecs::entity &entity);
	flecs::system _get_system(const RID &system_id, const RID &world_id);
	flecs::entity_t _get_type_id(const RID &type_id, const RID &world_id);
	FlecsScriptSystem _get_script_system(const RID &script_system_id, const RID &world_id);
	void set_world_singleton_with_name(const RID &world_id, const String& comp_type, const Dictionary& comp_data);
	void set_world_singleton_with_id(const RID &world_id, const RID &comp_type_id, const Dictionary& comp_data);
	Dictionary get_world_singleton_with_name(const RID &world_id, const String& comp_type);
	Dictionary get_world_singleton_with_id(const RID &world_id, const RID &comp_type_id);
	void remove_world_singleton_with_name(const RID &world_id, const String& comp_type);
	Ref<CommandHandler> get_command_handler(const RID &world_id, const String &name);
	void register_command_handler(const RID &world_id, const String &name, const Ref<CommandHandler> &command_handler);
	void unregister_command_handler(const RID &world_id, const String &name);
	void set_script_system_callback(const RID &world_id, const RID &script_system_id, const Callable &callable);
	Callable get_script_system_callback(const RID &world_id, const RID &script_system_id);
	void set_script_system_required_components(const RID &world_id, const RID &script_system_id, const PackedStringArray &required_components);
	PackedStringArray get_script_system_required_components(const RID &world_id, const RID &script_system_id);
	void set_script_system_world(const RID &world_id, const RID &script_system_id, const RID &script_system_world_id);
	RID get_script_system_world(const RID &world_id, const RID &script_system_id);
	// Script system advanced controls
	void set_script_system_dispatch_mode(const RID &world_id, const RID &script_system_id, int mode); // 0=per_entity,1=batch
	int get_script_system_dispatch_mode(const RID &world_id, const RID &script_system_id);
	void set_script_system_change_only(const RID &world_id, const RID &script_system_id, bool change_only);
	bool is_script_system_change_only(const RID &world_id, const RID &script_system_id);
	void set_script_system_instrumentation(const RID &world_id, const RID &script_system_id, bool enabled);
	Dictionary get_script_system_instrumentation(const RID &world_id, const RID &script_system_id);
	void reset_script_system_instrumentation(const RID &world_id, const RID &script_system_id);

	// Additional script system wrappers
	void set_script_system_paused(const RID &world_id, const RID &script_system_id, bool paused);
	bool is_script_system_paused(const RID &world_id, const RID &script_system_id);
	void set_script_system_dependency(const RID &world_id, const RID &script_system_id, uint32_t dep_id);
	Dictionary get_all_systems(const RID &world_id); // returns { cpp: [ {rid,name,depends_on}], script: [ {...} ] }
	void set_script_system_change_observe_add_and_set(const RID &world_id, const RID &script_system_id, bool both);
	bool get_script_system_change_observe_add_and_set(const RID &world_id, const RID &script_system_id);
	void set_script_system_auto_reset(const RID &world_id, const RID &script_system_id, bool auto_reset);
	bool get_script_system_auto_reset(const RID &world_id, const RID &script_system_id);
	Dictionary get_world_frame_summary(const RID &world_id); // aggregated per-frame summary
	void reset_world_frame_summary(const RID &world_id);
	Dictionary get_world_distribution_summary(const RID &world_id); // approximate aggregated distribution stats
	// Enable/disable collection of per-invocation timing samples (used for min/max & potential percentile calculations)
	void set_script_system_detailed_timing(const RID &world_id, const RID &script_system_id, bool enabled);
	// Returns whether detailed timing samples are currently being recorded
	bool get_script_system_detailed_timing(const RID &world_id, const RID &script_system_id);
	// Multi-threaded system control and batching behavior
	void set_script_system_multi_threaded(const RID &world_id, const RID &script_system_id, bool enable);
	bool get_script_system_multi_threaded(const RID &world_id, const RID &script_system_id);
	void set_script_system_batch_chunk_size(const RID &world_id, const RID &script_system_id, int chunk_size);
	int get_script_system_batch_chunk_size(const RID &world_id, const RID &script_system_id);
	void set_script_system_flush_min_interval_msec(const RID &world_id, const RID &script_system_id, double msec);
	double get_script_system_flush_min_interval_msec(const RID &world_id, const RID &script_system_id);
	void set_script_system_use_deferred_calls(const RID &world_id, const RID &script_system_id, bool use_deferred);
	bool get_script_system_use_deferred_calls(const RID &world_id, const RID &script_system_id);
	// Get cumulative event totals across the lifetime of the script system (OnAdd/OnSet/OnRemove)
	Dictionary get_script_system_event_totals(const RID &world_id, const RID &script_system_id);
	// Timing distribution helpers (useful only if detailed timing enabled this frame)
	double get_script_system_frame_median_usec(const RID &world_id, const RID &script_system_id);
	double get_script_system_frame_percentile_usec(const RID &world_id, const RID &script_system_id, double percentile);
	double get_script_system_frame_stddev_usec(const RID &world_id, const RID &script_system_id);
	double get_script_system_frame_p99_usec(const RID &world_id, const RID &script_system_id) { return get_script_system_frame_percentile_usec(world_id, script_system_id, 99.0); }
	int get_script_system_max_sample_count(const RID &world_id, const RID &script_system_id);
	void set_script_system_max_sample_count(const RID &world_id, const RID &script_system_id, int cap);
	int get_script_system_last_frame_entity_count(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_last_frame_dispatch_usec(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_frame_dispatch_invocations(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_frame_dispatch_accum_usec(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_frame_min_usec(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_frame_max_usec(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_last_frame_onadd(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_last_frame_onset(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_last_frame_onremove(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_total_callbacks(const RID &world_id, const RID &script_system_id);
	uint64_t get_script_system_total_entities_processed(const RID &world_id, const RID &script_system_id);

	Ref<Resource> make_script_system_inspector(const RID &world_id, const RID &script_system_id);

	Dictionary get_script_system_info(const RID &world_id, const RID &script_system_id);
	Dictionary get_system_info(const RID &world_id, const RID &system_id);
	void set_system_paused(const RID &world_id, const RID &system_id, bool paused);
	bool is_system_paused(const RID &world_id, const RID &system_id);
	void set_script_system_change_observe_remove(const RID &world_id, const RID &script_system_id, bool enable);
	bool get_script_system_change_observe_remove(const RID &world_id, const RID &script_system_id);

	// Batch helpers for regular systems
	void pause_systems(const RID &world_id, const PackedInt64Array &system_ids);
	void resume_systems(const RID &world_id, const PackedInt64Array &system_ids);
	void pause_all_systems(const RID &world_id);
	void resume_all_systems(const RID &world_id);

	// Convenience action (UI button) to reset script system instrumentation
	void reset_script_system_instrumentation_action(const RID &world_id, const RID &script_system_id) { reset_script_system_instrumentation(world_id, script_system_id); }


	void set_script_system_name(const RID &world_id, const RID &script_system_id, const String &name);
	String get_script_system_name(const RID &world_id, const RID &script_system_id);

	// ===== Query API (high-performance variant) =====
	// Create a new query for manual entity iteration
	RID create_query(const RID &world_id, const PackedStringArray &required_components);

	// Core query operations
	Array query_get_entities(const RID &world_id, const RID &query_id);
	Array query_get_entities_with_components(const RID &world_id, const RID &query_id);
	int query_get_entity_count(const RID &world_id, const RID &query_id);

	// Batched/limited fetch
	Array query_get_entities_limited(const RID &world_id, const RID &query_id, int max_count, int offset);
	Array query_get_entities_with_components_limited(const RID &world_id, const RID &query_id, int max_count, int offset);

	// Entity matching
	bool query_matches_entity(const RID &world_id, const RID &query_id, const RID &entity_rid);

	// Configuration
	void query_set_required_components(const RID &world_id, const RID &query_id, const PackedStringArray &components);
	PackedStringArray query_get_required_components(const RID &world_id, const RID &query_id);

	void query_set_caching_strategy(const RID &world_id, const RID &query_id, int strategy); // 0=NO_CACHE, 1=CACHE_ENTITIES, 2=CACHE_FULL
	int query_get_caching_strategy(const RID &world_id, const RID &query_id);

	void query_set_filter_name_pattern(const RID &world_id, const RID &query_id, const String &pattern);
	String query_get_filter_name_pattern(const RID &world_id, const RID &query_id);
	void query_clear_filter(const RID &world_id, const RID &query_id);

	// Cache control
	void query_force_cache_refresh(const RID &world_id, const RID &query_id);
	bool query_is_cache_dirty(const RID &world_id, const RID &query_id);

	// Instrumentation
	void query_set_instrumentation_enabled(const RID &world_id, const RID &query_id, bool enabled);
	bool query_get_instrumentation_enabled(const RID &world_id, const RID &query_id);
	Dictionary query_get_instrumentation_data(const RID &world_id, const RID &query_id);
	void query_reset_instrumentation(const RID &world_id, const RID &query_id);

	// Cleanup
	void free_query(const RID &world_id, const RID &query_id);

	// Internal access
	FlecsQuery _get_query(const RID &query_id, const RID &world_id);
	RID _create_rid_for_query(const RID &world_id, const FlecsQuery &query);

private:
	struct RID_Owner_Wrapper {
		RID world_id;
		RID_Owner<FlecsEntityVariant, true> entity_owner;
		RID_Owner<FlecsTypeIDVariant, true> type_id_owner;
		RID_Owner<FlecsSystemVariant, true> system_owner;
		RID_Owner<FlecsScriptSystem, true> script_system_owner;
		RID_Owner<FlecsQuery, true> query_owner;
		HashMap<String, Ref<CommandHandler>> command_handlers;
		RID_Owner_Wrapper() = default;
		RID_Owner_Wrapper(RID world_id) : world_id(world_id),
			entity_owner(ENTITY_OWNER_CHUNK_SIZE, MAX_ENTITY_COUNT),
			type_id_owner(TYPE_ID_OWNER_CHUNK_SIZE, MAX_TYPE_ID_COUNT),
			system_owner(SYSTEM_OWNER_CHUNK_SIZE, MAX_SYSTEM_COUNT),
			script_system_owner(SCRIPT_SYSTEM_OWNER_CHUNK_SIZE, MAX_SCRIPT_SYSTEM_COUNT),
			query_owner(QUERY_OWNER_CHUNK_SIZE, MAX_QUERY_COUNT) {}
		//fun hack to get around the lack of move semantics
		// Ensure world_id is initialized before using it when rebuilding owners from another wrapper.
		RID_Owner_Wrapper(const RID_Owner_Wrapper& other) {
			// Initialize world_id first to ensure lookups use the correct world
			world_id = other.world_id;
			for (RID rid : other.entity_owner.get_owned_list()) {
					entity_owner.make_rid(FlecsEntityVariant(FlecsServer::get_singleton()->_get_entity(rid, world_id)));
			}
			LocalVector<RID> other_type_ids = other.type_id_owner.get_owned_list();

			for (RID rid : other_type_ids) {
				type_id_owner.make_rid(FlecsTypeIDVariant(FlecsServer::get_singleton()->_get_type_id(rid, world_id)));
			}

			LocalVector<RID> other_system_ids = other.system_owner.get_owned_list();

			for (RID rid : other_system_ids) {
				system_owner.make_rid(FlecsSystemVariant(FlecsServer::get_singleton()->_get_system(rid, world_id)));
			}

			LocalVector<RID> other_script_ids = other.script_system_owner.get_owned_list();
			for (RID rid : other_script_ids) {
				script_system_owner.make_rid(FlecsScriptSystem(FlecsServer::get_singleton()->_get_script_system(rid, world_id)));
			}

			LocalVector<RID> other_query_ids = other.query_owner.get_owned_list();
			for (RID rid : other_query_ids) {
				query_owner.make_rid(FlecsQuery(FlecsServer::get_singleton()->_get_query(rid, world_id)));
			}

			command_handlers = other.command_handlers;
		}
		RID_Owner_Wrapper operator=(const RID_Owner_Wrapper& other) {
			if (this != &other) {
				world_id = other.world_id;
				for(RID rid : other.type_id_owner.get_owned_list()) {
					FlecsServer::get_singleton()->free_type_id(world_id, rid);
				}
				for (RID rid : other.system_owner.get_owned_list()) {
					FlecsServer::get_singleton()->free_system(world_id, rid, true);
				}
				for (RID rid : other.script_system_owner.get_owned_list()) {
					FlecsServer::get_singleton()->free_script_system(world_id, rid);
				}

				for (RID rid : other.entity_owner.get_owned_list()) {
					entity_owner.make_rid(FlecsEntityVariant(get_singleton()->_get_entity(rid, world_id)));
				}

				for (RID rid : other.type_id_owner.get_owned_list()) {
					type_id_owner.make_rid(FlecsTypeIDVariant(get_singleton()->_get_type_id(rid, world_id)));
				}

				for (RID rid : other.system_owner.get_owned_list()) {
					system_owner.make_rid(FlecsSystemVariant(get_singleton()->_get_system(rid, world_id)));
				}

				for (RID rid : other.script_system_owner.get_owned_list()) {
					script_system_owner.make_rid(FlecsScriptSystem(get_singleton()->_get_script_system(rid, world_id)));
				}

				for (RID rid : other.query_owner.get_owned_list()) {
					query_owner.make_rid(FlecsQuery(get_singleton()->_get_query(rid, world_id)));
				}

			}
			command_handlers = other.command_handlers;
			return *this;
		}
	};

	RID_Owner<FlecsWorldVariant, true> flecs_world_owners = RID_Owner<FlecsWorldVariant, true>(WORLD_OWNER_CHUNK_SIZE, MAX_WORLD_COUNT);
	Vector<RID> worlds;
	AHashMap<RID,RID_Owner_Wrapper> flecs_variant_owners = AHashMap<RID,RID_Owner_Wrapper>(MAX_WORLD_COUNT);
	Ref<CommandHandler> render_system_command_handler;
	AHashMap<RID, PipelineManager> pipeline_managers = AHashMap<RID, PipelineManager>(MAX_WORLD_COUNT);
	Callable command_handler_callback;
	AHashMap<RID, NodeStorage*> node_storages = AHashMap<RID, NodeStorage*>(MAX_WORLD_COUNT);
	AHashMap<RID, RefStorage*> ref_storages = AHashMap<RID, RefStorage*>(MAX_WORLD_COUNT);
	AHashMap<RID, Dictionary> last_frame_summaries = AHashMap<RID, Dictionary>(MAX_WORLD_COUNT);

};

class ScriptSystemInspector : public Resource {
	GDCLASS(ScriptSystemInspector, Resource);
	friend class FlecsServer;
	RID world_id;
	RID script_system_id;
	FlecsServer *server = nullptr;
protected:
	static void _bind_methods();
public:
	void _set_context(const RID &p_world, const RID &p_sys, FlecsServer *p_server) { world_id = p_world; script_system_id = p_sys; server = p_server; }
	int get_dispatch_mode() const;
	void set_dispatch_mode(int m);
	bool get_change_only() const;
	void set_change_only(bool v);
	bool get_observe_add_and_set() const;
	void set_observe_add_and_set(bool v);
	bool get_observe_remove() const;
	void set_observe_remove(bool v);
	bool get_multi_threaded() const;
	void set_multi_threaded(bool v);
	int get_batch_chunk_size() const;
	void set_batch_chunk_size(int v);
	double get_flush_min_interval_msec() const;
	void set_flush_min_interval_msec(double v);
	bool get_instrumentation() const;
	void set_instrumentation(bool v);
	bool get_detailed_timing() const;
	void set_detailed_timing(bool v);
	int get_max_sample_count() const;
	void set_max_sample_count(int cap);
	bool get_auto_reset() const;
	void set_auto_reset(bool v);
	bool get_paused() const;
	void set_paused(bool v);
	int get_last_frame_entities() const;
	uint64_t get_last_frame_dispatch_usec() const;
	uint64_t get_frame_invocations() const;
	uint64_t get_frame_accum_usec() const;
	uint64_t get_frame_min_usec() const;
	uint64_t get_frame_max_usec() const;
	double get_frame_median_usec() const;
	double get_frame_p99_usec() const;
	double get_frame_stddev_usec() const;
	uint64_t get_last_frame_onadd() const;
	uint64_t get_last_frame_onset() const;
	uint64_t get_last_frame_onremove() const;
	uint64_t get_total_callbacks() const;
	uint64_t get_total_entities_processed() const;
};
