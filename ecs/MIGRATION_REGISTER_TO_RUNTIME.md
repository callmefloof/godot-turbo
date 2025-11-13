# Migration Guide: register_component_type → create_runtime_component

## Overview

The `register_component_type()` method has been **deprecated** as of v1.1.1-a.1 and will be **removed in v2.0.0**.

**Why deprecated?**
- Uses heap-allocated `ScriptVisibleComponent` with a `Dictionary` data field
- Less efficient memory layout (indirection through Dictionary)
- No proper type information for reflection
- Cannot leverage Flecs' type system optimizations

**New method**: `create_runtime_component()` uses Flecs' reflection API with proper typed struct members.

## Quick Comparison

| Feature | register_component_type (OLD) | create_runtime_component (NEW) |
|---------|------------------------------|--------------------------------|
| **Type system** | Opaque Dictionary wrapper | Flecs reflection structs |
| **Performance** | Extra indirection via Dictionary | Direct member access |
| **Memory** | Heap-allocated Dictionary per component | Stack-allocated struct |
| **Reflection** | No type metadata | Full Flecs reflection support |
| **Type safety** | Runtime dictionary lookups | Typed members |
| **Status** | ⚠️ DEPRECATED (removal in v2.0.0) | ✅ Current standard |

## Migration Steps

### Step 1: Update Method Name

**Old:**
```gdscript
var comp_id = flecs.register_component_type(world_id, "Health", {})
```

**New:**
```gdscript
var comp_id = flecs.create_runtime_component(world_id, "Health", {
    "current": 100,
    "max": 100
})
```

### Step 2: Provide Field Definitions

The new method requires you to specify fields with representative values:

**Old:**
```gdscript
# Empty dictionary - component is just a wrapper
var player_comp = flecs.register_component_type(world_id, "Player", {})

# Later, set arbitrary data
flecs.set_component(entity, "Player", {
    "name": "Hero",
    "level": 5,
    "health": 100
})
```

**New:**
```gdscript
# Define fields with types upfront
var player_comp = flecs.create_runtime_component(world_id, "Player", {
    "name": "",          # String type
    "level": 0,          # int type
    "health": 0          # int type
})

# Set data (type-checked)
flecs.set_component(entity, "Player", {
    "name": "Hero",
    "level": 5,
    "health": 100
})
```

### Step 3: Update Component Definitions

#### Example 1: Simple Component

**Before:**
```gdscript
var health_comp = flecs.register_component_type(world_id, "Health", {})
```

**After:**
```gdscript
var health_comp = flecs.create_runtime_component(world_id, "Health", {
    "current": 100,
    "max": 100,
    "regen_rate": 0.0
})
```

#### Example 2: Complex Component

**Before:**
```gdscript
var inventory_comp = flecs.register_component_type(world_id, "Inventory", {})

# Usage - anything goes
flecs.set_component(entity, "Inventory", {
    "items": ["sword", "shield"],
    "gold": 500,
    "random_field": "oops"  # Typos not caught
})
```

**After:**
```gdscript
var inventory_comp = flecs.create_runtime_component(world_id, "Inventory", {
    "items": [],           # Array
    "gold": 0,             # int
    "capacity": 20         # int
})

# Usage - typed fields
flecs.set_component(entity, "Inventory", {
    "items": ["sword", "shield"],
    "gold": 500,
    "capacity": 20
    # "random_field" would be ignored (not defined in schema)
})
```

#### Example 3: Transform Component

**Before:**
```gdscript
var transform_comp = flecs.register_component_type(world_id, "Transform", {})
```

**After:**
```gdscript
var transform_comp = flecs.create_runtime_component(world_id, "Transform", {
    "position": Vector3.ZERO,
    "rotation": Quaternion.IDENTITY,
    "scale": Vector3.ONE
})
```

## Common Patterns

### Pattern 1: Component Registry

**Before:**
```gdscript
class_name ComponentRegistry

static func register_all(world_id: RID, flecs: FlecsServer) -> Dictionary:
    var components = {}
    components["Health"] = flecs.register_component_type(world_id, "Health", {})
    components["Position"] = flecs.register_component_type(world_id, "Position", {})
    components["Velocity"] = flecs.register_component_type(world_id, "Velocity", {})
    return components
```

