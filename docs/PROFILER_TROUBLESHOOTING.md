# Flecs Profiler Troubleshooting Guide

## Common Issues and Solutions

### Issue: Start Button is Disabled or Greyed Out

**Symptoms:**
- The "Start" button in the profiler cannot be clicked
- Button appears disabled/greyed out

**Solutions:**

1. **Check if FlecsServer singleton exists:**
   ```gdscript
   # In GDScript console or script
   print(Engine.has_singleton("FlecsServer"))
   # Should return true
   ```

2. **Verify the plugin loaded correctly:**
   - Check the console for: `FlecsProfilerPlugin: Initialized successfully using EditorProfiler`
   - If missing, the plugin failed to initialize

3. **Restart the editor:**
   - Close and reopen Godot
   - The profiler should initialize on startup

**Cause:** The profiler may not have connected to FlecsServer properly during initialization.

---

### Issue: Start Button Works But No Data Appears

**Symptoms:**
- "Start" button can be pressed and changes to "Stop"
- Graph remains empty
- No systems shown in the list

**Solutions:**

1. **Run a scene with Flecs systems:**
   - The profiler only shows data when a scene is running
   - Press F6 to run the current scene
   - Or press F5 to run the project

2. **Verify Flecs worlds exist:**
   ```gdscript
   var flecs = Engine.get_singleton("FlecsServer")
   var worlds = flecs.get_world_list()
   print("World count: ", worlds.size())
   # Should be > 0 when scene is running
   ```

3. **Check if systems are registered:**
   ```gdscript
   var flecs = Engine.get_singleton("FlecsServer")
   var worlds = flecs.get_world_list()
   if worlds.size() > 0:
       var all_systems = flecs.get_all_systems(worlds[0])
       print("Script systems: ", all_systems["script"].size())
       print("C++ systems: ", all_systems["cpp"].size())
   ```

4. **Verify systems are actually running:**
   - Systems with zero entities matching their queries won't show activity
   - Check that entities exist with the components your systems need
   - Paused systems won't show activity

5. **Check the info label:**
   - Should say "Tracking X active systems" if working
   - If says "No active systems", run a scene or add entities

**Cause:** Profiler needs active Flecs worlds and systems to display data.

---

### Issue: "No active systems" Message

**Symptoms:**
- Info label shows "No active systems (run scene to see data)"
- Profiler is started but empty

**Solutions:**

1. **Start your game/scene:**
   - Press F5 (run project) or F6 (run scene)
   - Systems only execute when the game is running

2. **Add entities to match system queries:**
   ```gdscript
   # Example: Create entities that match your systems
   var flecs = Engine.get_singleton("FlecsServer")
   var world = flecs.get_world_list()[0]
   
   # Create entity with components your systems need
   var entity = flecs.create_entity(world)
   # Add components...
   ```

3. **Check system queries:**
   - Verify your system queries match actual entities
   - Use the Flecs Worlds Editor to inspect entities

4. **Unpause systems:**
   ```gdscript
   var flecs = Engine.get_singleton("FlecsServer")
   var paused = flecs.is_script_system_paused(world_id, system_id)
   if paused:
       flecs.set_script_system_paused(world_id, system_id, false)
   ```

**Cause:** Systems need entities to process and must be running (not paused).

---

### Issue: Only Some Systems Appear

**Symptoms:**
- Some systems show in profiler
- Expected systems are missing

**Solutions:**

1. **Check if missing systems are paused:**
   ```gdscript
   var all_systems = flecs.get_all_systems(world_id)
   for sys_data in all_systems["script"]:
       var sys_rid = sys_data["rid"]
       var paused = flecs.is_script_system_paused(world_id, sys_rid)
       print(sys_data["rid"], " paused: ", paused)
   ```

2. **Verify systems have activity:**
   - Systems with 0 time and 0 calls are filtered out
   - Check if system queries match any entities
   - Look at `get_system_metrics()` directly to see all systems:
     ```gdscript
     var metrics = flecs.get_system_metrics(world_id)
     for system in metrics["systems"]:
         print(system["name"], " - time:", system["time_usec"], " calls:", system["call_count"])
     ```

3. **C++ systems show limited info:**
   - C++ systems only show name and pause state
   - They don't have timing instrumentation yet
   - This is expected behavior

**Cause:** Inactive systems are automatically filtered. C++ systems have limited metrics.

---

### Issue: Metrics Show Zero Time

