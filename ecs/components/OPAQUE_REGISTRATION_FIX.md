# Opaque Type Registration Fix for Flecs DWARF Auto-Discovery

## Problem

When compiling with debug symbols enabled, Flecs can automatically read DWARF debug information from the compiled binary to generate reflection metadata for components. This causes runtime errors when components contain Godot opaque types:

```
error: flecs.c: 51516: cannot open scope for 'MultiMeshComponent' (missing reflection data)
error: godot.linuxbsd.editor.dev.x86_64 0x045d5c6a: DW_TAG_member '_mem' refers to type 0x00000000045deb56 which extends beyond the bounds of 0x045d5bd3
```

The issue occurs because:
1. **Flecs reads DWARF debug symbols** from the binary and tries to auto-generate reflection
2. **Components contain Godot opaque types** (RID, Transform2D/3D, Vector<T>, AABB, etc.)
3. **Flecs cannot introspect** the internal structure of these opaque types
4. **Auto-discovery fails** when it encounters opaque types inside component structs

## Solution

Register components containing opaque types with **explicit empty opaque handlers** to prevent DWARF auto-discovery:

```cpp
// Before (causes DWARF errors):
world.component<MultiMeshComponent>();

// After (prevents DWARF introspection):
world.component<MultiMeshComponent>()
    .opaque([](flecs::world&) { 
        return flecs::opaque<MultiMeshComponent>().as_type(ecs_id(EcsOpaque)); 
    });
```

This tells Flecs:
- ✅ The component is **opaque** (no internal structure introspection)
- ✅ **Don't use DWARF** to auto-generate reflection
- ✅ **Don't try to open scopes** on member fields
- ✅ Component can still be **used normally** in queries and systems

## Implementation

### 1. Register Basic Godot Types as Opaque (`flecs_opaque_types.h`)

Register all Godot primitive types (RID, Vector2/3/4, Transform2D/3D, Color, etc.) as opaque with custom serializers:

```cpp
world.component<RID>()
    .opaque([](flecs::world& w) {
        return flecs::opaque<RID>()
            .as_type(ecs_id(EcsOpaque))
            .serialize(serialize_rid);
    });
```

### 2. Register Components Containing Opaque Types (`all_components.h`)

Any component containing Godot types must be registered as opaque:

```cpp
// Components with RID, Transform, AABB, Vector<T>, etc.
world.component<MultiMeshComponent>()
    .opaque([](flecs::world&) { 
        return flecs::opaque<MultiMeshComponent>().as_type(ecs_id(EcsOpaque)); 
    });

world.component<CameraComponent>()
    .opaque([](flecs::world&) { 
        return flecs::opaque<CameraComponent>().as_type(ecs_id(EcsOpaque)); 
    });

world.component<Transform3DComponent>()
    .opaque([](flecs::world&) { 
        return flecs::opaque<Transform3DComponent>().as_type(ecs_id(EcsOpaque)); 
    });
```

### 3. Keep Primitive-Only Components with Reflection

Components containing **only C++ primitives** can keep `.member<>()` reflection:

```cpp
world.component<VisibilityComponent>()
    .member<bool>("visible");  // ✅ Safe - bool is a primitive
```

## Components Requiring Opaque Registration

Any component containing one or more of these Godot types:

### Godot Engine Types (Opaque)
- **RID** - Resource ID
- **Transform2D** / **Transform3D** - Transforms
- **Vector2** / **Vector3** / **Vector4** - Vectors
- **Vector2i** / **Vector3i** / **Vector4i** - Integer vectors
- **Color** - RGBA color
- **Quaternion** - Rotation
- **AABB** - Axis-aligned bounding box
- **Plane** - 3D plane
- **Rect2** / **Rect2i** - 2D rectangles
- **Basis** - 3x3 matrix
- **Projection** - 4x4 matrix
- **String** / **StringName** - Godot strings
- **ObjectID** - Object identifier
- **Variant** - Universal type
- **Dictionary** - Hash map
- **Array** - Dynamic array
- **Vector<T>** - Templated vector

