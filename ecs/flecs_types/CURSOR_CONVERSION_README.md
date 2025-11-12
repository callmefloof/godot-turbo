# Flecs Cursor-Based Component Conversion

## Overview

This document describes the cursor-based approach for converting between Flecs component data and Godot Dictionary/Variant types. This replaces the previous `ComponentRegistry::to_dict()` / `from_dict()` approach that required each component to implement serialization methods manually.

## Motivation

The previous approach required every component to implement:
- `Dictionary to_dict() const`
- `void from_dict(const Dictionary& dict)`

This was:
1. **Repetitive** - Each component needed boilerplate serialization code
2. **Error-prone** - Easy to forget updating serialization when adding fields
3. **Limited** - Couldn't handle opaque Godot types (Variant, Dictionary, Array, String, StringName, etc.) without special handling

## Solution: Flecs Cursor API

The Flecs cursor API provides runtime introspection of component data using reflection metadata. This allows generic serialization/deserialization that works with any component type, including opaque Godot types.

## Key Functions

### `cursor_to_variant(flecs::cursor& cur) -> Variant`

Recursively converts a flecs cursor position to a Godot Variant. Handles:

- **Opaque Godot types**: Variant, Dictionary, Array, String, StringName, NodePath, Callable, Signal, RID
- **Packed arrays**: PackedByteArray, PackedInt32Array, PackedFloat32Array, PackedStringArray, PackedVector2Array, PackedVector3Array, PackedColorArray
- **Math types**: Vector2, Vector2i, Vector3, Vector3i, Vector4, Vector4i, Quaternion, Transform2D, Transform3D, Rect2, Rect2i, AABB, Plane, Basis, Color, Projection
- **Primitive types**: bool, char, int8-64, uint8-64, float, double, string, entity
- **Structs**: Recursively converts all members to a Dictionary

### `component_to_dict_cursor(flecs::entity entity, flecs::entity_t comp_type_id) -> Dictionary`

Converts a component on an entity to a Dictionary using the cursor API. This is the main function called by `get_component_by_name()` and `get_component_by_id()`.

### `component_from_dict_cursor(flecs::entity entity, flecs::entity_t comp_type_id, const Dictionary& dict)`

Sets component data from a Dictionary using the cursor API. This is the main function called by `set_component()`.

## Usage Examples

### Getting a Component

```cpp
// Old approach (required ComponentRegistry::to_dict implementation)
Dictionary data = ComponentRegistry::to_dict(entity, component_name);

// New approach (works with any component type)
flecs::entity comp = entity.world().lookup(component_name);
Dictionary data = component_to_dict_cursor(entity, comp.id());
```

### Setting a Component

```cpp
Dictionary data;
data["x"] = 10.0;
data["y"] = 20.0;

// Old approach (required ComponentRegistry::from_dict implementation)
ComponentRegistry::from_dict(entity, data, component_name);

// New approach (works with any component type)
flecs::entity comp = entity.world().lookup(component_name);
component_from_dict_cursor(entity, comp.id(), data);
```

## Opaque Type Handling

Godot types like `Variant`, `Dictionary`, `String`, etc. are treated as opaque types by Flecs (their internal structure is not reflected). The cursor-based approach handles these by:

1. Checking the type name using `cur.get_type().name()`
2. Getting a raw pointer to the data using `cur.get_ptr()`
3. Casting to the appropriate Godot type pointer
4. Dereferencing and returning/setting the value

Example for Dictionary:
```cpp
if (strcmp(type_name, "Dictionary") == 0) {
    Dictionary* ptr = static_cast<Dictionary*>(cur.get_ptr());
    return ptr ? *ptr : Dictionary();
}
```

**Note on Opaque Type Wrapping**: When a component IS an opaque type (e.g., the component itself is a RID, not a struct containing a RID), the `component_to_dict_cursor` function wraps it in a Dictionary with a `"value"` key. This ensures consistent Dictionary return types for all components:

```cpp
// Component that IS a RID
Dictionary result;
result["value"] = rid_instance;  // Wrapped in dictionary

// Component that HAS a RID field
Dictionary result;
result["my_rid_field"] = rid_instance;  // RID is a member
```

## Type Detection

The code uses Flecs' reflection system to detect type kinds:

```cpp
if (type.has<EcsType>()) {
    const EcsType& ecs_type = type.get<EcsType>();
    if (ecs_type.kind == EcsPrimitiveType) {
        // Handle primitives
    } else if (ecs_type.kind == EcsStructType) {
        // Handle structs
    }
}
```

