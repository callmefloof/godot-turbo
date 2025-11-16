# BadAppleSystem - Correct Usage Pattern

## Critical Requirements

When using the `BadAppleSystem`, you **MUST** follow these patterns to ensure proper initialization and functionality:

### 1. ✅ Call `FlecsServer.progress_world()` in `_process()`

This is **REQUIRED** for the ECS world to update every frame:

```gdscript
func _process(delta):
    # REQUIRED: Progress the ECS world every frame
    if world_rid.is_valid():
        FlecsServer.progress_world(world_rid, delta)
```

Without this call, the BadAppleSystem will not update and no visual changes will occur.

### 2. ✅ Use `call_deferred()` for Setup

Always defer your setup in `_ready()` to ensure proper initialization order:

```gdscript
func _ready():
    # IMPORTANT: Defer setup to ensure video player is fully initialized
    call_deferred("setup_basic_example")
```

### 3. ✅ Create VideoStreamPlayer Programmatically

Do **NOT** use `@export var video_player: VideoStreamPlayer` and assign from the scene.
Instead, create it in code:

```gdscript
func setup_basic_example():
    # Create video player programmatically
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://path/to/video.ogv")
    video_player.autoplay = true
```

### 4. ✅ Wait One Frame After Video Player Creation

This is **CRITICAL** to avoid race conditions:

```gdscript
func setup_basic_example():
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://path/to/video.ogv")
    video_player.autoplay = true
    
    # CRITICAL: Wait one frame for NOTIFICATION_ENTER_TREE
    await get_tree().process_frame
    
    # Now safe to create multimesh and BadAppleSystem
    mm_entity_rid = create_multimesh_entity(480, 360)
    # ... rest of setup
```

### 5. ✅ Use Setter Methods (Not Direct Property Access)

Always use the provided setter/getter methods:

```gdscript
# ❌ WRONG
bad_apple_system.use_multithreading = true
bad_apple_system.max_threads = 8
bad_apple_system.mode = 0

# ✅ CORRECT
bad_apple_system.set_use_multithreading(true)
bad_apple_system.set_max_threads(8)
bad_apple_system.set_mode(0)
bad_apple_system.set_flip_y(true)
```

## Complete Minimal Example

```gdscript
extends Node

var world_rid: RID
var bad_apple_system: BadAppleSystem
var video_player: VideoStreamPlayer
var mm_entity_rid: RID

func _ready():
    call_deferred("setup")

func setup():
    # 1. Create ECS world
    world_rid = FlecsServer.create_world()
    
    # 2. Create video player programmatically
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    video_player.stream = load("res://demos/bad apple/bad_apple.ogv")
    video_player.autoplay = true
    
    # 3. CRITICAL: Wait one frame
    await get_tree().process_frame
    
    # 4. Create multimesh entity
    mm_entity_rid = create_multimesh_entity(480, 360)
    
    # 5. Create and configure BadAppleSystem
    bad_apple_system = BadAppleSystem.new()
    bad_apple_system.set_world_id(world_rid)
    bad_apple_system.set_mm_entity(mm_entity_rid)
    bad_apple_system.set_video_player(video_player)
    bad_apple_system.set_use_multithreading(true)
    bad_apple_system.set_mode(0)
    bad_apple_system.set_flip_y(true)
    
    # 6. Start the system
    bad_apple_system.start()
    
    print("BadAppleSystem started successfully")

func _process(delta):
    # REQUIRED: Progress the ECS world every frame
    if world_rid.is_valid():
        FlecsServer.progress_world(world_rid, delta)

func create_multimesh_entity(width: int, height: int) -> RID:
    var instance_count = width * height
    
    var multi_mesh = MultiMesh.new()
    multi_mesh.transform_format = MultiMesh.TRANSFORM_3D
    multi_mesh.use_colors = true  # MUST be set BEFORE instance_count
    multi_mesh.instance_count = instance_count
    
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
    
    var entity_rid = FlecsServer.create_entity(world_rid)
    var mm_component_data = {
        "multi_mesh_id": multi_mesh.get_rid(),
        "instance_count": instance_count,
        "has_data": false,
        "has_color": true,
        "is_instanced": false,
        "transform_format": RenderingServer.MULTIMESH_TRANSFORM_3D
    }
    FlecsServer.set_component(entity_rid, "MultiMeshComponent", mm_component_data)
    
    return entity_rid

func _exit_tree():
    if bad_apple_system:
        bad_apple_system.queue_free()
    
    if world_rid.is_valid():
        FlecsServer.free_world(world_rid)
```

## Common Mistakes to Avoid

### ❌ Forgetting `FlecsServer.progress_world()`
```gdscript
func _process(delta):
    # Missing! System won't update!
    pass
```

### ❌ Using `@export` for VideoStreamPlayer
```gdscript
@export var video_player: VideoStreamPlayer  # WRONG!
```

### ❌ Not Waiting for Video Player Initialization
```gdscript
func setup():
    video_player = VideoStreamPlayer.new()
    add_child(video_player)
    # Missing await! Will cause race condition!
    bad_apple_system.start()  # Too early!
```

### ❌ Direct Property Access
```gdscript
bad_apple_system.mode = 0  # May not work correctly
```

### ❌ Not Using `call_deferred()`
```gdscript
func _ready():
    setup()  # Should be call_deferred("setup")
```

## Configuration Options

### Threading Configuration

```gdscript
# Enable/disable multithreading
bad_apple_system.set_use_multithreading(true)

# Minimum pixel count to use threading (default: 10000)
bad_apple_system.set_threading_threshold(10000)

# Maximum number of threads (default: 8, max: 32)
bad_apple_system.set_max_threads(8)
```

### Visual Modes

```gdscript
# 0 = REGULAR (normal black & white)
bad_apple_system.set_mode(0)

# 1 = INVERTED (inverted black & white)
bad_apple_system.set_mode(1)

# 2 = RANDOM (random pixel inversion using fast hash)
bad_apple_system.set_mode(2)
```

### Image Orientation

```gdscript
# Flip Y axis when reading image (true = correct for standard 3D Y-up)
bad_apple_system.set_flip_y(true)
```

## Performance Guidelines

### High-Performance Setup (8+ cores, HD video)
```gdscript
bad_apple_system.set_use_multithreading(true)
bad_apple_system.set_threading_threshold(5000)   # Lower threshold
bad_apple_system.set_max_threads(16)             # More threads
```

### Low-End Hardware (2-4 cores, SD video)
```gdscript
bad_apple_system.set_use_multithreading(true)
bad_apple_system.set_threading_threshold(30000)  # Higher threshold
bad_apple_system.set_max_threads(4)              # Fewer threads
```

### Single-Threaded (for debugging or very small resolutions)
```gdscript
bad_apple_system.set_use_multithreading(false)
```

## For More Examples

See `bad_apple_example.gd` for complete examples including:
- Basic setup
- High-performance configuration
- Low-end hardware optimization
- Benchmark comparisons
- Runtime mode switching
- Adaptive threading based on performance

## Troubleshooting

**Problem:** Video doesn't play or shows black screen
- **Solution:** Ensure you're using `call_deferred()` and `await get_tree().process_frame`

**Problem:** System doesn't update
- **Solution:** Make sure `FlecsServer.progress_world(world_rid, delta)` is called in `_process()`

**Problem:** Crashes on startup
- **Solution:** Verify you're creating VideoStreamPlayer programmatically, not from scene

**Problem:** Poor performance
- **Solution:** Adjust threading settings based on your hardware and video resolution