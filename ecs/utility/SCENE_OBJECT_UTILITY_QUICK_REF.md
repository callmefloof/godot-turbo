# SceneObjectUtility Quick Reference

## Purpose
Converts Godot scene graph nodes → Flecs ECS entities

## Quick Start

```gdscript
# Get singleton
var util = SceneObjectUtility.get_singleton()

# Convert entire scene
var world_id = FlecsServer.create_world()
var entities = util.create_entities_from_scene(world_id, get_tree())

# Convert subtree
var entities = util.create_entities(world_id, node, TypedArray(), 0, 100)

# Convert single node
var entities = util.create_entity(world_id, node)
```

## Methods

### `create_entities_from_scene(world_id, tree) → Array[RID]`
Entry point - converts entire SceneTree

### `create_entities(world_id, base_node, entities, depth=0, max_depth=10000) → Array[RID]`
Recursive conversion with depth limiting

### `create_entity(world_id, node) → Array[RID]`
Convert single node (1-N entities returned)

### `get_node_script(world_id, node, node_entity) → RID`
Create script resource entity

## Supported Nodes (33 Types)

| Category | Types |
|----------|-------|
| **3D Nav** | NavigationAgent3D, Link, Obstacle, Region |
| **2D Nav** | NavigationAgent2D, Link, Obstacle, Region |
| **3D Physics** | Area3D, RigidBody3D, PhysicsBody3D, Joint3D, SoftBody3D |
| **2D Physics** | Area2D, RigidBody2D, PhysicsBody2D, Joint2D |
| **3D Render** | MeshInstance3D, MultiMesh3D, Camera3D, Lights (3), Particles, ReflectionProbe, Skeleton3D, WorldEnvironment, Viewport, VoxelGI |
| **2D Render** | MeshInstance2D, MultiMesh2D, Camera2D, Lights (2), Particles, Skeleton2D, LightOccluder2D, CanvasItem |
| **Fallback** | Any other Node → generic entity |

## Return Values

| Node Type | Entities Returned |
|-----------|-------------------|
| Most nodes | [node_entity, script_entity?] |
| MultiMesh2D | [parent, instance1, instance2, ..., script?] |
| MultiMesh3D | [parent, instance1, instance2, ...] (⚠️ no script) |
| Unknown | [generic_entity, script?] |

## Key Points

✅ **Type dispatch pattern** - checks specific types before generic bases  
✅ **Delegates** to specialized utilities (Navigation, Physics, Render)  
✅ **Scripts** become child entities in ECS hierarchy  
✅ **Depth limiting** prevents stack overflow (default max: 10000)  
⚠️ **Not thread-safe** - use on main thread only  
⚠️ **MultiMesh3D** doesn't attach scripts (inconsistency)

## BUG FIX (Applied)

**Issue:** `create_entities()` didn't append child results  
**Fixed:** Added `result_entities.append_array(child_entity_result)`  
**Impact:** Child nodes now properly converted to entities

## Examples

### Full Scene Conversion
```gdscript
extends Node

func _ready():
    var world = FlecsServer.create_world()
    var util = SceneObjectUtility.get_singleton()
    var entities = util.create_entities_from_scene(world, get_tree())
    print("Converted %d nodes to entities" % entities.size())
```

### Subtree with Depth Limit
```gdscript
var entities = util.create_entities(
    world_id,
    get_node("Player"),
    TypedArray(),
    0,   # start depth
    5    # max 5 levels deep
)
```

### Single Node
```gdscript
var node = get_node("Enemy")
var entities = util.create_entity(world_id, node)
var main_entity = entities[0]
var has_script = entities.size() > 1
```

### Batch Processing
```gdscript
# Convert one subtree per frame to avoid lag
var nodes_to_convert = [node1, node2, node3]
var index = 0

func _process(delta):
    if index < nodes_to_convert.size():
        var entities = util.create_entities(world, nodes_to_convert[index], TypedArray())
        index += 1
```

## Performance

| Scene Size | Expected Time | Notes |
|------------|---------------|-------|
| Small (<100) | <1ms | Instant |
| Medium (1000) | ~10-50ms | May cause frame drop |
| Large (10000+) | >100ms | Use incremental conversion |

**Optimization:** Convert large scenes over multiple frames

## Error Handling

- Returns **empty array** on error (null pointers, depth exceeded)
- Uses `ERR_FAIL_V` / `ERR_FAIL_COND_V` macros
- Validates all pointer parameters
- Checks depth limits

## Delegation Targets

| Node Type | Utility | Method |
|-----------|---------|--------|
| Navigation3D | Navigation3DUtility | `create_nav_*_with_object()` |
| Navigation2D | Navigation2DUtility | `create_nav_*_with_object()` |
| Physics3D | Physics3DUtility | `create_*_with_object()` |
| Physics2D | Physics2DUtility | `create_*_with_object()` |
| Render3D | RenderUtility3D | `create_*_with_object()` |
| Render2D | RenderUtility2D | `create_*_with_object()` |
| Scripts | ResourceObjectUtility | `create_resource_entity()` |

## Common Patterns

### Pattern 1: Full Scene → ECS
```gdscript
var entities = SceneObjectUtility.get_singleton()
    .create_entities_from_scene(world_id, get_tree())
```

### Pattern 2: Subtree → ECS
```gdscript
var entities = SceneObjectUtility.get_singleton()
    .create_entities(world_id, root_node, TypedArray())
```

### Pattern 3: Node + Children → ECS with Limit
```gdscript
var entities = SceneObjectUtility.get_singleton()
    .create_entities(world_id, node, TypedArray(), 0, 3)
```

## Troubleshooting

| Issue | Cause | Solution |
|-------|-------|----------|
| Empty array returned | Null tree/node | Check pointers before calling |
| Missing child entities | Depth limit hit | Increase max_depth parameter |
| Script not in ECS | MultiMesh3D node | Known issue - use 2D or file bug |
| Performance lag | Large scene | Use incremental conversion |
| Crash | Thread safety | Call only from main thread |

## See Also

- **SCENE_OBJECT_UTILITY_DOC.md** - Full documentation (549 lines)
- **VALIDATION_SUMMARY.md** - Validation results and testing
- FlecsServer API documentation
- Navigation/Physics/Render utility classes

---

**Version:** 1.0  
**Status:** ✅ Production Ready  
**Last Updated:** 2024-11-12