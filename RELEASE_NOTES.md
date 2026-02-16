# Release Notes - Godot Turbo ECS v1.2.1-beta.1

**Release Date:** February 16, 2026  
**Status:** Beta Release  
**Godot Version:** 4.6+

---

## 🎉 Overview

This release of Godot Turbo ECS version 1.2.1 focuses on **stability improvements**, **component reflection**, **ECS tracing integration**, and **Godot 4.6+ compatibility fixes**. This release hardens the networking foundation introduced in 1.2.0-beta.1 and improves debugging capabilities.

### Highlights

- 🔍 **Component Reflection System** - Runtime type introspection for ECS components
- 📊 **ECS Tracing Integration** - Worker thread tracking and performance analysis
- 🔧 **FlecsRuntimeDebugger Improvements** - Robust retry mechanism and Object inheritance
- 🛠️ **API Consistency** - Normalized parameter names across all utilities
- ✅ **Godot 4.6+ Compatibility** - Fixed include paths for new server subdirectories
- 🐛 **Test Stability** - Fixed startup crashes and improved test reliability

---

## 🔍 Component Reflection System

### New Features

A new component reflection system provides runtime type introspection for all ECS components:

```gdscript
# Components now have full reflection support
# Allows runtime inspection of component fields and types
var component_info = FlecsServer.get_component_by_name(entity_rid, "Transform3DComponent")
```

### Implementation Details

- Added `register_reflection_components()` for automatic component registration
- Component resolution helpers added to `FlecsQuery` and `FlecsScriptSystem`
- Cached physics process check in `GDScriptRunnerSystem` for improved performance
- New `DispatchMode` enum exposed in FlecsServer bindings:

```gdscript
# Dispatch modes for script systems
FlecsServer.DISPATCH_PER_ENTITY  # Process each entity individually
FlecsServer.DISPATCH_BATCH       # Process entities in batches
```

---

## 📊 ECS Tracing Integration

### Worker Thread Tracking

The ECS now integrates with Godot's tracing system for comprehensive performance analysis:

- Worker threads are now registered with the traced OS API
- Query and system execution includes trace markers
- Native system timing tracked via `native_system_prev_time_spent` field

### Benefits

- Better visibility into multi-threaded ECS execution
- Improved profiler integration
- Easier identification of performance bottlenecks in worker threads

---

## 🔧 FlecsRuntimeDebugger Improvements

### Robust Initialization

The `FlecsRuntimeDebugger` has been significantly improved:

```cpp
// Now inherits from Object for proper Godot integration
class FlecsRuntimeDebugger : public Object {
    GDCLASS(FlecsRuntimeDebugger, Object);
    // ...
};
```

### Features

- **Object Inheritance** - Proper Godot class registration and bindings
- **Retry Timer Mechanism** - Handles cases where debugger isn't immediately available
- **Safe Timer Handling** - Proper cleanup prevents use-after-free issues
- **Maximum Retry Limit** - 50 retries (~5 seconds) prevents infinite loops

### Configuration

```cpp
static constexpr int MAX_RETRY_COUNT = 50; // ~5 seconds at 100ms intervals
```

---

## 🛠️ API Consistency Improvements

### Normalized Parameter Names

All utility functions now use consistent parameter naming:

| Before | After |
|--------|-------|
| `p_world` | `world_id` |
| `world_rid` | `world_id` |
| Various | Consistent `world_id` |

### ClassDB Binding Updates

- Switched many bindings to use `D_METHOD` macro for better documentation
- Improved method signatures for GDExtension compatibility
- Consistent naming across navigation, physics, and rendering utilities

### SceneObjectUtility Fixes

- Fixed recursion issues in scene object processing
- Added `cleanup_singleton()` for proper resource cleanup

---

## ✅ Godot 4.6+ Compatibility

### Updated Include Paths

Server headers have moved to new subdirectories in Godot 4.6+:

```cpp
// Old paths (Godot 4.5 and earlier)
#include "servers/rendering_server.h"
#include "servers/navigation_server_2d.h"

// New paths (Godot 4.6+)
#include "servers/rendering/rendering_server.h"
#include "servers/navigation_2d/navigation_server_2d.h"
#include "servers/navigation_3d/navigation_server_3d.h"
#include "servers/physics_2d/physics_server_2d.h"
#include "servers/physics_3d/physics_server_3d.h"
```

### Affected Files

- `navigation2d_utility.cpp`
- `navigation3d_utility.cpp`
- `physics2d_utility.cpp`
- `physics3d_utility.cpp`
- `render_utility_3d.cpp`
- `flecs_opaque_types.h`

---

## 🐛 Bug Fixes

| Issue | Solution |
|-------|----------|
| Test startup crash in FlecsRuntimeDebugger | Added safe timer handling and retry logic |
| Constructor parameter shadowing warnings | Renamed parameters to avoid shadowing |
| SceneObjectUtility recursion issues | Fixed recursive processing logic |
| Rendering server include path errors | Updated to new Godot 4.6+ paths |
| Noisy debug prints in profiler | Removed excessive logging |

---

## 🔄 Changes

### Profiler Enhancements

- Cleaner profiler label display
- Reduced noisy debug output
- Better native system timing tracking

### Test Infrastructure

- Updated test fixtures for new API signatures
- Fixed helper macros for component testing
- Improved test reliability with proper initialization

### Flecs Phases

Added `OnPhysicsUpdate` phase alias for compatibility:

