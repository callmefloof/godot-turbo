# BadAppleSystem Quick Start Guide

## TL;DR - Minimum Working Example

```gdscript
extends Node

var world_rid: RID
var bad_apple_system: BadAppleSystem
var video_player: VideoStreamPlayer
var mm_entity_rid: RID

func _ready():
    # IMPORTANT: Defer to avoid race condition
    call_deferred("setup")

func setup():
    # 1. Create ECS world
    world_rid = FlecsServer.create_world()
    
    # 2. Create and configure video player
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://videos/bad_apple.ogv")
    video_player.autoplay = true
    
    # 3. WAIT ONE FRAME - Critical for video player initialization!
    await get_tree().process_frame
    
    # 4. Create multimesh entity
    mm_entity_rid = create_multimesh_entity(480, 360)
    
    # 5. Create and start BadAppleSystem
    bad_apple_system = BadAppleSystem.new()
    bad_apple_system.set_world_id(world_rid)
    bad_apple_system.set_mm_entity(mm_entity_rid)
    bad_apple_system.set_video_player(video_player)
    bad_apple_system.start()
    
    print("BadAppleSystem started!")

func create_multimesh_entity(width: int, height: int) -> RID:
    var instance_count = width * height
    
    # Create multimesh
    var multi_mesh = MultiMesh.new()
    multi_mesh.transform_format = MultiMesh.TRANSFORM_3D
    multi_mesh.use_colors = true       # MUST be BEFORE instance_count!
    multi_mesh.instance_count = instance_count
    
    # Create mesh
    var mesh = QuadMesh.new()
    mesh.size = Vector2(0.01, 0.01)
    multi_mesh.mesh = mesh
    
    # Position instances in a grid
    for y in height:
        for x in width:
            var idx = y * width + x
            var transform = Transform3D()
            transform.origin = Vector3(
                (x - width / 2.0) * 0.01,
                (y - height / 2.0) * 0.01,
                0
            )
            multi_mesh.set_instance_transform(idx, transform)
            multi_mesh.set_instance_color(idx, Color.WHITE)
    
    # Create entity
    var entity_rid = FlecsServer.create_entity(world_rid)
    
    # Set MultiMeshComponent
    var mm_component = {
        "multi_mesh_id": multi_mesh.get_rid(),
        "instance_count": instance_count,
        "has_data": false,
        "has_color": true,
        "is_instanced": false,
        "transform_format": RenderingServer.MULTIMESH_TRANSFORM_3D
    }
    FlecsServer.set_component(entity_rid, "MultiMeshComponent", mm_component)
    
    return entity_rid

func _exit_tree():
    if bad_apple_system:
        bad_apple_system.queue_free()
    if world_rid.is_valid():
        FlecsServer.free_world(world_rid)
```

---

## Common Mistakes & Fixes

### ❌ MISTAKE #1: Not using call_deferred()
```gdscript
func _ready():
    setup()  # BAD! Video player may not be ready
```

✅ **FIX:**
```gdscript
func _ready():
    call_deferred("setup")  # GOOD! Ensures nodes are in tree
```

---

### ❌ MISTAKE #2: Not waiting for video player
```gdscript
func setup():
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://video.ogv")
    video_player.autoplay = true
    
    bad_apple_system.start()  # BAD! Too fast
```

✅ **FIX:**
```gdscript
func setup():
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://video.ogv")
    video_player.autoplay = true
    
    await get_tree().process_frame  # WAIT!
    
    bad_apple_system.start()  # GOOD!
```

---

### ❌ MISTAKE #3: Wrong MultiMesh property order
```gdscript
var multi_mesh = MultiMesh.new()
multi_mesh.instance_count = 10000
multi_mesh.use_colors = true  # BAD! Too late
```

✅ **FIX:**
```gdscript
var multi_mesh = MultiMesh.new()
multi_mesh.use_colors = true       # GOOD! Set FIRST
multi_mesh.instance_count = 10000  # Then this
```

---

### ❌ MISTAKE #4: Forgetting to set stream
```gdscript
video_player = VideoStreamPlayer.new()
add_child(video_player)
# Forgot to set stream!
bad_apple_system.set_video_player(video_player)
bad_apple_system.start()  # Won't work - no video!
```

✅ **FIX:**
```gdscript
video_player = VideoStreamPlayer.new()
add_child(video_player)
video_player.stream = load("res://video.ogv")  # Don't forget!
video_player.autoplay = true
await get_tree().process_frame
bad_apple_system.start()
```

