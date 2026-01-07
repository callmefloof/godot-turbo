# Changelog

All notable changes to the Godot Turbo ECS module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [1.2.0-beta.1] - 2025-01-29

### Added

#### Networking System (Major Feature)
- **NetworkServer Singleton** - Complete multiplayer networking layer built on top of ECS
  - Host/join game support with configurable tick rate
  - Automatic entity spawning and despawning across network
  - Component replication with multiple modes (continuous, on-change, reliable, once)
  - Authority management (server, client, transferable, shared modes)
  - Input prediction and reconciliation system
  - Transform interpolation for smooth networked movement
  - RPC queue system for remote method calls
  - Network statistics tracking (bytes sent/received, update counts)
  - Relevancy system for bandwidth optimization
  - ENet transport integration (optional/abstracted)

- **Network Components** - Full suite of ECS components for multiplayer
  - `NetworkIdentity` - Unique network ID, spawn tracking, scene path
  - `NetworkAuthority` - Authority mode, owner/authority peer IDs
  - `NetworkReplicated` - Per-component replication configuration
  - `NetworkDirty` - Change tracking for delta updates
  - `NetworkPendingSpawn` / `NetworkPendingDestroy` - Spawn queue management
  - `NetworkInterpolation` - Generic state interpolation buffer
  - `NetworkTransformInterpolation3D` / `NetworkTransformInterpolation2D` - Transform-specific interpolation
  - `NetworkPrediction` - Client-side prediction state buffer
  - `NetworkInput` - Input frame buffer with acknowledgment tracking
  - `NetworkStats` - Per-entity network statistics
  - `NetworkRelevancy` - Distance-based relevancy and peer filtering
  - `NetworkRPCQueue` - Queued RPC calls for batched sending

- **Network Editor Plugin** - New "Network Inspector" dock for debugging multiplayer
  - Connection status display
  - Entity replication monitoring
  - Network statistics visualization

#### Editor Improvements
- **InstanceManager** - Multi-editor-instance coordination system
  - Detects and manages multiple Godot editor instances
  - Lock file-based resource coordination
  - Primary/secondary instance election
  - Graceful degradation for secondary instances
  - Prevents debugger/profiler conflicts between instances

- **Profiler Enhancements**
  - Instance status display in profiler UI (primary/secondary)
  - Clear documentation that profiler is for local editor use only
  - Improved error messaging for multi-instance scenarios
  - Resource locking integration

#### Documentation
- **Network module documentation** (`network/README.md`) - Comprehensive networking guide
  - Quick start examples for server/client setup
  - Architecture overview and data flow diagrams
  - Complete API reference for NetworkServer
  - Component documentation with struct definitions
  - Configuration options (tick rate, interpolation, debug logging)
  - Best practices for authority, bandwidth, and error handling
  - Troubleshooting guide

- **Remote Debugging Guide** (`REMOTE_DEBUGGING_GUIDE.md`) - Complete remote debugging documentation
  - Architecture overview (runtime debugger, editor plugin, profiler)
  - Usage instructions for remote inspection
  - API reference for FlecsRuntimeDebugger
  - Message protocol specification
  - Troubleshooting section

- **Debugger Plugin Documentation** (`DEBUGGER_PLUGIN_DOCUMENTATION.md`)
  - Component architecture (FlecsDebuggerPlugin, FlecsWorldEditorPlugin)
  - Message protocol (request/response formats)
  - Data flow diagrams
  - Session management details
  - Future enhancement plans

- **Profiler Documentation** - Enhanced troubleshooting and usage guides
  - `docs/PROFILER_README.md` - Quick start guide
  - `docs/PROFILER_TROUBLESHOOTING.md` - Common issues and solutions
  - `docs/PROFILER_API.md` - Complete API reference

### Changed

- **NetworkServer API** - Changed bound method signatures to use `int` instead of namespaced enums
  - `NetworkTypes::DisconnectReason` → `int` in public bindings
  - Internal casting preserves type safety
  - Fixes Godot Variant binding compatibility issues

- **Profiler Plugin** - Updated to integrate with InstanceManager
  - Shows instance status in info panel
  - Falls back gracefully when not primary instance
  - Clearer messaging about local-only profiling

- **Editor Plugin** - InstanceManager lifecycle integration
  - Initialize on ENTER_TREE
  - Shutdown on EXIT_TREE
  - Proper resource cleanup

### Fixed

- **Enum Binding Errors** - Resolved compilation errors with namespaced enums in Godot bindings
  - `NetworkTypes::DisconnectReason` caused Variant binder errors
  - Solution: Use `int` in bound signatures with internal casting

