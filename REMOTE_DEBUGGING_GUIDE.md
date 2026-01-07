# Flecs Remote Debugging Guide

This guide explains how to use the remote debugging features for Flecs ECS worlds in Godot.

## Overview

The Flecs module now supports full remote debugging, allowing you to inspect and profile Flecs worlds running in a remote game instance from the Godot editor. This is particularly useful for:

- Debugging deployed builds
- Profiling performance on target devices
- Inspecting world state without stopping the game
- Monitoring system execution times in real-time

## Architecture

The remote debugging system consists of three main components:

### 1. Runtime Debugger (`FlecsRuntimeDebugger`)

Located in `runtime/flecs_runtime_debugger.cpp`, this component runs in the game instance and:

- Registers message handlers with Godot's `EngineDebugger`
- Responds to editor requests for world data
- Sends profiler metrics back to the editor
- Serializes entity and component information

**Supported Messages:**
- `flecs:request_worlds` - Get list of all Flecs worlds
- `flecs:request_entities` - Get entities in a specific world
- `flecs:request_components` - Get components for a specific entity
- `flecs:request_profiler_metrics` - Get system profiling data

### 2. Editor Debugger Plugin (`FlecsDebuggerPlugin`)

Located in `editor/flecs_debugger_plugin.cpp`, this EditorPlugin provides the UI in the editor:

- Creates a "Flecs Worlds" tab in the debugger panel
- Displays world hierarchy and entities
- Shows a profiler tab with system metrics
- Handles message capture from runtime

**UI Features:**
- **Worlds Tree**: Hierarchical view of all worlds and their entities
- **Components Tab**: View entity components and their data
- **Stats Tab**: World statistics and information
- **Profiler Tab**: Real-time system performance metrics

### 3. Profiler Integration (`FlecsProfiler`)

The existing local profiler (`editor/flecs_profiler.cpp`) can also be used for local debugging when running in the editor.

## Usage

### Remote Debugging Setup

1. **Start Your Game with Debugger Enabled:**
   ```bash
   ./your_game --remote-debug <editor-host>:<port>
   ```

2. **Open the Debugger in Editor:**
   - Run > Debug Remote Instance
   - Or click the "Remote" button in the debugger panel

3. **Access Flecs Debugger:**
   - Open the "Debugger" bottom panel
   - Look for the "Flecs Worlds" tab
   - Click "Refresh" to load world list

### Inspecting Worlds

1. Click on a world in the tree to select it
2. The world will expand to show entities (paginated)
3. Click on an entity to view its components in the right panel
4. Switch between "Components", "Stats", and "Profiler" tabs

### Using the Profiler Tab

The Profiler tab shows real-time metrics for all systems in the selected world:

**Columns:**
- **System Name**: The name of the system/query
- **Time (ms)**: Total execution time in milliseconds
- **Calls**: Number of times the system was invoked
- **Entities**: Number of entities processed
- **Min (µs)**: Minimum execution time in microseconds
- **Max (µs)**: Maximum execution time in microseconds

**Workflow:**
1. Select a world from the worlds tree
2. Switch to the "Profiler" tab
3. Profiler metrics will automatically refresh when world is selected
4. Click "Refresh Metrics" button to manually update
5. Look for systems with high execution times or unusual patterns

### Performance Tips

- Use batch size spinner to control how many entities are loaded at once
- Profiler data is only collected when `get_system_metrics()` is called
- Remote debugging adds minimal overhead when not actively inspecting
- Profiler metrics use the same API as local profiling

## API Reference

### FlecsRuntimeDebugger

```cpp
// Initialize the runtime debugger (called automatically at module init)
void initialize();

// Shutdown the runtime debugger
void shutdown();
```

The runtime debugger is automatically initialized when the Flecs module loads.

### FlecsServer Profiling API

```gdscript
# Get system metrics for a world
var metrics = FlecsServer.get_system_metrics(world_rid)

# Returns a Dictionary with:
# {
#   "systems": Array[Dictionary],  # Array of system metrics
#   "total_time_usec": int,        # Total execution time
#   "system_count": int            # Number of systems
# }

# Each system dictionary contains:
# {
#   "name": String,
#   "total_time_usec": int,
#   "call_count": int,
#   "entity_count": int,
#   "min_time_usec": int,
#   "max_time_usec": int,
#   "median_usec": float,      # If detailed timing enabled
#   "stddev_usec": float,      # If detailed timing enabled
#   "p99_usec": float          # If detailed timing enabled
# }
```

## Message Protocol

### Request Messages (Editor → Runtime)

**flecs:request_worlds**
```
Args: []
Response: {
  "type": "world_list",
  "worlds": [
    {
      "id": uint64,
      "name": String,
      "entity_count": int
    },
    ...
  ]
}
```

**flecs:request_entities**
```
Args: [world_id: uint64, offset: int, count: int]
Response: {
  "type": "entities",
  "world_id": uint64,
  "entities": Array[Dictionary],
  "offset": int,
  "count": int
}
```

**flecs:request_profiler_metrics**
```
Args: [world_id: uint64]
Response: {
  "type": "profiler_metrics",
  "world_id": uint64,
  "systems": Array[Dictionary],
  "total_time_usec": int,
  "system_count": int
}
```

## Troubleshooting

### Debugger Not Connecting
- Ensure the game is running with `--remote-debug` flag
- Check firewall settings allow the debug port (default 6007)
- Verify the editor's "Remote" debugger is active

### No Worlds Showing
- Click "Refresh" button to manually request worlds
- Check that your game has created Flecs worlds
- Verify `FlecsServer` singleton is accessible in runtime

### Profiler Metrics Empty
- Ensure the selected world has active systems
- Systems must be invoked at least once to have metrics
- Check that `get_system_metrics()` returns valid data locally

### High Memory Usage
- Reduce batch size for entity loading
- Avoid keeping large numbers of worlds expanded
- Profile data is aggregated per-system, not per-frame

## Best Practices

1. **Use Remote Debugging for Target Devices**: Test performance on actual hardware
2. **Profile Before Optimizing**: Let the metrics guide your optimization efforts
3. **Watch for Outliers**: Systems with high max times may have performance spikes
4. **Monitor Entity Count**: Systems processing many entities may need optimization
5. **Compare Local vs Remote**: Check if performance differs between editor and runtime

## Implementation Notes

### Safety Considerations

The remote debugger implementation includes several safety measures:

- All string construction uses safe Godot String methods
- Dictionary/Array marshalling is validated before transmission
- Message handlers check for null/invalid data
- Session data is properly cleaned up when debugger disconnects

### Performance Impact

- Message handling adds ~0.1ms overhead per request
- Profiler metrics collection reuses existing `FlecsServer` API
- No continuous polling - data is only sent on request
- UI updates are throttled to prevent editor lag

### Future Enhancements

Potential improvements for future versions:

- [ ] Auto-refresh option for profiler metrics
- [ ] Historical profiler data visualization
- [ ] Entity filtering and search
- [ ] Component editing in remote session
- [ ] System pause/resume control
- [ ] Frame-by-frame stepping
- [ ] Performance comparison between sessions

## See Also

- [FlecsProfiler Documentation](FLECS_PROFILER.md)
- [Godot Debugger Documentation](https://docs.godotengine.org/en/stable/tutorials/scripting/debug/overview_of_debugging_tools.html)
- [Flecs Systems Guide](https://www.flecs.dev/flecs/md_docs_Systems.html)