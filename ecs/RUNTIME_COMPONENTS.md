# Runtime Component Creation

## Overview

The `create_runtime_component()` method allows you to dynamically create ECS component types at runtime using Flecs' reflection API. This enables data-driven component definitions without requiring C++ compilation.

> **Note:** This method replaces the deprecated `register_component_type()` which used heap-allocated Dictionary wrappers. The old method will be removed in v2.0.0. See [MIGRATION_REGISTER_TO_RUNTIME.md](MIGRATION_REGISTER_TO_RUNTIME.md) for migration instructions.

## Method Signature

```gdscript
RID create_runtime_component(world_id: RID, component_name: String, fields: Dictionary) -> RID
```

### Parameters

- **world_id** (`RID`): The world in which to create the component type
- **component_name** (`String`): Unique name for the component type
- **fields** (`Dictionary`): Field definitions where keys are field names and values define the field types

### Returns

- **RID**: Component type identifier on success, invalid RID on failure

### Error Handling

The method returns an invalid RID and prints an error if:
- The world_id is invalid
- A component with the same name already exists
- Component creation fails internally

## Supported Types

The method automatically detects and supports all Godot Variant types based on the values provided:

### Primitives
- `bool` - Boolean values
- `int` - 64-bit integers
- `float` - Double-precision floats

### Godot Core Types
- `String` - String values
- `StringName` - Optimized string names
- `RID` - Resource identifiers

### Math Types - 2D
- `Vector2` - 2D float vectors
- `Vector2i` - 2D integer vectors
- `Rect2` - 2D rectangles (float)
- `Rect2i` - 2D rectangles (integer)
- `Transform2D` - 2D transformations

### Math Types - 3D
- `Vector3` - 3D float vectors
- `Vector3i` - 3D integer vectors
- `Vector4` - 4D float vectors
- `Vector4i` - 4D integer vectors
- `Quaternion` - Rotation quaternions
- `Plane` - 3D planes
- `AABB` - Axis-aligned bounding boxes
- `Basis` - 3x3 transformation matrices
- `Transform3D` - 3D transformations
- `Projection` - 4x4 projection matrices

### Visual Types
- `Color` - RGBA color values

### Container Types
- `Array` - Dynamic arrays
- `Dictionary` - Key-value dictionaries

### Fallback
- `Variant` - Generic variant type (used for unsupported types)

## Usage Examples

### Example 1: Simple Component with Primitives

```gdscript
var flecs = FlecsServer.get_singleton()
var world_id = flecs.create_world()
flecs.init_world(world_id)

# Create a Health component
var health_fields = {
    "current": 100,      # int
    "max": 100,          # int
    "regen_rate": 5.0    # float
}
var health_comp = flecs.create_runtime_component(world_id, "Health", health_fields)

# Use the component
var entity = flecs.create_entity(world_id)
flecs.add_component(entity, health_comp)
flecs.set_component(entity, "Health", {
    "current": 75,
    "max": 100,
    "regen_rate": 2.5
})

var data = flecs.get_component_by_name(entity, "Health")
print("Current health: ", data["current"])
```

### Example 2: Component with Godot Types

```gdscript
# Create a Transform component
var transform_fields = {
    "position": Vector3.ZERO,
    "rotation": Quaternion.IDENTITY,
    "scale": Vector3.ONE
}
var transform_comp = flecs.create_runtime_component(world_id, "Transform", transform_fields)

# Use it on an entity
var entity = flecs.create_entity(world_id)
flecs.add_component(entity, transform_comp)
flecs.set_component(entity, "Transform", {
    "position": Vector3(10, 5, 0),
    "rotation": Quaternion.from_euler(Vector3(0, PI/4, 0)),
    "scale": Vector3(2, 2, 2)
})
```

### Example 3: Component with Complex Types

