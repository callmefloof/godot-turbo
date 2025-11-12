# FlecsQuery API Documentation

## Overview

`FlecsQuery` is a high-performance query variant for direct entity iteration in the Flecs ECS module. Unlike `FlecsScriptSystem` which uses callbacks, `FlecsQuery` allows you to build a query once and fetch matching entities as needed, giving you manual control over iteration.

This is ideal for performance-critical systems where callback overhead is too expensive, or when you need more flexible control over when and how entities are processed.

## Performance Comparison

| Approach | Speed | Use Case |
|----------|-------|----------|
| FlecsScriptSystem (callback-based) | Medium | General purpose, event-driven logic |
| FlecsQuery (manual iteration) | Fast | Performance-critical loops, large entity batches |
| Direct C++ Query | Fastest | Ultimate performance, native code |

## Basic Usage

### Creating a Query

```gdscript
extends Node

var world_rid: RID
var query_rid: RID

func _ready():
    var server = FlecsServer
    
    # Create world
    world_rid = server.create_world()
    server.init_world(world_rid)
    
    # Create query for entities with Position and Velocity
    query_rid = server.create_query(world_rid, ["Position", "Velocity"])
```

### Fetching Entities (RID-only mode)

```gdscript
func _process(delta):
    var server = FlecsServer
    
    # Get all matching entity RIDs
    var entities = server.query_get_entities(world_rid, query_rid)
    
    # Manual iteration - you control the loop
    for entity_rid in entities:
        var pos = server.get_component_by_name(entity_rid, "Position")
        var vel = server.get_component_by_name(entity_rid, "Velocity")
        
        # Update position
        pos["x"] += vel["x"] * delta
        pos["y"] += vel["y"] * delta
        
        server.set_component(entity_rid, "Position", pos)
```

### Fetching Entities with Components

```gdscript
func _process(delta):
    var server = FlecsServer
    
    # Get entities with their components pre-fetched
    var entities = server.query_get_entities_with_components(world_rid, query_rid)
    
    # Each entry is: { "rid": RID, "components": { "Position": {...}, "Velocity": {...} } }
    for entity_data in entities:
        var rid = entity_data["rid"]
        var components = entity_data["components"]
        
        var pos = components["Position"]
        var vel = components["Velocity"]
        
        # Update position
        pos["x"] += vel["x"] * delta
        pos["y"] += vel["y"] * delta
        
        server.set_component(rid, "Position", pos)
```

## Advanced Features

### Caching Strategies

Caching can dramatically improve performance when entity lists don't change frequently:

```gdscript
# Caching strategies:
# 0 = NO_CACHE (default) - Always rebuild entity list
# 1 = CACHE_ENTITIES - Cache RID list, invalidate on changes
# 2 = CACHE_FULL - Cache RIDs + component data (fastest, use carefully)

server.query_set_caching_strategy(world_rid, query_rid, 1)  # Cache entity RIDs

# Force refresh if needed
server.query_force_cache_refresh(world_rid, query_rid)

# Check if cache is dirty
if server.query_is_cache_dirty(world_rid, query_rid):
    print("Cache will be rebuilt on next fetch")
```

**Caching Notes:**
- `NO_CACHE` (0): Safest, always up-to-date, but slower
- `CACHE_ENTITIES` (1): Good balance for stable entity sets
- `CACHE_FULL` (2): Fastest, but component data may be stale - use only if components are read-only

### Name Filtering

Filter entities by name pattern:

```gdscript
# Only match entities whose names start with "Player"
server.query_set_filter_name_pattern(world_rid, query_rid, "Player*")

var player_entities = server.query_get_entities(world_rid, query_rid)

# Clear filter
server.query_clear_filter(world_rid, query_rid)
```

### Limited/Paginated Fetching

Process entities in chunks:

```gdscript
# Fetch 100 entities at a time
var batch_size = 100
var offset = 0

while true:
    var batch = server.query_get_entities_limited(world_rid, query_rid, batch_size, offset)
    
    if batch.is_empty():
        break
    
    for entity_rid in batch:
        # Process entity...
        pass
    
    offset += batch_size
```

### Entity Count

Get count without fetching all entities:

