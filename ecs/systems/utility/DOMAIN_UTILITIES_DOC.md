# Domain-Specific Utilities Documentation

**Last Updated:** Phase 2 Complete  
**Module:** `godot_turbo/ecs/utility`  
**Status:** ✅ Comprehensive documentation added to all domain utilities

---

## Overview

This document summarizes the documentation work completed for the domain-specific ECS utility classes in Phase 2. These utilities bridge Godot's native systems (Physics, Navigation, Rendering) with the Flecs ECS architecture.

---

## Documented Utilities

### 1. Physics Utilities

#### **Physics2DUtility** (`physics2d_utility.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~220
- **Methods Documented:** 7 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (generally safe, with caveats)
  - 4 usage examples covering areas, bodies, joints
  - Parameter documentation for all methods
  - Return value documentation
  - Error condition notes

**Key Components Created:**
- `Area2DComponent` entities
- `Body2DComponent` entities (RigidBody2D, PhysicsBody2D)
- `Joint2DComponent` entities

#### **Physics3DUtility** (`physics3d_utility.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~270
- **Methods Documented:** 9 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (generally safe, with caveats)
  - 5 usage examples covering areas, bodies, joints, soft bodies
  - Parameter documentation for all methods
  - Return value documentation
  - Error condition notes

**Key Components Created:**
- `Area3DComponent` entities
- `Body3DComponent` entities (RigidBody3D, PhysicsBody3D)
- `Joint3DComponent` entities
- `SoftBody3DComponent` entities

---

### 2. Navigation Utilities

#### **Navigation2DUtility** (`navigation2d_utility.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~370
- **Methods Documented:** 15 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (generally safe, main-thread recommendations for map updates)
  - 6 usage examples covering agents, links, obstacles, regions, parsers
  - Detailed explanation of navigation concepts
  - Parameter documentation for all methods
  - Return value documentation

**Key Components Created:**
- `NavAgent2DComponent` entities
- `NavLink2DComponent` entities
- `NavObstacle2DComponent` entities
- `NavRegion2DComponent` entities
- `SourceGeometryParser2DComponent` entities

#### **Navigation3DUtility** (`navigation3d_utility.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~370
- **Methods Documented:** 15 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (generally safe, main-thread recommendations for map updates)
  - 6 usage examples covering agents, links, obstacles, regions, parsers
  - Detailed explanation of navigation concepts
  - Parameter documentation for all methods
  - Return value documentation

**Key Components Created:**
- `NavAgent3DComponent` entities
- `NavLink3DComponent` entities
- `NavObstacle3DComponent` entities
- `NavRegion3DComponent` entities
- `SourceGeometryParser3DComponent` entities

---

### 3. Rendering Utilities

#### **RenderUtility2D** (`render_utility_2d.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~330
- **Methods Documented:** 26 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (mixed safety, main-thread recommended)
  - 6 usage examples covering meshes, cameras, lights, particles, multimesh, skeletons
  - Detailed rendering pipeline explanations
  - Parameter documentation for all methods
  - Return value documentation

**Key Components Created:**
- `MeshInstance2DComponent` entities
- `MultiMeshInstance2DComponent` entities (with instance hierarchy)
- `Camera2DComponent` entities
- `DirectionalLight2DComponent` entities
- `PointLight2DComponent` entities
- `CanvasItemComponent` entities (generic)
- `Skeleton2DComponent` entities
- `LightOccluder2DComponent` entities
- `GPUParticles2DComponent` entities

#### **RenderUtility3D** (`render_utility_3d.h`)
- **Status:** ✅ Fully Documented
- **Lines of Documentation:** ~510
- **Methods Documented:** 35 public methods
- **Features:**
  - Complete Doxygen-style comments
  - Thread-safety notes (mixed safety, main-thread recommended)
  - 6 usage examples covering meshes, cameras, lights, particles, multimesh, environment, GI
  - Detailed 3D rendering pipeline explanations
  - Parameter documentation for all methods
  - Return value documentation
  - Advanced features documented (VoxelGI, ReflectionProbe, Compositor)