```gdscript
# Create an Inventory component
var inventory_fields = {
    "items": [],                    # Array
    "capacity": 20,                 # int
    "metadata": {},                 # Dictionary
    "equipped_weapon_id": RID()     # RID
}
var inventory_comp = flecs.create_runtime_component(world_id, "Inventory", inventory_fields)

# Use it
var player = flecs.create_entity(world_id)
flecs.add_component(player, inventory_comp)
flecs.set_component(player, "Inventory", {
    "items": ["sword", "shield", "potion"],
    "capacity": 20,
    "metadata": {"gold": 500, "keys": 3},
    "equipped_weapon_id": weapon_rid
})
```

### Example 4: 2D Physics Component

```gdscript
# Create a Physics2D component
var physics_fields = {
    "position": Vector2.ZERO,
    "velocity": Vector2.ZERO,
    "acceleration": Vector2.ZERO,
    "mass": 1.0,
    "friction": 0.1,
    "is_static": false
}
var physics_comp = flecs.create_runtime_component(world_id, "Physics2D", physics_fields)

# Create entities with physics
var ball = flecs.create_entity(world_id)
flecs.add_component(ball, physics_comp)
flecs.set_component(ball, "Physics2D", {
    "position": Vector2(100, 50),
    "velocity": Vector2(5, -10),
    "mass": 2.0,
    "is_static": false
})
```

### Example 5: Data-Driven Component Loading

```gdscript
# Load component definitions from JSON
func load_components_from_json(json_path: String, world_id: RID) -> Dictionary:
    var file = FileAccess.open(json_path, FileAccess.READ)
    var json_text = file.get_as_text()
    file.close()
    
    var json = JSON.new()
    var parse_result = json.parse(json_text)
    if parse_result != OK:
        push_error("Failed to parse JSON")
        return {}
    
    var component_defs = json.data
    var component_ids = {}
    var flecs = FlecsServer.get_singleton()
    
    for comp_name in component_defs.keys():
        var fields = component_defs[comp_name]
        var comp_id = flecs.create_runtime_component(world_id, comp_name, fields)
        if comp_id.is_valid():
            component_ids[comp_name] = comp_id
            print("Created component: ", comp_name)
        else:
            push_error("Failed to create component: " + comp_name)
    
    return component_ids

# Example JSON file (components.json):
# {
#     "Enemy": {
#         "health": 100,
#         "damage": 10,
#         "speed": 5.0,
#         "type": "goblin"
#     },
#     "Loot": {
#         "items": [],
#         "gold": 0,
#         "rarity": 1
#     }
# }
```

## Best Practices

### 1. Component Naming

Use clear, descriptive names for components:

```gdscript
# Good
flecs.create_runtime_component(world_id, "PlayerStats", fields)
flecs.create_runtime_component(world_id, "MovementController", fields)

# Avoid
flecs.create_runtime_component(world_id, "comp1", fields)
flecs.create_runtime_component(world_id, "data", fields)
```

### 2. Type Consistency

Provide representative default values to establish correct types:

```gdscript
# Good - type is clear
var stats_fields = {
    "strength": 10,      # Will be int
    "agility": 10,       # Will be int
    "intelligence": 10   # Will be int
}

# Risky - type ambiguity
var stats_fields = {
    "strength": 10,      # int
    "agility": 10.0,     # float - inconsistent!
    "intelligence": 10   # int
}
```

### 3. Check for Duplicates

Always verify component creation succeeded:

```gdscript
var comp_id = flecs.create_runtime_component(world_id, "MyComponent", fields)
if !comp_id.is_valid():
    push_error("Component creation failed - may already exist")
    # Handle error (use existing component, fallback, etc.)
    return
```

### 4. Organize Component Definitions

Keep component definitions in a central location:

```gdscript
# components_registry.gd
class_name ComponentRegistry

static func register_all(world_id: RID, flecs: FlecsServer) -> Dictionary:
    var components = {}
    
    components["Health"] = flecs.create_runtime_component(
        world_id, "Health", {
            "current": 100,
            "max": 100,
            "regen_rate": 0.0
        }
    )
    
    components["Position2D"] = flecs.create_runtime_component(
        world_id, "Position2D", {
            "position": Vector2.ZERO,
            "rotation": 0.0
        }
    )
    
    # ... more components
    
    return components
```

### 5. Limit Member Count