- **Network Editor Plugin Name** - Fixed missing dock name in TabContainer
  - Dock now properly labeled "Network Inspector" in editor UI

- **Multi-Instance Conflicts** - Resolved profiler/debugger issues when multiple editors running
  - Profiler data not appearing due to resource conflicts
  - Remote debugging messages going to wrong instance
  - InstanceManager now coordinates access

---

## [1.1.2-a.1] - 2025-01-28

### Changed

#### BadAppleSystem Refactoring
- **Refactored `process_pixels_rgb8` and `process_pixels_rgba8` for DRY principles**
  - Reduced code duplication by ~60% (180 lines → 75 lines)
  - Created unified template implementation `process_pixels_impl<BytesPerPixel, HasAlpha>`
  - Eliminated repetitive row iteration and pixel reading code across mode branches
  - Uses compile-time template parameters for zero runtime overhead
  - Introduced lambda function for pixel reading to eliminate 6 copies of identical code
  - Unified row iteration logic across all processing modes (REGULAR, INVERTED, RANDOM)
  - Mode checks moved outside innermost pixel loop (maintains performance)
  - All optimizations preserved:
    - Row-based processing (no per-pixel modulo/division)
    - Fast inner loops (only additions)
    - Multi-threading support unchanged
    - Y-flip support unchanged

#### Documentation
- **Fixed `bad_apple_example.gd` to demonstrate correct usage patterns**
  - Added critical `FlecsServer.progress_world(world_rid, delta)` call in `_process()`
  - Changed to programmatic VideoStreamPlayer creation (not scene-based `@export`)
  - Added `call_deferred()` for setup in `_ready()` to ensure proper initialization
  - Added critical `await get_tree().process_frame` after video player creation
  - Fixed all property access to use setter methods (`set_mode()`, `set_use_multithreading()`, etc.)
  - Updated all setup functions with correct initialization patterns
  - Fixed video path references
  - Added detailed comments explaining critical requirements
- **Created `CORRECT_USAGE.md`** - Comprehensive BadAppleSystem usage guide (286 lines)
  - Critical requirements section with detailed explanations
  - Complete minimal working example
  - Common mistakes to avoid with examples
  - Configuration options reference
  - Performance guidelines for different hardware
  - Troubleshooting section

### Performance
- **BadAppleSystem refactoring**: Zero performance impact
  - Template functions inline at compile time
  - Lambda functions inline at compile time
  - All original optimizations maintained
  - Compiler generates identical assembly code
  - Same 60+ FPS performance as before

### Fixed
- **Example code correctness**
  - `bad_apple_example.gd` now properly calls `FlecsServer.progress_world()` in `_process()`
  - Fixed race condition with video player initialization
  - Fixed incorrect property access patterns
  - All examples now follow best practices


## [1.1.1-a.2] - 2025-01-28

### Fixed

#### Documentation Examples
- **Fixed all GDScript example files** to use correct RID-based API patterns
  - `bad_apple_example.gd` - Fixed non-existent `MultiMeshComponent` class instantiation (line 247)
    - Now uses Dictionary-based API: `FlecsServer.set_component(entity_rid, "MultiMeshComponent", {...})`
    - Fixed `FlecsServer.get_singleton()` calls → use `FlecsServer` directly (static methods)
    - Fixed incorrect `destroy_world()` → `free_world()`
  - `example_query_usage.gd` - Replaced deprecated `register_component_type()` with `create_runtime_component()`
    - Fixed `TYPE_FLOAT` constants → use actual default values (e.g., `0.0`)
    - Fixed `FlecsServer.get_singleton()` → use `FlecsServer` static methods
  - `gdscript_runner_example.gd` - Fixed incorrect API signatures
    - Removed incorrect `world_rid` parameter from `get_component_by_name()` and `set_component()`
    - Correct signature: `FlecsServer.get_component_by_name(entity_rid, "ComponentName")`
    - Fixed `FlecsServer.get_singleton()` → use `FlecsServer` static methods
  - `runtime_component_example.gd` - Fixed `FlecsServer.get_singleton()` → use `FlecsServer` static methods

#### API Corrections
- All examples now demonstrate correct usage patterns:
  - **FlecsServer is singleton with static methods** - Call directly, not via `get_singleton()`
  - **Components are C++ structs** - Use Dictionary to set/get, not GDScript class instantiation
  - **Component methods take `entity_rid` only** - Don't pass `world_rid` to component get/set methods
  - **Use `create_runtime_component()`** - Not deprecated `register_component_type()`

