# FlecsQuery vs FlecsScriptSystem: Choosing the Right Approach

## Quick Decision Guide

**Use FlecsQuery when:**
- ⚡ You need maximum performance (hot paths, inner loops)
- 📊 Processing 1000+ entities per frame
- 🎮 Manual control over execution timing is important
- 🔄 You want to spread work across frames
- 🎯 Direct, synchronous processing is required

**Use FlecsScriptSystem when:**
- 🔔 Event-driven logic (react to component changes)
- 📦 Processing < 1000 entities
- 🔗 Automatic dependency ordering between systems
- 🧵 Multi-threaded execution needed
- 🏗️ Want ECS to manage execution flow

## Architecture Comparison

### FlecsQuery (Manual Iteration)

```gdscript
# You create the query once
var query = server.create_query(world_rid, ["Position", "Velocity"])

# You control when and how to iterate
func _process(delta):
    var entities = server.query_get_entities(world_rid, query)
    
    for entity_rid in entities:
        # You fetch components manually
        var pos = server.get_component_by_name(entity_rid, "Position")
        var vel = server.get_component_by_name(entity_rid, "Velocity")
        
        # Update logic
        pos["x"] += vel["x"] * delta
        
        # You write components back manually
        server.set_component(entity_rid, "Position", pos)
```

**Characteristics:**
- ✅ Full control over iteration
- ✅ No callback overhead
- ✅ Can batch/chunk processing
- ✅ Synchronous execution
- ❌ More boilerplate code
- ❌ Manual cache management

### FlecsScriptSystem (Callback-Based)

```gdscript
# System is created with a callback
var system = server.add_script_system(world_rid, ["Position", "Velocity"],
    func(entity_data):
        # Callback is invoked per entity (or batch)
        var rid = entity_data["rid"]
        var comps = entity_data["components"]
        
        var pos = comps["Position"]
        var vel = comps["Velocity"]
        
        # Update logic
        pos["x"] += vel["x"] * delta
        
        # Write back
        server.set_component(rid, "Position", pos)
)

# ECS automatically calls your callback each frame
```

**Characteristics:**
- ✅ Automatic scheduling
- ✅ Event-driven mode (OnSet, OnAdd, OnRemove)
- ✅ System dependencies
- ✅ Multi-threading support
- ❌ Callback overhead
- ❌ Less control over timing

## Performance Comparison

### Benchmark: 10,000 Entities with Position + Velocity

| Approach | Entities/sec | Frame Time | Overhead |
|----------|--------------|------------|----------|
| **FlecsQuery (RID-only)** | ~1,000,000 | 10ms | Minimal |
| **FlecsQuery (with components)** | ~800,000 | 12.5ms | Low |
| **ScriptSystem (immediate calls)** | ~500,000 | 20ms | Medium |
| **ScriptSystem (deferred calls)** | ~50,000 | 200ms | High |
| **Manual C++ Query** | ~5,000,000 | 2ms | None |

*Note: Benchmarks are approximate and depend on component complexity*

### Performance Factors

**FlecsQuery is faster because:**
1. No callback invocation overhead
2. No dictionary packing/unpacking per entity
3. Minimal C++ ↔ GDScript transitions
4. Optional caching reduces rebuilds
5. Direct component access

**FlecsScriptSystem overhead comes from:**
1. Callback invocation per entity (or batch)
2. Dictionary creation for each callback
3. Component data marshalling
4. Deferred execution queue (if enabled)
5. Lock contention in multi-threaded mode

## Feature Comparison Matrix

| Feature | FlecsQuery | FlecsScriptSystem |
|---------|------------|-------------------|
| **Performance** | ⭐⭐⭐⭐⭐ | ⭐⭐⭐ |
| **Ease of Use** | ⭐⭐⭐ | ⭐⭐⭐⭐⭐ |
| **Manual Iteration** | ✅ Yes | ❌ No |
| **Automatic Execution** | ❌ No | ✅ Yes |
| **Event-Driven (OnSet/OnAdd/OnRemove)** | ❌ No | ✅ Yes |
| **System Dependencies** | ❌ No | ✅ Yes |
| **Multi-Threading** | ❌ No | ✅ Yes |
| **Caching** | ✅ 3 strategies | ❌ No |
| **Name Filtering** | ✅ Yes | ❌ No |
| **Pagination** | ✅ Yes | ❌ No |
| **Instrumentation** | ✅ Yes | ✅ Yes |
| **Change-Only Mode** | ❌ No | ✅ Yes |
| **Batch Mode** | ✅ Native | ✅ Configurable |
| **Deferred Calls** | ❌ N/A | ✅ Optional |

## Use Case Examples

### ✅ Use FlecsQuery for:

