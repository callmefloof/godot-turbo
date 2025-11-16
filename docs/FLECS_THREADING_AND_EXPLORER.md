# Flecs Multithreading and REST Explorer

## Overview

This document explains how Flecs' built-in multithreading system works and how to access the Flecs REST explorer for debugging and inspecting your ECS world.

## Flecs Multithreading

### How It Works

Flecs has a built-in multithreading system that is separate from Godot's `WorkerThreadPool`. The multithreading is configured at two levels:

1. **World-level thread pool**: Configured via `world.set_threads(n)`
2. **System-level threading**: Individual systems opt-in via `.multi_threaded(true)`

### Configuration

#### World Thread Pool

When you call `FlecsServer.init_world()`, Flecs automatically:
- Detects hardware concurrency (number of CPU threads)
- Creates a thread pool with `world.set_threads(threads)`
- This pool is used by all systems marked as multi-threaded

```cpp
// In flecs_server.cpp init_world()
auto threads = std::thread::hardware_concurrency();
world.set_threads(threads);
```

#### System-level Threading

To make a Flecs system run in parallel across multiple threads:

```gdscript
# Enable multi-threading for a script system
var system_inspector = FlecsServer.get_script_system_inspector(world_id, system_id)
system_inspector.multi_threaded = true
```

Or in C++:
```cpp
// In flecs_script_system.cpp
if (required_components.size() > 0 && multi_threaded) {
    builder.multi_threaded(true);
}
```

### Important Notes

**DO NOT mix Flecs threading with Godot's WorkerThreadPool** for ECS systems:
- ✅ Use Flecs' `.multi_threaded(true)` for ECS queries/systems
- ❌ Don't use `WorkerThreadPool` to manually iterate entities
- ✅ Let Flecs handle the parallelization automatically
- ✅ Create task entities if you need to parallelize non-entity work

When a system is marked as multi-threaded:
- Flecs automatically splits the entity iteration across threads
- Each thread processes a chunk of entities
- Flecs handles synchronization and work distribution
- No need for manual thread pools or chunk calculations

### Example: BadAppleSystem Refactor

**Before (wrong - using WorkerThreadPool):**
```cpp
WorkerThreadPool* pool = WorkerThreadPool::get_singleton();
WorkerThreadPool::GroupID group_id = pool->add_template_group_task(
    this, &BadAppleSystem::process_chunk, data, num_threads, -1, true
);
pool->wait_for_group_task_completion(group_id);
```

**After (correct - using Flecs):**
```cpp
// 1. Create task entities for parallel work
for (uint32_t i = 0; i < instance_count; ++i) {
    world->entity().set<PixelProcessTask>({i, &data, mode, Color()});
}

// 2. Multi-threaded system processes tasks in parallel
auto process_system = world->system<PixelProcessTask>()
    .multi_threaded(true)
    .each([](flecs::entity e, PixelProcessTask& task) {
        // Process task - Flecs handles threading
    });

// 3. Flush system collects results on main thread
auto flush_system = world->system<>().run([](flecs::iter it) {
    // Collect results from all task entities
});
```

### Batch Mode with Multi-threading

When using multi-threaded systems with batching:
```gdscript
system_inspector.multi_threaded = true
system_inspector.dispatch_mode = FlecsScriptSystem.DISPATCH_BATCH
system_inspector.batch_chunk_size = 1000
```

This configuration:
1. Splits entities across worker threads
2. Each thread accumulates results in thread-local batches
3. A flush system (running on main thread) collects all batches
4. Calls your GDScript callback with batched data

## Flecs REST Explorer

### What is it?

The Flecs REST explorer is a web-based debugging tool that lets you:
- Inspect all entities in your world
- View component data in real-time
- Monitor system performance
- Query entities with filters
- Visualize entity relationships

### Configuration

The REST API is configured in `init_world()`:

```cpp
// Configure REST API with default port
world.set<flecs::Rest>({.port = 27750});
```

### Accessing the Explorer

1. **Start your Godot project** with a Flecs world initialized
2. **Open your browser** to: http://localhost:27750
3. **Explore your ECS world** through the web interface

### Features

- **Entity Browser**: View all entities and their components
- **Query Builder**: Filter entities by components
- **Component Inspector**: See live component data
- **System Performance**: Monitor system execution times
- **Statistics**: Track entity counts, memory usage, etc.

### Port Configuration

Default port: `27750`

To change the port, modify `flecs_server.cpp`:
```cpp
world.set<flecs::Rest>({.port = YOUR_PORT});
```

### Troubleshooting

**Explorer not accessible?**
- Check console for "Flecs REST explorer available at http://localhost:27750"
- Ensure no firewall is blocking the port
- Verify `FLECS_REST` is defined in the build (it is by default)

**Performance concerns?**
- The REST API has minimal overhead when not actively queried
- Stats collection can be disabled with `FLECS_DISABLE_COUNTERS` if needed
- Explorer is mainly for development/debugging, not production

## Best Practices

### For Performance

1. **Use multi-threading for heavy systems**:
   ```gdscript
   # System processing 10,000+ entities
   system_inspector.multi_threaded = true
   ```

2. **Combine with batch mode for GDScript callbacks**:
   ```gdscript
   system_inspector.multi_threaded = true
   system_inspector.dispatch_mode = FlecsScriptSystem.DISPATCH_BATCH
   ```

3. **For non-entity parallel work, use task entities**:
   - Create temporary entities with task components
   - Use multi-threaded systems to process them
   - Clean up task entities when done
   - See `BadAppleSystem` for a complete example

4. **Profile with the explorer**:
   - Open http://localhost:27750
   - Navigate to "Systems" to see execution times
   - Identify bottlenecks

### Common Pitfalls

❌ **Don't use static queries in system lambdas:**
```cpp
// WRONG - can cause reflection errors
auto system = world->system().run([](flecs::iter it) {
    static flecs::query<Component> q = world->query<Component>(); // BAD!
});
```

✅ **Use world->each() or create queries outside:**
```cpp
// RIGHT - create query outside system
auto query = world->query<Component>();
auto system = world->system().run([&query](flecs::iter it) {
    query.each([](flecs::entity e, Component& c) { });
});

// OR use world->each() directly
auto system = world->system().run([world](flecs::iter it) {
    world->each([](flecs::entity e, Component& c) { });
});
```

### For Debugging

1. **Use the explorer to inspect entity state**
2. **Monitor component values in real-time**
3. **Track system execution order and timing**
4. **Verify entity-component relationships**

## References

- [Flecs Threading Documentation](https://www.flecs.dev/flecs/md_docs_Manual.html#multithreading)
- [Flecs REST API](https://www.flecs.dev/explorer/)
- [Flecs Explorer UI](https://www.flecs.dev/explorer/)

## Summary

- **Flecs has its own threading system** - use it instead of Godot's WorkerThreadPool for ECS
- **Enable per-system** with `.multi_threaded(true)`
- **Thread pool is configured** via `world.set_threads(n)` in `init_world()`
- **For parallel non-entity work** - create task entities and use multi-threaded systems
- **Avoid static queries** in system lambdas (causes reflection errors)
- **REST explorer runs on port 27750** by default
- **Access at http://localhost:27750** for debugging and inspection

## Real-World Example: BadAppleSystem

The `BadAppleSystem` demonstrates the complete pattern:
1. Main system creates `PixelProcessTask` entities for each pixel
2. Multi-threaded system processes each task in parallel
3. Flush system collects results and sends to RenderingServer
4. Task entities are cleaned up each frame

This pattern can be used anywhere you need to parallelize work that doesn't naturally fit the entity-component model.