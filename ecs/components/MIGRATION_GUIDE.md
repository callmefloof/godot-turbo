# Component System Migration Guide: Flecs Reflection Refactor

## Overview

This guide explains how to migrate from the old `CompBase` + `REGISTER_COMPONENT` system to the new Flecs reflection-based approach.

## Key Changes

### Before (Old System)
- All components inherited from `CompBase`
- Required implementing 4+ virtual methods per component
- Manual dictionary serialization/deserialization
- Heavy macro-based registration with function pointers
- Verbose boilerplate code

### After (New System)
- Components are plain POD structs (no inheritance)
- Flecs handles component metadata via reflection
- Optional serialization via simple lambda functions
- Cleaner, more maintainable code
- Better performance (no virtual calls)

## Migration Steps

### Step 1: Remove CompBase Inheritance

**Old:**
```cpp
struct Transform2DComponent : CompBase {
    Transform2D transform;
    
    Dictionary to_dict() const override { /* ... */ }
    void from_dict(const Dictionary &dict) override { /* ... */ }
    Dictionary to_dict_with_entity(flecs::entity &entity) const override { /* ... */ }
    void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override { /* ... */ }
    StringName get_type_name() const override { return "Transform2DComponent"; }
};
REGISTER_COMPONENT(Transform2DComponent);
```

**New:**
```cpp
struct Transform2DComponent {
    Transform2D transform;
};

// Optional serialization (if needed)
namespace {
    Dictionary serialize_transform_2d(const void* data) {
        const Transform2DComponent* comp = static_cast<const Transform2DComponent*>(data);
        Dictionary dict;
        dict["transform"] = comp->transform;
        return dict;
    }

    void deserialize_transform_2d(void* data, const Dictionary& dict) {
        Transform2DComponent* comp = static_cast<Transform2DComponent*>(data);
        if (dict.has("transform")) {
            comp->transform = dict["transform"];
        }
    }
}

FLECS_COMPONENT_SERIALIZABLE(Transform2DComponent, serialize_transform_2d, deserialize_transform_2d)
```

### Step 2: Tag Components (Zero-Size Markers)

**Old:**
```cpp
struct DirtyTransform : CompBase {
    Dictionary to_dict() const override { return Dictionary(); }
    void from_dict(const Dictionary &p_dict) override {}
    // ... more boilerplate
};
REGISTER_COMPONENT(DirtyTransform);
```

**New:**
```cpp
struct DirtyTransform {};

FLECS_COMPONENT(DirtyTransform)
```

### Step 3: Components Without Serialization

If you don't need serialization (you mentioned it's optional), just use the simple macro:

```cpp
struct MyComponent {
    RID resource_id;
    float value;
    bool active;
};

FLECS_COMPONENT(MyComponent)
```

### Step 4: Register Opaque Types (One-Time Setup)

In your world initialization code, register Godot's opaque types:

```cpp
#include "ecs/components/flecs_opaque_types.h"

void initialize_ecs_world(flecs::world& world) {
    // Register Godot types as opaque to Flecs
    FlecsOpaqueTypes::register_opaque_types(world);
    
    // Register your components
    world.component<Transform2DComponent>();
    world.component<Transform3DComponent>();
    world.component<MeshComponent>();
    // ... etc
    
    // Bind component IDs to registry (if using serialization)
    FlecsReflection::Registry::get().bind_component_id(
        "Transform2DComponent", 
        world.id<Transform2DComponent>()
    );
}
```

## Benefits

### 1. Reduced Code Size
- **Before:** ~50 lines per component with serialization
- **After:** ~5-30 lines depending on serialization needs

### 2. Better Performance
- No virtual function calls
- Direct memory access
- Better cache locality
- Compiler can optimize POD structs more aggressively

### 3. Cleaner Code
- Components are just data structures
- Serialization is optional and separate from component definition
- Easier to understand and maintain

### 4. Flecs Integration
- Native Flecs reflection support
- Can use Flecs' built-in serialization (JSON, expr)
- Better tooling support (Flecs Explorer)

## Common Patterns

### Pattern 1: Simple POD Component

```cpp
struct VelocityComponent {
    Vector3 velocity;
};

FLECS_COMPONENT(VelocityComponent)
```

### Pattern 2: Component with RID (Opaque Type)

```cpp
struct MeshComponent {
    RID mesh_id;
    Vector<RID> material_ids;
    AABB bounds;
};

// Without serialization
FLECS_COMPONENT(MeshComponent)

// OR with serialization
namespace {
    Dictionary serialize_mesh(const void* data) {
        const MeshComponent* comp = static_cast<const MeshComponent*>(data);
        Dictionary dict;
        dict["mesh_id"] = comp->mesh_id.get_id();
        // ... serialize other fields
        return dict;
    }
    
    void deserialize_mesh(void* data, const Dictionary& dict) {
        MeshComponent* comp = static_cast<MeshComponent*>(data);
        if (dict.has("mesh_id")) {
            comp->mesh_id = RID::from_uint64(dict["mesh_id"]);
        }
    }
}

FLECS_COMPONENT_SERIALIZABLE(MeshComponent, serialize_mesh, deserialize_mesh)
```

