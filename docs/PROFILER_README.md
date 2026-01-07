# Flecs Profiler - Quick Start Guide

## Overview

The Flecs Profiler provides real-time performance monitoring for Flecs ECS systems directly in the Godot editor.

## Features

✅ **Real-time system monitoring** - See execution times, entity counts, and call counts  
✅ **Built-in profiler UI** - Uses Godot's EditorProfiler widget  
✅ **Automatic collection** - Metrics updated every 100ms  
✅ **Multi-world support** - Tracks multiple Flecs worlds simultaneously  
✅ **Detailed statistics** - Optional median, p99, and stddev metrics  
✅ **Zero configuration** - Works out of the box  

## Quick Start

### 1. Enable the Plugin

The Flecs Profiler Plugin is automatically enabled when the `godot_turbo` module is compiled.

### 2. Open the Profiler

In the Godot editor:
1. Open the **Flecs Profiler** dock (bottom-right by default)
2. Click the **Start** button to begin profiling
3. Run your scene with Flecs systems (F5 or F6)
4. Metrics will appear in the profiler

**Note:** The Flecs Profiler profiles systems running in the editor only (not remote game instances). The "Start" button controls local data collection, not remote connections.

### 3. Interpret the Display

Each system shows:
- **Name** - System identifier
- **Time** - Execution time this frame (microseconds)
- **Calls** - Number of invocations
- **Entities** - Number of entities processed (shown in parentheses)

## Using the API

### Basic Usage

```gdscript
# Get FlecsServer singleton
var flecs_server = Engine.get_singleton("FlecsServer")

# Get all worlds
var worlds = flecs_server.get_world_list()

# Get metrics for the first world
if worlds.size() > 0:
    var metrics = flecs_server.get_system_metrics(worlds[0])
    
    # Print all systems
    for system in metrics["systems"]:
        print("%s: %d µs" % [system["name"], system["time_usec"]])
```

### Find Slowest System

```gdscript
var metrics = flecs_server.get_system_metrics(world_id)
var slowest = null
var max_time = 0

for system in metrics["systems"]:
    if system["time_usec"] > max_time:
        max_time = system["time_usec"]
        slowest = system

print("Slowest: ", slowest["name"])
```

## Enabling Detailed Timing

For advanced statistics (median, p99, standard deviation):

```gdscript
# Enable detailed timing for a specific system
flecs_server.set_script_system_detailed_timing(world_id, system_id, true)

# Now get metrics
var metrics = flecs_server.get_system_metrics(world_id)
# Systems will have "median_usec", "p99_usec", "stddev_usec" fields
```

**Note:** Detailed timing has a small performance overhead.

## Metrics Reference

### Per-System Metrics

| Field | Type | Description |
|-------|------|-------------|
| `rid` | RID | System identifier |
| `name` | String | System name |
| `type` | String | "script" or "cpp" |
| `time_usec` | int | Last frame execution time (µs) |
| `call_count` | int | Invocations this frame |
| `total_time_usec` | int | Total time this frame (µs) |
| `avg_time_usec` | int | Average time per call (µs) |
| `min_time_usec` | int | Minimum invocation time (µs) |
| `max_time_usec` | int | Maximum invocation time (µs) |
| `entity_count` | int | Entities processed |
| `paused` | bool | Is system paused? |

### Script Systems Only

| Field | Type | Description |
|-------|------|-------------|
| `onadd_count` | int | OnAdd events this frame |
| `onset_count` | int | OnSet events this frame |
| `onremove_count` | int | OnRemove events this frame |
| `dispatch_mode` | int | Dispatch mode enum value |
| `total_callbacks` | int | Lifetime callback count |
| `total_entities_processed` | int | Lifetime entity count |

### With Detailed Timing

| Field | Type | Description |
|-------|------|-------------|
| `median_usec` | float | Median execution time |
| `p99_usec` | float | 99th percentile |
| `stddev_usec` | float | Standard deviation |

## Common Use Cases

### 1. Performance Monitoring

```gdscript
func _process(_delta):
    var metrics = flecs_server.get_system_metrics(world_id)
    var total_time = 0
    
    for system in metrics["systems"]:
        total_time += system["time_usec"]
    
    if total_time > 16666:  # > 16ms
        push_warning("ECS systems taking too long!")
```

### 2. System Health Check

```gdscript
func check_system_health():
    var metrics = flecs_server.get_system_metrics(world_id)
    
    for system in metrics["systems"]:
        var usec_per_entity = float(system["time_usec"]) / max(1, system["entity_count"])
        
        if usec_per_entity > 100.0:
            print("Warning: %s is slow per entity" % system["name"])
```

### 3. Export Metrics

```gdscript
func export_metrics():
    var metrics = flecs_server.get_system_metrics(world_id)
    var json = JSON.stringify(metrics, "\t")
    
    var file = FileAccess.open("user://metrics.json", FileAccess.WRITE)
    file.store_string(json)
    file.close()
```

## Performance Tips

1. **Collection Frequency**: The default 100ms interval is optimal. Don't reduce unless necessary.

2. **Detailed Timing**: Enable only for systems you're actively profiling.

3. **Sample Count**: Default sample count is sufficient. Higher counts use more memory.

4. **Inactive Systems**: Zero-activity systems are automatically filtered from display.

## Troubleshooting

### Start Button Disabled or Not Working?

- Restart the editor if the button is greyed out
- Check console for initialization messages
- Verify FlecsServer singleton exists: `Engine.has_singleton("FlecsServer")`

### No Systems Appearing?

- **Click the Start button first** to begin profiling
- Run your scene (F5 or F6) - profiler only shows data while scene is running
- Verify systems are registered: `flecs_server.get_all_systems(world_id)`
- Ensure systems have matching entities
- Check systems aren't paused

### "Remote Connection" Issues?

**This is expected.** The Flecs Profiler is for **local editor profiling only**:
- Does NOT connect to remote game instances
- Does NOT profile exported builds
- Profiles systems running in the editor when you run scenes (F5/F6)
- The "Start" button controls local data collection

### All Metrics Show Zero?

- Systems may be paused
- No entities match system queries
- World not progressing (verify `progress_world()` is called)

### Missing Detailed Statistics?

- Enable detailed timing first
- Systems must execute at least once
- Check sample count is set

## File Locations

- **API Implementation**: `modules/godot_turbo/ecs/flecs_types/flecs_server.cpp`
- **Plugin Implementation**: `modules/godot_turbo/editor/flecs_profiler_plugin.cpp`
- **Full Documentation**: `modules/godot_turbo/docs/PROFILER_API.md`
- **Example Scripts**: `modules/godot_turbo/docs/examples/profiler_example.gd`
- **Troubleshooting Guide**: `modules/godot_turbo/docs/PROFILER_TROUBLESHOOTING.md`

## Related Methods

- `get_system_metrics(world_id)` - Get per-system metrics
- `get_world_frame_summary(world_id)` - Get aggregated summary
- `get_world_distribution_summary(world_id)` - Get distribution stats
- `get_all_systems(world_id)` - List all systems
- `set_script_system_detailed_timing(world_id, system_id, enabled)` - Toggle detailed timing
- `is_script_system_paused(world_id, system_id)` - Check pause state

## Support

For issues or questions:
1. Check the full documentation in `PROFILER_API.md`
2. Review example scripts in `examples/profiler_example.gd`
3. Check implementation notes in `PROFILER_IMPLEMENTATION.md`

---

**Status**: ✅ Implemented and Working  
**Version**: 1.2  
**Last Updated**: 2025-01-29