```cpp
// flecs_phases.h
namespace flecs {
    inline const flecs::entity_t OnPhysicsUpdate = flecs::OnUpdate;
}
```

---

## 📁 Modified Files

```
godot_turbo/
├── SCsub                                        # Build configuration
├── register_types.cpp                           # Reflection registration
├── ecs/
│   ├── components/
│   │   ├── all_components.h                     # Reflection includes
│   │   └── flecs_opaque_types.h                 # Updated includes
│   ├── flecs_types/
│   │   ├── flecs_phases.h                       # ✨ NEW: Phase aliases
│   │   ├── flecs_query.cpp                      # Component resolution
│   │   ├── flecs_script_system.cpp              # Tracing integration
│   │   ├── flecs_server.cpp                     # Bindings, tracing
│   │   ├── flecs_server.h                       # DispatchMode enum
│   │   └── flecs_variant.h                      # Minor updates
│   └── systems/
│       ├── gdscript_runner_system.cpp/h         # Physics cache
│       ├── pipeline_manager.cpp/h               # Cleanup
│       └── utility/
│           ├── navigation2d_utility.cpp         # Include paths
│           ├── navigation3d_utility.cpp         # Include paths
│           ├── physics2d_utility.cpp            # Include paths
│           ├── physics3d_utility.cpp            # Include paths
│           ├── ref_storage.h                    # Refactoring
│           ├── render_utility_3d.cpp            # Include paths
│           ├── scene_object_utility.cpp/h       # Recursion fix
│           └── world_utility.h                  # Parameter names
├── editor/
│   ├── flecs_editor_plugin.cpp                  # Minor updates
│   ├── flecs_entity_inspector.cpp               # Cleanup
│   ├── flecs_profiler.cpp                       # Label cleanup
│   └── flecs_profiler_plugin.cpp                # Minor updates
├── network/
│   └── network_server.cpp                       # Include updates
├── runtime/
│   ├── flecs_runtime_debugger.cpp               # Retry mechanism
│   └── flecs_runtime_debugger.h                 # Object inheritance
└── tests/
    ├── test_fixtures.h                          # API updates
    ├── test_flecs_variant.h                     # API updates
    ├── test_ref_storage.h                       # API updates
    ├── test_resource_object_utility.h           # API updates
    ├── test_scene_object_utility.h              # API updates
    └── test_world_utility.h                     # API updates
```

---

## 🔄 Migration from 1.2.0-beta.1 / 1.2.0-beta.2

### Mostly Backward Compatible

Most code from beta.1 will work without changes. However:

### Parameter Name Changes (Optional)

If you have custom utilities that extend the built-in ones, consider updating parameter names:

```gdscript
# Old (still works but inconsistent)
func my_utility(world_rid: RID):
    pass

# New (recommended for consistency)
func my_utility(world_id: RID):
    pass
```

### DispatchMode Enum (New Feature)

You can now use the enum directly:

```gdscript
# Before (still works)
FlecsServer.set_script_system_dispatch_mode(system_rid, 0)  # Per-entity

# After (more readable)
FlecsServer.set_script_system_dispatch_mode(system_rid, FlecsServer.DISPATCH_PER_ENTITY)
```

---

## ⚠️ Known Issues

- Networking remains in beta - expect API refinements in future releases
- NetworkServer detailed timing not yet fully integrated with new tracing system
- FlecsRuntimeDebugger retry timer assumes scene tree is available (editor/game only)
- Requires Godot 4.6+ due to include path changes (not compatible with 4.5 or earlier)

---

## ✅ Test Coverage

| Area | Status |
|------|--------|
| Component Reflection | ✅ Verified |
| FlecsRuntimeDebugger | ✅ Startup crash fixed |
| Godot 4.6+ Includes | ✅ All paths updated |
| API Parameter Names | ✅ Consistent |
| SceneObjectUtility | ✅ Recursion fixed |
| Test Infrastructure | ✅ All tests passing |

---

## 🔜 What's Next

### Version 1.2.1 (Stable)
- Final networking API stabilization
- Complete ECS tracing documentation
- Performance benchmarks with tracing
- Comprehensive multiplayer testing

### Version 2.0.0 (Future)
- Godot 4.6+ exclusive features
- Advanced Flecs features (observers, prefabs)
- Enhanced reflection system
- Breaking API improvements based on feedback

---

## 🙏 Credits

**Contributors:**
- [@callmefloof](https://github.com/callmefloof) - Module development

**Third-Party Libraries:**
- [Flecs](https://github.com/SanderMertens/flecs) by Sander Mertens - ECS library
- [ENet](http://enet.bespin.org/) - Network transport (optional)
- [moodycamel::ConcurrentQueue](https://github.com/cameron314/concurrentqueue) - Lock-free queue
- [Godot Engine](https://godotengine.org) - Game engine

---

## 📞 Support & Feedback

- **Issues:** [GitHub Issues](https://github.com/callmefloof/godot-turbo/issues)
- **Documentation:** [DeepWiki](https://deepwiki.com/callmefloof/godot-turbo)
- **Discussions:** [GitHub Discussions](https://github.com/callmefloof/godot-turbo/discussions)

---

**Download:** [GitHub Releases](https://github.com/callmefloof/godot-turbo/releases/tag/v1.2.1-beta.1)  
**Tagged Commit:** v1.2.1-beta.1  
**Previous Version:** v1.2.0-beta.1

---

Thank you for using Godot Turbo ECS! 🚀

*This is a beta release focused on stability and Godot 4.6+ compatibility. Please report any issues you encounter.*