**Key Components Created:**
- `MeshInstance3DComponent` entities
- `MultiMeshInstance3DComponent` entities (with instance hierarchy)
- `Camera3DComponent` entities
- `DirectionalLight3DComponent` entities
- `OmniLight3DComponent` entities
- `SpotLight3DComponent` entities
- `Skeleton3DComponent` entities
- `GPUParticles3DComponent` entities
- `ReflectionProbeComponent` entities
- `VoxelGIComponent` entities
- `EnvironmentComponent` entities
- `ViewportComponent` entities
- `ScenarioComponent` entities
- `CompositorComponent` entities

---

## Documentation Metrics

| Utility | Total Lines | Methods | Examples | Components |
|---------|-------------|---------|----------|------------|
| Physics2DUtility | ~220 | 7 | 4 | 3 |
| Physics3DUtility | ~270 | 9 | 5 | 4 |
| Navigation2DUtility | ~370 | 15 | 6 | 5 |
| Navigation3DUtility | ~370 | 15 | 6 | 5 |
| RenderUtility2D | ~330 | 26 | 6 | 9 |
| RenderUtility3D | ~510 | 35 | 6 | 14 |
| **TOTAL** | **~2,070** | **107** | **33** | **40** |

---

## Thread Safety Summary

### Generally Thread-Safe (with caveats):
- **Physics Utilities**: Creating physics entities is safe from any thread. PhysicsServer2D/3D are thread-safe. However, accessing node properties should ideally be done from the main thread.
- **Navigation Utilities**: Creating navigation entities is safe from any thread. NavigationServer2D/3D are thread-safe. Navigation map updates should be synchronized with the physics frame.

### Requires Main Thread (recommended):
- **Rendering Utilities**: While RenderingServer is thread-safe for resource creation, accessing CanvasItem/Node3D properties and modifying scene state should be done from the main thread to avoid race conditions.

### Protection Mechanisms:
- All utilities use **NodeStorage** (protected by mutexes)
- All utilities delegate to **FlecsServer** (protected by mutexes)
- All utilities call thread-safe Godot server singletons

---

## Common Patterns Across Utilities

All domain utilities follow consistent patterns:

### 1. **Three Creation Variants**
Each entity type can be created in three ways:
- `create_*()` - Create from scratch with parameters
- `create_*_with_id()` - Wrap an existing RID
- `create_*_with_object()` - Convert a Godot node

