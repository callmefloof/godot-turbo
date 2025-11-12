# Flecs Types Documentation

This document provides comprehensive documentation for the core Flecs type wrappers and ECS management classes in the godot_turbo module.

## Table of Contents

1. [Overview](#overview)
2. [Variant Wrappers](#variant-wrappers)
3. [FlecsServer](#flecsserver)
4. [FlecsQuery](#flecsquery)
5. [FlecsScriptSystem](#flecsscriptsystem)
6. [Usage Examples](#usage-examples)
7. [Best Practices](#best-practices)
8. [Performance Considerations](#performance-considerations)

---

## Overview

The `flecs_types` module provides the bridge between Godot's RID-based resource management and the Flecs ECS library. These types enable:

- **Seamless integration** between Godot and Flecs
- **GDScript accessibility** to ECS functionality
- **Memory safety** through RID lifecycle management
- **High performance** through efficient data structures

### Core Components

| Type | Purpose | GDScript Accessible |
|------|---------|---------------------|
| `FlecsWorldVariant` | Wraps `flecs::world` for RID storage | No (internal) |
| `FlecsEntityVariant` | Wraps `flecs::entity` for RID storage | No (internal) |
| `FlecsSystemVariant` | Wraps `flecs::system` for RID storage | No (internal) |
| `FlecsTypeIDVariant` | Wraps component type IDs for RID storage | No (internal) |
| `FlecsServer` | Central ECS management singleton | Yes |
| `FlecsQuery` | High-performance entity queries | Yes (via FlecsServer) |
| `FlecsScriptSystem` | GDScript-accessible ECS systems | Yes (via FlecsServer) |

---

## Variant Wrappers

### FlecsWorldVariant

**File**: `flecs_variant.h`

Encapsulates a Flecs world instance for use with Godot's RID system.

#### Key Features
- Supports multiple independent worlds
- Proper copy/move semantics
- Shared world handles (multiple variants can reference same world)

#### C++ API

```cpp
// Construction
FlecsWorldVariant world_var;                         // Create new world
FlecsWorldVariant world_var(flecs::world{});        // From new world
FlecsWorldVariant world_var(std::move(world));      // Move from existing
FlecsWorldVariant world_var(existing_world);        // Copy (shared handle)

// Access
flecs::world& world = world_var.get_world();
```

#### Usage Example

```cpp
// Create isolated world for a specific scene
FlecsWorldVariant scene_world;
flecs::world& world = scene_world.get_world();

// Register components
world.component<Transform>();
world.component<Velocity>();

// Create entities
auto player = world.entity("Player")
    .set<Transform>({Vector3::ZERO})
    .set<Velocity>({Vector3(1, 0, 0)});
```

---

### FlecsEntityVariant

**File**: `flecs_variant.h`

Encapsulates a Flecs entity handle for RID storage.

#### Key Features
- Lightweight (just an entity ID)
- Validity checking
- Copy-friendly (entities are handles, not heavy objects)

#### C++ API

```cpp
// Construction
FlecsEntityVariant entity_var(entity);

// Access
flecs::entity entity = entity_var.get_entity();

// Validation
bool valid = entity_var.is_valid();  // Check if entity exists and is alive
```

#### Important Notes

⚠️ **Always check validity** before using entities from external storage:

```cpp
FlecsEntityVariant* var = entity_owner.get_or_null(entity_rid);
if (var && var->is_valid()) {
    flecs::entity e = var->get_entity();
    // Safe to use entity
}
```

---

### FlecsSystemVariant

**File**: `flecs_variant.h`

Encapsulates a Flecs system handle for RID storage.

#### Key Features
- References registered ECS systems
- Validity checking
- Compatible with Flecs pipeline phases

#### C++ API

```cpp
// Construction
FlecsSystemVariant sys_var(system);

// Access
flecs::system sys = sys_var.get_system();

// Validation
bool valid = sys_var.is_valid();
```

#### Usage Example

```cpp
// Create system
flecs::system movement_sys = world.system<Transform, Velocity>()
    .each([](Transform& t, Velocity& v) {
        t.position += v.direction * delta;
    });

// Store in variant
FlecsSystemVariant sys_var(movement_sys);

// Systems run automatically during world.progress()
```

---

### FlecsTypeIDVariant

**File**: `flecs_variant.h`

Encapsulates component type IDs for dynamic type operations.

#### Key Features
- Stores `flecs::entity_t` type identifiers
- Enables runtime component lookup
- Zero value indicates invalid type

#### C++ API

```cpp
// Get type ID
flecs::entity_t type_id = world.component<Transform>().id();
FlecsTypeIDVariant type_var(type_id);

// Access
flecs::entity_t id = type_var.get_type();

// Validation
bool valid = type_var.is_valid();  // Returns true if type != 0

// Use type ID
entity.add(type_var.get_type());
entity.remove(type_var.get_type());
bool has = entity.has(type_var.get_type());
```

---

## FlecsServer

**File**: `flecs_server.h`

Central singleton managing all ECS operations and world lifecycle.

### Architecture

```
FlecsServer (Singleton)
├── RID_Owner<FlecsWorldVariant> - Manages worlds
└── Per-World RID_Owners
    ├── RID_Owner<FlecsEntityVariant> - Entities
    ├── RID_Owner<FlecsTypeIDVariant> - Component types
    ├── RID_Owner<FlecsSystemVariant> - Native systems
    ├── RID_Owner<FlecsScriptSystem> - Script systems
    ├── RID_Owner<FlecsQuery> - Queries
    ├── NodeStorage - Node↔Entity mapping
    └── RefStorage - Resource↔Entity mapping
```

### Key Concepts

#### RID-Based API
All ECS objects are referenced by RIDs (Resource IDs), providing:
- **Type safety**: RIDs are strongly typed
- **Fast lookups**: O(1) access via RID_Owner
- **Memory safety**: Automatic cleanup when RIDs are freed

#### Multi-World Support
Each world is an isolated ECS instance:
- Independent entity/component spaces
- Separate system pipelines
- Useful for scene isolation or parallel processing

### GDScript API

#### World Management

```gdscript
# Create world
var world: RID = FlecsServer.create_world()

# Get world count
var count: int = FlecsServer.get_world_count()

# Progress world (run systems, update state)
FlecsServer.progress_world(world, delta_time)

# Initialize world (optional - sets up pipelines)
FlecsServer.init_world(world)

# Free world (cleanup all entities, systems)
FlecsServer.free_world(world)
```

#### Entity Management

```gdscript
# Create entity
var entity: RID = FlecsServer.create_entity(world)

# Create entity with name
var player: RID = FlecsServer.create_entity_with_name(world, "Player")

# Create entity with name and components
var enemy: RID = FlecsServer.create_entity_with_name_and_comps(
    world,
    "Enemy",
    PackedStringArray(["Transform", "Health"]),
    [
        {"position": Vector3.ZERO},
        {"value": 100}
    ]
)

# Lookup entity by name
var found: RID = FlecsServer.lookup(world, "Player")

# Get/set entity name
var name: String = FlecsServer.get_entity_name(world, entity)
FlecsServer.set_entity_name(world, entity, "NewName")

# Free entity
FlecsServer.free_entity(world, entity)
```

#### Component Management

```gdscript
# Register component type
var transform_type: RID = FlecsServer.register_component_type(world, "Transform")

# Set component (auto-registers if needed)
FlecsServer.set_component(world, entity, "Transform", {
    "position": Vector3(1, 2, 3),
    "rotation": Quaternion.IDENTITY
})

# Get component
var transform: Dictionary = FlecsServer.get_component_by_name(world, entity, "Transform")
print(transform["position"])  # Vector3(1, 2, 3)

# Check if entity has component
var has: bool = FlecsServer.has_component(world, entity, transform_type)

# Remove component
FlecsServer.remove_component_from_entity_with_name(world, entity, "Transform")

# Get all component types on entity
var type_names: PackedStringArray = FlecsServer.get_component_types_as_name(world, entity)
var type_rids: Array[RID] = FlecsServer.get_component_types_as_id(world, entity)

# Remove all components
FlecsServer.remove_all_components_from_entity(world, entity)
```

#### Hierarchy & Relationships

```gdscript
# Set parent-child relationship
FlecsServer.set_parent(world, child_entity, parent_entity)
var parent: RID = FlecsServer.get_parent(world, child_entity)

# Add/remove children
FlecsServer.add_child(world, parent_entity, child_entity)
FlecsServer.remove_child(world, parent_entity, child_entity)

# Get children
var children: Array[RID] = FlecsServer.get_children(world, parent_entity)
var child: RID = FlecsServer.get_child(world, parent_entity, 0)
var named_child: RID = FlecsServer.get_child_by_name(world, parent_entity, "ChildName")

# Set all children at once
FlecsServer.set_children(world, parent_entity, [child1, child2, child3])

# Remove children
FlecsServer.remove_child_by_name(world, parent_entity, "ChildName")
FlecsServer.remove_child_by_index(world, parent_entity, 0)
FlecsServer.remove_all_children(world, parent_entity)

# Generic relationships
FlecsServer.add_relationship(world, entity, relation_type, target_entity)
FlecsServer.remove_relationship(world, entity, relation_type, target_entity)
var target: RID = FlecsServer.get_relationship(world, entity, relation_type)
var targets: Array[RID] = FlecsServer.get_relationships(world, entity, relation_type)
```

#### Script Systems

See [FlecsScriptSystem](#flecsscriptsystem) section for detailed API.

```gdscript
# Add script system
var system: RID = FlecsServer.add_script_system(
    world,
    PackedStringArray(["Transform", "Velocity"]),
    _update_movement
)

func _update_movement(entities: Array) -> void:
    for entity_data in entities:
        var rid = entity_data["rid"]
        var transform = entity_data["components"]["Transform"]
        var velocity = entity_data["components"]["Velocity"]
        # Process entity...
```

### C++ API

#### Internal Access

```cpp
// Get FlecsServer singleton
FlecsServer* server = FlecsServer::get_singleton();

// Access raw flecs::world
flecs::world* world = server->_get_world(world_id);

// Access raw flecs::entity
flecs::entity entity = server->_get_entity(world_id, entity_id);

// Access raw flecs::system
flecs::system sys = server->_get_system(world_id, system_id);

// Access query
FlecsQuery query = server->_get_query(world_id, query_id);
```

---

## FlecsQuery

**File**: `flecs_query.h`

High-performance query variant for direct entity iteration without callbacks.

### Key Features

- **Batch fetching**: Get all matching entities at once
- **Minimal overhead**: No callback indirection
- **Flexible modes**: RID-only or with component data
- **Caching strategies**: Trade freshness for speed
- **Pagination**: Limit and offset support
- **Instrumentation**: Performance metrics
- **Filtering**: Name pattern matching

### Design Philosophy

Unlike `FlecsScriptSystem` which uses callbacks, `FlecsQuery` allows you to:
1. Build a query once with required components
2. Fetch matching entities as a batch (returns array of RIDs)
3. Manually iterate in GDScript with minimal overhead

This is ideal for performance-critical systems where callback overhead is too expensive.

### GDScript API

```gdscript
# Create query
var query: RID = FlecsServer.create_query(
    world,
    PackedStringArray(["Transform", "Velocity"])
)

# Get entity RIDs only (fastest)
var entities: Array = FlecsServer.query_get_entities(world, query)
for entity_rid in entities:
    var transform = FlecsServer.get_component_by_name(world, entity_rid, "Transform")
    # Process...

# Get entities with component data (convenience)
var entities_with_data: Array = FlecsServer.query_get_entities_with_components(world, query)
for entity_data in entities_with_data:
    var rid = entity_data["rid"]
    var components = entity_data["components"]  # Dictionary
    var transform = components["Transform"]
    # Process...

# Get entity count without fetching
var count: int = FlecsServer.query_get_entity_count(world, query)

# Pagination
var batch1: Array = FlecsServer.query_get_entities_limited(world, query, 100, 0)
var batch2: Array = FlecsServer.query_get_entities_limited(world, query, 100, 100)

# Check if specific entity matches query
var matches: bool = FlecsServer.query_matches_entity(world, query, entity_rid)

# Update query components
FlecsServer.query_set_required_components(
    world,
    query,
    PackedStringArray(["Transform", "Velocity", "Health"])
)
var components: PackedStringArray = FlecsServer.query_get_required_components(world, query)

# Free query
FlecsServer.free_query(world, query)
```

### Caching Strategies

```gdscript
# NO_CACHE (0) - Rebuild every fetch (safest, always up-to-date)
FlecsServer.query_set_caching_strategy(world, query, 0)

# CACHE_ENTITIES (1) - Cache entity list, invalidate on changes
FlecsServer.query_set_caching_strategy(world, query, 1)

# CACHE_FULL (2) - Cache entities + component data (fastest, use with caution)
FlecsServer.query_set_caching_strategy(world, query, 2)

# Check cache state
var is_dirty: bool = FlecsServer.query_is_cache_dirty(world, query)
FlecsServer.query_force_cache_refresh(world, query)
```

### Filtering

```gdscript
# Set name pattern filter (wildcard matching)
FlecsServer.query_set_filter_name_pattern(world, query, "Player*")
var pattern: String = FlecsServer.query_get_filter_name_pattern(world, query)

# Clear filter
FlecsServer.query_clear_filter(world, query)
```

### Instrumentation

```gdscript
# Enable instrumentation
FlecsServer.query_set_instrumentation_enabled(world, query, true)

# Get metrics
var data: Dictionary = FlecsServer.query_get_instrumentation_data(world, query)
print("Total fetches: ", data["total_fetches"])
print("Total entities returned: ", data["total_entities_returned"])
print("Last fetch entity count: ", data["last_fetch_entity_count"])
print("Last fetch duration (µs): ", data["last_fetch_usec"])
print("Cache hits: ", data["cache_hits"])
print("Cache misses: ", data["cache_misses"])

# Reset metrics
FlecsServer.query_reset_instrumentation(world, query)
```

### C++ API

```cpp
FlecsQuery query;

// Initialize
PackedStringArray components;
components.push_back("Transform");
components.push_back("Velocity");
query.init(world_id, components);

// Fetch modes
Array entities = query.get_entities();  // RID-only
Array full_data = query.get_entities_with_components();  // With component data

// Entity count
int count = query.get_entity_count();

// Pagination
Array batch = query.get_entities_limited(100, 0);

// Check match
bool matches = query.matches_entity(entity_rid);

// Configuration
query.set_caching_strategy(FlecsQuery::CACHE_ENTITIES);
query.set_filter_name_pattern("Player*");
query.force_cache_refresh();

// Instrumentation
query.set_instrumentation_enabled(true);
Dictionary instr = query.get_instrumentation_data();
query.reset_instrumentation();
```

---

## FlecsScriptSystem

**File**: `flecs_script_system.h`

GDScript-accessible ECS system with flexible dispatch modes and comprehensive instrumentation.

### Key Features

- **Dispatch Modes**: Per-entity or batched
- **Change Observers**: React only to component changes
- **Multi-threading**: Optional parallel processing
- **Batching Control**: Configurable chunk sizes and intervals
- **Instrumentation**: Detailed performance metrics
- **Deferred Calls**: Thread-safe GDScript invocation

### Dispatch Modes

#### Per-Entity Mode (DISPATCH_PER_ENTITY = 0)

Calls GDScript once for each matching entity.

**Pros:**
- Simple to understand
- Immediate processing

**Cons:**
- Higher GDScript call overhead
- Not suitable for thousands of entities

```gdscript
FlecsServer.set_script_system_dispatch_mode(system, 0)

func callback(entities: Array) -> void:
    # Called once per entity
    # entities.size() == 1
    var entity_data = entities[0]
```

#### Batch Mode (DISPATCH_BATCH = 1)

Accumulates entities and sends in batches.

**Pros:**
- ~10-100x fewer GDScript calls
- Suitable for large entity counts
- Configurable batch sizes

**Cons:**
- Slightly more complex processing
- Potential latency if flush interval is set

```gdscript
FlecsServer.set_script_system_dispatch_mode(system, 1)
FlecsServer.set_script_system_batch_chunk_size(system, 100)

func callback(entities: Array) -> void:
    # Called with batch of entities
    # entities.size() can be 1 to 100 (or more if chunk size = 0)
    for entity_data in entities:
        # Process each entity
```

### Change-Only Mode

Instead of running every frame, react only to component changes.

```gdscript
# Enable change-only mode
FlecsServer.set_script_system_change_only(system, true)

# Configure which events to observe
FlecsServer.set_script_system_change_observe_add_and_set(system, true)  # OnAdd + OnSet
FlecsServer.set_script_system_change_observe_remove(system, true)  # OnRemove

# System callback will only be invoked when components are added/set/removed
func callback(entities: Array) -> void:
    # Only called for entities with changed components
```

**Event Tracking:**

```gdscript
var event_totals: Dictionary = FlecsServer.get_script_system_event_totals(system)
print("OnAdd events: ", event_totals["last_frame_onadd"])
print("OnSet events: ", event_totals["last_frame_onset"])
print("OnRemove events: ", event_totals["last_frame_onremove"])
```

### Multi-Threading

```gdscript
# Enable multi-threaded execution
FlecsServer.set_script_system_multi_threaded(system, true)

# IMPORTANT: Must use deferred calls with multi-threading!
FlecsServer.set_script_system_use_deferred_calls(system, true)

# System will be scheduled across CPU cores automatically
# Batching is enabled automatically for thread safety
```

⚠️ **Thread Safety Requirements:**
- Always use deferred calls with multi-threading
- Ensure GDScript callback is thread-safe (no direct scene tree access)
- Consider using signals or queues to communicate with main thread

### Batching Configuration

```gdscript
# Batch flush chunk size
# 0 = send all entities at once
# >0 = send in chunks of this size
FlecsServer.set_script_system_batch_chunk_size(system, 100)

# Minimum flush interval (milliseconds)
# 0 = no limit (flush every frame)
# >0 = minimum time between flushes
FlecsServer.set_script_system_flush_min_interval_msec(system, 16.0)  # ~60 FPS
```

### Instrumentation

#### Basic Metrics

```gdscript
# Enable instrumentation
FlecsServer.set_script_system_instrumentation(system, true)

# Get basic metrics
var entity_count = FlecsServer.get_script_system_last_frame_entity_count(system)
var dispatch_usec = FlecsServer.get_script_system_last_frame_dispatch_usec(system)
var total_callbacks = FlecsServer.get_script_system_total_callbacks(system)
var total_entities = FlecsServer.get_script_system_total_entities_processed(system)
```

#### Detailed Timing

```gdscript
# Enable detailed timing (per-dispatch samples)
FlecsServer.set_script_system_detailed_timing(system, true)
FlecsServer.set_script_system_max_sample_count(system, 2048)

# Timing statistics
var min_usec = FlecsServer.get_script_system_frame_min_usec(system)
var max_usec = FlecsServer.get_script_system_frame_max_usec(system)
var median_usec = FlecsServer.get_script_system_frame_median_usec(system)
var p99_usec = FlecsServer.get_script_system_frame_p99_usec(system)
var stddev_usec = FlecsServer.get_script_system_frame_stddev_usec(system)

# Custom percentiles
var p95 = FlecsServer.get_script_system_frame_percentile_usec(system, 95.0)
```

#### Instrumentation Dictionary

```gdscript
var info: Dictionary = FlecsServer.get_script_system_info(system)
# Returns comprehensive system information:
# - dispatch_mode
# - change_only
# - multi_threaded
# - batch_chunk_size
# - flush_min_interval_msec
# - use_deferred_calls
# - instrumentation_enabled
# - detailed_timing_enabled
# - last_frame_entity_count
# - total_callbacks_invoked
# - total_entities_processed
# - etc.
```

### Auto-Reset

```gdscript
# Automatically reset per-frame counters each frame
FlecsServer.set_script_system_auto_reset(system, true)
```

### Pause/Resume

```gdscript
# Pause system (won't execute)
FlecsServer.set_script_system_paused(system, true)

# Resume system
FlecsServer.set_script_system_paused(system, false)

# Check pause state
var is_paused: bool = FlecsServer.is_script_system_paused(system)
```

### System Dependencies

```gdscript
# Make system2 run after system1
var system1_id = FlecsServer.get_script_system_id(system1)
FlecsServer.set_script_system_dependency(system2, system1_id)
```

### Complete Example

```gdscript
extends Node

var world: RID
var movement_system: RID

func _ready():
    var flecs = FlecsServer.get_singleton()
    
    # Create world
    world = flecs.create_world()
    
    # Add script system
    movement_system = flecs.add_script_system(
        world,
        PackedStringArray(["Transform", "Velocity"]),
        _update_movement
    )
    
    # Configure for high performance
    flecs.set_script_system_dispatch_mode(movement_system, 1)  # Batch mode
    flecs.set_script_system_batch_chunk_size(movement_system, 100)
    flecs.set_script_system_instrumentation(movement_system, true)
    flecs.set_script_system_multi_threaded(movement_system, true)
    flecs.set_script_system_use_deferred_calls(movement_system, true)
    
    # Create test entities
    for i in range(1000):
        var entity = flecs.create_entity(world)
        flecs.set_component(world, entity, "Transform", {"position": Vector3.ZERO})
        flecs.set_component(world, entity, "Velocity", {"direction": Vector3.RIGHT})

func _process(delta):
    FlecsServer.progress_world(world, delta)
    
    # Print performance metrics
    if Engine.get_frames_drawn() % 60 == 0:
        var entities = FlecsServer.get_script_system_last_frame_entity_count(movement_system)
        var usec = FlecsServer.get_script_system_last_frame_dispatch_usec(movement_system)
        print("Processed %d entities in %d µs" % [entities, usec])

func _update_movement(entities: Array) -> void:
    # This runs on worker thread with deferred calls
    # Queue updates instead of direct modification
    for entity_data in entities:
        var transform = entity_data["components"]["Transform"]
        var velocity = entity_data["components"]["Velocity"]
        # Process...
```

---

## Usage Examples

### Example 1: Simple Entity Management

```gdscript
extends Node

var world: RID
var player: RID

func _ready():
    var flecs = FlecsServer.get_singleton()
    
    # Create world
    world = flecs.create_world()
    
    # Create player entity
    player = flecs.create_entity_with_name_and_comps(
        world,
        "Player",
        PackedStringArray(["Transform", "Health", "Inventory"]),
        [
            {"position": Vector3.ZERO, "rotation": Quaternion.IDENTITY},
            {"current": 100, "max": 100},
            {"items": [], "capacity": 20}
        ]
    )
    
    # Query player health
    var health = flecs.get_component_by_name(world, player, "Health")
    print("Player health: %d/%d" % [health["current"], health["max"]])

func take_damage(amount: int):
    var health = FlecsServer.get_component_by_name(world, player, "Health")
    health["current"] = max(0, health["current"] - amount)
    FlecsServer.set_component(world, player, "Health", health)
    
    if health["current"] <= 0:
        print("Player died!")
```

### Example 2: Query-Based Processing

```gdscript
extends Node

var world: RID
var enemy_query: RID

func _ready():
    var flecs = FlecsServer.get_singleton()
    world = flecs.create_world()
    
    # Create query for all enemies
    enemy_query = flecs.create_query(
        world,
        PackedStringArray(["Transform", "Enemy", "Health"])
    )
    
    # Spawn enemies
    for i in range(100):
        var enemy = flecs.create_entity(world)
        flecs.set_component(world, enemy, "Transform", {"position": Vector3(randf() * 100, 0, randf() * 100)})
        flecs.set_component(world, enemy, "Enemy", {"type": "goblin"})
        flecs.set_component(world, enemy, "Health", {"current": 50, "max": 50})

func _process(delta):
    # Get all enemies
    var enemies = FlecsServer.query_get_entities(world, enemy_query)
    
    # Process each enemy
    for enemy_rid in enemies:
        var transform = FlecsServer.get_component_by_name(world, enemy_rid, "Transform")
        var health = FlecsServer.get_component_by_name(world, enemy_rid, "Health")
        
        # Update enemy...
```

### Example 3: Hierarchical Entities

```gdscript
func create_vehicle():
    var flecs = FlecsServer.get_singleton()
    
    # Create vehicle body
    var vehicle = flecs.create_entity_with_name(world, "Vehicle")
    flecs.set_component(world, vehicle, "Transform", {"position": Vector3.ZERO})
    
    # Create wheels as children
    var wheel_positions = [
        Vector3(-1, 0, 1),   # Front left
        Vector3(1, 0, 1),    # Front right
        Vector3(-1, 0, -1),  # Back left
        Vector3(1, 0, -1)    # Back right
    ]
    
    for i in range(4):
        var wheel = flecs.create_entity_with_name(world, "Wheel%d" % i)
        flecs.set_component(world, wheel, "Transform", {"position": wheel_positions[i]})
        flecs.add_child(world, vehicle, wheel)
    
    # Query all wheels
    var children = flecs.get_children(world, vehicle)
    print("Vehicle has %d wheels" % children.size())
    
    return vehicle
```

### Example 4: Change-Only System

```gdscript
var damage_system: RID

func setup_damage_system():
    var flecs = FlecsServer.get_singleton()
    
    # Create system that only runs when Health component changes
    damage_system = flecs.add_script_system(
        world,
        PackedStringArray(["Health"]),
        _on_health_changed
    )
    
    # Enable change-only mode
    flecs.set_script_system_change_only(damage_system, true)
    flecs.set_script_system_change_observe_add_and_set(damage_system, true)

func _on_health_changed(entities: Array):
    # Only called when Health component is added or modified
    for entity_data in entities:
        var rid = entity_data["rid"]
        var health = entity_data["components"]["Health"]
        
        if health["current"] <= 0:
            print("Entity %s died!" % FlecsServer.get_entity_name(world, rid))
            # Trigger death effects, remove entity, etc.
```

---

## Best Practices

### 1. RID Lifecycle Management

✅ **DO:**
```gdscript
# Always free RIDs when done
func _exit_tree():
    FlecsServer.free_query(world, query)
    FlecsServer.free_script_system(world, system)
    FlecsServer.free_world(world)
```

❌ **DON'T:**
```gdscript
# Leaking RIDs - memory leak!
func _ready():
    world = FlecsServer.create_world()
    # ... use world ...
# No cleanup in _exit_tree!
```

### 2. Batch Processing

✅ **DO:**
```gdscript
# Use batch mode for large entity counts
FlecsServer.set_script_system_dispatch_mode(system, 1)
FlecsServer.set_script_system_batch_chunk_size(system, 100)
```

❌ **DON'T:**
```gdscript
# Per-entity mode with 10,000 entities = 10,000 GDScript calls!
FlecsServer.set_script_system_dispatch_mode(system, 0)
```

### 3. Change-Only When Appropriate

✅ **DO:**
```gdscript
# Use change-only for infrequent updates
FlecsServer.set_script_system_change_only(damage_system, true)  # Health rarely changes
```

❌ **DON'T:**
```gdscript
# Don't use change-only for every-frame updates
FlecsServer.set_script_system_change_only(movement_system, true)  # Transform changes every frame!
```

### 4. Instrumentation in Development

✅ **DO:**
```gdscript
# Enable instrumentation during development
if OS.is_debug_build():
    FlecsServer.set_script_system_instrumentation(system, true)
    FlecsServer.set_script_system_detailed_timing(system, true)
```

❌ **DON'T:**
```gdscript
# Don't leave detailed timing on in release builds
FlecsServer.set_script_system_detailed_timing(system, true)  # Performance overhead!
```

### 5. Query Caching

✅ **DO:**
```gdscript
# Use caching for stable queries
FlecsServer.query_set_caching_strategy(world, static_enemies_query, 1)

# Force refresh when you know data changed
spawn_new_enemies()
FlecsServer.query_force_cache_refresh(world, static_enemies_query)
```

❌ **DON'T:**
```gdscript
# Don't use full caching with frequently changing data
FlecsServer.query_set_caching_strategy(world, dynamic_query, 2)  # Data constantly stale!
```

### 6. Thread Safety

✅ **DO:**
```gdscript
# Use deferred calls with multi-threading
FlecsServer.set_script_system_multi_threaded(system, true)
FlecsServer.set_script_system_use_deferred_calls(system, true)

func callback(entities: Array):
    # Queue changes, don't modify scene tree directly
    for entity_data in entities:
        update_queue.append(entity_data)
```

❌ **DON'T:**
```gdscript
# Direct scene tree access from worker threads - CRASHES!
FlecsServer.set_script_system_multi_threaded(system, true)

func callback(entities: Array):
    for entity_data in entities:
        some_node.position = entity_data["components"]["Transform"]["position"]  # CRASH!
```

---

## Performance Considerations

### Query vs Script System

| Scenario | Recommended | Reason |
|----------|-------------|--------|
| Simple iteration | Query | Lower overhead, direct control |
| Complex logic | Script System | Better organization, instrumentation |
| Thousands of entities | Script System (batch) | Amortized call overhead |
| Infrequent updates | Script System (change-only) | Only runs when needed |
| Manual control | Query | Explicit fetch timing |

### Batching Impact

| Entity Count | Per-Entity Mode | Batch Mode (100) | Speedup |
|--------------|----------------|------------------|---------|
| 100 | 100 calls | 1 call | 100x |
| 1,000 | 1,000 calls | 10 calls | 100x |
| 10,000 | 10,000 calls | 100 calls | 100x |

### Caching Impact

| Cache Strategy | Fetch Time | Staleness Risk |
|----------------|------------|----------------|
| NO_CACHE | ~1000 µs | None |
| CACHE_ENTITIES | ~100 µs | Low (auto-invalidate) |
| CACHE_FULL | ~10 µs | High (manual refresh) |

### Multi-Threading Considerations

**Pros:**
- Utilizes multiple CPU cores
- Parallel entity processing
- Better frame times with many entities

**Cons:**
- Requires deferred calls (overhead)
- More complex debugging
- Thread-safe callback requirements

**When to Use:**
- Entity count > 1000
- Processing is CPU-intensive
- System takes > 5ms single-threaded

---

## Testing

Comprehensive unit tests are available in:

- `tests/test_flecs_variant.h` - Variant wrapper tests
- `tests/test_flecs_query.h` - Query functionality tests
- `tests/test_flecs_script_system.h` - Script system tests

Run tests with:
```bash
# Build with tests enabled
scons tests=yes

# Run tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test
```

---

## API Reference

For complete API documentation, see:

- **Header files**: `flecs_types/*.h` (inline documentation)
- **Quick reference**: `QUICK_REFERENCE.md`
- **Query API**: `QUERY_API.md`
- **FlecsServer API**: `FLECS_SERVER_API.md`

---

## Troubleshooting

### Common Issues

#### Issue: "entity_id is not a valid entity"

**Cause**: Using an invalid or freed RID

**Solution**: Check RID validity before use
```gdscript
if entity.is_valid():
    var health = FlecsServer.get_component_by_name(world, entity, "Health")
```

#### Issue: System not executing

**Cause**: System is paused or world not progressing

**Solution**: Check system state and progress world
```gdscript
print("System paused: ", FlecsServer.is_script_system_paused(system))
FlecsServer.progress_world(world, delta)
```

#### Issue: Memory leaks

**Cause**: Not freeing RIDs

**Solution**: Always cleanup in `_exit_tree`
```gdscript
func _exit_tree():
    FlecsServer.free_script_system(world, system)
    FlecsServer.free_world(world)
```

#### Issue: Thread crashes with multi-threading

**Cause**: Not using deferred calls

**Solution**: Enable deferred calls
```gdscript
FlecsServer.set_script_system_use_deferred_calls(system, true)
```

---

## Version History

- **v1.0** (2025-01-21): Initial comprehensive documentation
  - Documented all variant types
  - Complete FlecsServer API reference
  - FlecsQuery usage guide
  - FlecsScriptSystem detailed documentation
  - Unit tests for all components

---

## Contributing

When adding new features to flecs_types:

1. **Add inline documentation** using Doxygen format
2. **Write unit tests** in `tests/test_*.h`
3. **Update this documentation** with examples
4. **Update QUICK_REFERENCE.md** with new APIs
5. **Run tests** to verify functionality

---

## License

Copyright (c) 2014-present Godot Engine contributors.
Licensed under the MIT License.