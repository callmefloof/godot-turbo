# Flecs Debugger Plugin Documentation

## Overview

The Flecs Debugger Plugin provides real-time inspection and debugging of ECS (Entity Component System) worlds while your game is running in the Godot debugger. It complements the editor dock by allowing you to monitor world state, entity counts, and component composition during gameplay.

## Architecture

### Components

#### 1. FlecsDebuggerPlugin (Editor-Side)
- **File**: `modules/godot_turbo/editor/flecs_debugger_plugin.h/cpp`
- **Role**: Main debugger plugin that runs in the editor
- **Inherits from**: `EditorDebuggerPlugin`
- **Responsibility**: 
  - Creates debugger tabs for each debug session
  - Sends requests to the running game
  - Receives and displays world data
  - Manages per-session UI state

#### 2. FlecsWorldEditorPlugin (Editor Dock)
- **File**: `modules/godot_turbo/editor/flecs_editor_plugin.h/cpp`
- **Role**: Static dock in the editor UI
- **Inherits from**: `EditorPlugin`
- **Responsibility**:
  - Provides permanent inspector in the editor
  - Shows FlecsServer singleton data
  - No network/debugging communication needed

### Message Protocol

The debugger plugin communicates with the running game using Godot's debugger message system.

#### Request Messages (Editor → Game)

```
Message: "flecs:request_worlds"
Data: []
Response: "flecs:worlds" with world list

Message: "flecs:request_entities"
Data: [world_id (uint64), offset (int), count (int)]
Response: "flecs:entities" with entity list

Message: "flecs:request_components"
Data: [world_id (uint64), entity_id (uint64)]
Response: "flecs:components" with component list
```

#### Response Messages (Game → Editor)

```
{
  "type": "world_list",
  "worlds": [
    {"id": 12345, "entity_count": 1500, "name": "MainWorld"},
    ...
  ]
}

{
  "type": "entities",
  "world_id": 12345,
  "entities": [
    {"id": 1, "name": "Player", "components": 5},
    {"id": 2, "name": "Enemy", "components": 3},
    ...
  ]
}

{
  "type": "components",
  "world_id": 12345,
  "entity_id": 1,
  "components": [
    {"name": "Transform", "size": 48},
    {"name": "Velocity", "size": 24},
    ...
  ]
}
```

## Usage

### For Players

1. **Launch Game in Debugger**
   - Start your game with the debugger attached (F5 or Debug → Start Debugger)
   - The game runs and you can set breakpoints

2. **Inspect Worlds**
   - Find the "Flecs Worlds" tab in the debugger panels
   - Worlds appear as they're created in the running game
   - Click worlds to expand and see entities

3. **Monitor Entities**
   - Expand a world to see all entities
   - Click an entity to view its components
   - See component types and sizes in the "Components" tab
   - View world statistics in the "Stats" tab

4. **Refresh Data**
   - Click the "Refresh" button to get latest data from game
   - Use "Batch Size" spinner to adjust how many entities load at once

### For Developers

#### Receiving Debugger Requests in Game Code

You need to implement handlers in your game that receive these requests:

```gdscript
# In your main game script or FlecsServer
func _on_debugger_request(message: String, data: Array):
    if message == "flecs:request_worlds":
        var worlds = flecs_server.get_world_list()
        # Send response back to debugger
        get_stack_debugger().send_message("flecs:worlds", {
            "type": "world_list",
            "worlds": worlds
        })
    elif message == "flecs:request_entities":
        var world_id = data[0]
        var offset = data[1]
        var count = data[2]
        # Fetch and send entities
```

#### C++ Implementation (FlecsServer)

Add methods to FlecsServer to handle debugger requests:

```cpp
void FlecsServer::_process_debugger_request(const String &p_message, const Array &p_data) {
    if (p_message == "flecs:request_worlds") {
        Array world_list = get_world_list();
        // Send back to debugger
        _send_debugger_message("flecs:worlds", world_list);
    }
}

void FlecsServer::_send_debugger_message(const String &p_type, const Variant &p_data) {
    // Use ScriptDebugger to send message back to editor
    if (Engine::get_singleton()->is_editor_hint()) {
        // Not available in non-editor builds
        return;
    }
    // Implementation details...
}
```

## Data Flow Diagram