### 2. **Component Attachment**
All `*_with_object` methods attach:
- Domain-specific component (e.g., `Body2DComponent`)
- `ObjectInstanceComponent` (links back to the node)
- Entity name (from node's name)

### 3. **NodeStorage Registration**
All `*_with_object` methods register nodes via:
```cpp
FlecsServer::get_singleton()->add_to_node_storage(node, world_id);
```

### 4. **Error Handling**
All methods validate:
- Null pointer checks (`ERR_FAIL_V` if null)
- RID validity checks (`ERR_FAIL_V` if invalid)

### 5. **GDScript Binding**
All utilities provide `_bind_methods()` for script access

---

## Usage Recommendations

### Best Practices:

1. **Entity Lifecycle**
   - Do not delete source Godot nodes while ECS entities reference them
   - Use `ObjectInstanceComponent` to maintain the node→entity link
   - NodeStorage tracks nodes automatically

2. **Thread Safety**
   - Create entities during scene initialization from the main thread when possible
   - If creating from worker threads, avoid accessing node properties
   - Physics/Navigation creation is generally safe from any thread
   - Rendering creation should be done from the main thread

3. **Performance**
   - Use MultiMesh utilities for efficient instancing (grass, debris, crowds)
   - Batch entity creation when possible
   - Consider incremental conversion for large scenes

4. **Integration**
   - Use domain utilities through `SceneObjectUtility` for automatic detection
   - Or call directly for fine-grained control
   - Combine with `ResourceObjectUtility` for complete scene conversion

---

## Integration with SceneObjectUtility

The domain utilities are invoked by `SceneObjectUtility::create_entities()` for automatic scene conversion:

```cpp
// Physics detection (example)
if (PhysicsBody2D *physics_body = Object::cast_to<PhysicsBody2D>(node)) {
    entity_rid = Physics2DUtility::create_physics_body_with_object(world_id, physics_body);
}

// Navigation detection (example)
if (NavigationAgent3D *nav_agent = Object::cast_to<NavigationAgent3D>(node)) {
    entity_rid = Navigation3DUtility::create_nav_agent_with_object(world_id, nav_agent);
}

// Rendering detection (example)
if (MeshInstance3D *mesh = Object::cast_to<MeshInstance3D>(node)) {
    entity_rid = RenderUtility3D::create_mesh_instance_with_object(world_id, mesh);
}
```

---

## Examples Gallery

### Physics Example
```cpp
RigidBody2D* player_body = get_node<RigidBody2D>("Player");
RID world_id = get_world_id();
RID entity = Physics2DUtility::create_rigid_body_with_object(world_id, player_body);

// Query the entity
flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
flecs::entity e = FlecsServer::get_singleton()->_get_entity(world_id, entity);
const Body2DComponent* comp = e.get<Body2DComponent>();
// Use comp->body_id with PhysicsServer2D
```

### Navigation Example
```cpp
NavigationAgent3D* npc_agent = get_node<NavigationAgent3D>("NPC/Agent");
RID world_id = get_world_id();
RID entity = Navigation3DUtility::create_nav_agent_with_object(world_id, npc_agent);

// Configure via NavigationServer3D
const NavAgent3DComponent* comp = /* ... get from entity ... */;
NavigationServer3D::get_singleton()->agent_set_radius(comp->agent_id, 0.5);
```

### Rendering Example
```cpp
MeshInstance3D* character = get_node<MeshInstance3D>("Character");
RID world_id = get_world_id();
RID entity = RenderUtility3D::create_mesh_instance_with_object(world_id, character);

// The entity tracks the mesh, materials, skeleton, and transform
```

### MultiMesh Example
```cpp
MultiMeshInstance3D* grass = get_node<MultiMeshInstance3D>("GrassField");
RID world_id = get_world_id();
TypedArray<RID> entities = RenderUtility3D::create_multi_mesh_with_object(world_id, grass);

// entities[0] = parent MultiMesh entity
// entities[1..N] = individual instance entities
```

---

## Testing Recommendations

### Unit Tests Needed:
1. **Physics Tests**
   - Create area/body/joint entities
   - Verify component attachment
   - Test null pointer handling
   - Test RID validity checks

2. **Navigation Tests**
   - Create agent/link/obstacle/region entities
   - Verify callback registration for parsers
   - Test thread-safe creation
   - Verify NodeStorage registration

3. **Rendering Tests**
   - Create mesh/camera/light entities
   - Test MultiMesh hierarchy creation
   - Verify instance count correctness
   - Test particle and skeleton creation

### Integration Tests Needed:
1. Scene conversion with mixed node types
2. Multithreaded entity creation stress test
3. Large-scale scene conversion (10k+ nodes)
4. Memory leak detection (create/destroy cycles)

---

## Related Documentation

- [SceneObjectUtility Documentation](SCENE_OBJECT_UTILITY_DOC.md)
- [Validation Summary](VALIDATION_SUMMARY.md)
- [Module README](README.md)
- [Documentation Status](DOCUMENTATION_STATUS.md)

---

## Future Improvements

### Phase 3 Candidates:
1. Add mutex protection to domain utilities (if profiling shows contention)
2. Add batched creation methods for performance
3. Add entity metadata (source path, creation time, etc.)
4. Add bidirectional sync (ECS → Scene updates)
5. Add filter/callback API for selective conversion
6. Performance benchmarking suite
7. Memory profiling tools
8. Incremental/streaming conversion support

---

## Compilation Status

All documented files compile successfully with no errors:
- ✅ `physics2d_utility.h`
- ✅ `physics3d_utility.h`
- ✅ `navigation2d_utility.h`
- ✅ `navigation3d_utility.h`
- ✅ `render_utility_2d.h`
- ✅ `render_utility_3d.h` (1 minor warning)

---

## Summary

**Phase 2 is complete.** All six domain-specific utilities now have:
- ✅ Comprehensive API documentation
- ✅ Thread-safety notes and constraints
- ✅ Usage examples for common scenarios
- ✅ Parameter and return value documentation
- ✅ Error handling documentation
- ✅ Integration notes

The utilities are production-ready from a documentation standpoint. The next phase should focus on testing, performance optimization, and potential enhancements like bidirectional sync or incremental conversion.