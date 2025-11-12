# ECS Systems Documentation

Complete documentation for the ECS systems in the `godot_turbo` module.

## Table of Contents

- [Overview](#overview)
- [System Classes](#system-classes)
  - [PipelineManager](#pipelinemanager)
  - [CommandQueue/CommandHandler](#commandqueuecommandhandler)
  - [GDScriptRunnerSystem](#gdscriptrunnersystem)
  - [BadAppleSystem](#badapplesystem)
- [Architecture](#architecture)
- [Quick Start](#quick-start)
- [Testing](#testing)
- [Performance Considerations](#performance-considerations)
- [Best Practices](#best-practices)

---

## Overview

The ECS systems module provides core infrastructure for managing entity-component-system workflows in Godot using the Flecs library. The module includes:

- **Pipeline Management** - Control system execution order and phases
- **Command Queue** - Thread-safe deferred command execution
- **Script Integration** - Bridge between GDScript and ECS
- **Demo Systems** - Example implementations (Bad Apple video player)

### Key Features

✅ **Lock-free concurrency** - Thread-safe command queues  
✅ **Flexible pipeline** - Custom phases and execution order  
✅ **Script bridge** - Call GDScript/C# methods from ECS  
✅ **High performance** - SIMD optimizations, pooling, batching  
✅ **Comprehensive tests** - 90+ unit tests across all systems  

---

## System Classes

### PipelineManager

**File:** `pipeline_manager.h/.cpp`  
**Purpose:** Manages Flecs ECS pipeline and system execution order

#### Features

- Register systems to execution phases (OnUpdate, OnPhysicsUpdate, etc.)
- Create custom phases with dependencies
- Look up systems by name
- Multi-world support

#### Basic Usage

```cpp
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"

// Create pipeline for a world
RID world_rid = flecs_server->create_world();
flecs::world* world = flecs_server->_get_world(world_rid);
PipelineManager pipeline(world_rid);

// Register a system to default phase (OnUpdate)
flecs::system my_system = world->system<TransformComponent>()
    .iter([](flecs::iter& it, TransformComponent* transforms) {
        // System logic
    });
pipeline.add_to_pipeline(my_system);

// Register to specific phase
flecs::system physics_sys = world->system<RigidBodyComponent>()
    .iter([](flecs::iter& it, RigidBodyComponent* bodies) {
        // Physics logic
    });
pipeline.add_to_pipeline(physics_sys, flecs::OnPhysicsUpdate);

// Create custom phase
flecs::entity late_update = pipeline.create_custom_phase("LateUpdate", "OnUpdate");
pipeline.add_to_pipeline(another_system, late_update);
```

#### API Reference

| Method | Description |
|--------|-------------|
| `PipelineManager(RID world_rid)` | Create manager for world |
| `add_to_pipeline(system)` | Add system to OnUpdate phase |
| `add_to_pipeline(system, phase)` | Add system to specific phase |
| `create_custom_phase(name, depends_on)` | Create custom execution phase |
| `try_get_system(name)` | Find system by name |
| `set_world(world_rid)` | Change associated world |
| `get_world()` | Get current world RID |

#### Execution Phases

**Built-in Flecs Phases (in order):**

1. `flecs::OnLoad` - Pre-frame initialization
2. `flecs::PostLoad` - After loading
3. `flecs::PreUpdate` - Before main update
4. `flecs::OnUpdate` - Main game logic (default)
5. `flecs::OnValidate` - Validation/constraints
6. `flecs::PostUpdate` - After main update
7. `flecs::PreStore` - Before storage
8. `flecs::OnStore` - Store/serialize
9. `flecs::PostFrame` - End of frame cleanup
10. `flecs::OnPhysicsUpdate` - Physics simulation

#### Thread Safety

- ✅ Safe to call from main thread
- ⚠️ Do not add systems during world progress
- ⚠️ System lookup is not thread-safe

---

### CommandQueue/CommandHandler

**File:** `command.h`  
**Purpose:** Lock-free thread-safe command queue for deferred execution

#### Features

- **Object pooling** - Pre-allocated command objects (1024 per type)
- **Lock-free queue** - moodycamel::ConcurrentQueue for multi-producer safety
- **Type erasure** - Polymorphic ICommand interface
- **Thread-local tokens** - Reduced contention on enqueue

#### Architecture

```
┌─────────────┐
│  ICommand   │  Base interface
└──────┬──────┘
       │
       ├─────────────────┐
       │                 │
┌──────▼──────┐   ┌──────▼──────────┐
│  Command<F> │   │ UnpooledCommand │
│  (pooled)   │   │  (heap alloc)   │
└─────────────┘   └─────────────────┘
       │
       ▼
┌─────────────┐
│    Pool     │  Lock-free object pool
└─────────────┘
```

#### Basic Usage

```cpp
#include "modules/godot_turbo/ecs/systems/command.h"

// Create command handler
Ref<CommandHandler> handler = memnew(CommandHandler);

// Enqueue commands from any thread
handler->enqueue_command([captured_data]() {
    // This runs later on the processing thread
    print_line("Deferred execution!");
});

// Process all pending commands (call once per frame on main thread)
handler->process_commands();
```

#### Advanced Usage

```cpp
// Complex captures
struct GameData {
    Vector3 position;
    float health;
    String name;
};

GameData data = {Vector3(1, 2, 3), 100.0f, "Player"};

handler->enqueue_command([data]() {
    // 'data' is captured by value
    update_game_state(data);
});

// Unpooled commands (for debugging)
handler->enqueue_command_unpooled([large_data]() {
    // Uses heap allocation instead of pool
});

// Direct queue usage (without RefCounted wrapper)
CommandQueue queue;
queue.enqueue([]() { print_line("Command!"); });
queue.process();
```

#### API Reference

**CommandHandler**

| Method | Description |
|--------|-------------|
| `enqueue_command(lambda)` | Enqueue pooled command |
| `enqueue_command_unpooled(lambda)` | Enqueue unpooled command |
| `process_commands()` | Execute all pending commands |

**CommandQueue**

| Method | Description |
|--------|-------------|
| `enqueue(lambda)` | Enqueue pooled command |
| `enqueue_raw(ICommand*)` | Enqueue pre-constructed command |
| `process()` | Execute all pending commands |
| `is_empty()` | Check if queue is empty (approx) |

**Pool**

| Method | Description |
|--------|-------------|
| `Pool(slot_size, slot_count)` | Create pool |
| `allocate()` | Get slot (returns nullptr if full) |
| `deallocate(ptr)` | Return slot to pool |

#### Performance

- **Pool allocation:** ~10-20ns per command
- **Enqueue:** ~50-100ns (lock-free)
- **Process:** ~30ns per command overhead
- **Default pool:** 1024 commands per unique lambda type

#### Thread Safety

- ✅ `enqueue()` safe from any thread (multi-producer)
- ✅ `process()` should be called from single thread (single-consumer)
- ⚠️ Do not destroy queue while enqueueing
- ⚠️ `is_empty()` returns approximate result

---

### GDScriptRunnerSystem

**File:** `gdscript_runner_system.h/.cpp`  
**Purpose:** Executes GDScript/C# methods on entities

#### Features

- **Method caching** - Checks method existence once per script type
- **Multi-phase support** - Separate process and physics process
- **Dual conventions** - Supports GDScript and C# naming
- **Entity-aware** - Scripts receive entity RID for component access

#### Virtual Methods

Scripts attached to converted nodes can implement:

**GDScript:**
```gdscript
func _flecs_process(entity_rid: RID, delta: float) -> void:
    # Called every frame
    pass

func _flecs_physics_process(entity_rid: RID, delta: float) -> void:
    # Called at physics rate
    pass
```

**C#:**
```csharp
public void _FlecsProcess(Rid entityRid, float delta)
{
    // Called every frame
}

public void _FlecsPhysicsProcess(Rid entityRid, float delta)
{
    // Called at physics rate
}
```

#### Usage Example

**C++ (System Setup):**
```cpp
#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"

GDScriptRunnerSystem* runner = memnew(GDScriptRunnerSystem);
runner->init(world_rid, world);

// Systems automatically execute during world->progress()
```

**GDScript (Entity Script):**
```gdscript
extends Node

func _flecs_process(entity_rid: RID, delta: float) -> void:
    # Access components via FlecsServer
    var transform = FlecsServer.get_component_by_name(
        world, entity_rid, "Transform3DComponent"
    )
    
    # Modify component
    transform["position"] += Vector3.RIGHT * delta
    
    # Update component
    FlecsServer.set_component(
        world, entity_rid, "Transform3DComponent", transform
    )

func _flecs_physics_process(entity_rid: RID, delta: float) -> void:
    # Physics updates
    var body = FlecsServer.get_component_by_name(
        world, entity_rid, "RigidBodyComponent"
    )
    body["velocity"] += Vector3.DOWN * 9.8 * delta
    FlecsServer.set_component(
        world, entity_rid, "RigidBodyComponent", body
    )
```

#### API Reference

| Method | Description |
|--------|-------------|
| `init(world_rid, world*)` | Initialize system |
| `set_process_enabled(bool)` | Enable/disable process |
| `set_physics_process_enabled(bool)` | Enable/disable physics |
| `is_process_enabled()` | Check if process is enabled |
| `is_physics_process_enabled()` | Check if physics enabled |
| `clear_cache()` | Clear method cache |
| `get_cache_size()` | Get number of cached types |
| `is_cached(type_name)` | Check if type is cached |

#### Method Resolution

The system checks for methods in this order:

1. `_flecs_process` (GDScript convention)
2. `_FlecsProcess` (C# convention)
3. Cache result for future frames

#### Performance

- **Method check:** Once per script type (cached)
- **Method call:** ~500ns-1µs per entity
- **Cache lookup:** ~10ns per entity
- **Overhead:** Minimal after initial cache population

#### Thread Safety

- ✅ Safe when called from main thread
- ⚠️ Scripts must be thread-safe if using multi-threaded ECS
- ⚠️ Do not modify cache during entity iteration

---

### BadAppleSystem

**File:** `demo/bad_apple_system.h/.cpp`  
**Purpose:** Demo system - renders Bad Apple video using MultiMesh

#### Features

- **Multi-threaded** - Parallel pixel processing with WorkerThreadPool
- **SIMD optimized** - SSE2/NEON vectorization (4 pixels at a time)
- **Format-specific** - Optimized loops per image format
- **Configurable** - Threading threshold, max threads, display modes

#### Performance Optimizations

1. **Format-specific processing** - RGBA8, RGB8, fallback paths
2. **Fast luminance** - Integer arithmetic instead of float
3. **SIMD vectorization** - 4x speedup on supported platforms
4. **Multi-threading** - Near-linear scaling with cores
5. **Batch rendering** - Single RenderingServer command per frame

#### Usage Example

```cpp
#include "modules/godot_turbo/ecs/systems/demo/bad_apple_system.h"

BadAppleSystem* bad_apple = memnew(BadAppleSystem);
bad_apple->init(world_rid, world);

// Configure (optional)
bad_apple->set_use_multithreading(true);
bad_apple->set_threading_threshold(1000); // Thread for 1000+ instances
bad_apple->set_max_threads(4); // Use up to 4 worker threads
bad_apple->set_display_mode(BadAppleSystem::DISPLAY_MODE_REGULAR);

// System runs automatically during world->progress()
```

#### Performance Numbers

**Configuration:** 6400 instances (80x80 grid), 640x480 video

| Optimization | FPS | Speedup |
|--------------|-----|---------|
| Baseline | 5 FPS | 1x |
| Format-specific | 12 FPS | 2.4x |
| + SIMD | 23 FPS | 4.6x |
| + 4 threads | 60+ FPS | 12+x |

See `demo/docs/OPTIMIZATION_COMPLETE.md` for full details.

---

## Architecture

### System Execution Flow

```
┌─────────────────────────────────────────┐
│         FlecsServer::progress()         │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│      PipelineManager (controls order)   │
└────────────────┬────────────────────────┘
                 │
    ┌────────────┼────────────┐
    ▼            ▼            ▼
┌─────────┐ ┌─────────┐ ┌──────────┐
│ OnLoad  │ │OnUpdate │ │OnPhysics │
└────┬────┘ └────┬────┘ └────┬─────┘
     │           │           │
     ▼           ▼           ▼
┌─────────────────────────────────────────┐
│  Systems execute in phase order         │
│  - GDScriptRunnerSystem                 │
│  - BadAppleSystem                       │
│  - Custom systems                       │
└─────────────────────────────────────────┘
```

### Command Queue Flow

```
┌──────────────┐
│  Any Thread  │
└──────┬───────┘
       │ enqueue()
       ▼
┌──────────────────────┐
│  Lock-Free Queue     │
│  (ConcurrentQueue)   │
└──────┬───────────────┘
       │
       ▼
┌──────────────────────┐
│   Pool Allocation    │
│  (per-type pools)    │
└──────┬───────────────┘
       │
       │ process()
       ▼
┌──────────────────────┐
│   Main Thread        │
│   Execute commands   │
└──────────────────────┘
```

---

## Quick Start

### Minimal Example

```cpp
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"

// 1. Create world
FlecsServer* server = FlecsServer::get_singleton();
RID world_rid = server->create_world();
flecs::world* world = server->_get_world(world_rid);

// 2. Setup pipeline
PipelineManager pipeline(world_rid);

// 3. Register components
world->component<TransformComponent>();
world->component<VelocityComponent>();

// 4. Create systems
flecs::system move_system = world->system<TransformComponent, VelocityComponent>()
    .kind(flecs::OnUpdate)
    .iter([](flecs::iter& it, TransformComponent* transforms, VelocityComponent* velocities) {
        for (auto i : it) {
            transforms[i].position += velocities[i].velocity * it.delta_time();
        }
    });

pipeline.add_to_pipeline(move_system);

// 5. Setup script runner (optional)
GDScriptRunnerSystem* script_runner = memnew(GDScriptRunnerSystem);
script_runner->init(world_rid, world);

// 6. Create entities
flecs::entity player = world->entity()
    .set<TransformComponent>({Vector3(0, 0, 0)})
    .set<VelocityComponent>({Vector3(1, 0, 0)});

// 7. Run game loop
while (running) {
    world->progress(delta_time);
}
```

---

## Testing

### Running Tests

```bash
# Build with tests
scons tests=yes target=editor dev_build=yes

# Run all ECS system tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[Command]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[GDScriptRunnerSystem]*"
```

### Test Coverage

- **PipelineManager:** 20+ tests
- **CommandQueue:** 40+ tests
- **GDScriptRunnerSystem:** 30+ tests
- **Total:** 90+ unit tests

See `tests/README.md` for detailed testing documentation.

---

## Performance Considerations

### PipelineManager

- ✅ System lookup is O(n) - cache results if frequent
- ✅ System registration is O(1)
- ⚠️ Avoid adding systems during world progress

### CommandQueue

- ✅ Lock-free enqueue: ~50-100ns
- ✅ Pool allocation: ~10-20ns
- ⚠️ Pool exhaustion returns nullptr - check result!
- ⚠️ Each unique lambda type gets separate pool

### GDScriptRunnerSystem

- ✅ Method checks cached per script type
- ✅ Cache lookup: ~10ns per entity
- ⚠️ Method call overhead: ~500ns per entity
- ⚠️ Consider batching component updates in scripts

### BadAppleSystem

- ✅ SIMD: 4x speedup on x86/ARM
- ✅ Multi-threading: near-linear scaling
- ⚠️ Enable threading only for 1000+ instances
- ⚠️ SIMD requires aligned data for best performance

---

## Best Practices

### Pipeline Management

**✅ Do:**
- Use descriptive system names for debugging
- Group related systems in custom phases
- Set up pipeline before creating entities
- Use built-in phases when possible

**❌ Don't:**
- Add systems during world progress
- Create circular phase dependencies
- Register same system twice
- Modify pipeline from multiple threads

### Command Queue

**✅ Do:**
- Use pooled commands for hot paths
- Enqueue from any thread freely
- Process commands once per frame
- Check for pool exhaustion in critical code

**❌ Don't:**
- Process from multiple threads
- Capture stack references (use values)
- Assume immediate execution
- Create new queue per frame

### Script Integration

**✅ Do:**
- Cache component lookups in scripts
- Batch component updates
- Use clear_cache() after script reloads
- Check if methods exist before adding scripts

**❌ Don't:**
- Call FlecsServer methods in tight loops
- Modify component structure in scripts
- Assume scripts are always present
- Forget to handle missing components

### General

**✅ Do:**
- Profile before optimizing
- Use SIMD for pixel/batch processing
- Enable threading for large workloads
- Write unit tests for new systems

**❌ Don't:**
- Premature optimization
- Ignore thread safety
- Skip teardown in tests
- Leave resources allocated

---

## Additional Resources

- **Flecs Documentation:** https://www.flecs.dev/flecs/
- **Test Documentation:** `tests/README.md`
- **BadApple Optimization:** `demo/docs/OPTIMIZATION_COMPLETE.md`
- **GDScript Runner Guide:** `GDSCRIPT_RUNNER_SYSTEM.md`

---

**Version:** 1.0  
**Last Updated:** 2025-01-28  
**Maintainers:** ProjectVYGR Team  
**License:** MIT