**Symptoms:**
- Systems appear in list
- All timing values are 0 microseconds
- Call counts are 0

**Solutions:**

1. **Ensure the world is progressing:**
   ```gdscript
   # In your game loop
   var flecs = Engine.get_singleton("FlecsServer")
   var delta = get_process_delta_time()
   flecs.progress_world(world_id, delta)
   ```

2. **Check if systems are actually executing:**
   - Add print statements in system callbacks
   - Verify entities exist with matching components

3. **Verify system isn't paused:**
   ```gdscript
   if flecs.is_script_system_paused(world_id, system_id):
       print("System is paused!")
   ```

**Cause:** Systems need to execute to generate timing data. World must progress.

---

### Issue: Remote Connection Fails

**Symptoms:**
- Trying to profile a running game instance remotely
- Connection fails or times out

**Solution:**

**This is expected behavior.** The Flecs Profiler is designed for **editor-only** profiling:
- Profiles systems while running scenes in the editor (F5/F6)
- Does NOT support remote profiling of exported builds
- Does NOT connect to external game instances

If you need remote profiling:
1. Use `get_system_metrics()` API in your game code
2. Export metrics to JSON or network
3. Or use Godot's built-in remote debugger for general profiling

**Cause:** Profiler collects data from editor's FlecsServer, not remote instances.

---

### Issue: Profiler Crashes Editor

**Symptoms:**
- Editor crashes when opening profiler
- SIGSEGV or SIGABRT errors

**Solutions:**

1. **Verify clean build:**
   ```bash
   cd godot
   scons -j8 target=editor module_godot_turbo_enabled=yes --clean
   scons -j8 target=editor module_godot_turbo_enabled=yes
   ```

2. **Check for StringName errors:**
   - Look for "Condition '!configured' is true" in console
   - This indicates static initialization issues
   - Report as a bug if this occurs

3. **Disable the profiler plugin:**
   - Temporarily disable in module registration
   - Check if editor works without it

**Cause:** Build issues or static initialization problems (should be fixed in current version).

---

### Issue: Performance Impact While Profiling

**Symptoms:**
- Game runs slower when profiler is active
- Frame rate drops

**Solutions:**

1. **This is normal:** Profiling has overhead
   - Typical overhead: 0.1-0.5 ms per frame
   - Acceptable for development profiling

2. **Reduce collection frequency:**
   ```cpp
   // In flecs_profiler_plugin.cpp
   collect_timer->set_wait_time(0.2); // 200ms instead of 100ms
   ```

3. **Disable detailed timing:**
   ```gdscript
   # Only enable for specific systems you're profiling
   flecs.set_script_system_detailed_timing(world_id, system_id, false)
   ```

4. **Stop profiling when not needed:**
   - Click "Stop" button when not actively profiling
   - Profiling stops data collection

**Cause:** Profiling requires collecting and processing metrics every frame.

---

### Issue: Detailed Statistics Missing

**Symptoms:**
- No median_usec, p99_usec, or stddev_usec fields
- Only basic timing shown

**Solutions:**

1. **Enable detailed timing:**
   ```gdscript
   var flecs = Engine.get_singleton("FlecsServer")
   flecs.set_script_system_detailed_timing(world_id, system_id, true)
   ```

2. **Wait for system to execute:**
   - Detailed stats require at least one invocation
   - Run the scene and let systems process entities

3. **Check if enabled:**
   ```gdscript
   var enabled = flecs.get_script_system_detailed_timing(world_id, system_id)
   print("Detailed timing enabled: ", enabled)
   ```

**Cause:** Detailed timing is opt-in to reduce overhead. Must be explicitly enabled.

---

### Issue: Multiple Worlds Confusing Display

**Symptoms:**
- Multiple categories shown in profiler
- Hard to tell which systems belong to which world

**Solutions:**

1. **Typically only one world exists:**
   - Most games use a single Flecs world
   - Multiple worlds are rare

2. **Check world count:**
   ```gdscript
   var worlds = flecs.get_world_list()
   print("Worlds: ", worlds.size())
   ```

3. **Categories show world index:**
   - "Flecs World 0", "Flecs World 1", etc.
   - Systems grouped by their world

**Cause:** Multiple Flecs worlds create separate profiler categories.

---

## Diagnostic Commands

### Check Profiler Status
```gdscript
# Get plugin instance (if exposed)
# Or check via console output
```

