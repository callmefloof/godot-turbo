# Release Notes - Godot Turbo ECS v1.2.0-beta.1

**Release Date:** January 29, 2025  
**Status:** Beta Release  
**Godot Version:** 4.4+

---

## 🎉 Overview

This is the first beta release of Godot Turbo ECS version 1.2.0, introducing a **complete multiplayer networking layer** built on top of the ECS foundation. This release also includes significant **editor improvements** for multi-instance coordination and enhanced profiling tools.

### Highlights

- 🌐 **Full Networking System** - Entity replication, authority, prediction, interpolation
- 🔧 **Multi-Instance Support** - InstanceManager prevents editor conflicts
- 📊 **Enhanced Profiler** - Instance status display, clearer local-only messaging
- 📚 **Comprehensive Documentation** - Network guides, remote debugging, troubleshooting
- ✨ **Zero Breaking Changes** - Fully backward compatible with 1.1.x

---

## 🌐 Networking System (Major Feature)

### NetworkServer Singleton

A complete multiplayer networking layer that integrates seamlessly with the Flecs ECS:

```gdscript
# Host a game server
NetworkServer.host_game(7777, 16)  # port, max_clients

# Join an existing game
NetworkServer.join_game("192.168.1.100", 7777)

# Register entity for network replication
var entity = FlecsServer.create_entity(world)
NetworkServer.register_networked_entity(entity, "res://player.tscn")

# Configure replication
NetworkServer.set_replicated_component(entity, "Transform3DComponent", NetworkTypes.CONTINUOUS)
NetworkServer.set_replicated_component(entity, "HealthComponent", NetworkTypes.ON_CHANGE)
```

### Network Components

Full suite of ECS components for multiplayer:

| Component | Purpose |
|-----------|---------|
| `NetworkIdentity` | Unique network ID, spawn tracking |
| `NetworkAuthority` | Authority mode, owner peer ID |
| `NetworkReplicated` | Per-component replication config |
| `NetworkDirty` | Change tracking for delta updates |
| `NetworkInterpolation` | Generic state interpolation buffer |
| `NetworkTransformInterpolation3D/2D` | Transform-specific interpolation |
| `NetworkPrediction` | Client-side prediction buffer |
| `NetworkInput` | Input frame buffer with acknowledgment |
| `NetworkStats` | Per-entity network statistics |
| `NetworkRelevancy` | Distance-based relevancy filtering |
| `NetworkRPCQueue` | Batched RPC calls |

### Authority Modes

```cpp
enum AuthorityMode {
    SERVER,       // Server has full authority
    CLIENT,       // Owning client has authority
    TRANSFERABLE, // Authority can be transferred
    SHARED        // Multiple peers share authority
};
```

### Replication Modes

```cpp
enum ReplicationMode {
    CONTINUOUS,  // Update every tick
    ON_CHANGE,   // Update only when changed
    RELIABLE,    // Guaranteed delivery
    ONCE,        // Single update at spawn
    NONE         // No automatic replication
};
```

### Features

- ✅ Host/join game with configurable tick rate
- ✅ Automatic entity spawning/despawning across network
- ✅ Component replication with multiple modes
- ✅ Authority management (server, client, transferable, shared)
- ✅ Input prediction and reconciliation
- ✅ Transform interpolation for smooth movement
- ✅ RPC queue for remote method calls
- ✅ Network statistics tracking
- ✅ Relevancy system for bandwidth optimization
- ✅ ENet transport integration

---

## 🔧 Editor Improvements

### InstanceManager

Manages multiple Godot editor instances to prevent conflicts:

```cpp
// Check if this is the primary instance
if (InstanceManager::get_singleton()->is_primary_instance()) {
    // Safe to use shared resources (debugger, profiler)
} else {
    // Show warning or use local-only mode
}

// Try to acquire a named resource lock
if (InstanceManager::get_singleton()->try_acquire_resource("profiler")) {
    // We own the profiler resource
}
```

**Features:**
- Instance identification using unique IDs
- Lock file management for resource coordination
- Detection of other running instances
- Graceful degradation when conflicts detected
- Primary/secondary instance election

### Profiler Enhancements

- **Instance Status Display** - Shows "Primary" or "Secondary" status in profiler UI
- **Improved Messaging** - Clearer indication that profiler is for local editor use only
- **Resource Locking** - Integrates with InstanceManager for conflict prevention
- **Fallback Behavior** - Secondary instances gracefully fall back to local mode

### Network Editor Plugin

New "Network Inspector" dock for debugging multiplayer:

- Connection status display
- Entity replication monitoring
- Network statistics visualization

---

## 📚 Documentation

### New Documentation

| Document | Description |
|----------|-------------|
| `network/README.md` | Complete networking guide with examples |
| `REMOTE_DEBUGGING_GUIDE.md` | Remote debugging setup and usage |
| `DEBUGGER_PLUGIN_DOCUMENTATION.md` | Debugger plugin architecture |
| `docs/PROFILER_TROUBLESHOOTING.md` | Common profiler issues and solutions |

### Network Documentation Highlights

