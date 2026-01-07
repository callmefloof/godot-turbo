# Flecs Profiler API Documentation

## Overview

The Flecs profiler integration provides real-time performance monitoring for Flecs ECS systems in the Godot editor. This document describes the profiling API implemented in `FlecsServer` and how it's used by the `FlecsProfilerPlugin`.

## Architecture

The profiling system consists of three main components:

1. **FlecsServer API** - Collects and exposes system metrics
2. **FlecsProfilerPlugin** - Editor plugin that displays the metrics
3. **EditorProfiler** - Built-in Godot profiler widget (reused for display)

## FlecsServer Profiling API

### `get_system_metrics(RID world_id) -> Dictionary`

Returns comprehensive profiling metrics for all systems (both C++ and script systems) in the specified world.

#### Return Value Structure

```gdscript
{
    "systems": [  # Array of system metrics
        {
            "rid": RID,              # System RID
            "name": String,          # System name
            "type": String,          # "script" or "cpp"
            
            # Timing metrics (in microseconds)
            "time_usec": int,        # Last frame execution time
            "call_count": int,       # Number of invocations this frame
            "total_time_usec": int,  # Accumulated time this frame
            "avg_time_usec": int,    # Average time per invocation
            "min_time_usec": int,    # Minimum invocation time
            "max_time_usec": int,    # Maximum invocation time
            
            # Entity metrics
            "entity_count": int,     # Entities processed this frame
            
            # Event counts (script systems only)
            "onadd_count": int,      # OnAdd events
            "onset_count": int,      # OnSet events
            "onremove_count": int,   # OnRemove events
            
            # State
            "paused": bool,          # Is system paused?
            "dispatch_mode": int,    # Dispatch mode (script systems)
            
            # Lifetime statistics
            "total_callbacks": int,           # Total callbacks invoked (all time)
            "total_entities_processed": int,  # Total entities processed (all time)
            
            # Advanced timing (if detailed_timing enabled)
            "median_usec": float,    # Median invocation time
            "p99_usec": float,       # 99th percentile time
            "stddev_usec": float     # Standard deviation
        },
        # ... more systems
    ],
    "frame_count": int  # Current frame number
}
```

#### Example Usage

```gdscript
# Get the FlecsServer singleton
var flecs_server = Engine.get_singleton("FlecsServer")

# Get all worlds
var worlds = flecs_server.get_world_list()

# Get metrics for the first world
if worlds.size() > 0:
    var metrics = flecs_server.get_system_metrics(worlds[0])
    
    # Iterate through systems
    for system in metrics["systems"]:
        print("System: ", system["name"])
        print("  Type: ", system["type"])
        print("  Time: ", system["time_usec"], " µs")
        print("  Entities: ", system["entity_count"])
```

### Related Methods

#### `get_world_frame_summary(RID world_id) -> Dictionary`

Returns aggregated per-frame summary statistics for all script systems in a world.

```gdscript
{
    "script_systems": int,               # Number of script systems
    "total_entities_this_frame": int,    # Total entities processed
    "total_callbacks_all_time": int,     # Lifetime callback count
    "batch_system_count": int,           # Number of batch-mode systems
    "max_dispatch_usec": int,            # Maximum system time this frame
    "dispatch_invocations": int,         # Total invocations this frame
    "dispatch_accum_usec": int,          # Total accumulated time
    "dispatch_avg_usec": int,            # Average time per invocation
    "systems": [                         # Per-system breakdown
        {
            "rid": RID,
            "entities": int,
            "last_dispatch_usec": int,
            "dispatch_invocations": int,
            "dispatch_accum_usec": int,
            "dispatch_avg_usec": int,
            "mode": int,
            "min_dispatch_usec": int,
            "max_dispatch_usec_system": int,
            "median_dispatch_usec": float,  # If detailed timing enabled
            "p99_dispatch_usec": float,
            "stddev_dispatch_usec": float,
            "onadd": int,
            "onset": int,
            "onremove": int
        }
    ]
}
```

#### `reset_world_frame_summary(RID world_id)`

Clears the cached frame summary for the specified world.

#### `get_world_distribution_summary(RID world_id) -> Dictionary`

Returns approximate distribution statistics across all script systems.

