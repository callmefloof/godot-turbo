# Quick Start: Flecs Editor Tools

## 🎯 Overview

The Flecs module provides two main editor tools:
1. **Flecs Worlds Dock** - Browse and inspect ECS worlds/entities in the editor
2. **Remote Debugger** - Debug running game instances

## 🚀 Quick Start

### Step 1: Build the Editor

```bash
cd godot
scons platform=linuxbsd target=editor -j8
```

### Step 2: Launch Editor

```bash
./bin/godot.linuxbsd.editor.x86_64
```

### Step 3: Open Flecs Worlds Dock

1. Look at the **bottom panel** in the editor
2. Find and click the **"Flecs Worlds"** tab
3. You should see a split view with:
   - Left: World/entity tree
   - Right: Inspector and Profiler tabs

## 📊 Using the Profiler

### Access the Profiler

1. Open **Flecs Worlds** dock (bottom panel)
2. Select a world from the tree on the left
3. Click the **"Profiler"** tab on the right panel

### Start Profiling

1. Click **"Start Profiling"** button
2. Run your scene (F5 or Play button)
3. Watch metrics populate in real-time

### Profiler Features

| Feature | Description |
|---------|-------------|
| **Graph** | Visual timeline of system execution times |
| **Metrics Tree** | Detailed statistics per system/query |
| **Display Modes** | Frame time, Average, or Percentage view |
| **Clear** | Reset all collected metrics |
| **Pause/Resume** | Control system execution (requires backend) |
| **CSV Export** | Export data for analysis (requires dialog) |

### Understanding Metrics

- **Time**: Execution time in microseconds (μs)
- **Calls**: Number of times system executed
- **Entities**: Number of entities processed
- **Min/Max**: Fastest/slowest execution
- **Avg**: Average execution time
- **Median**: Middle value (50th percentile)
- **StdDev**: Variation in execution time

## 🐛 Remote Debugger

### Start Remote Debugging

1. Press **F5** to run your project from the editor
2. In the editor, find the **Debugger panel** (bottom)
3. Look for the **"Flecs Worlds"** debugger tab
4. Click **"Refresh"** to query the running game

### Remote Features

- View worlds/entities in the running game
- Inspect component data at runtime
- Real-time updates as the game executes
- Separate from editor-local inspection

### Message Flow

```
Editor                 Game
  |                     |
  |--request_worlds---->|
  |<----worlds----------|
  |                     |
  |--request_entities-->|
  |<----entities--------|
  |                     |
  |--request_components>|
  |<----components------|
```

## 🎨 UI Layout

```
┌─────────────────────────────────────────────────────────┐
│ Editor (Godot)                                          │
├─────────────────────────────────────────────────────────┤
│                                                         │
│  [Scene] [2D] [3D] [Script]                            │
│                                                         │
├─────────────────────────────────────────────────────────┤
│ Bottom Panel:                                           │
│  [Output] [Debugger] [Audio] [Animation] [Flecs Worlds]│
│                                                         │
│  ┌─────────────────┬──────────────────────────────────┐│
│  │ Worlds Tree     │ ┌──────────────────────────────┐ ││
│  │                 │ │ [Inspector] [Profiler]       │ ││
│  │ ▼ Flecs Worlds  │ └──────────────────────────────┘ ││
│  │   ▶ World [123] │                                  ││
│  │   ▶ World [456] │  Profiler:                       ││
│  │                 │  ┌──────────────────────────────┐││
│  │ [Refresh]       │  │ [Start] [Clear] [Export]     │││
│  │ [Expand All]    │  ├──────────────────────────────┤││
│  │ [Collapse All]  │  │ ░░░░░░▃▃▃▃░░░░░ (graph)      │││
│  │                 │  ├──────────────────────────────┤││
│  │                 │  │ System_A    | 1.2ms | 60 fps│││
│  │                 │  │ System_B    | 0.8ms | 60 fps│││
│  │                 │  │ System_C    | 2.1ms | 30 fps│││
│  │                 │  └──────────────────────────────┘││
│  └─────────────────┴──────────────────────────────────┘│
└─────────────────────────────────────────────────────────┘
```

## ⚙️ Toolbar Controls

### Flecs Worlds Toolbar

| Button | Function |
|--------|----------|
| **Refresh** | Reload world list and entities |
| **Expand All** | Expand entire tree hierarchy |
| **Collapse All** | Collapse entire tree hierarchy |
| **Batch Size** | Number of entities to load per batch |

### Profiler Toolbar

| Control | Function |
|---------|----------|
| **Start/Stop** | Toggle profiling on/off |
| **Clear** | Clear all metrics |
| **Pause Systems** | Pause all systems in world |
| **Resume Systems** | Resume all systems in world |
| **Display Mode** | Frame Time / Average / Percentage |
| **Cursor Metric** | Jump to specific frame in timeline |