- Quick start examples for server/client setup
- Architecture overview with data flow diagrams
- Complete API reference for NetworkServer
- Component documentation with struct definitions
- Configuration options (tick rate, interpolation, debug logging)
- Best practices for authority, bandwidth, and error handling

---

## 🔄 Changes

### API Changes

**NetworkServer Bindings:**
- Changed bound method signatures to use `int` instead of namespaced enums
- `NetworkTypes::DisconnectReason` → `int` in public bindings
- Internal casting preserves type safety
- Fixes Godot Variant binding compatibility issues

### Editor Changes

- Profiler plugin now integrates with InstanceManager
- Shows instance status in info panel
- Falls back gracefully when not primary instance
- Editor plugin properly initializes/shuts down InstanceManager

---

## 🐛 Fixes

| Issue | Solution |
|-------|----------|
| Enum binding errors | Use `int` in bound signatures with internal casting |
| Network dock missing name | Set TabContainer name to "Network Inspector" |
| Multi-instance profiler conflicts | InstanceManager coordinates access |
| Remote debugging to wrong instance | Lock file coordination |

---

## 📁 New Files

```
godot_turbo/
├── network/
│   ├── README.md                    # ✨ Networking guide
│   ├── network_server.h/cpp         # ✨ NetworkServer singleton
│   ├── network_types.h              # ✨ Network enums and types
│   ├── components/
│   │   └── network_components.h     # ✨ All network components
│   └── systems/                     # ✨ Network systems (future)
├── editor/
│   ├── instance_manager.h           # ✨ Multi-instance coordination
│   ├── network_editor_plugin.h/cpp  # ✨ Network inspector dock
│   └── ... (existing files enhanced)
├── REMOTE_DEBUGGING_GUIDE.md        # ✨ Remote debugging docs
├── DEBUGGER_PLUGIN_DOCUMENTATION.md # ✨ Debugger architecture
└── docs/
    └── PROFILER_TROUBLESHOOTING.md  # ✨ Profiler troubleshooting
```

---

## 🚀 Getting Started with Networking

### Server Setup

```gdscript
extends Node

func _ready():
    # Connect signals
    NetworkServer.peer_connected.connect(_on_peer_connected)
    NetworkServer.peer_disconnected.connect(_on_peer_disconnected)
    
    # Start server
    var result = NetworkServer.host_game(7777, 16)
    if result == OK:
        print("Server started on port 7777")

func _on_peer_connected(peer_id: int):
    print("Peer connected: ", peer_id)
    # Spawn player entity for this peer
    var player = FlecsServer.create_entity(world)
    NetworkServer.register_networked_entity(player, "res://player.tscn")
    NetworkServer.set_authority(player, NetworkServer.CLIENT, peer_id)
```

### Client Setup

```gdscript
extends Node

func _ready():
    NetworkServer.connected_to_server.connect(_on_connected)
    NetworkServer.connection_failed.connect(_on_failed)
    
    var result = NetworkServer.join_game("127.0.0.1", 7777)
    if result == OK:
        print("Connecting to server...")

func _on_connected():
    print("Connected to server!")

func _process(delta):
    if NetworkServer.is_connected():
        # Send input to server
        var input = {
            "move": Input.get_vector("left", "right", "up", "down"),
            "jump": Input.is_action_just_pressed("jump")
        }
        NetworkServer.send_input(local_player, input)
```

---

## 🔄 Migration from 1.1.x

**Good news:** This release is **100% backward compatible**!

No code changes required. Existing ECS code continues to work as-is.

### Optional: Adopt New Features

1. **Add Networking** - Use NetworkServer for multiplayer support
2. **Multi-Instance Awareness** - Check InstanceManager for editor conflicts
3. **Enhanced Profiling** - Use new troubleshooting guide for issues

---

## ⚠️ Known Issues

- Networking is in beta - expect API refinements in future releases
- NetworkServer detailed timing not yet integrated with profiler
- Some network edge cases may need additional testing
- Multi-instance detection assumes processes don't die unexpectedly

---

## ✅ Test Coverage

| Area | Status |
|------|--------|
| NetworkServer API | ✅ Manual testing |
| Network Components | ✅ Struct validation |
| InstanceManager | ✅ Multi-instance scenarios |
| Profiler Integration | ✅ Instance status display |
| Editor Plugin | ✅ Dock creation/naming |

---

## 🔜 What's Next

### Version 1.2.0 (Stable)
- Network system stabilization
- Additional network examples
- Performance profiling for network systems
- Comprehensive multiplayer testing
- API documentation refinements

### Version 2.0.0 (Future)
- Godot 4.5+ full support
- Advanced Flecs features (observers, queries)
- Enhanced reflection system
- Breaking API improvements based on feedback
- Removal of deprecated methods

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

**Download:** [GitHub Releases](https://github.com/callmefloof/godot-turbo/releases/tag/v1.2.0-beta.1)  
**Tagged Commit:** v1.2.0-beta.1  
**Previous Version:** v1.1.2-a.1

---

Thank you for using Godot Turbo ECS! 🚀

*This is a beta release. Please report any issues you encounter.*