```gdscript
{
    "median_usec": float,        # Median execution time
    "p99_usec": float,           # 99th percentile
    "mean_usec": float,          # Mean execution time
    "stddev_usec": float,        # Standard deviation
    "samples_used": int,         # Number of samples in calculation
    "total_invocations": int,    # Total invocations
    "approximation_cap": int     # Sample limit (4096)
}
```

## Script System Profiling Configuration

### Enabling Detailed Timing

To get median, p99, and standard deviation metrics, enable detailed timing:

```gdscript
flecs_server.set_script_system_detailed_timing(world_id, system_id, true)
```

**Note:** Detailed timing has a small performance overhead as it stores timing samples for each invocation.

### Setting Sample Count

Control the number of timing samples to keep:

```gdscript
# Default is usually around 128-256 samples
flecs_server.set_script_system_max_sample_count(world_id, system_id, 512)
```

### Enabling Instrumentation

Basic instrumentation is automatically enabled, but you can toggle it:

```gdscript
flecs_server.set_script_system_instrumentation(world_id, system_id, true)
```

## FlecsProfilerPlugin Usage

The profiler plugin automatically:

1. Connects to FlecsServer on initialization
2. Starts collecting metrics every 100ms
3. Displays systems in the EditorProfiler widget
4. Shows entity counts in the system names

### Programmatic Control

```gdscript
# Get the plugin (if you need to control it)
var profiler_plugin = # ... get plugin reference
profiler_plugin.enable_profiling(true)   # Enable
profiler_plugin.enable_profiling(false)  # Disable
```

## C++ Systems vs Script Systems

### Script Systems
- Full profiling support with timing metrics
- Entity counts, event counts (OnAdd/OnSet/OnRemove)
- Detailed timing statistics (when enabled)
- Lifetime statistics

### C++ Systems
- Basic information only (name, RID, type)
- Timing metrics not currently tracked
- Pause state available

To add profiling to C++ systems, you would need to instrument them manually using Flecs' built-in profiling hooks or custom timing code.

## Performance Considerations

1. **Collection Frequency**: The default 100ms collection interval is a good balance. More frequent collection increases overhead.

2. **Detailed Timing**: Enabling detailed timing on many systems can impact performance. Use selectively on systems you want to profile deeply.

3. **Sample Limits**: Higher sample counts use more memory but provide more accurate statistics.

4. **Zero-Activity Systems**: Systems with no activity (0 time, 0 calls) are automatically filtered out of the profiler display.

## Troubleshooting

### No Systems Showing in Profiler

1. Check that worlds exist: `flecs_server.get_world_list().size() > 0`
2. Verify systems are registered and running
3. Ensure systems are actually executing (not paused, have matching entities)
4. Check console for error messages

### Metrics Show Zero

1. Systems may be paused: check `is_script_system_paused()`
2. No matching entities for the system's query
3. World not progressing: verify `progress_world()` is being called

### Missing Detailed Statistics

1. Enable detailed timing: `set_script_system_detailed_timing(world_id, system_id, true)`
2. Check that systems have been invoked at least once this frame
3. Ensure sample count is sufficient: `set_script_system_max_sample_count()`

## Example: Custom Profiler Display

```gdscript
extends Node

var flecs_server

func _ready():
    flecs_server = Engine.get_singleton("FlecsServer")

func _process(delta):
    var worlds = flecs_server.get_world_list()
    if worlds.is_empty():
        return
    
    var metrics = flecs_server.get_system_metrics(worlds[0])
    
    # Find slowest system
    var slowest = null
    var max_time = 0
    
    for system in metrics["systems"]:
        if system["time_usec"] > max_time:
            max_time = system["time_usec"]
            slowest = system
    
    if slowest:
        print("Slowest system: ", slowest["name"], " - ", max_time, "µs")
```

## Future Enhancements

Potential improvements for the profiling API:

1. **C++ System Timing** - Add instrumentation for C++ systems
2. **Query Profiling** - Profile query execution time separately
3. **Memory Profiling** - Track component memory usage
4. **Historical Data** - Store and display trends over time
5. **Export/Import** - Save profiling sessions for comparison
6. **Remote Profiling** - Profile running game instances

## See Also

- `flecs_profiler_plugin.h` - Editor plugin implementation
- `flecs_profiler_plugin.cpp` - Plugin implementation details
- `flecs_script_system.h` - Script system instrumentation
- Godot EditorProfiler documentation