### Verify FlecsServer
```gdscript
var flecs = Engine.get_singleton("FlecsServer")
print("FlecsServer exists: ", flecs != null)
print("Worlds: ", flecs.get_world_list().size())
```

### List All Systems
```gdscript
var flecs = Engine.get_singleton("FlecsServer")
var worlds = flecs.get_world_list()
if worlds.size() > 0:
    var systems = flecs.get_all_systems(worlds[0])
    print("Script systems:")
    for sys in systems["script"]:
        print("  - ", sys)
    print("C++ systems:")
    for sys in systems["cpp"]:
        print("  - ", sys)
```

### Get Raw Metrics
```gdscript
var flecs = Engine.get_singleton("FlecsServer")
var worlds = flecs.get_world_list()
if worlds.size() > 0:
    var metrics = flecs.get_system_metrics(worlds[0])
    print(JSON.stringify(metrics, "  "))
```

### Export Metrics to File
```gdscript
var flecs = Engine.get_singleton("FlecsServer")
var worlds = flecs.get_world_list()
if worlds.size() > 0:
    var metrics = flecs.get_system_metrics(worlds[0])
    var json = JSON.stringify(metrics, "  ")
    var file = FileAccess.open("user://profiler_debug.json", FileAccess.WRITE)
    if file:
        file.store_string(json)
        file.close()
        print("Metrics exported to user://profiler_debug.json")
```

---

## Expected Behavior

### When Profiler Works Correctly

1. **Editor startup:**
   - Console shows: "FlecsProfilerPlugin: Initialized successfully using EditorProfiler"
   - Profiler dock appears in bottom-right (or where you placed it)
   - Info label shows: "Press 'Start' to begin profiling"

2. **After pressing Start (without running scene):**
   - Button changes to "Stop"
   - Info label shows: "Profiling active..." or "No active systems"
   - Graph remains empty (expected)

3. **After pressing Start AND running scene (F5/F6):**
   - Systems appear in the list
   - Graph shows activity
   - Entity counts visible in system names
   - Info label shows: "Tracking X active systems"

4. **Graph display:**
   - X-axis: Frame number
   - Y-axis: Time (microseconds or milliseconds)
   - Lines for each active system
   - Zoomable and scrollable

5. **After pressing Stop:**
   - Button changes to "Start"
   - Data collection stops
   - Graph frozen at current state
   - Info label shows: "Press 'Start' to begin profiling"

---

## Performance Expectations

### Normal Overhead
- **Collection frequency:** 100ms (10 times per second)
- **Overhead per frame:** 0.1-0.5 ms
- **Memory per system:** ~50-200 bytes
- **With detailed timing:** +8 bytes per sample per system

### Acceptable Frame Times (at 60 FPS)
- **Total ECS time:** < 16ms (one frame budget)
- **Individual system:** < 5ms (typical)
- **Warning threshold:** > 10ms (slow system)

### When to Worry
- Total ECS time > 16ms consistently → Performance bottleneck
- Individual system > 10ms → Investigate optimization
- Many systems with high entity counts → Consider batching
- System time increasing over frames → Memory leak or accumulation issue

---

## Getting Help

If issues persist after trying these solutions:

1. **Check documentation:**
   - `PROFILER_API.md` - Complete API reference
   - `PROFILER_README.md` - Quick start guide
   - `examples/profiler_example.gd` - Working examples

2. **Verify build:**
   - Clean rebuild: `scons --clean && scons`
   - Check for compile errors
   - Ensure module is enabled

3. **Collect diagnostic info:**
   - Console output
   - Metrics from `get_system_metrics()`
   - System list from `get_all_systems()`
   - World count

4. **Report bugs with:**
   - Godot version
   - Module version
   - Steps to reproduce
   - Console output
   - Diagnostic data

---

## Quick Reference

| Symptom | Most Likely Cause | Quick Fix |
|---------|------------------|-----------|
| Start button disabled | Plugin init failed | Restart editor |
| No data appears | Scene not running | Press F5/F6 |
| "No active systems" | No entities | Create entities |
| All zeros | World not progressing | Call progress_world() |
| Missing systems | Systems paused or inactive | Check pause state |
| No detailed stats | Not enabled | Enable detailed timing |
| Slow performance | Normal profiling overhead | Acceptable for dev |
| Crash on open | Build issue | Clean rebuild |

---

**Last Updated:** 2025-01-29  
**Applies to:** Godot Turbo Module v1.2+