### Pointer-Containing Types (Also Opaque)
- **void*** - Raw pointers (used for runtime tasks, cross-system communication)

### Component Examples Requiring Opaque Registration

```cpp
// Contains RID → register as opaque
struct MultiMeshComponent {
    RID multi_mesh_id;
    uint32_t instance_count;
    // ...
};

// Contains Transform3D → register as opaque
struct Transform3DComponent {
    Transform3D transform;
};

// Contains Vector<Plane> and Projection → register as opaque
struct CameraComponent {
    RID camera_id;
    Vector<Plane> frustum;
    Projection projection;
    // ...
};

// Contains AABB → register as opaque
struct MeshComponent {
    RID mesh_id;
    Vector<RID> material_ids;
    AABB custom_aabb;
};

// Contains void* pointer → register as opaque
struct PixelProcessTask {
    uint32_t instance_index;
    const void* img_data;  // Pointer to ImageData
    int mode;
    Color result;
};
```

## Why This Works

1. **`.opaque()` tells Flecs**: "Don't introspect this type's internal structure"
2. **Prevents DWARF reading**: Flecs won't try to auto-generate reflection from debug symbols
3. **Avoids scope errors**: Flecs won't try to open scopes on opaque member fields
4. **Components still work**: You can still use them in queries, systems, add/remove, etc.

## Tradeoffs

### ✅ Advantages
- **No runtime errors** from DWARF/reflection conflicts
- **Components work correctly** in all Flecs operations
- **Prevents crashes** from opaque type introspection
- **Clean separation** between Godot types and Flecs reflection

### ⚠️ Limitations
- **No detailed field view** in Flecs explorer (components show as opaque blobs)
- **No automatic serialization** of individual fields (unless you add custom serializers)
- **Manual serializer required** if you need field-level inspection

## Optional: Custom Serializers

If you want components visible in the Flecs explorer, add custom serializers to `flecs_opaque_types.h`:

```cpp
inline int serialize_multimesh_component(const flecs::serializer* s, const MultiMeshComponent* data) {
    s->member("multi_mesh_id");
    s->value(data->multi_mesh_id.get_id());
    s->member("instance_count");
    s->value(data->instance_count);
    // ... serialize other fields
    return 0;
}

// Then register with serializer:
world.component<MultiMeshComponent>()
    .opaque([](flecs::world& w) {
        return flecs::opaque<MultiMeshComponent>()
            .as_type(ecs_id(EcsOpaque))
            .serialize(serialize_multimesh_component);
    });
```

## Summary

**Golden Rule**: If a component contains any Godot engine type (RID, Transform, Vector, Color, etc.), register it as opaque to prevent DWARF auto-discovery errors.

**Files Modified**:
- `flecs_opaque_types.h` - Godot primitive type opaque registration
- `all_components.h` - Component opaque registration for types containing Godot types or pointers

**Result**: Clean compilation and runtime with no DWARF errors, no "cannot open scope" errors, and all components working correctly in Flecs systems.

## Special Case: Runtime Task Components

Components used for per-frame runtime tasks (like `PixelProcessTask`) that contain raw pointers **must** be registered as opaque:

```cpp
// BAD - Flecs will try to introspect the void* pointer via DWARF
world.component<PixelProcessTask>();

// GOOD - Prevents DWARF introspection
world.component<PixelProcessTask>()
    .opaque([](flecs::world&) { 
        return flecs::opaque<PixelProcessTask>().as_type(ecs_id(EcsOpaque)); 
    });
```

**Why this matters**: Raw pointers (`void*`) in components point to data that Flecs cannot safely introspect. When debug symbols are enabled, Flecs reads DWARF metadata and attempts to follow pointer members, causing crashes or corruption when the pointed-to data has different lifetimes or memory layouts.