## Opaque Type Wrapping

When a component itself **IS** an opaque type (not a struct), it's wrapped in a Dictionary:

### Reading Opaque Components
```cpp
// If component is RID itself
Dictionary data = get_component_by_name(entity, "MyRIDComponent");
RID my_rid = data["value"];  // Access via "value" key
```

### Writing Opaque Components
```cpp
// Setting a RID component
Dictionary data;
data["value"] = some_rid;
set_component(entity, "MyRIDComponent", data);
```

This wrapping ensures all components return Dictionary for consistency, even primitive/opaque ones.

## Benefits

1. **No boilerplate**: Components don't need to implement serialization
2. **Automatic**: Works with any component type that has reflection metadata
3. **Opaque type support**: Handles Godot types seamlessly (with automatic wrapping)
4. **Type-safe**: Uses Flecs' type system for safe conversions
5. **Maintainable**: Single implementation for all components
6. **Consistent API**: All components return Dictionary, even opaque types

## Limitations

1. **Requires reflection**: Components must be registered with Flecs reflection metadata for struct types
2. **Performance**: Slightly slower than direct access (uses runtime introspection)
3. **Opaque types**: Require explicit type name checking (not ideal but necessary)

## Migration Guide

### For Component Authors

You can now **remove** `to_dict()` and `from_dict()` implementations from your components if they only serialize/deserialize basic fields. The cursor-based approach will handle it automatically.

Keep custom implementations only if you need:
- Custom validation logic
- Computed fields
- Complex transformations
- Special initialization

### For Component Users

No changes needed! The API remains the same:
- `FlecsServer::get_component_by_name()`
- `FlecsServer::get_component_by_id()`
- `FlecsServer::set_component()`

All now use cursor-based conversion internally.

## Debugging

If you see warnings like:
```
cursor_to_variant: Unhandled type 'SomeType' - returning empty Variant
```

This means:
1. The type is not in the opaque type list (lines 56-195 in flecs_server.cpp)
2. The type doesn't have `EcsType` reflection metadata
3. You need to either:
   - Add it to the opaque type checks in `cursor_to_variant()`
   - Register it with Flecs reflection using `.member<>()` calls

## Important Implementation Details

### Avoiding `get_mut()` Assertions

The Flecs `get_mut()` method will assert if the entity doesn't have the component. To avoid crashes:
- **For reading**: Use `get()` (const access) which returns nullptr if component doesn't exist
- **For writing**: Use `ensure()` which creates the component if it doesn't exist
- **Never** call `get_mut()` without first checking `has()`

### World Singleton Handling

Flecs stores singleton components on the component entity itself (not on a separate singleton entity). When accessing world singletons:
```cpp
// Create an entity representing the component type
flecs::entity comp_entity(world.c_ptr(), comp_type);

// Use the component entity to get/set the singleton
component_to_dict_cursor(comp_entity, comp_type);
```

### Handling Opaque Types Without Reflection Metadata

**Problem**: When using `flecs::cursor` with opaque types (like RID, Variant, etc.), the cursor's `get_type()` may return an invalid entity (ID = 0) because these types don't have Flecs reflection metadata (EcsType component).

**Solution**: 
1. **Always check if type is valid** before accessing type properties:
   ```cpp
   flecs::entity type = cur.get_type();
   if (!type.is_valid()) {
       // Handle invalid type
   }
   ```

2. **Get type name from component entity** instead of cursor type:
   ```cpp
   // Instead of: const char* type_name = cur.get_type().name().c_str();
   // Use:
   flecs::entity comp_entity(world.c_ptr(), comp_type_id);
   const char* type_name = comp_entity.name().c_str();
   ```

3. **Access data directly** for opaque types:
   ```cpp
   // For opaque types, use the raw pointer directly
   RID* ptr = static_cast<RID*>(comp_ptr);
   // Don't rely on cursor.get_ptr() until after checking type validity
   ```

This fix ensures opaque Godot types (RID, Variant, Dictionary, String, etc.) work correctly even though they're registered as components without full reflection metadata.

## Future Improvements

1. Register Godot opaque types with Flecs' opaque type API for cleaner handling
2. Add caching of type metadata to reduce lookup overhead
3. Support for arrays/collections within components
4. Better error messages for unsupported type conversions
5. Add validation for type compatibility when setting values