```gdscript
var count = server.query_get_entity_count(world_rid, query_rid)
print("Found %d matching entities" % count)
```

### Entity Matching

Check if a specific entity matches the query:

```gdscript
var entity_rid = server.create_entity(world_rid)

if server.query_matches_entity(world_rid, query_rid, entity_rid):
    print("Entity matches query")
```

### Instrumentation

Track query performance:

```gdscript
# Enable instrumentation
server.query_set_instrumentation_enabled(world_rid, query_rid, true)

# Fetch entities...
var entities = server.query_get_entities(world_rid, query_rid)

# Get performance data
var stats = server.query_get_instrumentation_data(world_rid, query_rid)
print("Total fetches: ", stats["total_fetches"])
print("Total entities returned: ", stats["total_entities_returned"])
print("Last fetch entity count: ", stats["last_fetch_entity_count"])
print("Last fetch time (µs): ", stats["last_fetch_usec"])
print("Cache hit rate: ", stats["cache_hit_rate"])

# Reset stats
server.query_reset_instrumentation(world_rid, query_rid)
```

### Dynamic Component Requirements

Change query components at runtime:

```gdscript
# Initially query Position + Velocity
query_rid = server.create_query(world_rid, ["Position", "Velocity"])

# Later, change to Position + Health
server.query_set_required_components(world_rid, query_rid, ["Position", "Health"])

# Get current requirements
var components = server.query_get_required_components(world_rid, query_rid)
print("Querying for: ", components)
```

## Complete Example: High-Performance Movement System

```gdscript
extends Node

var server: FlecsServer
var world_rid: RID
var movement_query: RID

func _ready():
    server = FlecsServer
    
    # Create world and register components
    world_rid = server.create_world()
    server.init_world(world_rid)
    
    server.register_component_type(world_rid, "Position", {
        "x": TYPE_FLOAT,
        "y": TYPE_FLOAT
    })
    
    server.register_component_type(world_rid, "Velocity", {
        "x": TYPE_FLOAT,
        "y": TYPE_FLOAT
    })
    
    # Create query
    movement_query = server.create_query(world_rid, ["Position", "Velocity"])
    
    # Enable caching since entity set is stable
    server.query_set_caching_strategy(world_rid, movement_query, 1)
    
    # Enable instrumentation for monitoring
    server.query_set_instrumentation_enabled(world_rid, movement_query, true)
    
    # Spawn entities
    spawn_entities(1000)

func spawn_entities(count: int):
    for i in range(count):
        var entity = server.create_entity_with_name(world_rid, "Entity_%d" % i)
        
        server.set_component(entity, "Position", {
            "x": randf() * 1000.0,
            "y": randf() * 1000.0
        })
        
        server.set_component(entity, "Velocity", {
            "x": randf() * 100.0 - 50.0,
            "y": randf() * 100.0 - 50.0
        })

func _process(delta):
    # Progress ECS world
    server.progress_world(world_rid, delta)
    
    # Manual movement update (runs in addition to ECS systems)
    update_movement(delta)
    
    # Print stats every second
    if Engine.get_frames_drawn() % 60 == 0:
        print_stats()

func update_movement(delta: float):
    # Fetch all entities with Position + Velocity
    var entities = server.query_get_entities(world_rid, movement_query)
    
    for entity_rid in entities:
        var pos = server.get_component_by_name(entity_rid, "Position")
        var vel = server.get_component_by_name(entity_rid, "Velocity")
        
        # Update position
        pos["x"] += vel["x"] * delta
        pos["y"] += vel["y"] * delta
        
        # Wrap around screen
        pos["x"] = fmod(pos["x"], 1000.0)
        pos["y"] = fmod(pos["y"], 1000.0)
        
        server.set_component(entity_rid, "Position", pos)

func print_stats():
    var stats = server.query_get_instrumentation_data(world_rid, movement_query)
    print("Query Stats:")
    print("  Entities: ", stats["last_fetch_entity_count"])
    print("  Fetch time: ", stats["last_fetch_usec"], " µs")
    print("  Cache hits: ", stats["cache_hits"])
    print("  Cache misses: ", stats["cache_misses"])
    print("  Hit rate: %.1f%%" % (stats["cache_hit_rate"] * 100.0))

func _exit_tree():
    # Cleanup
    server.free_query(world_rid, movement_query)
    server.free_world(world_rid)
```

