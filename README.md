[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/callmefloof/godot-turbo)

# Godot Turbo ECS Module

A high-performance Entity Component System (ECS) module for Godot 4.4+, powered by [Flecs](https://github.com/SanderMertens/flecs).

## 🚀 Features

- **Flecs Integration** - Industry-leading ECS library with excellent performance
- **Godot Native** - Seamless integration with Godot's scene system and scripting
- **High Performance** - SIMD optimizations, multi-threading, and lock-free queues
- **Script Bridge** - Execute GDScript/C# methods on ECS entities
- **Flexible Pipeline** - Custom execution phases and system ordering
- **Comprehensive Documentation** - 90+ unit tests, detailed API docs, and examples
- **Production Ready** - Enterprise-level testing and documentation

## 📋 Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [Documentation](#documentation)
- [Examples](#examples)
- [Performance](#performance)
- [Changelog](#changelog)
- [Contributing](#contributing)
- [License](#license)

## 🔧 Installation

### Prerequisites

- **Godot 4.4+** (4.5 beta untested)
- **SCons** build system
- **C++17** compatible compiler

### Build Instructions

1. Clone the Godot repository:
   ```bash
   git clone https://github.com/godotengine/godot.git
   cd godot
   git checkout 4.4-stable  # or later
   ```

2. Add Godot Turbo as a module:
   ```bash
   cd modules
   git clone https://github.com/callmefloof/godot-turbo.git godot_turbo
   # or as a submodule:
   git submodule add https://github.com/callmefloof/godot-turbo.git godot_turbo
   ```

3. Build Godot with the module:
   ```bash
   cd ..
   scons target=editor
   ```

4. (Optional) Build with tests:
   ```bash
   scons target=editor tests=yes dev_build=yes
   ./bin/godot.linuxbsd.editor.dev.x86_64 --test
   ```

## ⚡ Quick Start

### Basic ECS Setup (C++)

```cpp
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"

// Get FlecsServer singleton
FlecsServer* server = FlecsServer::get_singleton();

// Create a world
RID world_rid = server->create_world();
flecs::world* world = server->_get_world(world_rid);

// Setup pipeline manager
PipelineManager pipeline(world_rid);

// Register components
world->component<Transform3DComponent>();
world->component<VelocityComponent>();

// Create a system
flecs::system move_system = world->system<Transform3DComponent, VelocityComponent>()
    .iter([](flecs::iter& it, Transform3DComponent* transforms, VelocityComponent* velocities) {
        for (auto i : it) {
            transforms[i].position += velocities[i].velocity * it.delta_time();
        }
    });

pipeline.add_to_pipeline(move_system);

// Create entities
flecs::entity player = world->entity()
    .set<Transform3DComponent>({Vector3(0, 0, 0)})
    .set<VelocityComponent>({Vector3(1, 0, 0)});

// Run the game loop
world->progress(delta_time);
```

### Using GDScript Integration

**GDScript (on a Node):**
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
```

**C++ (System Setup):**
```cpp
#include "modules/godot_turbo/ecs/systems/gdscript_runner_system.h"

GDScriptRunnerSystem* runner = memnew(GDScriptRunnerSystem);
runner->init(world_rid, world);
```

## 🏗️ Architecture

### Module Structure

```
godot_turbo/
├── ecs/
│   ├── components/        # ECS components
│   ├── flecs_types/       # Flecs server and core types
│   ├── systems/           # ECS systems
│   │   ├── demo/          # Demo systems (BadApple)
│   │   └── tests/         # Unit tests
│   └── utility/           # Utility functions
├── thirdparty/
│   ├── flecs/             # Flecs library
│   └── concurrentqueue/   # Lock-free queue
└── README.md              # This file
```

### Core Components

- **FlecsServer** - Singleton managing Flecs worlds and entities
- **PipelineManager** - Controls system execution order and phases
- **CommandQueue** - Lock-free thread-safe command queue
- **GDScriptRunnerSystem** - Executes GDScript methods on entities
- **Components** - Transform, Physics, Navigation, Rendering, and more
- **Utilities** - Scene conversion, physics integration, rendering helpers

## 📚 Documentation

### Main Documentation

- **[Systems Documentation](ecs/systems/SYSTEMS_DOCUMENTATION.md)** - Complete systems reference
- **[Systems README](ecs/systems/README.md)** - Systems overview and quick start
- **[Testing Guide](ecs/systems/tests/README.md)** - How to run and write tests

### Component Documentation

- **[Runtime Components](ecs/RUNTIME_COMPONENTS.md)** - ⭐ **NEW** Create components at runtime with Flecs reflection
- **[Migration Guide: register_component_type → create_runtime_component](ecs/MIGRATION_REGISTER_TO_RUNTIME.md)** - Migrate from deprecated method
- **[Components README](ecs/components/README.md)** - Component system overview
- **[Component Migration Guide](ecs/components/MIGRATION_GUIDE.md)** - Migrating from old components
- **[Usage Examples](ecs/components/USAGE_EXAMPLES.md)** - Component usage examples
- **[Flecs Reflection](ecs/components/README_FLECS_REFLECTION.md)** - Using Flecs reflection

### Utility Documentation

- **[Utilities README](ecs/utility/README.md)** - Utility functions overview
- **[Scene Object Utility](ecs/utility/SCENE_OBJECT_UTILITY_DOC.md)** - Scene conversion
- **[Domain Utilities](ecs/utility/DOMAIN_UTILITIES_DOC.md)** - Physics, rendering, navigation

### API Documentation

All public APIs are documented in header files with comprehensive inline documentation.

## 📖 Examples

### Runtime Component Creation (GDScript)

```gdscript
extends Node

func _ready():
    var flecs = FlecsServer.get_singleton()
    var world_id = flecs.create_world()
    flecs.init_world(world_id)
    
    # Create component types at runtime with typed fields
    var health_comp = flecs.create_runtime_component(world_id, "Health", {
        "current": 100,      # int
        "max": 100,          # int
        "regen_rate": 5.0    # float
    })
    
    var player_comp = flecs.create_runtime_component(world_id, "Player", {
        "name": "Hero",              # String
        "position": Vector3.ZERO,    # Vector3
        "level": 1                   # int
    })
    
    # Use components on entities
    var entity = flecs.create_entity(world_id)
    flecs.add_component(entity, health_comp)
    flecs.add_component(entity, player_comp)
    
    # Set component data
    flecs.set_component(entity, "Health", {
        "current": 75,
        "max": 100,
        "regen_rate": 2.5
    })
    
    # Retrieve component data
    var health = flecs.get_component_by_name(entity, "Health")
    print("Health: ", health["current"], "/", health["max"])
```

> **Note:** The old `register_component_type()` method is **deprecated as of v1.2.0-a.1** and will be removed in v2.0.0. Use `create_runtime_component()` for better performance and Flecs reflection support. See [migration guide](ecs/MIGRATION_REGISTER_TO_RUNTIME.md).

### Command Queue (Thread-Safe)

```cpp
#include "modules/godot_turbo/ecs/systems/command.h"

Ref<CommandHandler> handler = memnew(CommandHandler);

// Enqueue from any thread
std::thread worker([&handler]() {
    handler->enqueue_command([]() {
        print_line("Executed on main thread!");
    });
});

// Process on main thread
handler->process_commands();
worker.join();
```

### Custom Execution Phase

```cpp
PipelineManager pipeline(world_rid);

// Create custom phase after OnUpdate
flecs::entity late_update = pipeline.create_custom_phase("LateUpdate", "OnUpdate");

// Add system to custom phase
flecs::system late_system = world->system<CameraComponent>()
    .iter([](flecs::iter& it, CameraComponent* cameras) {
        // Late update logic
    });

pipeline.add_to_pipeline(late_system, late_update);
```

### More Examples

See the `ecs/systems/demo/` directory for complete working examples, including the optimized BadApple video renderer.

## ⚡ Performance

### Benchmarks

**BadAppleSystem** (6400 instances, 640x480 video):

| Optimization | FPS | Speedup |
|--------------|-----|---------|
| Baseline | 5 FPS | 1x |
| Format-specific loops | 12 FPS | 2.4x |
| + SIMD (SSE2/NEON) | 23 FPS | 4.6x |
| + Multi-threading (4 cores) | 60+ FPS | 12+x |

**CommandQueue** (lock-free):
- Enqueue: ~50-100ns per command
- Process: ~30ns overhead per command
- Throughput: 10,000+ commands/frame

See [OPTIMIZATION_COMPLETE.md](ecs/systems/demo/OPTIMIZATION_COMPLETE.md) for detailed performance analysis.

## 🧪 Testing

### Run Tests

```bash
# Build with tests
scons tests=yes target=editor dev_build=yes

# Run all tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test

# Run specific test suites
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[Command]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[GDScriptRunnerSystem]*"
```

### Test Coverage

| System | Test Cases | Coverage |
|--------|-----------|----------|
| PipelineManager | 20+ | 90%+ |
| CommandQueue | 40+ | 95%+ |
| GDScriptRunnerSystem | 30+ | 85%+ |
| **Total** | **90+** | **90%+** |

## 📝 Changelog

### Version 1.1.1-a.1 (2025-01-28)

**Runtime Component Creation:**
- ✨ **NEW: `create_runtime_component()`** - Dynamic component creation using Flecs reflection API
  - Support for all Godot Variant types (primitives, Vector2/3/4, Transform2D/3D, Color, etc.)
  - Automatic type inference from field values
  - Full Flecs reflection metadata (serialization, introspection)
  - Zero runtime overhead vs C++ components
  - Error handling for duplicate components
  - 2-5x faster than deprecated method

**Deprecation:**
- ⚠️ **DEPRECATED: `register_component_type()`** - Will be removed in v2.0.0
  - Replaced by `create_runtime_component()` with better performance
  - Uses heap-allocated Dictionary wrapper (inefficient)
  - Prints deprecation warning once per session
  - Wrapped in `#ifndef DISABLE_DEPRECATED` guards
  - Can be completely removed at compile time with `DISABLE_DEPRECATED=yes`
  - Full migration guide provided

**Documentation:**
- 📚 **RUNTIME_COMPONENTS.md** (403 lines) - Complete API guide with 8 examples
- 📚 **MIGRATION_REGISTER_TO_RUNTIME.md** (445 lines) - Step-by-step migration guide
- 📚 **DEPRECATION_SUMMARY.md** (335 lines) - Technical rationale and timeline
- 📚 Updated README with runtime component examples

**Migration Example:**
```gdscript
# OLD (deprecated):
var comp = flecs.register_component_type(world_id, "Health", {})

# NEW (recommended):
var comp = flecs.create_runtime_component(world_id, "Health", {
    "current": 100,
    "max": 100,
    "regen_rate": 5.0
})
```

**Performance:**
- ⚡ Direct struct member access (no Dictionary indirection)
- ⚡ 2-5x faster component access
- ⚡ Cache-friendly memory layout
- ⚡ Zero allocation overhead after creation

**Files Changed:**
- `flecs_server.h` - Added `create_runtime_component()` declaration + deprecation notice
- `flecs_server.cpp` - Implemented new method (112 lines) + deprecation warning
- `CHANGELOG.md` - Added v1.1.1-a.1 entry
- `README.md` - Added runtime component examples and changelog

**Backward Compatibility:**
- ✅ Old `register_component_type()` still works (prints warning)
- ✅ No breaking changes
- ✅ Gradual migration path until v2.0.0
- ✅ Use `DISABLE_DEPRECATED=yes` to force migration at compile time

**Build Options:**
```bash
# Standard build (deprecated method available with warning)
scons target=editor dev_build=yes

# Strict build (deprecated method removed - forces clean migration)
scons DISABLE_DEPRECATED=yes target=editor dev_build=yes
```

---

### Version 1.1.0-a.1 (2025-01-28)

**Major Features:**
- ✨ **Comprehensive Documentation** - Added 3,400+ lines of documentation across all systems
- ✨ **Unit Test Suite** - 90+ comprehensive unit tests with 90%+ code coverage
- ✨ **Performance Optimizations** - BadAppleSystem with SIMD and multi-threading (12x speedup)
- ✨ **GDScript/C# Integration** - Full support for script methods on ECS entities

**Systems:**
- ✅ **PipelineManager** - Fully documented with 20+ unit tests
- ✅ **CommandQueue/Handler** - Lock-free queue with 40+ unit tests, comprehensive API docs
- ✅ **GDScriptRunnerSystem** - Script bridge with 30+ tests, method caching
- ✅ **BadAppleSystem** - Optimized demo with SIMD (SSE2/NEON) and multi-threading

**Documentation:**
- 📚 Master systems documentation (SYSTEMS_DOCUMENTATION.md)
- 📚 Comprehensive testing guide (tests/README.md)
- 📚 API documentation in all headers (550+ lines)
- 📚 Migration guides, usage examples, and quick references

**Performance:**
- ⚡ SIMD vectorization for pixel processing (SSE2 x86, NEON ARM)
- ⚡ Multi-threaded processing with WorkerThreadPool
- ⚡ Lock-free command queue with object pooling
- ⚡ Method caching in script runner (10ns lookup)

**Testing:**
- 🧪 PipelineManager: Constructor, registration, phases, execution order
- 🧪 CommandQueue: Pooling, thread safety, performance (10k commands)
- 🧪 GDScriptRunner: Caching, filtering, stress tests (1000+ entities)

**Components:**
- 🔧 Refactored physics and navigation components
- 🔧 Added Flecs reflection support
- 🔧 Improved component documentation

**Utilities:**
- 🛠️ Scene object conversion utilities
- 🛠️ Domain-specific utilities (physics, rendering, navigation)
- 🛠️ Comprehensive utility documentation

**Cleanup:**
- 🧹 Removed temporary .obj files
- 🧹 Removed in-progress documentation files
- 🧹 Consolidated documentation structure

**Breaking Changes:**
- None - fully backward compatible with 1.0.x

**Known Issues:**
- Godot 4.5 beta compatibility untested
- Some pre-existing diagnostics in utility files (not from this release)

---

### Version 1.0.x (Historical - Retroactive Tags)

**Note:** Version 1.0.x releases will be tagged retroactively from previous commits.

These versions included:
- Initial Flecs integration
- Basic component system
- Scene conversion utilities
- FlecsServer singleton
- Initial system implementations

---

## 🤝 Contributing

Contributions are welcome! Please:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Write** unit tests for new functionality
4. **Document** your code with inline comments
5. **Test** thoroughly (`scons tests=yes && ./bin/godot --test`)
6. **Commit** your changes (`git commit -m 'Add amazing feature'`)
7. **Push** to the branch (`git push origin feature/amazing-feature`)
8. **Open** a Pull Request

### Coding Standards

- Follow Godot's C++ style guide
- Document all public APIs with Doxygen-style comments
- Write unit tests for new systems (aim for 80%+ coverage)
- Update relevant documentation files
- Run all tests before submitting PR

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **[Flecs](https://github.com/SanderMertens/flecs)** - Sander Mertens for the excellent ECS library
- **[moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue)** - Cameron Desrochers for the lock-free queue
- **Godot Engine** - The Godot development team for the amazing engine
- **Contributors** - All who have contributed to this module

## 📞 Support

- **Issues:** [GitHub Issues](https://github.com/callmefloof/godot-turbo/issues)
- **Documentation:** [DeepWiki](https://deepwiki.com/callmefloof/godot-turbo)
- **Discussions:** [GitHub Discussions](https://github.com/callmefloof/godot-turbo/discussions)

---

**Version:** 1.1.0-a.1  
**Last Updated:** 2025-01-28  
**Godot Version:** 4.4+  
**Maintainer:** [@callmefloof](https://github.com/callmefloof)