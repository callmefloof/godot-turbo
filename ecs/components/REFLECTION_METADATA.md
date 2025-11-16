# Flecs Reflection Metadata Documentation

## Overview

This document describes the reflection metadata system used in the Godot Turbo ECS module to integrate with Flecs' reflection capabilities.

## Important: Godot Types and Reflection

**⚠️ CRITICAL:** Components containing Godot types (RID, Transform2D, Vector3, Color, etc.) should **NOT** have `.member<>()` reflection metadata. These types are registered as **opaque types** in Flecs, and mixing opaque type registration with struct reflection causes runtime errors.

### What Components Should NOT Have Reflection:
- Any component containing `RID` fields
- Any component containing Godot math types (Vector2/3/4, Transform2D/3D, Color, Quaternion, etc.)
- Any component containing Godot containers (String, Array, Dictionary, Vector<T>, etc.)
- Any component containing raw pointers (`void*`, `T*`)

### What Components CAN Have Reflection:
- Components with only primitive types (bool, int, float, uint32_t, etc.)
- Tag components (empty structs)

## What is Reflection Metadata?

Reflection metadata allows Flecs to introspect component structures at runtime. However, for Godot types we use **opaque type serialization** instead, which provides:

1. **Serialization**: Godot types can be serialized through custom handlers
2. **Type Safety**: Types are known to Flecs but not structurally introspected
3. **Compatibility**: Avoids conflicts between Godot's type system and Flecs reflection

## How Component Registration Works

### Compile-Time Components (C++)

For components defined in C++ (in `all_components.h`), there are two registration patterns:

**1. Opaque Components (Most Godot Components)**
Components containing Godot types should be registered WITHOUT `.member<>()` calls:

```cpp
// CORRECT - No .member<>() for components with Godot types
world.component<Transform2DComponent>();  // Contains Transform2D
world.component<CameraComponent>();        // Contains RID, Vector3, etc.
world.component<MultiMeshComponent>();     // Contains RID, enum
world.component<MeshComponent>();          // Contains RID, Vector<RID>, AABB
```

**2. Primitive Components (Rare)**
Only components with ONLY primitive types can have reflection:

```cpp
// CORRECT - Only primitive types
world.component<VisibilityComponent>()
    .member<bool>("visible");

// WRONG - Contains opaque types!
world.component<CameraComponent>()
    .member<float>("far")
    .member<float>("near");  // This will cause "unknown member" errors!
```

### Runtime Components (GDScript/Dictionary)

For components created at runtime via `create_runtime_component()`, reflection metadata is added automatically using the C API:

```cpp
// Uses ecs_struct_init which provides reflection data
ecs_member_t members[ECS_MEMBER_DESC_CACHE_SIZE];
// ... populate members array ...
ecs_struct_desc_t struct_desc = {};
struct_desc.entity = world->entity(component_name.utf8().get_data());
memcpy(struct_desc.members, members, sizeof(members));
flecs::entity_t comp_id = ecs_struct_init(world->c_ptr(), &struct_desc);
```

This provides similar functionality to the C++ `.member<>()` API, but works correctly with runtime component creation.

**Note:** Runtime components should also avoid using Godot opaque types in their field definitions when possible.

## Component Registration

### Static Components

All static components are registered in `AllComponents::register_all()` in `all_components.h`:

```cpp
inline void register_all(flecs::world& world, bool enable_serialization = false) {
    // First, register Godot's opaque types
    FlecsOpaqueTypes::register_opaque_types(world);
    
    // Register components WITHOUT .member<>() for Godot types
    world.component<Transform2DComponent>();  // Contains Transform2D (opaque)
    world.component<MultiMeshComponent>();     // Contains RID (opaque)
    world.component<MeshComponent>();          // Contains RID, Vector<RID> (opaque)
    
    // Only primitives can have reflection
    world.component<VisibilityComponent>()
        .member<bool>("visible");
    
    // ... more components ...
}
```

### Tag Components

Tag components (empty structs used as markers) don't need `.member<>()` calls:

```cpp
struct DirtyTransform {}; // Tag component

// Registration:
world.component<DirtyTransform>(); // No .member() calls needed
```

## Opaque Types

Godot types that Flecs doesn't know about (like `RID`, `Vector3`, `Color`, etc.) are registered as "opaque types" in `FlecsOpaqueTypes::register_opaque_types()`:

```cpp
FlecsOpaqueTypes::register_opaque_types(world);
```

This must be called **before** registering components that use these types.

## Best Practices

### 1. Check for Opaque Types First

When creating new components, **first check if it contains any Godot types**:

```cpp
// Component with Godot types - NO REFLECTION
struct MyRenderComponent {
    RID mesh_id;
    Color tint;
    Vector3 position;
};

// Registration - NO .member<>() calls
world.component<MyRenderComponent>();

// Component with only primitives - CAN have reflection
struct MyStatsComponent {
    int health;
    float speed;
    bool is_active;
};

// Registration - Can use .member<>()
world.component<MyStatsComponent>()
    .member<int>("health")
    .member<float>("speed")
    .member<bool>("is_active");
```

### 2. Runtime-Only Fields

**Important:** If a component contains pointers (`void*`, raw pointers) or other runtime-only data, it's safest to **NOT register any reflection metadata** for that component:

```cpp
struct PixelProcessTask {
    uint32_t instance_index;
    const void* img_data;     // Pointer field - problematic for reflection
    int mode;
    Color result;             // Runtime-only output
};

// DO NOT register .member<>() for components with pointers!
// Flecs may try to introspect all fields even if some are omitted
world.component<PixelProcessTask>();  // Register without .member() calls
```