## Performance Tips

1. **Use RID-only mode when possible**: `query_get_entities()` is faster than `query_get_entities_with_components()` if you only need entity references.

2. **Enable caching for stable entity sets**: If your entities rarely change, use `CACHE_ENTITIES` or `CACHE_FULL`.

3. **Batch processing**: Use `query_get_entities_limited()` to process entities in chunks, spreading work across frames.

4. **Name filtering**: Only use name patterns when needed - they add overhead.

5. **Instrumentation overhead**: Disable instrumentation in production builds for maximum performance.

6. **Minimize component fetches**: Fetch components once, update locally, then write back.

## Comparison: Query vs Script System

### When to use FlecsQuery:
- ✅ Performance-critical inner loops
- ✅ Large batches of entities (1000s+)
- ✅ Need manual control over iteration timing
- ✅ Want to spread work across multiple frames
- ✅ Direct, synchronous processing

### When to use FlecsScriptSystem:
- ✅ Event-driven logic (react to component changes)
- ✅ Smaller entity counts (< 1000)
- ✅ Automatic scheduling and dependencies
- ✅ Need deferred/threaded execution
- ✅ Want ECS to manage execution order

### Hybrid Approach:
You can use both! Use FlecsQuery for hot paths and FlecsScriptSystem for everything else.

```gdscript
# Fast query-based movement
var entities = server.query_get_entities(world_rid, movement_query)
for e in entities:
    # High-performance manual loop
    pass

# Script system for AI (runs automatically)
server.add_script_system(world_rid, ["AI", "Health"], 
    func(entity_data):
        # Less frequent, event-driven logic
        pass
)
```

## API Reference Summary

### Core Operations
- `create_query(world_rid, required_components)` → RID
- `query_get_entities(world_rid, query_rid)` → Array[RID]
- `query_get_entities_with_components(world_rid, query_rid)` → Array[Dictionary]
- `query_get_entity_count(world_rid, query_rid)` → int
- `query_get_entities_limited(world_rid, query_rid, max_count, offset)` → Array[RID]
- `query_get_entities_with_components_limited(world_rid, query_rid, max_count, offset)` → Array[Dictionary]
- `query_matches_entity(world_rid, query_rid, entity_rid)` → bool

### Configuration
- `query_set_required_components(world_rid, query_rid, components)`
- `query_get_required_components(world_rid, query_rid)` → PackedStringArray
- `query_set_caching_strategy(world_rid, query_rid, strategy)` (0=NO_CACHE, 1=CACHE_ENTITIES, 2=CACHE_FULL)
- `query_get_caching_strategy(world_rid, query_rid)` → int
- `query_set_filter_name_pattern(world_rid, query_rid, pattern)`
- `query_get_filter_name_pattern(world_rid, query_rid)` → String
- `query_clear_filter(world_rid, query_rid)`

### Cache Control
- `query_force_cache_refresh(world_rid, query_rid)`
- `query_is_cache_dirty(world_rid, query_rid)` → bool

### Instrumentation
- `query_set_instrumentation_enabled(world_rid, query_rid, enabled)`
- `query_get_instrumentation_enabled(world_rid, query_rid)` → bool
- `query_get_instrumentation_data(world_rid, query_rid)` → Dictionary
- `query_reset_instrumentation(world_rid, query_rid)`

### Cleanup
- `free_query(world_rid, query_rid)`

## Troubleshooting

### Query returns empty array
- Verify component names are registered: `server.register_component_type(...)`
- Check that entities actually have all required components
- Ensure world and query RIDs are valid

### Performance is slower than expected
- Enable instrumentation to identify bottlenecks
- Consider enabling caching if entity set is stable
- Use RID-only mode instead of fetching components
- Batch large queries with `query_get_entities_limited()`

### Cache not invalidating
- Cache invalidation uses observers - ensure components are being modified correctly
- Try `query_force_cache_refresh()` to manually refresh
- Consider using `NO_CACHE` if entity changes are unpredictable

### Out of date component data
- Don't use `CACHE_FULL` if components are frequently modified
- Use `CACHE_ENTITIES` and fetch components manually for best balance