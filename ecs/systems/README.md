# ECS Systems Module

Comprehensive Entity-Component-System infrastructure for Godot using Flecs.

## 📚 Documentation Index

### Quick Links

- **[Master Documentation](SYSTEMS_DOCUMENTATION.md)** - Complete reference for all systems
- **[Testing Guide](tests/README.md)** - How to run and write tests
- **[Documentation Summary](DOCUMENTATION_SUMMARY.md)** - What's been documented and tested

### System Documentation

| System | Header | Documentation | Tests |
|--------|--------|---------------|-------|
| **PipelineManager** | [pipeline_manager.h](pipeline_manager.h) | In header | [test_pipeline_manager.h](tests/test_pipeline_manager.h) |
| **CommandQueue** | [command.h](command.h) | In header | [test_command.h](tests/test_command.h) |
| **GDScriptRunnerSystem** | [gdscript_runner_system.h](gdscript_runner_system.h) | In header + [GDSCRIPT_RUNNER_SYSTEM.md](GDSCRIPT_RUNNER_SYSTEM.md) | [test_gdscript_runner_system.h](tests/test_gdscript_runner_system.h) |
| **BadAppleSystem** | [demo/bad_apple_system.h](demo/bad_apple_system.h) | [OPTIMIZATION_COMPLETE.md](demo/OPTIMIZATION_COMPLETE.md) | N/A (demo) |

---

## 🚀 Quick Start

### 1. Basic ECS Setup

```cpp
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"

// Create world
FlecsServer* server = FlecsServer::get_singleton();
RID world_rid = server->create_world();
flecs::world* world = server->_get_world(world_rid);

// Setup pipeline
PipelineManager pipeline(world_rid);

// Register components
world->component<TransformComponent>();

// Create system
flecs::system move_system = world->system<TransformComponent>()
    .iter([](flecs::iter& it, TransformComponent* transforms) {
        // System logic
    });

pipeline.add_to_pipeline(move_system);

// Run
world->progress(delta_time);
```

### 2. Using Command Queue

```cpp
#include "modules/godot_turbo/ecs/systems/command.h"

Ref<CommandHandler> handler = memnew(CommandHandler);

// Enqueue from any thread
handler->enqueue_command([data]() {
    // Executed later on main thread
});

// Process each frame
handler->process_commands();
```

### 3. Script Integration

```cpp
#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"

GDScriptRunnerSystem* runner = memnew(GDScriptRunnerSystem);
runner->init(world_rid, world);

// Scripts can now implement _flecs_process(entity_rid, delta)
```

---

## 📖 What's in This Module

### Core Systems

- **PipelineManager** - Controls system execution order and phases
  - Register systems to execution phases
  - Create custom phases with dependencies
  - Look up systems by name

- **CommandQueue** - Thread-safe deferred command execution
  - Lock-free multi-producer queue
  - Object pooling for performance
  - Type-erased command interface

- **GDScriptRunnerSystem** - Bridge between scripts and ECS
  - Execute GDScript/C# methods on entities
  - Method caching for performance
  - Multi-phase support (process, physics)

### Demo Systems

- **BadAppleSystem** - Optimized video renderer demo
  - Multi-threaded pixel processing
  - Template-based format-specific optimizations
  - Row-based processing (eliminates per-pixel division)
  - 12x+ performance improvement

---

## 🧪 Testing

### Run All Tests

```bash
scons tests=yes target=editor dev_build=yes
./bin/godot.linuxbsd.editor.dev.x86_64 --test
```

### Run Specific System Tests

```bash
# PipelineManager tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager]*"

# Command system tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[Command]*"

# GDScriptRunner tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[GDScriptRunnerSystem]*"
```

### Test Coverage

| System | Test Cases | Coverage |
|--------|-----------|----------|
| PipelineManager | 20+ | 90%+ |
| CommandQueue | 40+ | 95%+ |
| GDScriptRunnerSystem | 30+ | 85%+ |
| **Total** | **90+** | **90%+** |

---

## 📊 Performance

### CommandQueue

- **Enqueue:** ~50-100ns (lock-free)
- **Pool allocation:** ~10-20ns
- **Process overhead:** ~30ns per command
- **Throughput:** 10,000+ commands/frame

### GDScriptRunnerSystem

- **Method cache:** Once per script type
- **Cache lookup:** ~10ns per entity
- **Method call:** ~500ns per entity

### BadAppleSystem

- **Baseline:** 5 FPS (6400 instances)
- **Optimized:** 60+ FPS (12x improvement)
- **Template optimizations:** 2-3x speedup
- **Multi-threading:** Near-linear scaling

---

## 🏗️ Architecture

### Execution Flow

```
FlecsServer::progress()
    │
    ├─> PipelineManager
    │   ├─> OnLoad Phase
    │   ├─> OnUpdate Phase
    │   │   ├─> GDScriptRunnerSystem
    │   │   └─> Custom Systems
    │   └─> OnPhysicsUpdate Phase
    │
    └─> CommandHandler::process_commands()
```

### Thread Safety

| Component | Multi-thread Safe? | Notes |
|-----------|-------------------|-------|
| PipelineManager | ⚠️ Main thread only | Don't modify during progress |
| CommandQueue | ✅ Enqueue from any thread | Process from single thread |
| GDScriptRunnerSystem | ⚠️ Main thread only | Scripts must be thread-safe |

---

## 📝 Best Practices

### Do ✅

- Use descriptive system names
- Process commands once per frame
- Cache component lookups in scripts
- Write unit tests for new systems
- Profile before optimizing

### Don't ❌

- Add systems during world progress
- Process commands from multiple threads
- Capture stack references in commands
- Skip teardown in tests
- Ignore thread safety

---

## 📂 Directory Structure

```
systems/
├── README.md                          (this file)
├── SYSTEMS_DOCUMENTATION.md           (master reference)
├── DOCUMENTATION_SUMMARY.md           (completion summary)
│
├── pipeline_manager.h/.cpp            (pipeline management)
├── command.h                          (command queue system)
├── gdscript_runner_system.h/.cpp      (script bridge)
├── GDSCRIPT_RUNNER_SYSTEM.md          (script docs)
│
├── demo/
│   ├── bad_apple_system.h/.cpp        (optimized demo)
│   └── docs/
│       └── OPTIMIZATION_COMPLETE.md   (optimization guide)
│
└── tests/
    ├── README.md                      (testing guide)
    ├── test_pipeline_manager.h        (20+ tests)
    ├── test_command.h                 (40+ tests)
    └── test_gdscript_runner_system.h  (30+ tests)
```

---

## 🔗 Related Documentation

- [Flecs Documentation](https://www.flecs.dev/flecs/)
- [Godot Unit Testing](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/unit_testing.html)
- [GodotTurbo ECS Module](../../README.md)

---

## 📈 Statistics

- **Total Documentation:** 3,441+ lines
- **API Documentation:** 550+ lines
- **Unit Tests:** 1,800+ lines (90+ test cases)
- **Guides:** 1,091+ lines
- **Code Coverage:** 90%+

---

## 🤝 Contributing

When adding new systems:

1. Add comprehensive header documentation
2. Write unit tests (aim for 80%+ coverage)
3. Update this README
4. Add examples to SYSTEMS_DOCUMENTATION.md
5. Run full test suite before PR

---

## 📜 License

MIT License - See project root for details

---

**Version:** 1.0  
**Last Updated:** 2025-01-28  
**Maintainers:** ProjectVYGR Team