**After:**
```gdscript
class_name ComponentRegistry

static func register_all(world_id: RID, flecs: FlecsServer) -> Dictionary:
    var components = {}
    
    components["Health"] = flecs.create_runtime_component(world_id, "Health", {
        "current": 100,
        "max": 100,
        "regen_rate": 0.0
    })
    
    components["Position"] = flecs.create_runtime_component(world_id, "Position", {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
    })
    
    components["Velocity"] = flecs.create_runtime_component(world_id, "Velocity", {
        "x": 0.0,
        "y": 0.0,
        "z": 0.0
    })
    
    return components
```

### Pattern 2: JSON-Driven Components

**Before:**
```gdscript
# JSON schema doesn't matter - components are typeless
func load_components(json_data: Dictionary, world_id: RID):
    var flecs = FlecsServer.get_singleton()
    for comp_name in json_data.keys():
        flecs.register_component_type(world_id, comp_name, {})
```

**After:**
```gdscript
# JSON schema now defines component structure
# Example JSON:
# {
#   "Enemy": {
#     "health": 100,
#     "damage": 10,
#     "speed": 5.0
#   }
# }

func load_components(json_data: Dictionary, world_id: RID):
    var flecs = FlecsServer.get_singleton()
    for comp_name in json_data.keys():
        var fields = json_data[comp_name]
        flecs.create_runtime_component(world_id, comp_name, fields)
```

### Pattern 3: Conditional Component Creation

**Before:**
```gdscript
if not component_exists:
    flecs.register_component_type(world_id, "MyComponent", {})
```

**After:**
```gdscript
var comp_id = flecs.create_runtime_component(world_id, "MyComponent", {
    "value": 0
})

if !comp_id.is_valid():
    # Component already exists or creation failed
    comp_id = flecs.lookup(world_id, "MyComponent")
```

## Breaking Changes

### None (Backward Compatible)

The old `register_component_type()` method still works but prints a deprecation warning:

```
WARNING: FlecsServer::register_component_type() is deprecated and will be removed in v2.0.0.
         Use create_runtime_component() instead, which provides better performance...
```

You can continue using the old method until v2.0.0, but we recommend migrating as soon as possible.

### Compile-Time Deprecation Control

Godot's `DISABLE_DEPRECATED` flag allows you to completely remove deprecated methods at compile time:

```bash
# Standard build (deprecated method available with warning)
scons target=editor dev_build=yes

# Strict build (deprecated method removed - forces migration)
scons DISABLE_DEPRECATED=yes target=editor dev_build=yes
```

**When `DISABLE_DEPRECATED=yes` is set:**
- `register_component_type()` is not compiled into the binary
- Calling it results in compilation errors
- Forces immediate migration to `create_runtime_component()`
- Reduces binary size slightly
- Ensures clean codebase before v2.0.0

**Recommendation:** After migrating your code, test with `DISABLE_DEPRECATED=yes` to verify all usages are updated.

## Benefits of Migration

### 1. Performance Improvements

**Old (Dictionary indirection):**
```
Component Access: ~50-100ns
  - Dictionary lookup
  - Variant conversion
  - Heap access
```

**New (Direct struct access):**
```
Component Access: ~10-20ns
  - Direct member access
  - No dictionary overhead
  - Cache-friendly layout
```

### 2. Type Safety

**Old:**
```gdscript
# Typos not caught
flecs.set_component(entity, "Player", {
    "helth": 100  # Oops! Should be "health"
})
```

**New:**
```gdscript
# Type schema is defined upfront
# Non-existent fields are ignored
flecs.set_component(entity, "Player", {
    "helth": 100  # Ignored (not in schema)
})
```

### 3. Reflection Support

**Old:**
- No type information
- Cannot introspect component structure
- Limited serialization options

**New:**
- Full Flecs reflection metadata
- Can query component structure
- Native serialization support
- Better debugging

### 4. Better Integration

**New method integrates with:**
- Flecs query filters
- Component introspection
- Serialization systems
- Editor tools (future)

## Troubleshooting

### Issue 1: "Component already exists" Error

```gdscript
# Old method could be called multiple times (no-op)
flecs.register_component_type(world_id, "Health", {})  # OK
flecs.register_component_type(world_id, "Health", {})  # Also OK

# New method prevents duplicates
flecs.create_runtime_component(world_id, "Health", {"hp": 100})  # OK
flecs.create_runtime_component(world_id, "Health", {"hp": 100})  # ERROR!
```

**Solution:**
```gdscript
var health_comp = flecs.create_runtime_component(world_id, "Health", {"hp": 100})
if !health_comp.is_valid():
    # Already exists, look it up
    health_comp = flecs.lookup(world_id, "Health")
```

