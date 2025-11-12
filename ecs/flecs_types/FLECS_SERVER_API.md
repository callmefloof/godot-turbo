# FlecsServer API Documentation

> **Central server managing Flecs ECS worlds, entities, and systems for Godot**

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [World Management](#world-management)
- [Entity Operations](#entity-operations)
- [Component Operations](#component-operations)
- [System Management](#system-management)
- [Query System](#query-system)
- [Parent-Child Relationships](#parent-child-relationships)
- [Storage Systems](#storage-systems)
- [Performance & Instrumentation](#performance--instrumentation)
- [Thread Safety](#thread-safety)
- [Best Practices](#best-practices)
- [GDScript Examples](#gdscript-examples)

---

## Overview

**FlecsServer** is a singleton Godot Object that bridges the Flecs ECS library with GDScript, providing thread-safe access to multiple ECS worlds, entities, components, and systems.

### Key Features

- **Multiple Worlds**: Create and manage independent ECS worlds
- **RID-Based API**: All ECS objects are accessed via Godot RIDs for safety
- **Thread-Safe**: Mutex-protected operations for multi-threaded access
- **GDScript Integration**: Full API exposure to GDScript
- **Resource Management**: Automatic cleanup via RID_Owner
- **Query System**: High-performance entity queries with caching
- **Script Systems**: Callback-based systems with instrumentation
- **Component Reflection**: Dynamic component registration and serialization

### Design Patterns

- **Singleton**: Global access via `FlecsServer.get_singleton()`
- **Handle-Based**: All objects referenced by RID, not raw pointers
- **Validation Macros**: Comprehensive error checking for all operations
- **RID_Owner**: Godot's resource management for ECS objects

---

## Architecture

### Class Hierarchy

```
RefCounted (Godot)
  └── FlecsServer (Singleton)
       ├── RID_Owner<FlecsWorldVariant> (world storage)
       └── Per-World RID_Owner_Wrapper
            ├── RID_Owner<FlecsEntityVariant> (entities)
            ├── RID_Owner<FlecsSystemVariant> (systems)
            ├── RID_Owner<FlecsScriptSystem> (script systems)
            ├── RID_Owner<FlecsQuery> (queries)
            ├── RID_Owner<FlecsTypeIDVariant> (component types)
            ├── HashMap<String, CommandHandler> (command handlers)
            ├── NodeStorage (Godot Node references)
            └── RefStorage (Godot Resource references)
```

### Data Structures

```cpp
struct RID_Owner_Wrapper {
    RID world_id;                                    // Parent world
    RID_Owner<FlecsEntityVariant> entity_owner;      // Entities in this world
    RID_Owner<FlecsTypeIDVariant> type_id_owner;     // Component types
    RID_Owner<FlecsSystemVariant> system_owner;      // Native systems
    RID_Owner<FlecsScriptSystem> script_system_owner;// GDScript systems
    RID_Owner<FlecsQuery> query_owner;               // Queries
    HashMap<String, CommandHandler> command_handlers; // Custom handlers
};
```

### Memory Management

- **RID_Owner**: Godot's chunk-based allocator with configurable sizes
- **World Isolation**: Each world has independent RID spaces
- **Automatic Cleanup**: RIDs freed when owner destroyed
- **Thread-Safe**: Mutex protects all RID operations

---

## World Management

### Creating Worlds

```gdscript
# Create a new ECS world
var world_rid = FlecsServer.create_world()

# Initialize with default components
FlecsServer.init_world(world_rid)

# Progress the world (run systems)
var delta = 0.016  # 60 FPS
FlecsServer.progress_world(world_rid, delta)
```

### World Lifecycle

```cpp
RID create_world()              // Create new world
void init_world(RID world_id)   // Register default components
bool progress_world(RID, float) // Step simulation
void free_world(RID world_id)   // Destroy world and all contents
int8_t get_world_count()        // Count active worlds
```

### World Properties

```gdscript
# Get world from entity
var world_rid = FlecsServer.get_world_of_entity(entity_rid)

# Set logging level (0=none, 1=errors, 2=warnings, 3=info, 4=debug)
FlecsServer.set_log_level(2)
```

### Internal Access

```cpp
flecs::world* _get_world(RID world_id)  // Get raw world pointer (internal)
RID _get_rid_for_world(RID world_id)    // Validate world RID (internal)
```

---

## Entity Operations

### Creating Entities

```gdscript
# Create anonymous entity
var entity_rid = FlecsServer.create_entity(world_rid)

# Create named entity
var player_rid = FlecsServer.create_entity_with_name(world_rid, "Player")

# Create with components
var comp_ids = [transform_comp_id, velocity_comp_id]
var entity_rid = FlecsServer.create_entity_with_name_and_comps(
    world_rid, "Bullet", comp_ids
)

# Lookup entity by name
var player_rid = FlecsServer.lookup(world_rid, "Player")
```

### Entity Names

```gdscript
# Set entity name
FlecsServer.set_entity_name(entity_rid, "Player1")

# Get entity name
var name = FlecsServer.get_entity_name(entity_rid)
```

### Entity Lifecycle

```cpp
RID create_entity(RID world_id)
RID create_entity_with_name(RID world_id, String name)
RID create_entity_with_name_and_comps(RID world_id, String name, TypedArray<RID> comp_ids)
RID lookup(RID world_id, String name)
void free_entity(RID entity_id)
```

### Internal Entity Operations

```cpp
flecs::entity _get_entity(RID entity_id)
RID _create_rid_for_entity(RID world_id, flecs::entity entity)
RID _get_or_create_rid_for_entity(RID world_id, flecs::entity entity)
```

---

## Component Operations

### Component Registration

```gdscript
# Register component type dynamically
var comp_schema = {
    "x": TYPE_FLOAT,
    "y": TYPE_FLOAT,
    "z": TYPE_FLOAT
}
var position_type_id = FlecsServer.register_component_type(
    world_rid, "Position", comp_schema
)

# Get component type by name
var type_id = FlecsServer.get_component_type_by_name(world_rid, "Position")
```

### Adding Components

```gdscript
# Add component to entity (empty)
FlecsServer.add_component(entity_rid, position_type_id)

# Set component with data
var comp_data = {"x": 10.0, "y": 20.0, "z": 5.0}
FlecsServer.set_component(entity_rid, "Position", comp_data)
```

### Getting Components

```gdscript
# Get component by name
var position = FlecsServer.get_component_by_name(entity_rid, "Position")
print(position["x"], position["y"], position["z"])

# Get component by type ID
var position = FlecsServer.get_component_by_id(entity_rid, position_type_id)

# Check if entity has component
if FlecsServer.has_component(entity_rid, "Position"):
    print("Entity has Position")
```

### Removing Components

```gdscript
# Remove component by name
FlecsServer.remove_component_from_entity_with_name(entity_rid, "Position")

# Remove component by type ID
FlecsServer.remove_component_from_entity_with_id(entity_rid, position_type_id)

# Remove all components
FlecsServer.remove_all_components_from_entity(entity_rid)
```

### Querying Components

```gdscript
# Get all component types on entity (as names)
var comp_names = FlecsServer.get_component_types_as_name(entity_rid)
for comp_name in comp_names:
    print("Has component: ", comp_name)

# Get all component type IDs
var comp_ids = FlecsServer.get_component_types_as_id(entity_rid)
```

### Component API Reference

```cpp
RID register_component_type(RID world_id, String type_name, Dictionary schema)
RID get_component_type_by_name(RID world_id, String name)
void add_component(RID entity_id, RID comp_type_id)
void set_component(RID entity_id, String comp_name, Dictionary data)
Dictionary get_component_by_name(RID entity_id, String comp_name)
Dictionary get_component_by_id(RID entity_id, RID comp_type_id)
void remove_component_from_entity_with_name(RID entity_id, String comp_name)
void remove_component_from_entity_with_id(RID entity_id, RID comp_type_id)
void remove_all_components_from_entity(RID entity_id)
bool has_component(RID entity_id, String comp_name)
PackedStringArray get_component_types_as_name(RID entity_id)
TypedArray<RID> get_component_types_as_id(RID entity_id)
```

---

## System Management

### Script Systems

```gdscript
# Create script system
var system_rid = FlecsServer.add_script_system(
    world_rid,
    PackedStringArray(["Position", "Velocity"]),
    update_movement
)

func update_movement(entities: Array):
    for entity_data in entities:
        var rid = entity_data["rid"]
        var pos = entity_data["components"]["Position"]
        var vel = entity_data["components"]["Velocity"]
        # Update logic...
```

### System Configuration

```gdscript
# Set dispatch mode (0=per-entity, 1=batch)
FlecsServer.set_script_system_dispatch_mode(system_rid, 1)

# Enable change-only mode (react to changes, not every frame)
FlecsServer.set_script_system_change_only(system_rid, true)

# Enable multi-threading
FlecsServer.set_script_system_multi_threaded(system_rid, true)

# Set batch size
FlecsServer.set_script_system_batch_chunk_size(system_rid, 100)

# Pause/resume
FlecsServer.set_script_system_paused(system_rid, true)
FlecsServer.set_script_system_paused(system_rid, false)
```

### System Instrumentation

```gdscript
# Enable instrumentation
FlecsServer.set_script_system_instrumentation(system_rid, true)

# Get performance data
var stats = FlecsServer.get_script_system_instrumentation(system_rid)
print("Entities processed: ", stats["total_entities_processed"])
print("Average time: ", stats["frame_median_usec"], " µs")

# Reset counters
FlecsServer.reset_script_system_instrumentation(system_rid)
```

### System Queries

```gdscript
# Get all systems
var all_systems = FlecsServer.get_all_systems(world_rid)
for sys_rid in all_systems:
    var info = FlecsServer.get_script_system_info(sys_rid)
    print("System: ", info["name"])

# Get system info
var info = FlecsServer.get_script_system_info(system_rid)
print("Components: ", info["required_components"])
print("Paused: ", info["is_paused"])
```

### System API Reference

```cpp
// Creation & Lifecycle
RID add_script_system(RID world_id, Array comp_types, Callable callback)
void free_script_system(RID system_id)
void set_script_system_name(RID system_id, String name)
String get_script_system_name(RID system_id)

// Configuration
void set_script_system_callback(RID system_id, Callable callback)
Callable get_script_system_callback(RID system_id)
void set_script_system_required_components(RID system_id, PackedStringArray comps)
PackedStringArray get_script_system_required_components(RID system_id)
void set_script_system_world(RID system_id, RID world_id)
RID get_script_system_world(RID system_id)

// Dispatch Mode
void set_script_system_dispatch_mode(RID system_id, int mode)
int get_script_system_dispatch_mode(RID system_id)

// Change-Only Mode
void set_script_system_change_only(RID system_id, bool enabled)
bool is_script_system_change_only(RID system_id)
void set_script_system_change_observe_add_and_set(RID system_id, bool both)
bool get_script_system_change_observe_add_and_set(RID system_id)
void set_script_system_change_observe_remove(RID system_id, bool enabled)
bool get_script_system_change_observe_remove(RID system_id)

// Performance
void set_script_system_multi_threaded(RID system_id, bool enabled)
bool get_script_system_multi_threaded(RID system_id)
void set_script_system_batch_chunk_size(RID system_id, int size)
int get_script_system_batch_chunk_size(RID system_id)
void set_script_system_flush_min_interval_msec(RID system_id, double ms)
double get_script_system_flush_min_interval_msec(RID system_id)
void set_script_system_use_deferred_calls(RID system_id, bool deferred)
bool get_script_system_use_deferred_calls(RID system_id)

// Instrumentation
void set_script_system_instrumentation(RID system_id, bool enabled)
Dictionary get_script_system_instrumentation(RID system_id)
void reset_script_system_instrumentation(RID system_id)
void set_script_system_detailed_timing(RID system_id, bool enabled)
bool get_script_system_detailed_timing(RID system_id)
void set_script_system_auto_reset(RID system_id, bool enabled)
bool get_script_system_auto_reset(RID system_id)
void set_script_system_max_sample_count(RID system_id, int count)
int get_script_system_max_sample_count(RID system_id)

// Metrics (getters)
int get_script_system_last_frame_entity_count(RID system_id)
uint64_t get_script_system_last_frame_dispatch_usec(RID system_id)
uint64_t get_script_system_frame_dispatch_invocations(RID system_id)
uint64_t get_script_system_frame_dispatch_accum_usec(RID system_id)
uint64_t get_script_system_frame_min_usec(RID system_id)
uint64_t get_script_system_frame_max_usec(RID system_id)
double get_script_system_frame_median_usec(RID system_id)
double get_script_system_frame_percentile_usec(RID system_id, double percentile)
double get_script_system_frame_stddev_usec(RID system_id)
double get_script_system_frame_p99_usec(RID system_id)
uint64_t get_script_system_last_frame_onadd(RID system_id)
uint64_t get_script_system_last_frame_onset(RID system_id)
uint64_t get_script_system_last_frame_onremove(RID system_id)
uint64_t get_script_system_total_callbacks(RID system_id)
uint64_t get_script_system_total_entities_processed(RID system_id)
Dictionary get_script_system_event_totals(RID system_id)

// Control
void set_script_system_paused(RID system_id, bool paused)
bool is_script_system_paused(RID system_id)
void set_script_system_dependency(RID system_id, uint32_t depends_on_id)

// Inspection
Dictionary get_script_system_info(RID system_id)
Ref<Resource> make_script_system_inspector(RID system_id)

// Batch Operations
void pause_systems(RID world_id)
void resume_systems(RID world_id)
void pause_all_systems(RID world_id)
void resume_all_systems(RID world_id)
```

---

## Query System

### Creating Queries

```gdscript
# Create query
var query_rid = FlecsServer.create_query(
    world_rid,
    PackedStringArray(["Position", "Velocity"])
)

# Get matching entities
var entities = FlecsServer.query_get_entities(query_rid)
for entity_rid in entities:
    # Process entity...

# Get entities with component data
var results = FlecsServer.query_get_entities_with_components(query_rid)
for result in results:
    var entity_rid = result["rid"]
    var pos = result["components"]["Position"]
    var vel = result["components"]["Velocity"]
```

### Query Configuration

```gdscript
# Set required components
FlecsServer.query_set_required_components(
    query_rid,
    PackedStringArray(["Transform", "Mesh"])
)

# Set caching strategy (0=none, 1=entities, 2=full)
FlecsServer.query_set_caching_strategy(query_rid, 1)

# Set name filter
FlecsServer.query_set_filter_name_pattern(query_rid, "Player*")

# Enable instrumentation
FlecsServer.query_set_instrumentation_enabled(query_rid, true)
```

### Query Operations

```gdscript
# Get entity count (fast, no fetch)
var count = FlecsServer.query_get_entity_count(query_rid)

# Check if entity matches query
if FlecsServer.query_matches_entity(query_rid, entity_rid):
    print("Entity matches query")

# Limited fetch (pagination)
var page_size = 100
var offset = 0
var page = FlecsServer.query_get_entities_limited(query_rid, page_size, offset)
```

### Query Cache Control

```gdscript
# Force cache refresh
FlecsServer.query_force_cache_refresh(query_rid)

# Check if cache is dirty
if FlecsServer.query_is_cache_dirty(query_rid):
    print("Cache needs refresh")

# Clear filters
FlecsServer.query_clear_filter(query_rid)
```

### Query Instrumentation

```gdscript
# Get performance metrics
var stats = FlecsServer.query_get_instrumentation_data(query_rid)
print("Total fetches: ", stats["total_fetches"])
print("Cache hits: ", stats["cache_hits"])

# Reset instrumentation
FlecsServer.query_reset_instrumentation(query_rid)
```

### Query API Reference

```cpp
// Creation & Lifecycle
RID create_query(RID world_id, PackedStringArray required_comps)
void free_query(RID query_id)

// Entity Fetching
Array query_get_entities(RID query_id)
Array query_get_entities_with_components(RID query_id)
int query_get_entity_count(RID query_id)
Array query_get_entities_limited(RID query_id, int max_count, int offset)
Array query_get_entities_with_components_limited(RID query_id, int max_count, int offset)
bool query_matches_entity(RID query_id, RID entity_id)

// Configuration
void query_set_required_components(RID query_id, PackedStringArray comps)
PackedStringArray query_get_required_components(RID query_id)
void query_set_caching_strategy(RID query_id, int strategy)
int query_get_caching_strategy(RID query_id)
void query_set_filter_name_pattern(RID query_id, String pattern)
String query_get_filter_name_pattern(RID query_id)
void query_clear_filter(RID query_id)

// Cache Control
void query_force_cache_refresh(RID query_id)
bool query_is_cache_dirty(RID query_id)

// Instrumentation
void query_set_instrumentation_enabled(RID query_id, bool enabled)
bool query_get_instrumentation_enabled(RID query_id)
Dictionary query_get_instrumentation_data(RID query_id)
void query_reset_instrumentation(RID query_id)
```

---

## Parent-Child Relationships

### Hierarchy Operations

```gdscript
# Set parent
FlecsServer.set_parent(child_rid, parent_rid)

# Get parent
var parent_rid = FlecsServer.get_parent(child_rid)

# Add child
FlecsServer.add_child(parent_rid, child_rid)

# Remove child
FlecsServer.remove_child(parent_rid, child_rid)

# Get all children
var children = FlecsServer.get_children(parent_rid)
for child_rid in children:
    print("Child: ", FlecsServer.get_entity_name(child_rid))

# Get child by index
var first_child = FlecsServer.get_child(parent_rid, 0)

# Get child by name
var weapon = FlecsServer.get_child_by_name(parent_rid, "Weapon")

# Remove child by name
FlecsServer.remove_child_by_name(parent_rid, "Shield")

# Remove child by index
FlecsServer.remove_child_by_index(parent_rid, 2)

# Remove all children
FlecsServer.remove_all_children(parent_rid)

# Set all children at once
FlecsServer.set_children(parent_rid, [child1, child2, child3])
```

### Generic Relationships

```gdscript
# Add relationship (entity -> target with relation)
FlecsServer.add_relationship(entity_rid, relation_rid, target_rid)

# Remove relationship
FlecsServer.remove_relationship(entity_rid, relation_rid, target_rid)

# Get relationship target
var target_rid = FlecsServer.get_relationship(entity_rid, relation_rid)

# Get all relationships
var rels = FlecsServer.get_relationships(entity_rid)
```

### Hierarchy API Reference

```cpp
// Parent-Child
RID get_parent(RID entity_id)
void set_parent(RID entity_id, RID parent_id)
void add_child(RID parent_id, RID child_id)
void remove_child(RID parent_id, RID child_id)
TypedArray<RID> get_children(RID parent_id)
RID get_child(RID parent_id, int index)
void set_children(RID parent_id, TypedArray<RID> children)
RID get_child_by_name(RID parent_id, String name)
void remove_child_by_name(RID parent_id, String name)
void remove_child_by_index(RID parent_id, int index)
void remove_all_children(RID parent_id)

// Generic Relationships
void add_relationship(RID entity_id, RID relation_id, RID target_id)
void remove_relationship(RID entity_id, RID relation_id, RID target_id)
RID get_relationship(RID entity_id, RID relation_id)
TypedArray<RID> get_relationships(RID entity_id)
```

---

## Storage Systems

### Node Storage

```gdscript
# Associate Godot Node with entity
FlecsServer.add_to_node_storage(entity_rid, node)

# Retrieve Node
var node = FlecsServer.get_node_from_node_storage(entity_rid)

# Remove Node reference
FlecsServer.remove_from_node_storage(entity_rid)
```

### Resource Storage

```gdscript
# Associate Resource with entity
FlecsServer.add_to_ref_storage(entity_rid, resource)

# Retrieve Resource
var resource = FlecsServer.get_resource_from_ref_storage(entity_rid)

# Remove Resource reference
FlecsServer.remove_from_ref_storage(entity_rid)
```

### Storage API

```cpp
// Node Storage
void add_to_node_storage(RID entity_id, Node* node)
void remove_from_node_storage(RID entity_id)
Node* get_node_from_node_storage(RID entity_id)

// Resource Storage
void add_to_ref_storage(RID entity_id, Ref<Resource> resource)
void remove_from_ref_storage(RID entity_id)
Ref<Resource> get_resource_from_ref_storage(RID entity_id)
```

---

## Performance & Instrumentation

### World Frame Summary

```gdscript
# Get frame performance summary
var summary = FlecsServer.get_world_frame_summary(world_rid)
print("Total systems: ", summary["system_count"])
print("Total entities: ", summary["entity_count"])
print("Frame time: ", summary["frame_time_usec"], " µs")

# Reset summary
FlecsServer.reset_world_frame_summary(world_rid)
```

### Distribution Summary

```gdscript
# Get workload distribution across systems
var dist = FlecsServer.get_world_distribution_summary(world_rid)
for system_name in dist:
    print(system_name, ": ", dist[system_name]["percentage"], "%")
```

### Performance API

```cpp
Dictionary get_world_frame_summary(RID world_id)
void reset_world_frame_summary(RID world_id)
Dictionary get_world_distribution_summary(RID world_id)
```

---

## Thread Safety

### Locking Mechanism

```cpp
// Manual locking (advanced use)
FlecsServer::get_singleton()->lock();
// ... critical section ...
FlecsServer::get_singleton()->unlock();
```

### Thread-Safe Operations

All public API methods are thread-safe and internally use mutex protection. No manual locking required for normal use.

### Validation Macros

```cpp
CHECK_WORLD_VALIDITY_V(world_id, return_value, method_name)
CHECK_ENTITY_VALIDITY_V(entity_id, return_value, method_name)
CHECK_SCRIPT_SYSTEM_VALIDITY_V(system_id, return_value, method_name)
CHECK_QUERY_VALIDITY_V(query_id, return_value, method_name)
```

These macros validate RIDs and print errors if invalid.

---

## Best Practices

### 1. World Management

```gdscript
# Good: One world per scene
var game_world = FlecsServer.create_world()
FlecsServer.init_world(game_world)

# Good: Cleanup on scene exit
func _exit_tree():
    FlecsServer.free_world(game_world)
```

### 2. Component Design

```gdscript
# Good: Small, focused components
var position_schema = {"x": TYPE_FLOAT, "y": TYPE_FLOAT}
var health_schema = {"current": TYPE_INT, "max": TYPE_INT}

# Avoid: Large, monolithic components
# Bad: {"position_x": ..., "velocity_x": ..., "health": ..., "name": ...}
```

### 3. System Performance

```gdscript
# Good: Batch mode for many entities
FlecsServer.set_script_system_dispatch_mode(system_rid, 1)  # BATCH
FlecsServer.set_script_system_batch_chunk_size(system_rid, 100)

# Good: Change-only for reactive systems
FlecsServer.set_script_system_change_only(health_system, true)

# Good: Multi-threading for heavy computation
FlecsServer.set_script_system_multi_threaded(physics_system, true)
FlecsServer.set_script_system_use_deferred_calls(physics_system, true)
```

### 4. Query Optimization

```gdscript
# Good: Cache entities list for stable queries
FlecsServer.query_set_caching_strategy(query_rid, 1)  # CACHE_ENTITIES

# Good: Use limited fetch for large result sets
var page = FlecsServer.query_get_entities_limited(query_rid, 100, 0)
```

### 5. Error Handling

```gdscript
# Good: Check validity
if FlecsServer.has_component(entity_rid, "Health"):
    var health = FlecsServer.get_component_by_name(entity_rid, "Health")

# Good: Validate RIDs before use
if entity_rid.is_valid():
    # ... use entity ...
```

---

## GDScript Examples

### Complete Game Loop

```gdscript
extends Node

var world_rid: RID
var player_rid: RID
var movement_system: RID
var render_system: RID

func _ready():
    # Create world
    world_rid = FlecsServer.create_world()
    FlecsServer.init_world(world_rid)
    
    # Register components
    var pos_type = FlecsServer.register_component_type(world_rid, "Position", {
        "x": TYPE_FLOAT, "y": TYPE_FLOAT
    })
    var vel_type = FlecsServer.register_component_type(world_rid, "Velocity", {
        "x": TYPE_FLOAT, "y": TYPE_FLOAT
    })
    
    # Create player
    player_rid = FlecsServer.create_entity_with_name(world_rid, "Player")
    FlecsServer.set_component(player_rid, "Position", {"x": 0.0, "y": 0.0})
    FlecsServer.set_component(player_rid, "Velocity", {"x": 100.0, "y": 0.0})
    
    # Create movement system
    movement_system = FlecsServer.add_script_system(
        world_rid,
        PackedStringArray(["Position", "Velocity"]),
        update_movement
    )
    FlecsServer.set_script_system_dispatch_mode(movement_system, 1)  # Batch mode
    
    # Create render system
    render_system = FlecsServer.add_script_system(
        world_rid,
        PackedStringArray(["Position", "Sprite"]),
        update_sprites
    )

func _process(delta):
    FlecsServer.progress_world(world_rid, delta)

func update_movement(entities: Array):
    for entity_data in entities:
        var pos = entity_data["components"]["Position"]
        var vel = entity_data["components"]["Velocity"]
        
        # Update position
        pos["x"] += vel["x"] * get_process_delta_time()
        pos["y"] += vel["y"] * get_process_delta_time()
        
        # Write back
        FlecsServer.set_component(entity_data["rid"], "Position", pos)

func update_sprites(entities: Array):
    for entity_data in entities:
        var pos = entity_data["components"]["Position"]
        var sprite = entity_data["components"]["Sprite"]
        
        # Update sprite position from ECS
        var node = FlecsServer.get_node_from_node_storage(entity_data["rid"])
        if node:
            node.position = Vector2(pos["x"], pos["y"])

func _exit_tree():
    FlecsServer.free_world(world_rid)
```

### Performance Monitoring

```gdscript
extends Control

var world_rid: RID
var system_rid: RID

func _ready():
    world_rid = FlecsServer.create_world()
    system_rid = # ... create system
    
    # Enable instrumentation
    FlecsServer.set_script_system_instrumentation(system_rid, true)
    FlecsServer.set_script_system_detailed_timing(system_rid, true)

func _process(_delta):
    # Update performance display
    var stats = FlecsServer.get_script_system_instrumentation(system_rid)
    
    $EntityCount.text = "Entities: %d" % stats["last_frame_entity_count"]
    $CallbackTime.text = "Time: %.2f µs" % stats["frame_median_usec"]
    $TotalProcessed.text = "Total: %d" % stats["total_entities_processed"]
    
    # Show P99 latency
    var p99 = FlecsServer.get_script_system_frame_p99_usec(system_rid)
    $P99Latency.text = "P99: %.2f µs" % p99
```

---

## Validation Macros Reference

```cpp
// These macros are used internally for RID validation

CHECK_WORLD_VALIDITY_V(world_id, return_value, method_name)
CHECK_ENTITY_VALIDITY_V(entity_id, return_value, method_name)
CHECK_SYSTEM_VALIDITY_V(system_id, return_value, method_name)
CHECK_SCRIPT_SYSTEM_VALIDITY_V(system_id, return_value, method_name)
CHECK_QUERY_VALIDITY_V(query_id, return_value, method_name)
CHECK_TYPE_ID_VALIDITY_V(type_id, return_value, method_name)

// Non-void variants (for methods with no return value)
CHECK_WORLD_VALIDITY(world_id, method_name)
CHECK_ENTITY_VALIDITY(entity_id, method_name)
CHECK_SYSTEM_VALIDITY(system_id, method_name)
CHECK_SCRIPT_SYSTEM_VALIDITY(system_id, method_name)
CHECK_QUERY_VALIDITY(query_id, method_name)
CHECK_TYPE_ID_VALIDITY(type_id, method_name)
```

---

## Constants & Limits

```cpp
// Maximum object counts
static const int MAX_WORLD_COUNT = 32;
static const int MAX_ENTITY_COUNT = 524288;  // 512K entities per world
static const int MAX_COMPONENT_COUNT = 8192;
static const int MAX_SYSTEM_COUNT = 1024;
static const int MAX_SCRIPT_SYSTEM_COUNT = 1024;
static const int MAX_TYPE_ID_COUNT = 8192;
static const int MAX_COMMAND_HANDLER_COUNT = 256;
static const int MAX_QUERY_COUNT = 1024;

// RID_Owner chunk sizes (memory allocation granularity)
static const int WORLD_OWNER_CHUNK_SIZE = 8;
static const int ENTITY_OWNER_CHUNK_SIZE = 1024;
static const int COMPONENT_OWNER_CHUNK_SIZE = 256;
static const int SYSTEM_OWNER_CHUNK_SIZE = 64;
static const int SCRIPT_SYSTEM_OWNER_CHUNK_SIZE = 64;
static const int TYPE_ID_OWNER_CHUNK_SIZE = 256;
static const int COMMAND_HANDLER_OWNER_CHUNK_SIZE = 32;
static const int QUERY_OWNER_CHUNK_SIZE = 64;
```

---

## Error Handling

All API methods validate inputs and print errors via Godot's error macros:

```cpp
ERR_PRINT("FlecsServer::method_name: Invalid world_id");
ERR_FAIL_V_MSG(default_value, "Error message");
```

In GDScript, check the console for error messages when operations fail.

---

## Internal Methods (Advanced)

Methods prefixed with `_` are for internal use or advanced scenarios:

```cpp
flecs::world* _get_world(RID world_id)
flecs::entity _get_entity(RID entity_id)
flecs::system _get_system(RID system_id)
flecs::entity_t _get_type_id(RID type_id)
FlecsScriptSystem _get_script_system(RID system_id)
FlecsQuery _get_query(RID query_id)

RID _create_rid_for_entity(RID world_id, flecs::entity entity)
RID _create_rid_for_system(RID world_id, flecs::system system)
RID _create_rid_for_type_id(RID world_id, flecs::entity_t type)
RID _create_rid_for_script_system(RID world_id, FlecsScriptSystem system)
RID _create_rid_for_query(RID world_id, FlecsQuery query)
RID _get_or_create_rid_for_entity(RID world_id, flecs::entity entity)
RID _get_rid_for_world(RID world_id)
```

**Warning**: These bypass validation and should only be used if you know what you're doing!

---

**Documentation Version**: 1.0  
**Last Updated**: 2025  
**Flecs Version**: Latest  
**Godot Version**: 4.x