### Pattern 3: Relationship/Tag Components

```cpp
// Parent-child relationship (Flecs built-in)
entity.add<flecs::ChildOf>(parent_entity);

// Custom tags
struct Selected {};
struct Disabled {};

FLECS_COMPONENT(Selected)
FLECS_COMPONENT(Disabled)
```

## Serialization Strategy

### When to Use Serialization

**Use serialization when:**
- Saving/loading game state
- Network synchronization
- Editor integration
- Debugging/inspection

**Skip serialization when:**
- Transient/runtime-only data
- Performance-critical components
- Data that can be reconstructed

### Minimal Serialization Example

```cpp
struct PlayerComponent {
    int health;
    int max_health;
    String player_name;
};

namespace {
    Dictionary serialize_player(const void* data) {
        auto* p = static_cast<const PlayerComponent*>(data);
        Dictionary d;
        d["health"] = p->health;
        d["max_health"] = p->max_health;
        d["player_name"] = p->player_name;
        return d;
    }
    
    void deserialize_player(void* data, const Dictionary& dict) {
        auto* p = static_cast<PlayerComponent*>(data);
        p->health = dict.get("health", 100);
        p->max_health = dict.get("max_health", 100);
        p->player_name = dict.get("player_name", "Player");
    }
}

FLECS_COMPONENT_SERIALIZABLE(PlayerComponent, serialize_player, deserialize_player)
```

## Using the Registry

### Serialize Entity Components

```cpp
flecs::entity player = world.entity("Player");

// Get component by ID
flecs::entity_t comp_id = world.id<PlayerComponent>();
Dictionary data = FlecsReflection::Registry::get().serialize(player, comp_id);

// Modify and deserialize back
data["health"] = 50;
FlecsReflection::Registry::get().deserialize(player, comp_id, data);
```

### Bulk Component Registration

```cpp
namespace MyComponents {
    inline void register_all(flecs::world& world) {
        world.component<Transform2DComponent>();
        world.component<VelocityComponent>();
        world.component<PlayerComponent>();
        
        // Bind IDs for serialization
        auto& registry = FlecsReflection::Registry::get();
        registry.bind_component_id("Transform2DComponent", world.id<Transform2DComponent>());
        registry.bind_component_id("VelocityComponent", world.id<VelocityComponent>());
        registry.bind_component_id("PlayerComponent", world.id<PlayerComponent>());
    }
}
```

## Compatibility Notes

### Keeping Old Code Working

If you need to maintain compatibility during migration:

1. Keep both header files temporarily (old and new)
2. Use `#ifdef` to switch between implementations
3. Gradually migrate component-by-component

```cpp
#ifdef USE_FLECS_REFLECTION
    #include "all_components.h"
#else
    #include "transform_2d_component.h"
    // ... other old headers
#endif
```

### Testing

After migration, test:
- Component creation/destruction
- Serialization round-trip (if used)
- Query performance
- Memory usage

## Performance Considerations

### Memory Layout

The new system produces more compact memory:
- No vtable pointers
- Better struct packing
- Cache-friendly layouts

### Query Performance

Flecs queries work better with POD structs:
- Direct memory access
- SIMD optimization opportunities
- Better prefetching

## Troubleshooting

### Issue: Component not found in registry

**Solution:** Make sure you called `world.component<YourComponent>()` and bound the ID:

```cpp
world.component<YourComponent>();
FlecsReflection::Registry::get().bind_component_id("YourComponent", world.id<YourComponent>());
```

### Issue: Serialization not working

**Solution:** Verify you used `FLECS_COMPONENT_SERIALIZABLE` and provided both serialize/deserialize functions.

### Issue: Opaque type errors

**Solution:** Call `FlecsOpaqueTypes::register_opaque_types(world)` before using Godot types in components.

## Advanced: Custom Reflection

For complex types, you can use Flecs' native reflection:

```cpp
struct ComplexComponent {
    int value;
    float weight;
};

// Register with Flecs reflection
world.component<ComplexComponent>()
    .member<int>("value")
    .member<float>("weight");
```

This enables:
- Flecs Explorer visualization
- Built-in JSON serialization
- Runtime introspection

## Summary

The new system:
- ✅ Removes ~80% of boilerplate code
- ✅ Improves performance (no virtual calls)
- ✅ Makes components simple data structures
- ✅ Serialization is optional and clean
- ✅ Better Flecs integration
- ✅ Easier to maintain and extend

Start by migrating simple components first, then gradually move complex ones.