```
┌─────────────────────────────────────────────────────────────────────┐
│                         GODOT EDITOR                                │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │           FlecsDebuggerPlugin (EditorDebuggerPlugin)        │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │                                                              │   │
│  │  _setup_session()         Setup UI for debug session        │   │
│  │  _capture()               Handle incoming messages          │   │
│  │  _build_debugger_ui()     Create inspector UI               │   │
│  │  _request_world_list()    Send request to game              │   │
│  │                                                              │   │
│  │  ┌───────────────────────────────────────────────────────┐ │   │
│  │  │   UI Components (per session)                         │ │   │
│  │  ├───────────────────────────────────────────────────────┤ │   │
│  │  │ • Worlds Tree (left panel)                            │ │   │
│  │  │ • Entity Inspector (right panel)                      │ │   │
│  │  │ • Components Tab                                      │ │   │
│  │  │ • Stats Tab                                           │ │   │
│  │  │ • Refresh Button                                      │ │   │
│  │  └───────────────────────────────────────────────────────┘ │   │
│  │                                                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                              ▲                                      │
│                              │                                      │
│                 Debugger Message Protocol                          │
│         (debugger_server.send_message/capture)                     │
│                              │                                      │
│                              ▼                                      │
└─────────────────────────────────────────────────────────────────────┘
                               │
                               │ Network (TCP/IP)
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      RUNNING GAME INSTANCE                          │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              FlecsServer (Runtime)                          │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │                                                              │   │
│  │  get_world_list()           Return all worlds              │   │
│  │  get_world_entities()       Return entities in world       │   │
│  │  get_entity_components()    Return components on entity    │   │
│  │  _process_debugger_request()Handle debugger messages       │   │
│  │                                                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │              Flecs ECS Worlds                               │   │
│  ├─────────────────────────────────────────────────────────────┤   │
│  │                                                              │   │
│  │  World 1                  World 2                           │   │
│  │  ├─ Entity 1             ├─ Entity X                       │   │
│  │  ├─ Entity 2             ├─ Entity Y                       │   │
│  │  └─ Entity 3             └─ Entity Z                       │   │
│  │                                                              │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

## File Structure

```
modules/godot_turbo/editor/
├── flecs_editor_plugin.h          # Static editor dock
├── flecs_editor_plugin.cpp
├── flecs_debugger_plugin.h        # Runtime debugger plugin (NEW)
├── flecs_debugger_plugin.cpp      # (NEW)
├── world_info.h                   # World introspection
├── world_info.cpp
└── ...
```

## Session Management

Each debug session (each running game instance) gets its own:
- UI panel/tab in debugger
- World cache
- Entity cache
- Request handler

```cpp
HashMap<int, SessionData> session_data;

struct SessionData {
    Ref<EditorDebuggerSession> session;  // Connection to game
    VBoxContainer *debugger_panel;       // UI container
    Tree *worlds_tree;                   // World hierarchy
    Dictionary world_cache;              // Cached world data
    Dictionary world_dirty;              // Which worlds need refresh
    // ... other UI elements
};
```

## Key Methods

### FlecsDebuggerPlugin

| Method | Purpose |
|--------|---------|
| `_setup_session(int)` | Create UI for new debug session |
| `_capture(String, Array, int)` | Handle incoming debugger messages |
| `_has_capture(String)` | Check if we handle this message type |
| `_request_world_list(int)` | Ask game for world list |
| `_request_world_entities(int, RID)` | Ask game for entities in world |
| `_build_debugger_ui(SessionData&)` | Create UI components |
| `_update_worlds_tree(int)` | Populate tree with world data |

## Building and Compiling

The debugger plugin is automatically compiled when building the editor:

```bash
# Include in SCsub (already done)
if env.editor_build:
    base_sources.extend([
        "editor/flecs_debugger_plugin.cpp"
    ])

# Register in register_types.cpp (already done)
#ifdef TOOLS_ENABLED
    GDREGISTER_CLASS(FlecsDebuggerPlugin);
    EditorDebuggerNode::get_singleton()->add_debugger_plugin(
        memnew(FlecsDebuggerPlugin)
    );
#endif
```

## Future Enhancements

Potential features to add:

1. **Breakpoint on Entity Creation**
   - Break debugger when specific entity is created
   - Break when entity with certain components is created

2. **Live Filtering**
   - Filter worlds by name/ID
   - Filter entities by component type
   - Search entities by ID or name

3. **Component Editing**
   - Edit component values in debugger
   - Reset entity to default state
   - Clone entities during debugging

4. **Performance Monitoring**
   - Show system execution times
   - Monitor frame times per world
   - Profile component query performance

5. **Breakpoint Integration**
   - Set breakpoints on entity operations
   - Break on component add/remove
   - Conditional breakpoints based on entity state

6. **Multi-World Visualization**
   - Show world relationships
   - Display entity ownership
   - Highlight entity spawning chains

## Troubleshooting

### Debugger Tab Doesn't Appear

1. Check that `FlecsDebuggerPlugin` is registered in `register_types.cpp`
2. Verify plugin is in SCsub source list
3. Ensure you're running the editor build (not export template)
4. Check Godot console for initialization errors

### No World Data Appears

1. Verify game is actually running in debugger (not just playing)
2. Check that `FlecsServer` is initialized in running game
3. Ensure debugger messages are being sent from game
4. Look for errors in Godot debugger console

### Crash on Debugger Connection

1. Check `EditorDebuggerSession` is valid before using
2. Verify all pointers are non-null before dereferencing
3. Check for buffer overflows in message parsing
4. Enable address sanitizer: `scons ... use_asan=yes`

## Related Files

- `flecs_editor_plugin.cpp` - Editor dock implementation
- `register_types.cpp` - Module initialization
- `editor/debugger/editor_debugger_plugin.h` - Base class
- `editor/debugger/script_editor_debugger.h` - Debugger API