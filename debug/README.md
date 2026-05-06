# Turbo Debug System

The Turbo Debug System is a comprehensive runtime debugging infrastructure for Godot Turbo. It provides in-game entity visualization, selection, and inspection capabilities that work both in the editor and at runtime.

## Features

- **Entity Picking**: Click-to-select entities in the 3D viewport
- **Entity Gizmos**: Visual representation of entity bounds and transforms
- **Debug Panel**: In-game HUD showing entity info, performance stats, and visualization controls
- **Debug Draw**: Runtime 3D line/shape drawing for visualization

## Architecture

```
TurboDebugSystem (Singleton)
├── TurboDebugDraw       - 3D debug visualization (lines, boxes, spheres, gizmos)
├── TurboEntityPicker    - Entity selection via ray casting
└── TurboDebugPanel      - In-game debug HUD (CanvasLayer)
```

## Usage

### Getting the Debug System

```gdscript
var debug = TurboDebugSystem.get_singleton()

# Initialize with scene tree
debug.initialize(get_tree())

# Or with a specific viewport
debug.initialize_with_viewport(get_viewport())
```

### Entity Picking

```gdscript
# Pick entity at current mouse position
var entity_rid = debug.pick_at_mouse()

# Pick and immediately select
var entity_rid = debug.pick_and_select_at_mouse()

# Pick at specific screen position
var entity_rid = debug.pick_at_position(Vector2(400, 300))

# Pick along a ray in world space
var entity_rid = debug.pick_ray(origin, direction, 1000.0)
```

### Entity Tracking

To enable picking and gizmos for entities, you must track them:

```gdscript
# Track an entity
debug.track_entity(entity_rid, world_rid)

# Update entity transform and bounds (call each frame or when changed)
debug.update_tracked_entity(entity_rid, transform, aabb)

# Stop tracking
debug.untrack_entity(entity_rid)
```

### Selection

```gdscript
# Select an entity programmatically
debug.select_entity(entity_rid, world_rid)

# Clear selection
debug.clear_selection()

# Check selection
if debug.has_selection():
    var selected = debug.get_selected_entity()
    var info = debug.get_selected_entity_info()
    var components = debug.get_selected_entity_components()
```

### Debug Panel

```gdscript
# Show/hide the debug panel
debug.show_panel()
debug.hide_panel()
debug.toggle_panel()

# Log messages to the panel
debug.log("Entity spawned")
debug.log_warning("Performance degraded")
debug.log_error("Failed to load component")

# Register custom visualization toggles
debug.register_visualization_toggle("Show Navmesh", "Display navigation mesh", false, "Navigation")

# Check toggle state
if debug.get_visualization_toggle("Show Navmesh"):
    draw_navmesh()
```

### Debug Drawing

```gdscript
# Draw primitives (cleared each frame)
debug.draw_line(from, to, Color.RED)
debug.draw_box(aabb, Color.GREEN)
debug.draw_sphere(center, radius, Color.BLUE)
debug.draw_arrow(from, to, Color.YELLOW)
debug.draw_transform(transform, 1.0)

# Clear all draws
debug.clear_debug_draws()
```

### Gizmo Management

```gdscript
# Add gizmo for an entity
debug.add_gizmo(entity_rid, transform, aabb)

# Update gizmo
debug.update_gizmo(entity_rid, new_transform)
debug.update_gizmo_bounds(entity_rid, new_aabb)

# Customize gizmo appearance
debug.set_gizmo_color(entity_rid, Color.MAGENTA)
debug.set_gizmo_show_bounds(entity_rid, true)
debug.set_gizmo_show_axes(entity_rid, true)

# Remove gizmo
debug.remove_gizmo(entity_rid)
```

## Configuration

### TurboDebugSystemConfig

```gdscript
var config = {
    "enabled": true,
    "enable_picking": true,
    "enable_gizmos": true,
    "enable_panel": true,
    "enable_hotkeys": true,
    "auto_find_camera": true,
    "default_show_bounds": true,
    "default_show_axes": true,
    "default_depth_test": true,
    "pick_cooldown": 0.05,
    "max_gizmos": 1000,
}
debug.set_config_from_dict(config)
```

### Default Hotkeys

| Action | Default Key |
|--------|-------------|
| Pick Entity | Ctrl + Left Click |
| Toggle Panel | F3 |
| Toggle Gizmos | F4 |

## Signals

```gdscript
# Entity was selected
debug.entity_selected.connect(func(entity_rid, world_rid):
    print("Selected: ", entity_rid))

# Entity was deselected
debug.entity_deselected.connect(func(entity_rid, world_rid):
    print("Deselected: ", entity_rid))

# Selection changed (includes previous/new info)
debug.selection_changed.connect(func(event: Dictionary):
    print("Selection changed: ", event))

# Entity was picked (click hit an entity)
debug.entity_picked.connect(func(entity_rid, world_rid, position):
    print("Picked at: ", position))

# Panel visibility changed
debug.panel_visibility_changed.connect(func(visible: bool):
    print("Panel visible: ", visible))
```

## Performance Considerations