**Why?** Flecs' reflection system may attempt to access component memory layout even for fields you didn't explicitly register, which can cause runtime errors or crashes when encountering pointer types.

**Alternative:** If you need reflection for some fields, consider splitting the component:
```cpp
struct PixelTaskConfig {
    uint32_t instance_index;
    int mode;
};

struct PixelTaskRuntime {
    const void* img_data;
    Color result;
};

// Register reflection only for the config component
world.component<PixelTaskConfig>()
    .member<uint32_t>("instance_index")
    .member<int>("mode");

world.component<PixelTaskRuntime>();  // No reflection
```

### 3. Call Order Matters

Always register opaque types first, then components:

```cpp
// 1. Opaque types
FlecsOpaqueTypes::register_opaque_types(world);

// 2. Components
AllComponents::register_all(world);
```

## Troubleshooting

### Error: "unknown member 'field_name' for type 'ComponentName'"

**Cause**: Component contains Godot opaque types (RID, Vector3, Transform2D, etc.) and was registered with `.member<>()` calls. This creates a conflict between opaque type serialization and struct reflection.

**Solution**: Remove ALL `.member<>()` calls from that component. Register it as just `world.component<ComponentName>();`

**Example:**
```cpp
// WRONG - Causes "unknown member 'multi_mesh_id'" error
world.component<MultiMeshComponent>()
    .member<RID>("multi_mesh_id")           // RID is opaque!
    .member<uint32_t>("instance_count");

// CORRECT - No .member<>() calls
world.component<MultiMeshComponent>();
```

### Error: "DW_TAG_member refers to type which extends beyond bounds"

**Cause**: Same as above - mixing opaque types with struct reflection, or using pointer fields.

**Solution**: Remove all `.member<>()` calls. Components with opaque Godot types or pointers must be registered without reflection metadata.

### Error: "unknown type 'TypeName'"

**Cause**: Godot type wasn't registered as an opaque type before components were registered.

**Solution**: Ensure `FlecsOpaqueTypes::register_opaque_types(world)` is called **before** `AllComponents::register_all(world)`.

### Components not visible in Flecs Explorer

**Cause**: Components without reflection metadata won't show detailed field information in the explorer.

**Solution**: This is expected behavior for components with Godot types. The Flecs explorer will show the component exists, but won't display individual fields. This is a necessary tradeoff to avoid runtime errors.

**Alternatives:**
- Use custom serialization functions for complex components
- Split components into primitive and opaque parts if field visibility is critical

## Examples

### Primitive Component (With Reflection)

```cpp
struct HealthComponent {
    float current_health;
    float max_health;
    bool is_regenerating;
};

// CORRECT - Only primitives
world.component<HealthComponent>()
    .member<float>("current_health")
    .member<float>("max_health")
    .member<bool>("is_regenerating");
```

### Godot Type Component (Without Reflection)

```cpp
struct PlayerComponent {
    RID player_id;          // Opaque
    String player_name;     // Opaque
    Vector3 spawn_position; // Opaque
    int level;
    bool is_alive;
};

// CORRECT - Contains opaque types, no .member<>()
world.component<PlayerComponent>();
```

### Split Component Approach (Advanced)

If you need some fields visible in the explorer:

```cpp
// Primitives only - can have reflection
struct PlayerStats {
    int level;
    int experience;
    bool is_alive;
};

// Godot types - no reflection
struct PlayerData {
    RID player_id;
    String player_name;
    Vector3 spawn_position;
};

world.component<PlayerStats>()
    .member<int>("level")
    .member<int>("experience")
    .member<bool>("is_alive");

world.component<PlayerData>();
```

### Runtime Component (GDScript)

```gdscript
# Create a runtime component from GDScript
var fields = {
    "damage": 10.0,        # float
    "attack_speed": 1.5,   # float
    "weapon_type": "sword" # String
}

var component_id = FlecsServer.create_runtime_component(world_id, "WeaponComponent", fields)
```

This automatically creates reflection metadata equivalent to:

```cpp
world.component<WeaponComponent>()
    .member<double>("damage")
    .member<double>("attack_speed")
    .member<String>("weapon_type");
```

## References

- [Flecs Reflection Manual](https://www.flecs.dev/flecs/md_docs_2Reflection.html)
- [Flecs REST API](https://www.flecs.dev/flecs/md_docs_2RestApi.html)
- `all_components.h` - Component definitions and registration
- `flecs_opaque_types.h` - Godot type registration
- `flecs_server.cpp` - Runtime component creation

## Summary

**Key Takeaways:**

1. ❌ **DO NOT** add `.member<>()` calls to components containing:
   - RID fields
   - Godot math types (Vector2/3/4, Transform2D/3D, Color, etc.)
   - Godot containers (String, Dictionary, Array, Vector<T>)
   - Raw pointers (void*, T*)

2. ✅ **DO** add `.member<>()` calls only to components with:
   - Only primitive types (bool, int, float, uint32_t, etc.)
   - No Godot opaque types

3. 🔧 **Opaque type serialization** (in `flecs_opaque_types.h`) provides:
   - Serialization for Godot types
   - Compatibility with Flecs
   - Avoids "unknown member" and runtime errors

4. 📝 **Tradeoff**: Components without reflection won't show detailed fields in Flecs explorer, but this prevents runtime errors and crashes.

**When in doubt, don't add `.member<>()` calls!** It's safer to have no reflection than to mix opaque types with struct reflection.