#### Files Modified
- `ecs/systems/demo/docs/bad_apple_example.gd` - 4 errors fixed
- `ecs/flecs_types/example_query_usage.gd` - 1 error fixed  
- `ecs/systems/gdscript_runner_example.gd` - 6 errors fixed
- `ecs/examples/runtime_component_example.gd` - 1 error fixed

**Result:** All example files now have **0 errors** and demonstrate correct API usage patterns.

---

## [1.1.1-a.1] - 2025-01-28

### Added

#### Features
- **Runtime Component Creation API** - `create_runtime_component()` method for dynamic component definition
  - Create components at runtime using Flecs reflection API
  - Support for all Godot Variant types (primitives, Vector2/3, Color, Transform2D/3D, etc.)
  - Support for complex types (Array, Dictionary, RID, Quaternion, etc.)
  - Automatic type inference from provided field values
  - Error handling for duplicate component names
  - Full integration with existing ECS systems
  - Zero runtime overhead compared to C++ components
  - Replaces deprecated `register_component_type()` method
  - Compile-time control via `DISABLE_DEPRECATED` flag

#### Documentation
- Complete runtime component creation guide (RUNTIME_COMPONENTS.md, 400+ lines)
  - Comprehensive API documentation
  - 8+ usage examples covering all type categories
  - Best practices and design patterns
  - Data-driven component loading examples
  - Integration guide with script systems
  - Performance considerations and limitations
  - Troubleshooting section
- Migration guide from old to new component system (MIGRATION_REGISTER_TO_RUNTIME.md, 445 lines)
  - Step-by-step migration instructions
  - Before/after code examples
  - Common patterns and troubleshooting
  - Automated migration helper script
  - Deprecation timeline
- GDScript example demonstrating runtime component usage (runtime_component_example.gd)
  - 8 practical examples
  - Error handling demonstrations
  - Multi-component entity examples

### Deprecated
- **`register_component_type()`** - Marked for removal in v2.0.0
  - Uses inefficient heap-allocated Dictionary wrapper
  - Replaced by `create_runtime_component()` with proper Flecs reflection
  - Prints deprecation warning on use (once per session)
  - Wrapped in `#ifndef DISABLE_DEPRECATED` guards
  - Can be completely removed at compile time with `DISABLE_DEPRECATED=yes`
  - Migration guide provided (MIGRATION_REGISTER_TO_RUNTIME.md)

### Performance
- **Runtime components**: Identical performance to C++ components
  - Type resolution at creation time only
  - No runtime overhead for component access
  - Production-ready
- **vs. old register_component_type()**: 2-5x faster component access (no Dictionary indirection)

### Build Options
```bash
# Standard build (deprecated method available with warning)
scons target=editor dev_build=yes

# Strict build (deprecated method removed - forces clean migration)
scons DISABLE_DEPRECATED=yes target=editor dev_build=yes
```

---

## [1.1.0-a.1] - 2025-01-28

### Added

#### Documentation
- Comprehensive API documentation for all system classes (550+ lines)
- Master systems documentation guide (SYSTEMS_DOCUMENTATION.md)
- Testing guide with examples and best practices (tests/README.md)
- Complete documentation for PipelineManager class (216 lines)
- Complete documentation for CommandQueue/CommandHandler (337 lines)
- Navigation README for easy access to all documentation
- Component migration guide and usage examples
- Utility function documentation

#### Testing
- **90+ comprehensive unit tests** with 90%+ code coverage
- **PipelineManager tests** (20+ test cases, 545 lines)
  - Constructor and initialization tests
  - Copy/move semantics validation
  - System registration and lookup tests
  - Custom phase creation and execution order tests
  - Edge case handling
- **Command system tests** (40+ test cases, 721 lines)
  - Pool allocation and thread safety tests
  - Queue operations and FIFO ordering tests
  - Performance stress tests (10,000+ commands)
  - Multi-threaded producer tests
- **GDScriptRunnerSystem tests** (30+ test cases, 553 lines)
  - Cache management tests
  - Component filtering tests
  - Stress tests with 1000+ entities

#### Features
- **GDScriptRunnerSystem** - Bridge for executing GDScript/C# methods on ECS entities
  - Method caching for performance (~10ns lookup)
  - Support for both GDScript and C# naming conventions
  - Separate process and physics process phases
- **BadAppleSystem optimizations** - Demo system with major performance improvements
  - Multi-threaded pixel processing with WorkerThreadPool
  - Format-specific optimized loops (template-based, zero overhead)
  - Row-based processing (eliminates per-pixel modulo/division)
  - 12x+ overall performance improvement
- **Lock-free CommandQueue** - Thread-safe command queue with object pooling
  - Multi-producer safe enqueueing
  - Object pooling for zero-allocation performance
  - ~50-100ns enqueue time