### Issue 2: Type Mismatches

```gdscript
# Old - accepts anything
flecs.set_component(entity, "Score", {"points": "100"})  # String accepted

# New - type enforced
var score = flecs.create_runtime_component(world_id, "Score", {"points": 100})
flecs.set_component(entity, "Score", {"points": "100"})  # Converted to int
```

**Solution:** Ensure your field definitions use the correct types:
```gdscript
var score = flecs.create_runtime_component(world_id, "Score", {
    "points": 0  # int, not "0" (String)
})
```

### Issue 3: Empty Components (Tags)

```gdscript
# Old - could create empty components
var tag = flecs.register_component_type(world_id, "IsEnemy", {})
```

**Solution:** For tag components (no data), just use component name directly:
```gdscript
# No need to create tag components, use entity lookup
var is_enemy = flecs.lookup(world_id, "IsEnemy")
if !is_enemy.is_valid():
    # Create as empty entity (tag)
    var world = flecs._get_world(world_id)
    var tag_entity = world.entity("IsEnemy")
    is_enemy = flecs._create_rid_for_type_id(tag_entity.id())
```

Or use a simple dummy field:
```gdscript
var is_enemy = flecs.create_runtime_component(world_id, "IsEnemy", {
    "tag": true  # Dummy field
})
```

## Timeline

- **v1.1.1-a.1** (Current): `register_component_type()` deprecated, prints warning
- **v1.x.x**: Old method remains functional with warnings
- **v2.0.0** (Future): `register_component_type()` removed entirely

### Deprecation Enforcement Options

| Build Mode | Status | Behavior |
|------------|--------|----------|
| Default | Deprecated (soft) | Method available, prints warning once |
| `DISABLE_DEPRECATED=yes` | Deprecated (hard) | Method not compiled, forces migration |
| v2.0.0+ | Removed | Method does not exist |

## Automated Migration Script

```gdscript
# migration_helper.gd
# Helper script to identify usage of deprecated method

static func scan_project_for_deprecated_calls(project_path: String):
    var files = []
    _scan_directory_recursive(project_path, files, ".gd")
    
    var deprecated_usages = []
    for file_path in files:
        var file = FileAccess.open(file_path, FileAccess.READ)
        if file:
            var line_num = 0
            while not file.eof_reached():
                line_num += 1
                var line = file.get_line()
                if "register_component_type" in line:
                    deprecated_usages.append({
                        "file": file_path,
                        "line": line_num,
                        "content": line.strip_edges()
                    })
            file.close()
    
    return deprecated_usages

static func _scan_directory_recursive(path: String, files: Array, extension: String):
    var dir = DirAccess.open(path)
    if dir:
        dir.list_dir_begin()
        var file_name = dir.get_next()
        while file_name != "":
            var file_path = path + "/" + file_name
            if dir.current_is_dir():
                if not file_name.begins_with("."):
                    _scan_directory_recursive(file_path, files, extension)
            elif file_name.ends_with(extension):
                files.append(file_path)
            file_name = dir.get_next()
        dir.list_dir_end()

# Usage example:
# var usages = MigrationHelper.scan_project_for_deprecated_calls("res://")
# for usage in usages:
#     print("%s:%d - %s" % [usage["file"], usage["line"], usage["content"]])
# 
# After fixing all usages, verify with:
# scons DISABLE_DEPRECATED=yes target=editor
# If build succeeds, migration is complete!
```

## Support

For questions or migration help:
- See: [RUNTIME_COMPONENTS.md](RUNTIME_COMPONENTS.md) for full documentation
- Example: [runtime_component_example.gd](examples/runtime_component_example.gd)
- Issues: Report migration problems on GitHub

## Summary

✅ **Do This:**
```gdscript
var comp = flecs.create_runtime_component(world_id, "Health", {
    "current": 100,
    "max": 100
})
```

❌ **Not This:**
```gdscript
var comp = flecs.register_component_type(world_id, "Health", {})
```

The migration is straightforward: define your component fields upfront instead of treating components as opaque data containers. This gives you better performance, type safety, and integration with Flecs' powerful reflection system.

**Testing Migration Completion:**
```bash
# After migrating all code, verify with strict build
scons DISABLE_DEPRECATED=yes target=editor dev_build=yes

# If build succeeds, migration is 100% complete!
# If build fails, the compiler will show exactly which files still use the old method.
```