## 🔧 Common Tasks

### Task: Profile a Specific World

```
1. Open Flecs Worlds dock
2. Click on the world you want to profile
3. Switch to Profiler tab
4. Click "Start Profiling"
5. Run your scene
6. Observe metrics in real-time
```

### Task: Find Slow Systems

```
1. Start profiling
2. Run your scene for ~5 seconds
3. Look at Metrics Tree
4. Sort by "Time" column (click header)
5. Top entries are your slowest systems
```

### Task: Debug Remote Entity

```
1. Run project (F5)
2. Open Debugger panel
3. Click "Flecs Worlds" debugger tab
4. Click "Refresh"
5. Expand world → expand entity
6. View components in Inspector tab
```

### Task: Export Profiling Data

```
1. Collect profiling data (let it run)
2. Click "Export CSV" button
3. Choose save location
4. Open in spreadsheet software
5. Analyze trends, create charts
```

## 📋 Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| **F5** | Run project (starts remote debug) |
| **F8** | Run project without debugger |
| **Ctrl+R** | Refresh worlds (when focused) |
| **Spacebar** | Toggle profiling (when focused) |

## ⚠️ Troubleshooting

### Profiler Tab Not Visible

**Problem**: Only see Inspector tab, no Profiler

**Solution**:
1. Ensure you built with `target=editor`
2. Check SCsub includes `flecs_profiler.cpp`
3. Rebuild: `scons platform=linuxbsd target=editor --clean`

### Remote Debugger Empty

**Problem**: Debugger tab shows no worlds

**Solution**:
1. Ensure game is running (F5, not F8)
2. Click "Refresh" button
3. Check FlecsRuntimeDebugger is initialized (console output)
4. Verify project has active Flecs worlds

### No Profiling Data

**Problem**: Profiler shows "No data"

**Solution**:
1. Select a world from the left tree
2. Ensure "Start Profiling" button is pressed (toggled on)
3. Run the scene (profiler needs active systems)
4. Check FlecsServer has running systems

### Dock Not Appearing

**Problem**: No "Flecs Worlds" tab in bottom panel

**Solution**:
1. Check module compiled: `ls modules/godot_turbo/*.os`
2. Rebuild entire project
3. Check register_types.cpp has `GDREGISTER_CLASS(FlecsWorldEditorPlugin)`
4. Verify `#ifdef TOOLS_ENABLED` is active

## 🎓 Advanced Usage

### Performance Optimization Workflow

1. **Baseline**: Profile normal gameplay
2. **Identify**: Find slow systems (>5ms)
3. **Isolate**: Test each system individually
4. **Optimize**: Refactor slow code paths
5. **Verify**: Re-profile to confirm improvements

### Batch Tuning

Large worlds may have thousands of entities. Adjust batch size:

```
Small worlds (< 1000 entities): Batch 200-500
Medium worlds (1000-10k entities): Batch 100-200
Large worlds (> 10k entities): Batch 50-100
```

Lower batch = faster initial load, more round trips
Higher batch = slower initial load, fewer round trips

### Remote Debugging Tips

- Use remote debugger for **runtime state** (actual gameplay)
- Use local dock for **design-time inspection** (editor mode)
- Remote data updates automatically as game runs
- Pause game to inspect frozen state

## 🔗 Related Documentation

- `FLECS_EDITOR_UI_FIX.md` - Detailed architecture and implementation
- `FLECS_SERVER_API.md` - FlecsServer API reference
- `TYPES_DOCUMENTATION.md` - ECS type system
- `QUERY_API.md` - Query system documentation

## 📝 Notes

### Current Status

✅ **Working**:
- Dock UI and layout
- World/entity tree browsing
- Entity inspector
- Profiler UI and data collection
- Remote debugger protocol
- Real entity loading
- System metrics display
- Multi-instance coordination (InstanceManager)
- Network inspector dock (v1.2.0+)

### Performance Considerations

- Profiling adds ~1-2% overhead
- Remote debugger uses network bandwidth
- Large worlds (>10k entities) may be slow to browse
- Consider pagination for huge entity lists

## 🆘 Getting Help

1. Check diagnostics: Look for errors in editor console
2. Review logs: `~/.local/share/godot/app_userdata/[project]/logs/`
3. Rebuild clean: `scons --clean && scons platform=linuxbsd target=editor`
4. Check module: `grep -r "FlecsWorldEditorPlugin" godot/modules/godot_turbo/`

## 🎉 You're Ready!

The Flecs editor tools are now set up and ready to use. Start by:
1. Opening the Flecs Worlds dock
2. Selecting a world
3. Exploring the Inspector and Profiler tabs

Happy profiling! 🚀