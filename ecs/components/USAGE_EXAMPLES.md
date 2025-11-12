# Usage Examples: Flecs Reflection Component System

This document provides practical examples of using the refactored component system.

## Table of Contents
1. [Basic Setup](#basic-setup)
2. [Creating Entities with Components](#creating-entities-with-components)
3. [Querying Entities](#querying-entities)
4. [Component Serialization](#component-serialization)
5. [Integration with Godot](#integration-with-godot)
6. [Performance Tips](#performance-tips)

---

## Basic Setup

### Initialize Your World

```cpp
#include "ecs/components/all_components.h"

// In your initialization code
flecs::world world;

// Register all components (without serialization for better performance)
AllComponents::register_all(world, false);

// OR with serialization enabled
AllComponents::register_all(world, true);
```

### Manual Component Registration

If you only need specific components:

```cpp
#include "ecs/components/flecs_opaque_types.h"
#include "ecs/components/all_components.h"

flecs::world world;

// Register Godot opaque types first
FlecsOpaqueTypes::register_opaque_types(world);

// Register specific components
world.component<Transform3DComponent>();
world.component<VisibilityComponent>();
world.component<DirtyTransform>();
```

---

## Creating Entities with Components

### Simple Entity Creation

```cpp
// Create an entity with Transform3D
flecs::entity player = world.entity("Player")
    .set<Transform3DComponent>({
        Transform3D()  // Default transform
    });

// Add more components
player.set<VisibilityComponent>({ true });
player.add<DirtyTransform>();  // Tag component - no data needed
```

### Complex Entity with Multiple Components

```cpp
flecs::entity enemy = world.entity("Enemy")
    .set<Transform3DComponent>({
        Transform3D(Basis(), Vector3(10, 0, 5))
    })
    .set<MeshComponent>({
        .mesh_id = mesh_rid,
        .material_ids = { material_rid },
        .custom_aabb = AABB()
    })
    .set<VisibilityComponent>({ true })
    .set<DirectionalLight3DComponent>({
        .light_id = light_rid,
        .light_color = Color(1, 0, 0),
        .intensity = 2.0f
    });
```

### Using Object Instance Components

```cpp
// Link Godot Object to ECS entity
Node3D* godot_node = memnew(Node3D);
ObjectID node_object_id = godot_node->get_instance_id();

flecs::entity entity = world.entity("MyNode")
    .set<ObjectInstanceComponent>(ObjectInstanceComponent(node_object_id))
    .set<SceneNodeComponent>({
        .node_id = node_object_id,
        .class_name = StringName("Node3D")
    });

// Later, retrieve the Godot object
if (entity.has<ObjectInstanceComponent>()) {
    const ObjectInstanceComponent& comp = entity.get<ObjectInstanceComponent>();
    Object* obj = ObjectDB::get_instance(comp.object_instance_id);
    if (obj) {
        Node3D* node = Object::cast_to<Node3D>(obj);
        // Use the node...
    }
}
```

---

## Querying Entities

### Basic Query

```cpp
// Query all entities with Transform3D and Visibility
world.query<Transform3DComponent, VisibilityComponent>()
    .each([](flecs::entity e, Transform3DComponent& transform, VisibilityComponent& vis) {
        if (vis.visible) {
            // Process visible entities
            transform.transform.origin.y += 0.1f;
        }
    });
```

### Query with Tag Filters

```cpp
// Only process dirty transforms
world.query<Transform3DComponent>()
    .term<DirtyTransform>()  // Filter by tag
    .each([](flecs::entity e, Transform3DComponent& transform) {
        // Update transform
        // ...
        
        // Remove dirty tag when done
        e.remove<DirtyTransform>();
    });
```

### Query with Optional Components

```cpp
// Query entities with Transform3D, optionally with Mesh
world.query_builder<Transform3DComponent>()
    .term<MeshComponent>().optional()
    .build()
    .each([](flecs::entity e, Transform3DComponent& transform) {
        // All entities have transform
        
        // Check for optional mesh
        if (e.has<MeshComponent>()) {
            const MeshComponent& mesh = e.get<MeshComponent>();
            // Do something with mesh...
        }
    });
```

### High-Performance Iteration

```cpp
// Batch processing with direct memory access
world.query<Transform3DComponent, VisibilityComponent>()
    .iter([](flecs::iter& it, Transform3DComponent* transforms, VisibilityComponent* visibility) {
        for (auto i : it) {
            if (visibility[i].visible) {
                // SIMD-friendly batch processing
                transforms[i].transform.origin.y += it.delta_time() * 2.0f;
            }
        }
    });
```

### Query Lighting Components

```cpp
// Update all point lights
world.query<PointLightComponent, Transform3DComponent>()
    .each([](flecs::entity e, PointLightComponent& light, Transform3DComponent& transform) {
        RenderingServer* rs = RenderingServer::get_singleton();
        rs->light_set_color(light.light_id, light.light_color);
        rs->instance_set_transform(light.light_id, transform.transform);
    });
```

---

## Component Serialization

### Serialize Single Component

```cpp
// Enable serialization during world setup
AllComponents::register_all(world, true);

flecs::entity player = world.entity("Player")
    .set<Transform3DComponent>({ Transform3D() });

// Serialize the transform component
flecs::entity_t comp_id = world.id<Transform3DComponent>();
Dictionary transform_data = AllComponents::get_component_dict(player, comp_id);

// Result: { "transform": Transform3D(...) }
print_line(transform_data);
```

### Deserialize Component

```cpp
// Modify the data
Dictionary modified_data;
modified_data["transform"] = Transform3D(Basis(), Vector3(5, 10, 15));

// Apply to entity
AllComponents::set_component_from_dict(player, comp_id, modified_data);

// Verify
const Transform3DComponent& comp = player.get<Transform3DComponent>();
print_line(comp.transform.origin);  // Vector3(5, 10, 15)
```

### Serialize Entire Entity

```cpp
Dictionary serialize_entity(flecs::entity e, const Vector<StringName>& component_names) {
    Dictionary entity_data;
    entity_data["name"] = String(e.name().c_str());
    
    Dictionary components;
    for (const StringName& comp_name : component_names) {
        FlecsReflection::ComponentMeta* meta = 
            FlecsReflection::Registry::get().get_by_name(comp_name);
        
        if (meta && e.has(meta->component_id)) {
            components[comp_name] = AllComponents::get_component_dict(e, meta->component_id);
        }
    }
    
    entity_data["components"] = components;
    return entity_data;
}

// Usage
Vector<StringName> components_to_save;
components_to_save.push_back("Transform3DComponent");
components_to_save.push_back("VisibilityComponent");

Dictionary saved = serialize_entity(player, components_to_save);
```

### Deserialize Entire Entity

```cpp
flecs::entity deserialize_entity(flecs::world& world, const Dictionary& entity_data) {
    String name = entity_data["name"];
    flecs::entity e = world.entity(name.utf8().get_data());
    
    Dictionary components = entity_data["components"];
    Array keys = components.keys();
    
    for (int i = 0; i < keys.size(); i++) {
        StringName comp_name = keys[i];
        Dictionary comp_data = components[comp_name];
        
        FlecsReflection::ComponentMeta* meta = 
            FlecsReflection::Registry::get().get_by_name(comp_name);
        
        if (meta) {
            AllComponents::set_component_from_dict(e, meta->component_id, comp_data);
        }
    }
    
    return e;
}
```

---

## Integration with Godot

### Scene-to-ECS Conversion

```cpp
void convert_node_to_entity(Node3D* node, flecs::world& world, flecs::entity parent = {}) {
    flecs::entity entity = world.entity(node->get_name().utf8().get_data());
    
    // Add basic components
    entity.set<Transform3DComponent>({ node->get_transform() });
    entity.set<VisibilityComponent>({ node->is_visible() });
    entity.set<SceneNodeComponent>({
        .node_id = node->get_instance_id(),
        .class_name = node->get_class()
    });
    
    // Set parent relationship
    if (parent.is_valid()) {
        entity.child_of(parent);
    }
    
    // Check for mesh instance
    MeshInstance3D* mesh_instance = Object::cast_to<MeshInstance3D>(node);
    if (mesh_instance && mesh_instance->get_mesh().is_valid()) {
        RID mesh_rid = mesh_instance->get_mesh()->get_rid();
        entity.set<MeshComponent>({
            .mesh_id = mesh_rid,
            .custom_aabb = mesh_instance->get_aabb()
        });
    }
    
    // Check for light
    Light3D* light = Object::cast_to<Light3D>(node);
    if (light) {
        RID light_rid = light->get_rid();
        if (Object::cast_to<DirectionalLight3D>(light)) {
            entity.set<DirectionalLight3DComponent>({
                .light_id = light_rid,
                .light_color = light->get_color(),
                .intensity = light->get_param(Light3D::PARAM_ENERGY)
            });
        }
    }
    
    // Recursively convert children
    for (int i = 0; i < node->get_child_count(); i++) {
        Node3D* child = Object::cast_to<Node3D>(node->get_child(i));
        if (child) {
            convert_node_to_entity(child, world, entity);
        }
    }
}
```

### ECS-to-Scene Synchronization System

```cpp
void create_transform_sync_system(flecs::world& world) {
    world.system<Transform3DComponent, SceneNodeComponent>()
        .term<DirtyTransform>()
        .kind(flecs::OnUpdate)
        .each([](flecs::entity e, 
                 Transform3DComponent& transform, 
                 SceneNodeComponent& scene_node) {
            
            // Get the Godot node
            Object* obj = ObjectDB::get_instance(scene_node.node_id);
            Node3D* node = Object::cast_to<Node3D>(obj);
            
            if (node) {
                // Sync transform from ECS to Godot
                node->set_transform(transform.transform);
                
                // Clear dirty flag
                e.remove<DirtyTransform>();
            }
        });
}
```

---

## Performance Tips

### 1. Prefer Tag Components

```cpp
// Instead of:
struct IsSelected {
    bool selected = true;
};

// Use:
struct Selected {};  // Zero-size tag

// Usage
entity.add<Selected>();
if (entity.has<Selected>()) { /* ... */ }
```

### 2. Batch Operations

```cpp
// Bad: Individual operations
for (int i = 0; i < 1000; i++) {
    world.entity()
        .set<Transform3DComponent>({ Transform3D() });
}

// Good: Batch with defer
world.defer([&]() {
    for (int i = 0; i < 1000; i++) {
        world.entity()
            .set<Transform3DComponent>({ Transform3D() });
    }
});
```

### 3. Use Systems Instead of Queries in Hot Loops

```cpp
// Define system once
auto update_system = world.system<Transform3DComponent, VisibilityComponent>()
    .kind(flecs::OnUpdate)
    .iter([](flecs::iter& it, Transform3DComponent* transforms, VisibilityComponent* vis) {
        for (auto i : it) {
            if (vis[i].visible) {
                transforms[i].transform.origin.y += it.delta_time();
            }
        }
    });

// Then just call world.progress() each frame
world.progress(delta_time);
```

### 4. Avoid Serialization in Hot Paths

```cpp
// Don't serialize/deserialize every frame
// Instead, work directly with components:

world.query<Transform3DComponent>()
    .each([](flecs::entity e, Transform3DComponent& t) {
        // Direct access - fast!
        t.transform.origin.x += 1.0f;
    });
```

### 5. Use Prefabs for Common Entities

```cpp
// Create prefab
flecs::entity enemy_prefab = world.prefab("EnemyPrefab")
    .set<MeshComponent>({ enemy_mesh_rid })
    .set<VisibilityComponent>({ true });

// Instantiate many times (very fast)
for (int i = 0; i < 100; i++) {
    world.entity()
        .is_a(enemy_prefab)
        .set<Transform3DComponent>({ 
            Transform3D(Basis(), Vector3(i * 2, 0, 0)) 
        });
}
```

### 6. Minimize Component Size

```cpp
// Bad: Large component
struct EnemyComponent {
    String name;                    // 24+ bytes
    Vector<Vector3> waypoints;      // 24+ bytes
    Dictionary properties;          // 16+ bytes
};  // Total: 64+ bytes

// Good: Keep it small
struct EnemyComponent {
    uint32_t waypoint_index;        // 4 bytes
    uint8_t enemy_type;             // 1 byte
    bool is_aggressive;             // 1 byte
};  // Total: 8 bytes (better cache usage)
```

---

## Complete Example: Simple Game System

```cpp
#include "ecs/components/all_components.h"

class GameWorld {
    flecs::world world;
    
public:
    GameWorld() {
        // Initialize
        AllComponents::register_all(world, false);
        setup_systems();
    }
    
    void setup_systems() {
        // Movement system
        world.system<Transform3DComponent>()
            .term<DirtyTransform>()
            .kind(flecs::OnUpdate)
            .each([](flecs::entity e, Transform3DComponent& transform) {
                // Process movement
                e.remove<DirtyTransform>();
            });
        
        // Visibility culling
        world.system<Transform3DComponent, VisibilityComponent, CameraComponent>()
            .term<MainCamera>()
            .kind(flecs::OnUpdate)
            .each([](flecs::entity cam_entity, 
                     Transform3DComponent& cam_transform,
                     VisibilityComponent& cam_vis,
                     CameraComponent& camera) {
                
                // Update frustum from camera
                // Cull invisible objects
            });
    }
    
    flecs::entity spawn_player(const Transform3D& transform) {
        return world.entity("Player")
            .set<Transform3DComponent>({ transform })
            .set<VisibilityComponent>({ true })
            .add<DirtyTransform>();
    }
    
    void update(float delta_time) {
        world.progress(delta_time);
    }
};
```

---

## Summary

The refactored component system provides:
- ✅ **Simple POD structs** - No inheritance needed
- ✅ **Optional serialization** - Only when you need it
- ✅ **High performance** - No virtual calls, cache-friendly
- ✅ **Clean integration** - Works seamlessly with Flecs and Godot
- ✅ **Flexible** - Use only what you need

Start simple, add serialization only where needed, and enjoy the performance benefits!