- **Entity Tracking**: Only track entities that need to be pickable/have gizmos
- **Max Gizmos**: Default limit is 1000 simultaneous gizmos
- **Debug Draw**: Lines/shapes are cleared each frame; budget is 10000 lines
- **Picking**: Has a cooldown (default 0.05s) to prevent excessive raycasts

## Debug Panel Tabs

1. **Entity**: Selected entity info and component inspector
2. **Stats**: FPS, frame time, entity count, memory usage
3. **Visualization**: Toggle buttons for various debug overlays
4. **Log**: Runtime log messages with timestamps
5. **Commands**: Custom debug command buttons

## Files

| File | Description |
|------|-------------|
| `turbo_debug_system.h/cpp` | Main coordinator singleton |
| `turbo_debug_draw.h/cpp` | 3D visualization (ImmediateMesh-based) |
| `turbo_entity_picker.h/cpp` | Entity selection via raycasting |
| `turbo_debug_panel.h/cpp` | In-game debug HUD |
| `ecs_trace_bridge.h` | Runtime bridge to AxiomScript's Neural Visualizer |
| `flecs_os_api_traced.h` | Flecs OS API wrapper for worker thread tracing |

---

## ECS Trace Bridge

The ECS Trace Bridge provides a runtime bridge from Godot Turbo's FlecsServer to AxiomScript's Neural Web Visualizer. It enables tracing of ECS operations without requiring hard module dependencies.

### Features

- **Zero-overhead when disabled**: Singleton lookup is cached; null checks are fast
- **Automatic thread registration**: Flecs worker threads are automatically registered for tracing
- **Full event coverage**: Entity lifecycle, component operations, and query iterations are traced

### Instrumentation Points

The following ECS operations emit trace events:

| Operation | Trace Type | Location |
|-----------|------------|----------|
| `create_entity()` | ENTITY_CREATE | FlecsServer |
| `free_entity()` | ENTITY_DESTROY | FlecsServer |
| `get_component_by_name()` | READ | FlecsServer |
| `set_component()` | WRITE | FlecsServer |
| `add_component()` | ADD | FlecsServer |
| `remove_component_*()` | REMOVE | FlecsServer |
| Query iteration | QUERY | FlecsQuery, FlecsScriptSystem |

### Usage

The trace bridge is automatically initialized when Godot Turbo loads. To use tracing in custom code:

```cpp
#include "modules/godot_turbo/debug/ecs_trace_bridge.h"

// In your ECS operation
void my_custom_operation(flecs::entity e, flecs::entity comp) {
    // Trace a read operation
    ECS_TRACE_READ(e.id(), comp.id(), 0);
    
    // ... your operation code ...
    
    // Trace a write operation
    ECS_TRACE_WRITE(e.id(), comp.id(), 0);
}
```

### Available Macros

| Macro | Description |
|-------|-------------|
| `ECS_TRACE_INIT()` | Initialize the trace bridge (called automatically) |
| `ECS_TRACE_SHUTDOWN()` | Shutdown the trace bridge |
| `ECS_TRACE_REGISTER_THREAD()` | Register current thread for tracing |
| `ECS_TRACE_UNREGISTER_THREAD()` | Unregister current thread |
| `ECS_TRACE_READ(entity_id, component_id, field_hash)` | Trace component read |
| `ECS_TRACE_WRITE(entity_id, component_id, field_hash)` | Trace component write |
| `ECS_TRACE_ADD(entity_id, component_id)` | Trace component addition |
| `ECS_TRACE_REMOVE(entity_id, component_id)` | Trace component removal |
| `ECS_TRACE_QUERY(entity_id, component_id)` | Trace query iteration |
| `ECS_TRACE_ENTITY_CREATE(entity_id)` | Trace entity creation |
| `ECS_TRACE_ENTITY_DESTROY(entity_id)` | Trace entity destruction |
| `ECS_TRACE_REGISTER_ENTITY_NAME(entity_id, name)` | Register human-readable entity name |
| `ECS_TRACE_REGISTER_COMPONENT_NAME(component_id, name)` | Register human-readable component name |

---

## Flecs OS API Traced

The Flecs OS API Traced module provides automatic worker thread registration for Flecs multi-threaded systems. When Flecs spawns worker threads (via `world.set_threads()`), these threads are automatically registered with the trace bridge.

### How It Works

1. **Installation**: The traced OS API is installed when FlecsServer is constructed
2. **Thread Wrapping**: When Flecs creates worker threads, a wrapper intercepts the thread start
3. **Registration**: The wrapper calls `ECS_TRACE_REGISTER_THREAD()` at thread start
4. **Cleanup**: When threads exit, `ECS_TRACE_UNREGISTER_THREAD()` is called automatically

### Monitoring

```cpp
#include "modules/godot_turbo/debug/flecs_os_api_traced.h"

// Check if traced API is active
if (godot_turbo::FlecsOSApiTraced::is_installed()) {
    // Get count of active worker threads
    int workers = godot_turbo::FlecsOSApiTraced::get_active_thread_count();
    print_line("Active Flecs workers: " + itos(workers));
}
```

### Integration with Neural Visualizer

When the AxiomScript Neural Web Panel is active:

1. Events from worker threads appear in the trace stream
2. The panel displays active thread count in the stats section
3. Coactivation graphs show cross-thread entity access patterns
4. Hotspot detection works correctly for multi-threaded systems