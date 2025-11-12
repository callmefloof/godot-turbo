# Flecs Types Quick Reference Card

> **One-page reference for common Flecs ECS operations**

## 🚀 Essential Setup

```gdscript
# Create world
var world = FlecsServer.create_world()
FlecsServer.init_world(world)

# Register component
var pos_type = FlecsServer.register_component_type(world, "Position", {
    "x": TYPE_FLOAT, "y": TYPE_FLOAT, "z": TYPE_FLOAT
})

# Create entity
var entity = FlecsServer.create_entity_with_name(world, "Player")
FlecsServer.set_component(entity, "Position", {"x": 0, "y": 0, "z": 0})

# Run world
func _process(delta):
    FlecsServer.progress_world(world, delta)

# Cleanup
func _exit_tree():
    FlecsServer.free_world(world)
```

---

## 📦 Component Operations

```gdscript
# Add component (empty)
FlecsServer.add_component(entity, component_type_id)

# Set component data
FlecsServer.set_component(entity, "Position", {"x": 10, "y": 20, "z": 5})

# Get component
var pos = FlecsServer.get_component_by_name(entity, "Position")
print(pos["x"], pos["y"], pos["z"])

# Check if has component
if FlecsServer.has_component(entity, "Position"):
    print("Has position")

# Remove component
FlecsServer.remove_component_from_entity_with_name(entity, "Position")

# Get all components
var comp_names = FlecsServer.get_component_types_as_name(entity)
```

---

## 🎯 Script Systems

### Basic System
```gdscript
var system = FlecsServer.add_script_system(
    world,
    PackedStringArray(["Position", "Velocity"]),
    update_movement
)

func update_movement(entities: Array):
    for entity_data in entities:
        var pos = entity_data["components"]["Position"]
        var vel = entity_data["components"]["Velocity"]
        # Process...
```

### Performance Modes
```gdscript
# Batch mode (10-100x faster for many entities)
FlecsServer.set_script_system_dispatch_mode(system, 1)
FlecsServer.set_script_system_batch_chunk_size(system, 100)

# Change-only mode (react to changes)
FlecsServer.set_script_system_change_only(system, true)

# Multi-threaded
FlecsServer.set_script_system_multi_threaded(system, true)
FlecsServer.set_script_system_use_deferred_calls(system, true)

# Pause/Resume
FlecsServer.set_script_system_paused(system, true)
```

### Instrumentation
```gdscript
# Enable metrics
FlecsServer.set_script_system_instrumentation(system, true)
FlecsServer.set_script_system_detailed_timing(system, true)

# Get stats
var stats = FlecsServer.get_script_system_instrumentation(system)
print("Entities: ", stats["last_frame_entity_count"])
print("Time: ", stats["frame_median_usec"], " µs")
print("P99: ", FlecsServer.get_script_system_frame_p99_usec(system))
```

---

## 🔍 Queries

### Basic Query
```gdscript
# Create query
var query = FlecsServer.create_query(
    world,
    PackedStringArray(["Position", "Velocity"])
)

# Get entities
var entities = FlecsServer.query_get_entities(query)
for entity_rid in entities:
    # Process...

# Get entities with components
var results = FlecsServer.query_get_entities_with_components(query)
for result in results:
    var entity = result["rid"]
    var pos = result["components"]["Position"]
    var vel = result["components"]["Velocity"]
```

### Query Optimization
```gdscript
# Enable caching (10-100x faster)
FlecsServer.query_set_caching_strategy(query, 1)  # CACHE_ENTITIES

# Pagination
var page = FlecsServer.query_get_entities_limited(query, 100, offset)

# Count (fast, no fetch)
var count = FlecsServer.query_get_entity_count(query)

# Check match
if FlecsServer.query_matches_entity(query, entity):
    print("Matches!")

# Force refresh
FlecsServer.query_force_cache_refresh(query)
```

---

## 👨‍👩‍👧‍👦 Hierarchy