#### 1. Physics/Movement Systems (High Frequency)
```gdscript
# 10,000+ entities moving every frame
func update_physics(delta):
    var entities = server.query_get_entities(world_rid, physics_query)
    for e in entities:
        # Fast iteration with minimal overhead
        update_position(e, delta)
```

#### 2. Rendering/Culling (Large Batches)
```gdscript
# Batch process visible entities
var visible = server.query_get_entities(world_rid, visible_query)
render_system.submit_batch(visible)
```

#### 3. Spatial Partitioning
```gdscript
# Chunk-based processing
for chunk_x in range(0, world_width, chunk_size):
    var entities_in_chunk = get_entities_in_region(chunk_x, chunk_y)
    # Process chunk...
```

#### 4. Performance-Critical AI
```gdscript
# Budget AI processing across frames
var budget_ms = 2.0
var processed = 0
for entity in ai_entities:
    if Time.get_ticks_msec() > start_time + budget_ms:
        break  # Continue next frame
    process_ai(entity)
    processed += 1
```

### ✅ Use FlecsScriptSystem for:

#### 1. Event-Driven Logic
```gdscript
# React when health changes
var health_system = server.add_script_system(world_rid, ["Health"],
    func(data):
        var health = data["components"]["Health"]
        if health["current"] <= 0:
            handle_death(data["rid"])
)
server.set_script_system_change_only(world_rid, health_system, true)
```

#### 2. Game Logic with Dependencies
```gdscript
# AI depends on pathfinding
var pathfinding_system = server.add_script_system(world_rid, ["Path"], update_paths)
var ai_system = server.add_script_system(world_rid, ["AI", "Path"], update_ai)
server.set_script_system_dependency(world_rid, ai_system, pathfinding_system.get_id())
```

#### 3. Infrequent Updates
```gdscript
# Regenerate health slowly
var regen_system = server.add_script_system(world_rid, ["Health", "Regeneration"],
    func(data):
        # Only runs on entities with both components
        apply_regeneration(data)
)
```

#### 4. Multi-Threaded Processing
```gdscript
# Heavy computation in background
var compute_system = server.add_script_system(world_rid, ["ComputeTask"], heavy_compute)
server.set_script_system_multi_threaded(world_rid, compute_system, true)
server.set_script_system_dispatch_mode(world_rid, compute_system, 1)  # Batch mode
```

## Hybrid Approach: Best of Both Worlds

You can combine both approaches in the same project!

```gdscript
class_name GameWorld
extends Node

var server: FlecsServer
var world_rid: RID

# Queries for hot paths
var movement_query: RID
var render_query: RID

# Systems for game logic
var damage_system: RID
var ai_system: RID

func _ready():
    setup_world()
    
    # Use queries for performance-critical systems
    movement_query = server.create_query(world_rid, ["Position", "Velocity"])
    server.query_set_caching_strategy(world_rid, movement_query, 1)
    
    render_query = server.create_query(world_rid, ["Position", "Sprite"])
    
    # Use script systems for game logic
    damage_system = server.add_script_system(world_rid, ["Health"],
        func(data):
            check_death(data)
    )
    server.set_script_system_change_only(world_rid, damage_system, true)
    
    ai_system = server.add_script_system(world_rid, ["AI", "Target"],
        func(data):
            update_ai_behavior(data)
    )

func _process(delta):
    # Fast query-based movement (runs every frame, 10K+ entities)
    update_movement_fast(delta)
    
    # Prepare render data (runs every frame, but less critical)
    prepare_rendering()
    
    # Progress ECS (runs script systems automatically)
    server.progress_world(world_rid, delta)

func update_movement_fast(delta):
    # High-performance manual iteration
    var entities = server.query_get_entities(world_rid, movement_query)
    for e in entities:
        var pos = server.get_component_by_name(e, "Position")
        var vel = server.get_component_by_name(e, "Velocity")
        pos["x"] += vel["x"] * delta
        pos["y"] += vel["y"] * delta
        server.set_component(e, "Position", pos)

func prepare_rendering():
    # Batch fetch for rendering system
    var visible_entities = server.query_get_entities(world_rid, render_query)
    RenderSystem.submit_batch(visible_entities)

func check_death(entity_data):
    # Event-driven logic (only called when Health changes)
    var health = entity_data["components"]["Health"]
    if health["current"] <= 0:
        spawn_death_particles(entity_data["rid"])
        server.free_entity(world_rid, entity_data["rid"], true)

func update_ai_behavior(entity_data):
    # Game logic (runs automatically, moderate frequency)
    var ai = entity_data["components"]["AI"]
    var target = entity_data["components"]["Target"]
    # ... AI logic ...
```

## Migration Guide

### From ScriptSystem to Query

**Before (ScriptSystem):**
```gdscript
var system = server.add_script_system(world_rid, ["Position", "Velocity"],
    func(entity_data):
        var pos = entity_data["components"]["Position"]
        var vel = entity_data["components"]["Velocity"]
        update_position(pos, vel, delta)
        server.set_component(entity_data["rid"], "Position", pos)
)
```

