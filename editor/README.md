# Flecs Editor Integration

High-performance editor tools for inspecting and debugging Flecs ECS worlds in Godot.

## Components

### Flecs Worlds Dock

The main editor dock for browsing ECS worlds and entities.

**Features:**
- Browse all active Flecs worlds
- Expand worlds to view entities (lazy-loaded for performance)
- Entity search/filter
- Component inspector with inline editors
- Configurable batch loading for large worlds (100k+ entities)

**Location:** Bottom panel → "Flecs Worlds" tab

### Flecs Profiler

Real-time performance monitoring for ECS systems.

**Features:**
- Per-system timing metrics
- Entity count tracking
- Call count monitoring
- Graphical timeline display
- CSV export capability

**Location:** Bottom panel → "Flecs Profiler" tab

**Usage:**
1. Click "Start" to begin profiling
2. Run your scene (F5)
3. View real-time metrics

See `docs/PROFILER_README.md` and `docs/PROFILER_API.md` for detailed documentation.

### Entity Inspector

Detailed component inspection for selected entities.

**Features:**
- Collapsible component blocks
- Type-aware field editors (bool, int, float, Vector2/3, Color, etc.)
- Nested Dictionary/Array support
- Tag component display

## Quick Start

### Opening the Docks

1. Build Godot with `tools=yes`:
   ```bash
   scons platform=linux tools=yes target=editor
   ```

2. Launch the editor

3. Find "Flecs Worlds" and "Flecs Profiler" in the bottom panel

### Inspecting Entities

1. Open "Flecs Worlds" dock
2. Click "Refresh" to load worlds
3. Expand a world to see entities
4. Click an entity to inspect components

### Using the Profiler

1. Open "Flecs Profiler" dock
2. Click "Start"
3. Run your scene
4. View system performance metrics

## Architecture

```
FlecsWorldEditorPlugin (EditorPlugin)
├── Toolbar (Refresh, Expand All, Collapse All, Batch Size)
├── Search Field
├── Worlds Tree (lazy-loaded entities)
└── Entity Inspector (component viewer)

FlecsProfilerPlugin (EditorPlugin)
└── EditorProfiler (built-in Godot profiler widget)
    └── Timer → FlecsServer.get_system_metrics()
```

## Performance

The editor tools are optimized for large worlds:

| Operation | Performance |
|-----------|-------------|
| Open dock | <5ms |
| Refresh worlds | <5ms |
| Expand 100k entities | <500ms (first load) |
| Expand cached | <1ms |
| Select entity | <10ms |

**Key optimizations:**
- Lazy-loading (only load on expand)
- Batch processing (configurable page size)
- Caching with dirty flags
- Zero-copy iteration helpers

## Files

| File | Purpose |
|------|---------|
| `flecs_editor_plugin.h/cpp` | Main worlds dock plugin |
| `flecs_profiler_plugin.h/cpp` | Profiler dock plugin |
| `flecs_entity_inspector.h/cpp` | Component inspector widget |
| `flecs_profiler.h/cpp` | Profiler implementation |
| `world_info.h/cpp` | Efficient world iteration helpers |
| `proxy_data.h` | Cache data structures |
| `editor_integration.h` | Integration utilities |

## Troubleshooting

### Dock doesn't appear

- Verify build used `tools=yes`
- Check for compilation errors
- Restart the editor after rebuild

### "No Flecs worlds" message

- Create a Flecs world in your game code first
- Click "Refresh" button

### Slow expansion

- Reduce batch size (default: 200)
- Large worlds take longer on first load
- Subsequent loads are cached

### Profiler shows no data

- Click "Start" button first
- Run your scene (F5/F6)
- Verify systems are registered and not paused

## Related Documentation

- `../README.md` - Main module documentation
- `../docs/PROFILER_README.md` - Profiler quick start
- `../docs/PROFILER_API.md` - Profiler API reference
- `../docs/PROFILER_TROUBLESHOOTING.md` - Profiler troubleshooting
- `../REMOTE_DEBUGGING_GUIDE.md` - Remote debugging setup
- `../DEBUGGER_PLUGIN_DOCUMENTATION.md` - Debugger plugin details