- **PipelineManager** - Flexible system execution control
  - Custom phase creation with dependencies
  - System lookup by name
  - Multi-world support

#### Components
- Refactored physics components (RigidBody2D/3D, CollisionShape2D/3D)
- Refactored navigation components (NavigationAgent2D/3D, NavigationObstacle2D/3D)
- Added Flecs reflection support for all components
- Improved component documentation

#### Utilities
- Scene object conversion utilities
- Domain-specific utilities (physics, rendering, navigation)
- Comprehensive utility documentation

### Changed
- Enhanced README.md with quick start guide, examples, and changelog
- Improved .gitignore to exclude build artifacts and temporary files
- Consolidated documentation structure for better organization

### Removed
- Temporary .obj build files (now properly gitignored)
- In-progress documentation files:
  - `GDSCRIPT_RUNNER_IMPLEMENTATION_SUMMARY.md`
  - `DOCUMENTATION_SUMMARY.md`
  - `COMPLETION_CHECKLIST.md`
  - `REFACTOR_SUMMARY.md`
  - `IMPLEMENTATION_COMPLETE.md`
  - `COMPONENT_VERIFICATION.md`
  - `VALIDATION_SUMMARY.md`
  - `DOCUMENTATION_STATUS.md`
  - `PHASE2_COMPLETION_SUMMARY.md`

### Performance
- **BadAppleSystem**: 5 FPS → 60+ FPS (12x improvement)
  - Baseline: 5 FPS
  - Format-specific loops: 12 FPS (2.4x)
  - + Multi-threading: 60+ FPS (12+x)
- **CommandQueue**: 10,000+ commands/frame throughput
- **GDScriptRunnerSystem**: ~10ns cache lookup per entity

### Fixed
- Build artifact accumulation (added to .gitignore)
- Documentation organization and accessibility

---

## [1.0.x] - Historical (Retroactive Tags)

**Note:** Version 1.0.x releases will be tagged retroactively from previous commits.

### Initial Release Features
- Flecs ECS library integration
- FlecsServer singleton for world management
- Basic component system
- Scene conversion utilities
- Initial system implementations
- Basic pipeline management

---

## Version Naming Convention

- **Major.Minor.Patch-Stage.Build**
  - Major: Breaking changes
  - Minor: New features (backward compatible)
  - Patch: Bug fixes
  - Stage: `a` (alpha), `b` (beta), `rc` (release candidate), or omitted (stable)
  - Build: Build number within stage

**Examples:**
- `1.1.0-a.1` - Version 1.1.0, alpha stage, build 1
- `1.2.0-beta.1` - Version 1.2.0, beta stage, build 1
- `1.2.0` - Version 1.2.0, stable release

---

## Migration Guide

### From 1.1.x to 1.2.0-beta.1

**No breaking changes** - This release is fully backward compatible.

**New Features to Adopt:**

1. **Networking** - Add multiplayer support to your game:
   ```gdscript
   # Host a game
   NetworkServer.host_game(7777, 16)
   
   # Join a game  
   NetworkServer.join_game("127.0.0.1", 7777)
   
   # Register entity for replication
   NetworkServer.register_networked_entity(entity_rid, "res://player.tscn")
   ```

2. **Multi-Instance Awareness** - Check for instance conflicts:
   ```cpp
   if (InstanceManager::get_singleton()->is_primary_instance()) {
       // Safe to use remote debugging
   }
   ```

### From 1.0.x to 1.1.0-a.1

**No breaking changes** - This release is fully backward compatible.

**New Features to Adopt:**
1. Use `GDScriptRunnerSystem` for script integration with ECS entities
2. Use `CommandQueue` for thread-safe deferred command execution
3. Use `PipelineManager` for better system execution control
4. Review new documentation for best practices

**Performance Improvements:**
- Existing code will benefit from internal optimizations
- Consider adopting template-based optimized patterns for pixel/batch processing
- Use object pooling patterns from CommandQueue for your systems

---

## Future Plans

### Version 1.2.0 (Stable)
- Networking system stabilization
- Additional network examples
- Performance profiling for network systems
- Comprehensive multiplayer testing

### Version 2.0.0 (Future)
- Godot 4.5+ compatibility
- Advanced Flecs features (pipelines, observers)
- Enhanced reflection system
- Breaking API improvements based on feedback
- Removal of deprecated `register_component_type()`

---

**Maintained by:** [@callmefloof](https://github.com/callmefloof)  
**Repository:** [godot-turbo](https://github.com/callmefloof/godot-turbo)