Flecs has a limit of 32 members per struct (`ECS_MEMBER_DESC_CACHE_SIZE`):

```gdscript
# OK - 5 members
var small_comp = {
    "a": 1, "b": 2, "c": 3, "d": 4, "e": 5
}

# AVOID - too many members (>32)
# Split into multiple components instead
```

## Integration with Systems

Runtime components work seamlessly with script systems:

```gdscript
# Create runtime component
var velocity_comp = flecs.create_runtime_component(world_id, "Velocity", {
    "x": 0.0,
    "y": 0.0
})

# Create system that uses it
var system_id = flecs.add_script_system(
    world_id,
    ["Position2D", "Velocity"],  # Component names
    func(entities, components, delta):
        for i in range(entities.size()):
            var pos = components[i][0]  # Position2D
            var vel = components[i][1]  # Velocity
            
            pos["x"] += vel["x"] * delta
            pos["y"] += vel["y"] * delta
            
            # Update component
            flecs.set_component(entities[i], "Position2D", pos)
)
```

## Limitations

1. **Member Limit**: Maximum 32 fields per component (Flecs limitation)
2. **No Nested Structs**: Fields must be flat - nested structs not supported
3. **No Arrays of Custom Structs**: Only arrays of built-in types
4. **Type Inference**: Type is determined by initial value - cannot change later
5. **Name Uniqueness**: Component names must be unique per world

## Performance Considerations

- Runtime components have identical performance to C++ components
- Type information is resolved once at creation time
- No runtime overhead for component access
- Suitable for production use

## Troubleshooting

### Component Already Exists Error

```gdscript
# Check if component exists before creating
var existing = flecs.lookup(world_id, "MyComponent")
if existing.is_valid():
    print("Component already exists")
else:
    var comp_id = flecs.create_runtime_component(world_id, "MyComponent", fields)
```

### Component Not Found After Creation

```gdscript
var comp_id = flecs.create_runtime_component(world_id, "Test", {"value": 1})
if !comp_id.is_valid():
    push_error("Creation failed")
    return

# Use component name, not RID, for get/set operations
flecs.set_component(entity, "Test", {"value": 42})  # Correct
```

### Type Mismatch Issues

```gdscript
# Created with int
var comp = flecs.create_runtime_component(world_id, "Score", {"points": 100})

# Setting with float will convert
flecs.set_component(entity, "Score", {"points": 99.9})  # Converts to 99

# To use float, create with float
var comp = flecs.create_runtime_component(world_id, "Score", {"points": 100.0})
```

## Deprecated Methods

### register_component_type() [DEPRECATED]

The old `register_component_type(world_id, type_name, script_visible_component_data)` method is **deprecated as of v1.1.1-a.1** and will be **removed in v2.0.0**.

**Why deprecated?**
- Uses heap-allocated `ScriptVisibleComponent` with Dictionary indirection
- Less efficient (2-5x slower component access)
- No proper type reflection support
- Cannot leverage Flecs type system optimizations

**Migration:**
```gdscript
# Old (deprecated):
var comp = flecs.register_component_type(world_id, "Health", {})

# New (recommended):
var comp = flecs.create_runtime_component(world_id, "Health", {
    "current": 100,
    "max": 100
})
```

See [MIGRATION_REGISTER_TO_RUNTIME.md](MIGRATION_REGISTER_TO_RUNTIME.md) for complete migration guide.

## Version Information

- **Introduced in**: v1.1.1-a.1
- **API Stability**: Alpha (subject to change)
- **Flecs Version**: Compatible with Flecs 3.2+
- **Replaces**: `register_component_type()` (deprecated, removal in v2.0.0)

## See Also

- [Migration Guide](MIGRATION_REGISTER_TO_RUNTIME.md) - Migrate from deprecated `register_component_type()`
- [Component Registration (C++)](components/README.md)
- [Script Systems](systems/SYSTEMS_DOCUMENTATION.md)
- [ECS Examples](examples/)
- [Flecs Reflection Documentation](https://www.flecs.dev/flecs/md_docs_2Reflection.html)