---

## Troubleshooting

### Problem: Blank screen, no video

**Check #1: Is video file valid?**
```gdscript
if not ResourceLoader.exists("res://videos/bad_apple.ogv"):
    print("ERROR: Video file not found!")
```

**Check #2: Is stream loaded?**
```gdscript
if not video_player.stream:
    print("ERROR: No stream set on video player!")
```

**Check #3: Is video actually playing?**
```gdscript
print("Playing: ", video_player.is_playing())  # Should be true
```

**Check #4: Is texture valid?**
```gdscript
var tex = video_player.get_video_texture()
print("Texture valid: ", tex != null and tex.is_valid())
```

---

### Problem: Video plays but no visual output

**Check #1: MultiMesh entity valid?**
```gdscript
if not mm_entity_rid.is_valid():
    print("ERROR: MultiMesh entity RID is invalid!")
```

**Check #2: use_colors set before instance_count?**
```gdscript
# Wrong order will cause silent failure
multi_mesh.use_colors = true       # MUST be first
multi_mesh.instance_count = count  # Then this
```

**Check #3: Component set correctly?**
```gdscript
# Verify component was set
var comp = FlecsServer.get_component_by_name(mm_entity_rid, "MultiMeshComponent")
print("Component: ", comp)
```

---

### Problem: Performance issues

**Solution: Tune threading parameters**

For **high-end hardware** (8+ cores):
```gdscript
bad_apple_system.use_multithreading = true
bad_apple_system.threading_threshold = 5000
bad_apple_system.max_threads = 16
```

For **low-end hardware** (2-4 cores):
```gdscript
bad_apple_system.use_multithreading = true
bad_apple_system.threading_threshold = 30000
bad_apple_system.max_threads = 4
```

For **debugging** (disable threading):
```gdscript
bad_apple_system.use_multithreading = false
```

---

## Video Formats

### Recommended: Theora (.ogv)
```gdscript
video_player.stream = load("res://video.ogv")  # Best compatibility
```

### Converting videos to Theora:
```bash
# Using FFmpeg
ffmpeg -i input.mp4 -c:v libtheora -q:v 7 -c:a libvorbis -q:a 5 output.ogv
```

---

## Resolution Guidelines

| Resolution | Instances | Recommended Hardware | Threading Threshold |
|------------|-----------|---------------------|---------------------|
| 160x120    | 19,200    | Low-end (2 cores)   | 30,000              |
| 240x180    | 43,200    | Mid-range (4 cores) | 20,000              |
| 480x360    | 172,800   | Good (6 cores)      | 10,000              |
| 1280x720   | 921,600   | High-end (8+ cores) | 5,000               |

---

## Diagnostic Helper

Use the diagnostic script to troubleshoot issues:

```gdscript
extends Node

@export var video_player_path: NodePath = ^"VideoStreamPlayer"

func _ready():
    var debugger = load("res://addons/godot_turbo/ecs/systems/demo/docs/debug_video_player.gd").new()
    add_child(debugger)
    debugger.video_player_path = video_player_path
    debugger.auto_diagnose = true
```

This will print detailed diagnostic information about your video player setup.

---

## Complete Initialization Checklist

- [ ] Use `call_deferred()` in `_ready()`
- [ ] Create video player with `VideoStreamPlayer.new()`
- [ ] Add to tree with `add_child()`
- [ ] Set stream: `video_player.stream = load(...)`
- [ ] Enable autoplay: `video_player.autoplay = true`
- [ ] **Wait one frame**: `await get_tree().process_frame`
- [ ] Create multimesh with `use_colors` **before** `instance_count`
- [ ] Create entity and set MultiMeshComponent
- [ ] Configure BadAppleSystem with world, entity, and video player
- [ ] Call `bad_apple_system.start()`

---

## Performance Tips

1. **Start small**: Test with 160x120 before going to HD
2. **Profile first**: Check if you need multithreading at all
3. **Adjust threshold**: Higher = less threading overhead
4. **Limit threads**: More threads ≠ better (diminishing returns after 8)
5. **Use SIMD**: Automatically enabled on x86_64 and ARM with NEON

---

## See Also

- `bad_apple_example.gd` - Full examples with benchmarking
- `test_bad_apple_fix.gd` - Automated test suite
- `debug_video_player.gd` - Diagnostic helper
- `BAD_APPLE_FIX.md` - Detailed explanation of the race condition fix