```gdscript
# Set parent
FlecsServer.set_parent(child, parent)

# Get parent
var parent = FlecsServer.get_parent(child)

# Get children
var children = FlecsServer.get_children(parent)
for child in children:
    print(FlecsServer.get_entity_name(child))

# Get child by name
var weapon = FlecsServer.get_child_by_name(parent, "Weapon")

# Remove all children
FlecsServer.remove_all_children(parent)
```

---

## 🗄️ Storage

```gdscript
# Store Godot Node
FlecsServer.add_to_node_storage(entity, node)
var node = FlecsServer.get_node_from_node_storage(entity)

# Store Godot Resource
FlecsServer.add_to_ref_storage(entity, resource)
var res = FlecsServer.get_resource_from_ref_storage(entity)
```

---

## ⚡ Performance Comparison

| Mode | Entities/sec | Use Case |
|------|-------------|----------|
| Per-entity | 10K | Simple logic |
| Batch | 100K | Many entities |
| Batch + MT | 400K | Heavy computation |
| Change-only | 1M+ | Reactive systems |

| Query Cache | Speed | When to Use |
|------------|-------|-------------|
| None | 1x | Constantly changing |
| Entities | 10x | Stable entity sets |
| Full | 100x | Static queries |

---

## 🎛️ Common Patterns

### Spawner System
```gdscript
func spawn_bullets(entities: Array):
    for entity_data in entities:
        if entity_data["components"]["ShootTimer"]["ready"]:
            var bullet = FlecsServer.create_entity(world)
            FlecsServer.set_component(bullet, "Position", 
                entity_data["components"]["Position"])
            FlecsServer.set_component(bullet, "Velocity", {"x": 100, "y": 0, "z": 0})
```

### Cleanup System
```gdscript
func cleanup_dead(entities: Array):
    for entity_data in entities:
        if entity_data["components"]["Health"]["current"] <= 0:
            FlecsServer.free_entity(entity_data["rid"])
```

### Query-Based Processing
```gdscript
func update_visible_sprites():
    var query = FlecsServer.create_query(world, 
        PackedStringArray(["Position", "Sprite", "Visible"]))
    var entities = FlecsServer.query_get_entities_with_components(query)
    for e in entities:
        var node = FlecsServer.get_node_from_node_storage(e["rid"])
        node.position = Vector3(e["components"]["Position"]["x"], 
                                e["components"]["Position"]["y"], 
                                e["components"]["Position"]["z"])
```

---

## 📊 Validation & Debugging

```gdscript
# Check RID validity
if world.is_valid():
    FlecsServer.progress_world(world, delta)

# Enable logging
FlecsServer.set_log_level(4)  # 0=none, 1=errors, 2=warnings, 3=info, 4=debug

# Debug RID
FlecsServer.debug_check_rid(entity)

# Get entity name
print("Entity: ", FlecsServer.get_entity_name(entity))
```

---

## 🚦 System Control

```gdscript
# Pause single system
FlecsServer.set_script_system_paused(system, true)

# Pause all systems
FlecsServer.pause_all_systems(world)

# Resume all
FlecsServer.resume_all_systems(world)

# Get all systems
var systems = FlecsServer.get_all_systems(world)
for sys in systems:
    var info = FlecsServer.get_script_system_info(sys)
    print(info["name"])
```

---

## 🎓 Best Practices

✅ **DO**
- Use batch mode for 100+ entities
- Enable caching for stable queries
- Check RID validity before use
- Free worlds on cleanup
- Use change-only for reactive logic
- Enable instrumentation during development

❌ **DON'T**
- Forget to free worlds
- Use CACHE_FULL without understanding invalidation
- Call setters inside query loops
- Mix multi-threading without deferred calls
- Store raw pointers in GDScript

---

## 📚 Full Documentation

- **FlecsServer API**: `FLECS_SERVER_API.md`
- **Query Guide**: `QUERY_API.md`
- **Performance**: `PERFORMANCE_FIX.md`
- **Overview**: `README.md`

---

**Cheat Sheet Version**: 1.0 | **Flecs ECS** + **Godot 4.x**