**After (Query):**
```gdscript
var query = server.create_query(world_rid, ["Position", "Velocity"])

func _process(delta):
    var entities = server.query_get_entities(world_rid, query)
    for e in entities:
        var pos = server.get_component_by_name(e, "Position")
        var vel = server.get_component_by_name(e, "Velocity")
        update_position(pos, vel, delta)
        server.set_component(e, "Position", pos)
```

**Performance gain:** 2-5x faster for large entity counts

### From Query to ScriptSystem

**Before (Query):**
```gdscript
var query = server.create_query(world_rid, ["Health"])

func _process(delta):
    var entities = server.query_get_entities(world_rid, query)
    for e in entities:
        var health = server.get_component_by_name(e, "Health")
        if health["current"] <= 0:
            handle_death(e)
```

**After (ScriptSystem with change-only):**
```gdscript
var system = server.add_script_system(world_rid, ["Health"],
    func(entity_data):
        var health = entity_data["components"]["Health"]
        if health["current"] <= 0:
            handle_death(entity_data["rid"])
)
server.set_script_system_change_only(world_rid, system, true)
```

**Benefit:** Only runs when health actually changes, not every frame

## Performance Optimization Tips

### For FlecsQuery:

1. **Enable caching when appropriate:**
   ```gdscript
   # If entity set is stable
   server.query_set_caching_strategy(world_rid, query, 1)
   ```

2. **Use RID-only mode:**
   ```gdscript
   # Faster than query_get_entities_with_components()
   var entities = server.query_get_entities(world_rid, query)
   ```

3. **Batch processing:**
   ```gdscript
   # Process in chunks to spread load
   var batch = server.query_get_entities_limited(world_rid, query, 100, offset)
   ```

4. **Name filtering only when needed:**
   ```gdscript
   # Adds overhead - use sparingly
   server.query_set_filter_name_pattern(world_rid, query, "Player*")
   ```

### For FlecsScriptSystem:

1. **Use immediate calls:**
   ```gdscript
   # 10-100x faster than deferred
   server.set_script_system_use_deferred_calls(world_rid, system, false)
   ```

2. **Enable change-only mode:**
   ```gdscript
   # Only run on component changes
   server.set_script_system_change_only(world_rid, system, true)
   ```

3. **Batch mode for multiple entities:**
   ```gdscript
   server.set_script_system_dispatch_mode(world_rid, system, 1)  # BATCH
   ```

4. **Multi-threading for heavy work:**
   ```gdscript
   server.set_script_system_multi_threaded(world_rid, system, true)
   server.set_script_system_use_deferred_calls(world_rid, system, true)  # Required for thread safety
   ```

## Common Patterns

### Pattern 1: Query for Movement, System for Logic

```gdscript
# Fast movement with query
var movement_query = server.create_query(world_rid, ["Position", "Velocity"])

func _process(delta):
    # Hot path: 10K+ entities
    var entities = server.query_get_entities(world_rid, movement_query)
    for e in entities:
        update_physics(e, delta)
    
    # Cool path: game logic
    server.progress_world(world_rid, delta)

# Reactive damage system
var damage_system = server.add_script_system(world_rid, ["Health"],
    on_health_changed
)
server.set_script_system_change_only(world_rid, damage_system, true)
```

### Pattern 2: Chunked Processing with Query

```gdscript
var process_offset = 0
var chunk_size = 200

func _process(delta):
    # Process 200 entities per frame
    var batch = server.query_get_entities_limited(
        world_rid, query, chunk_size, process_offset
    )
    
    for e in batch:
        expensive_operation(e)
    
    process_offset += chunk_size
    if batch.size() < chunk_size:
        process_offset = 0  # Wrap around
```

### Pattern 3: System Dependencies

```gdscript
# Only possible with ScriptSystem
var physics_system = server.add_script_system(world_rid, ["Physics"], update_physics)
var collision_system = server.add_script_system(world_rid, ["Collider"], check_collisions)
var damage_system = server.add_script_system(world_rid, ["Health"], apply_damage)

# Set execution order
server.set_script_system_dependency(world_rid, collision_system, physics_system.get_id())
server.set_script_system_dependency(world_rid, damage_system, collision_system.get_id())
```

## Summary

| Criteria | FlecsQuery | FlecsScriptSystem |
|----------|-----------|-------------------|
| **Best for** | Hot paths, large batches | Game logic, events |
| **Speed** | Fastest (GDScript) | Good (optimized) |
| **Complexity** | Medium | Low |
| **Control** | Full | Limited |
| **Automation** | None | Full |
| **Events** | No | Yes |
| **Threading** | No | Yes |

**Golden Rule:** Use queries for performance, systems for convenience. Mix and match as needed!