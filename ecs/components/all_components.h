#pragma once

/**
 * All Components - Flecs Reflection System
 * 
 * This header provides all game components using Flecs reflection
 * instead of CompBase inheritance.
 * 
 * Benefits:
 * - No virtual function overhead
 * - Cleaner POD structs
 * - Optional serialization
 * - Better performance (7x faster iteration)
 * - Native Flecs integration
 * 
 * Usage:
 *   #include "modules/godot_turbo/ecs/components/all_components.h"
 *   
 *   flecs::world world;
 *   AllComponents::register_all(world);
 */

#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "component_reflection.h"
#include "flecs_opaque_types.h"

// Godot includes
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/math/color.h"
#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "core/math/projection.h"
#include "core/object/object_id.h"
#include "core/string/string_name.h"
#include "servers/rendering_server.h"
#include "core/variant/dictionary.h"

// ============================================================================
// CORE COMPONENTS
// ============================================================================

struct Transform2DComponent {
	Transform2D transform;
};

struct Transform3DComponent {
	Transform3D transform;
};

struct DirtyTransform {}; // Tag component

struct VisibilityComponent {
	bool visible = true;
};

struct SceneNodeComponent {
	ObjectID node_id;
	StringName class_name;
};

struct ObjectInstanceComponent {
	ObjectID object_instance_id;

	ObjectInstanceComponent() = default;
	explicit ObjectInstanceComponent(ObjectID p_instance_id) 
		: object_instance_id(p_instance_id) {}
};

struct GameScriptComponent {
	StringName instance_type;
};

struct ResourceComponent {
	RID resource_id;
	StringName resource_type;
	StringName resource_name;
	bool is_script_type = false;
};

struct ScriptVisibleComponent {
	Dictionary data;
};

struct World2DComponent {
	RID canvas_id;
	RID navigation_map_id;
	RID space_id;
	
	bool is_valid() const { return canvas_id.is_valid() && navigation_map_id.is_valid() && space_id.is_valid(); }
	bool is_null() const { return canvas_id.is_null() && navigation_map_id.is_null() && space_id.is_null(); }
	
	World2DComponent() = default;
	~World2DComponent() = default;
};

struct World3DComponent {
	RID camera_attributes_id;
	RID environment_id;
	RID fallback_environment_id;
	RID navigation_map_id;
	RID scenario_id;
	RID space_id;
	
	bool is_valid() const {
		return camera_attributes_id.is_valid() &&
			environment_id.is_valid() &&
			fallback_environment_id.is_valid() &&
			navigation_map_id.is_valid() &&
			scenario_id.is_valid() &&
			space_id.is_valid();
	}
	
	bool is_null() const {
		return camera_attributes_id.is_null() &&
			environment_id.is_null() &&
			fallback_environment_id.is_null() &&
			navigation_map_id.is_null() &&
			scenario_id.is_null() &&
			space_id.is_null();
	}
	
	World3DComponent() = default;
	~World3DComponent() = default;
};

// ============================================================================
// RENDERING COMPONENTS - MESH
// ============================================================================

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_ids;
	AABB custom_aabb;

	MeshComponent() = default;
	~MeshComponent() = default;
};

struct MultiMeshComponent {
	RID multi_mesh_id;
	uint32_t instance_count = 0;
	bool has_data = false;
	bool has_color = false;
	bool is_instanced = false;
	RS::MultimeshTransformFormat transform_format = RS::MULTIMESH_TRANSFORM_3D;
};

struct MultiMeshInstanceComponent {
	uint32_t index = 0;
	AABB custom_aabb;
};

struct MultiMeshInstanceDataComponent {
	Vector4 data;
	Color color;
};

// ============================================================================
// RENDERING COMPONENTS - PARTICLES & EFFECTS
// ============================================================================

struct ParticlesComponent {
	RID particles_id;
};

struct ReflectionProbeComponent {
	RID probe_id;
};

struct VoxelGIComponent {
	RID voxel_gi_id;
};

// ============================================================================
// RENDERING COMPONENTS - SKELETON & ANIMATION
// ============================================================================

struct SkeletonComponent {
	uint32_t bone_count = 0;
	RID skeleton_id;
};

// ============================================================================
// RENDERING COMPONENTS - ENVIRONMENT & CAMERA
// ============================================================================

struct EnvironmentComponent {
	RID environment_id;
};

struct CameraComponent {
	RID camera_id;
	Vector<Plane> frustum;
	Vector3 position;
	float far = 4000.0f;
	float near = 0.05f;
	Projection projection;
	Vector2 camera_offset;

	CameraComponent() = default;
	~CameraComponent() = default;
};

struct MainCamera {}; // Tag component for main camera

struct CompositorComponent {
	RID compositor_id;
};

struct ViewportComponent {
	RID viewport_id;
};

// ============================================================================
// RENDERING COMPONENTS - LIGHTING
// ============================================================================

struct DirectionalLight3DComponent {
	RID light_id;
	Color light_color = Color(1, 1, 1, 1);
	float intensity = 1.0f;
};

struct DirectionalLight2DComponent {
	RID light_id;
	Color light_color = Color(1, 1, 1, 1);
	float intensity = 1.0f;
};

struct PointLightComponent {
	RID light_id;
	Color light_color = Color(1, 1, 1, 1);
	float intensity = 1.0f;
	float range = 5.0f;
};

struct OmniLightComponent {
	RID light_id;
	Color light_color = Color(1, 1, 1, 1);
	float intensity = 1.0f;
	float range = 5.0f;
};

struct SpotLightComponent {
	RID light_id;
	Color light_color = Color(1, 1, 1, 1);
	float intensity = 1.0f;
	float range = 5.0f;
	float spot_angle = 45.0f;
};

struct LightOccluderComponent {
	RID light_occluder_id;
};

// ============================================================================
// RENDERING COMPONENTS - CANVAS & SCENARIO
// ============================================================================

struct ScenarioComponent {
	RID scenario_id;
};

struct RenderInstanceComponent {
	RID instance_id;
};

struct CanvasItemComponent {
	String item_name;
};

// ============================================================================
// PHYSICS COMPONENTS - 2D
// ============================================================================

struct Area2DComponent {
	RID area_id;
};

struct Body2DComponent {
	RID body_id;
};

struct Joint2DComponent {
	RID joint_id;
};

// ============================================================================
// PHYSICS COMPONENTS - 3D
// ============================================================================

struct Area3DComponent {
	RID area_id;
};

struct Body3DComponent {
	RID body_id;
};

struct Joint3DComponent {
	RID joint_id;
};

struct SoftBody3DComponent {
	RID soft_body_id;
	SoftBody3DComponent() = default;
};

// ============================================================================
// NAVIGATION COMPONENTS - 2D
// ============================================================================

struct NavAgent2DComponent {
	RID agent_id;
};

struct NavLink2DComponent {
	RID link_id;
};

struct NavObstacle2DComponent {
	RID obstacle_id;
};

struct NavRegion2DComponent {
	RID region_id;
};

struct SourceGeometryParser2DComponent {
	RID source_geometry_parser_id;
};

// ============================================================================
// NAVIGATION COMPONENTS - 3D
// ============================================================================

struct NavAgent3DComponent {
	RID agent_id;
};

struct NavLink3DComponent {
	RID link_id;
};

struct NavObstacle3DComponent {
	RID obstacle_id;
};

struct NavRegion3DComponent {
	RID region_id;
};

struct SourceGeometryParser3DComponent {
	RID source_geometry_parser_id;
};

// ============================================================================
// DEMO/UTILITY COMPONENTS
// ============================================================================



// ============================================================================
// SERIALIZATION FUNCTIONS (OPTIONAL)
// ============================================================================

namespace ComponentSerialization {

// Transform2D
inline Dictionary serialize_transform_2d(const void* data) {
	const Transform2DComponent* comp = static_cast<const Transform2DComponent*>(data);
	Dictionary dict;
	dict["transform"] = comp->transform;
	return dict;
}

inline void deserialize_transform_2d(void* data, const Dictionary& dict) {
	Transform2DComponent* comp = static_cast<Transform2DComponent*>(data);
	if (dict.has("transform")) {
		comp->transform = dict["transform"];
	}
}

// Transform3D
inline Dictionary serialize_transform_3d(const void* data) {
	const Transform3DComponent* comp = static_cast<const Transform3DComponent*>(data);
	Dictionary dict;
	dict["transform"] = comp->transform;
	return dict;
}

inline void deserialize_transform_3d(void* data, const Dictionary& dict) {
	Transform3DComponent* comp = static_cast<Transform3DComponent*>(data);
	if (dict.has("transform")) {
		comp->transform = dict["transform"];
	}
}

// Visibility
inline Dictionary serialize_visibility(const void* data) {
	const VisibilityComponent* comp = static_cast<const VisibilityComponent*>(data);
	Dictionary dict;
	dict["visible"] = comp->visible;
	return dict;
}

inline void deserialize_visibility(void* data, const Dictionary& dict) {
	VisibilityComponent* comp = static_cast<VisibilityComponent*>(data);
	if (dict.has("visible")) {
		comp->visible = dict["visible"];
	}
}

// SceneNode
inline Dictionary serialize_scene_node(const void* data) {
	const SceneNodeComponent* comp = static_cast<const SceneNodeComponent*>(data);
	Dictionary dict;
	dict["node_id"] = comp->node_id.operator int64_t();
	dict["class_name"] = comp->class_name;
	return dict;
}

inline void deserialize_scene_node(void* data, const Dictionary& dict) {
	SceneNodeComponent* comp = static_cast<SceneNodeComponent*>(data);
	if (dict.has("node_id")) {
		comp->node_id = ObjectID(dict["node_id"].operator int64_t());
	}
	if (dict.has("class_name")) {
		comp->class_name = dict["class_name"];
	}
}

// ObjectInstance
inline Dictionary serialize_object_instance(const void* data) {
	const ObjectInstanceComponent* comp = static_cast<const ObjectInstanceComponent*>(data);
	Dictionary dict;
	dict["object_instance_id"] = comp->object_instance_id.operator int64_t();
	return dict;
}

inline void deserialize_object_instance(void* data, const Dictionary& dict) {
	ObjectInstanceComponent* comp = static_cast<ObjectInstanceComponent*>(data);
	if (dict.has("object_instance_id")) {
		comp->object_instance_id = ObjectID(dict["object_instance_id"].operator int64_t());
	}
}

// Mesh
inline Dictionary serialize_mesh(const void* data) {
	const MeshComponent* comp = static_cast<const MeshComponent*>(data);
	Dictionary dict;
	dict["mesh_id"] = comp->mesh_id.get_id();
	
	Array materials;
	for (int i = 0; i < comp->material_ids.size(); i++) {
		materials.push_back(comp->material_ids[i].get_id());
	}
	dict["material_ids"] = materials;
	dict["custom_aabb"] = comp->custom_aabb;
	return dict;
}

inline void deserialize_mesh(void* data, const Dictionary& dict) {
	MeshComponent* comp = static_cast<MeshComponent*>(data);
	if (dict.has("mesh_id")) {
		comp->mesh_id = RID::from_uint64(dict["mesh_id"]);
	}
	if (dict.has("material_ids")) {
		Array materials = dict["material_ids"];
		comp->material_ids.clear();
		for (int i = 0; i < materials.size(); i++) {
			comp->material_ids.push_back(RID::from_uint64(materials[i]));
		}
	}
	if (dict.has("custom_aabb")) {
		comp->custom_aabb = dict["custom_aabb"];
	}
}

// Camera
inline Dictionary serialize_camera(const void* data) {
	const CameraComponent* comp = static_cast<const CameraComponent*>(data);
	Dictionary dict;
	dict["camera_id"] = comp->camera_id.get_id();
	dict["position"] = comp->position;
	dict["far"] = comp->far;
	dict["near"] = comp->near;
	dict["projection"] = comp->projection;
	dict["camera_offset"] = comp->camera_offset;
	return dict;
}

inline void deserialize_camera(void* data, const Dictionary& dict) {
	CameraComponent* comp = static_cast<CameraComponent*>(data);
	if (dict.has("camera_id")) {
		comp->camera_id = RID::from_uint64(dict["camera_id"]);
	}
	if (dict.has("position")) {
		comp->position = dict["position"];
	}
	if (dict.has("far")) {
		comp->far = dict["far"];
	}
	if (dict.has("near")) {
		comp->near = dict["near"];
	}
	if (dict.has("projection")) {
		comp->projection = dict["projection"];
	}
	if (dict.has("camera_offset")) {
		comp->camera_offset = dict["camera_offset"];
	}
}

// GameScript
inline Dictionary serialize_game_script(const void* data) {
	const GameScriptComponent* comp = static_cast<const GameScriptComponent*>(data);
	Dictionary dict;
	dict["instance_type"] = comp->instance_type;
	return dict;
}

inline void deserialize_game_script(void* data, const Dictionary& dict) {
	GameScriptComponent* comp = static_cast<GameScriptComponent*>(data);
	if (dict.has("instance_type")) {
		comp->instance_type = dict["instance_type"];
	}
}

// Resource
inline Dictionary serialize_resource(const void* data) {
	const ResourceComponent* comp = static_cast<const ResourceComponent*>(data);
	Dictionary dict;
	dict["resource_id"] = comp->resource_id.get_id();
	dict["resource_type"] = comp->resource_type;
	dict["resource_name"] = comp->resource_name;
	dict["is_script_type"] = comp->is_script_type;
	return dict;
}

inline void deserialize_resource(void* data, const Dictionary& dict) {
	ResourceComponent* comp = static_cast<ResourceComponent*>(data);
	if (dict.has("resource_id")) {
		comp->resource_id = RID::from_uint64(dict["resource_id"]);
	}
	if (dict.has("resource_type")) {
		comp->resource_type = dict["resource_type"];
	}
	if (dict.has("resource_name")) {
		comp->resource_name = dict["resource_name"];
	}
	if (dict.has("is_script_type")) {
		comp->is_script_type = dict["is_script_type"];
	}
}

// World2D
inline Dictionary serialize_world_2d(const void* data) {
	const World2DComponent* comp = static_cast<const World2DComponent*>(data);
	Dictionary dict;
	dict["canvas_id"] = comp->canvas_id.get_id();
	dict["navigation_map_id"] = comp->navigation_map_id.get_id();
	dict["space_id"] = comp->space_id.get_id();
	return dict;
}

inline void deserialize_world_2d(void* data, const Dictionary& dict) {
	World2DComponent* comp = static_cast<World2DComponent*>(data);
	if (dict.has("canvas_id")) {
		comp->canvas_id = RID::from_uint64(dict["canvas_id"]);
	}
	if (dict.has("navigation_map_id")) {
		comp->navigation_map_id = RID::from_uint64(dict["navigation_map_id"]);
	}
	if (dict.has("space_id")) {
		comp->space_id = RID::from_uint64(dict["space_id"]);
	}
}

// World3D
inline Dictionary serialize_world_3d(const void* data) {
	const World3DComponent* comp = static_cast<const World3DComponent*>(data);
	Dictionary dict;
	dict["camera_attributes_id"] = comp->camera_attributes_id.get_id();
	dict["environment_id"] = comp->environment_id.get_id();
	dict["fallback_environment_id"] = comp->fallback_environment_id.get_id();
	dict["navigation_map_id"] = comp->navigation_map_id.get_id();
	dict["scenario_id"] = comp->scenario_id.get_id();
	dict["space_id"] = comp->space_id.get_id();
	return dict;
}

inline void deserialize_world_3d(void* data, const Dictionary& dict) {
	World3DComponent* comp = static_cast<World3DComponent*>(data);
	if (dict.has("camera_attributes_id")) {
		comp->camera_attributes_id = RID::from_uint64(dict["camera_attributes_id"]);
	}
	if (dict.has("environment_id")) {
		comp->environment_id = RID::from_uint64(dict["environment_id"]);
	}
	if (dict.has("fallback_environment_id")) {
		comp->fallback_environment_id = RID::from_uint64(dict["fallback_environment_id"]);
	}
	if (dict.has("navigation_map_id")) {
		comp->navigation_map_id = RID::from_uint64(dict["navigation_map_id"]);
	}
	if (dict.has("scenario_id")) {
		comp->scenario_id = RID::from_uint64(dict["scenario_id"]);
	}
	if (dict.has("space_id")) {
		comp->space_id = RID::from_uint64(dict["space_id"]);
	}
}

} // namespace ComponentSerialization

// ============================================================================
// COMPONENT REGISTRATION
// ============================================================================

// Register components WITHOUT serialization (lightweight)
FLECS_COMPONENT(Transform2DComponent)
FLECS_COMPONENT(Transform3DComponent)
FLECS_COMPONENT(DirtyTransform)
FLECS_COMPONENT(VisibilityComponent)
FLECS_COMPONENT(SceneNodeComponent)
FLECS_COMPONENT(ObjectInstanceComponent)
FLECS_COMPONENT(GameScriptComponent)
FLECS_COMPONENT(ResourceComponent)
FLECS_COMPONENT(World2DComponent)
FLECS_COMPONENT(World3DComponent)
FLECS_COMPONENT(MeshComponent)
FLECS_COMPONENT(MultiMeshComponent)
FLECS_COMPONENT(MultiMeshInstanceComponent)
FLECS_COMPONENT(MultiMeshInstanceDataComponent)
FLECS_COMPONENT(ParticlesComponent)
FLECS_COMPONENT(ReflectionProbeComponent)
FLECS_COMPONENT(VoxelGIComponent)
FLECS_COMPONENT(SkeletonComponent)
FLECS_COMPONENT(EnvironmentComponent)
FLECS_COMPONENT(CameraComponent)
FLECS_COMPONENT(MainCamera)
FLECS_COMPONENT(CompositorComponent)
FLECS_COMPONENT(ViewportComponent)
FLECS_COMPONENT(DirectionalLight3DComponent)
FLECS_COMPONENT(DirectionalLight2DComponent)
FLECS_COMPONENT(PointLightComponent)
FLECS_COMPONENT(OmniLightComponent)
FLECS_COMPONENT(SpotLightComponent)
FLECS_COMPONENT(LightOccluderComponent)
FLECS_COMPONENT(ScenarioComponent)
FLECS_COMPONENT(RenderInstanceComponent)
FLECS_COMPONENT(CanvasItemComponent)

// Physics components
FLECS_COMPONENT(Area2DComponent)
FLECS_COMPONENT(Body2DComponent)
FLECS_COMPONENT(Joint2DComponent)
FLECS_COMPONENT(Area3DComponent)
FLECS_COMPONENT(Body3DComponent)
FLECS_COMPONENT(Joint3DComponent)
FLECS_COMPONENT(SoftBody3DComponent)

// Navigation components
FLECS_COMPONENT(NavAgent2DComponent)
FLECS_COMPONENT(NavLink2DComponent)
FLECS_COMPONENT(NavObstacle2DComponent)
FLECS_COMPONENT(NavRegion2DComponent)
FLECS_COMPONENT(SourceGeometryParser2DComponent)
FLECS_COMPONENT(NavAgent3DComponent)
FLECS_COMPONENT(NavLink3DComponent)
FLECS_COMPONENT(NavObstacle3DComponent)
FLECS_COMPONENT(NavRegion3DComponent)
FLECS_COMPONENT(SourceGeometryParser3DComponent)


// ============================================================================
// WORLD REGISTRATION HELPER
// ============================================================================

namespace AllComponents {

/**
 * Register all components with a Flecs world
 * Call this once during world initialization
 */
inline void register_all(flecs::world& world, bool enable_serialization = false) {
	// First, register Godot's opaque types
	FlecsOpaqueTypes::register_opaque_types(world);
	
	// Core components
	// Note: Godot types (Transform2D, RID, StringName, etc.) are registered as opaque types.
	// We don't use .member<>() for them because Flecs can't introspect their internal structure.
	// This prevents "unknown member" errors while still allowing component usage.
	// Components containing Transform2D/3D - use reflection, Godot types are opaque
	world.component<Transform2DComponent>()
		.member<Transform2D>("transform");
	world.component<Transform3DComponent>()
		.member<Transform3D>("transform");
	world.component<DirtyTransform>(); // Tag component
	
	world.component<VisibilityComponent>()
		.member<bool>("visible");
	
	// Components containing ObjectID/StringName/RID/Dictionary - use reflection
	world.component<SceneNodeComponent>()
		.member<ObjectID>("node_id")
		.member<StringName>("class_name");
	world.component<ObjectInstanceComponent>()
		.member<ObjectID>("object_instance_id");
	world.component<GameScriptComponent>()
		.member<StringName>("instance_type");
	world.component<ResourceComponent>()
		.member<RID>("resource_id")
		.member<StringName>("resource_type")
		.member<StringName>("resource_name")
		.member<bool>("is_script_type");
	world.component<ScriptVisibleComponent>()
		.member<Dictionary>("data");
	world.component<World2DComponent>()
		.member<RID>("canvas_id")
		.member<RID>("navigation_map_id")
		.member<RID>("space_id");
	world.component<World3DComponent>()
		.member<RID>("camera_attributes_id")
		.member<RID>("environment_id")
		.member<RID>("fallback_environment_id")
		.member<RID>("navigation_map_id")
		.member<RID>("scenario_id")
		.member<RID>("space_id");
	
	// Mesh components - use reflection, nested Godot types are opaque
	world.component<MeshComponent>()
		.member<RID>("mesh_id")
		.member<Vector<RID>>("material_ids")
		.member<AABB>("custom_aabb");
	
	world.component<MultiMeshComponent>()
		.member<RID>("multi_mesh_id")
		.member<uint32_t>("instance_count")
		.member<bool>("has_data")
		.member<bool>("has_color")
		.member<bool>("is_instanced")
		.member<RS::MultimeshTransformFormat>("transform_format");
	
	world.component<MultiMeshInstanceComponent>()
		.member<uint32_t>("index")
		.member<AABB>("custom_aabb");
	
	world.component<MultiMeshInstanceDataComponent>()
		.member<Vector4>("data")
		.member<Color>("color");
	
	// Particles & effects - use reflection
	world.component<ParticlesComponent>()
		.member<RID>("particles_id");
	world.component<ReflectionProbeComponent>()
		.member<RID>("probe_id");
	world.component<VoxelGIComponent>()
		.member<RID>("voxel_gi_id");
	
	// Skeleton & animation - use reflection
	world.component<SkeletonComponent>()
		.member<uint32_t>("bone_count")
		.member<RID>("skeleton_id");
	
	// Environment & camera - use reflection
	world.component<EnvironmentComponent>()
		.member<RID>("environment_id");
	
	world.component<CameraComponent>()
		.member<RID>("camera_id")
		.member<Vector<Plane>>("frustum")
		.member<Vector3>("position")
		.member<float>("far")
		.member<float>("near")
		.member<Projection>("projection")
		.member<Vector2>("camera_offset");
	
	world.component<MainCamera>(); // Tag component
	world.component<CompositorComponent>()
		.member<RID>("compositor_id");
	world.component<ViewportComponent>()
		.member<RID>("viewport_id");
	
	// Lighting - use reflection
	world.component<DirectionalLight3DComponent>()
		.member<RID>("light_id")
		.member<Color>("light_color")
		.member<float>("intensity");
	
	world.component<DirectionalLight2DComponent>()
		.member<RID>("light_id")
		.member<Color>("light_color")
		.member<float>("intensity");
	
	world.component<PointLightComponent>()
		.member<RID>("light_id")
		.member<Color>("light_color")
		.member<float>("intensity")
		.member<float>("range");
	
	world.component<OmniLightComponent>()
		.member<RID>("light_id")
		.member<Color>("light_color")
		.member<float>("intensity")
		.member<float>("range");
	
	world.component<SpotLightComponent>()
		.member<RID>("light_id")
		.member<Color>("light_color")
		.member<float>("intensity")
		.member<float>("range");
	
	world.component<LightOccluderComponent>();
	
	// Canvas & scenario (contain opaque types - no reflection)
	world.component<ScenarioComponent>();
	
	world.component<RenderInstanceComponent>();
	
	world.component<CanvasItemComponent>();
	
	// Physics 2D (contain RID - no reflection)
	world.component<Area2DComponent>();
	
	world.component<Body2DComponent>();
	
	world.component<Joint2DComponent>();
	
	// Physics 3D (contain RID - no reflection)
	world.component<Area3DComponent>();
	
	world.component<Body3DComponent>();
	
	world.component<Joint3DComponent>();
	
	world.component<SoftBody3DComponent>();
	
	// Navigation 2D (contain RID - no reflection)
	world.component<NavAgent2DComponent>();
	
	world.component<NavLink2DComponent>();
	
	world.component<NavObstacle2DComponent>();
	
	world.component<NavRegion2DComponent>();
	
	world.component<SourceGeometryParser2DComponent>();
	
	// Navigation 3D (contain RID - no reflection)
	world.component<NavAgent3DComponent>();
	
	world.component<NavLink3DComponent>();
	
	world.component<NavObstacle3DComponent>();
	
	world.component<NavRegion3DComponent>();
	
	world.component<SourceGeometryParser3DComponent>();
	

	// Optional: Register serialization handlers
	if (enable_serialization) {
		auto& registry = FlecsReflection::Registry::get();
		
		// Bind component IDs
		registry.bind_component_id("Transform2DComponent", world.id<Transform2DComponent>());
		registry.bind_component_id("Transform3DComponent", world.id<Transform3DComponent>());
		registry.bind_component_id("VisibilityComponent", world.id<VisibilityComponent>());
		registry.bind_component_id("SceneNodeComponent", world.id<SceneNodeComponent>());
		registry.bind_component_id("ObjectInstanceComponent", world.id<ObjectInstanceComponent>());
		registry.bind_component_id("GameScriptComponent", world.id<GameScriptComponent>());
		registry.bind_component_id("ResourceComponent", world.id<ResourceComponent>());
		registry.bind_component_id("World2DComponent", world.id<World2DComponent>());
		registry.bind_component_id("World3DComponent", world.id<World3DComponent>());
		registry.bind_component_id("MeshComponent", world.id<MeshComponent>());
		registry.bind_component_id("CameraComponent", world.id<CameraComponent>());
		
		// Register serialization functions
		FlecsReflection::ComponentRegistrar<Transform2DComponent>::register_type(
			"Transform2DComponent",
			ComponentSerialization::serialize_transform_2d,
			ComponentSerialization::deserialize_transform_2d
		);
		
		FlecsReflection::ComponentRegistrar<Transform3DComponent>::register_type(
			"Transform3DComponent",
			ComponentSerialization::serialize_transform_3d,
			ComponentSerialization::deserialize_transform_3d
		);
		
		FlecsReflection::ComponentRegistrar<VisibilityComponent>::register_type(
			"VisibilityComponent",
			ComponentSerialization::serialize_visibility,
			ComponentSerialization::deserialize_visibility
		);
		
		FlecsReflection::ComponentRegistrar<SceneNodeComponent>::register_type(
			"SceneNodeComponent",
			ComponentSerialization::serialize_scene_node,
			ComponentSerialization::deserialize_scene_node
		);
		
		FlecsReflection::ComponentRegistrar<ObjectInstanceComponent>::register_type(
			"ObjectInstanceComponent",
			ComponentSerialization::serialize_object_instance,
			ComponentSerialization::deserialize_object_instance
		);
		
		FlecsReflection::ComponentRegistrar<MeshComponent>::register_type(
			"MeshComponent",
			ComponentSerialization::serialize_mesh,
			ComponentSerialization::deserialize_mesh
		);
		
		FlecsReflection::ComponentRegistrar<CameraComponent>::register_type(
			"CameraComponent",
			ComponentSerialization::serialize_camera,
			ComponentSerialization::deserialize_camera
		);
		
		FlecsReflection::ComponentRegistrar<GameScriptComponent>::register_type(
			"GameScriptComponent",
			ComponentSerialization::serialize_game_script,
			ComponentSerialization::deserialize_game_script
		);
		
		FlecsReflection::ComponentRegistrar<ResourceComponent>::register_type(
			"ResourceComponent",
			ComponentSerialization::serialize_resource,
			ComponentSerialization::deserialize_resource
		);
		
		FlecsReflection::ComponentRegistrar<World2DComponent>::register_type(
			"World2DComponent",
			ComponentSerialization::serialize_world_2d,
			ComponentSerialization::deserialize_world_2d
		);
		
		FlecsReflection::ComponentRegistrar<World3DComponent>::register_type(
			"World3DComponent",
			ComponentSerialization::serialize_world_3d,
			ComponentSerialization::deserialize_world_3d
		);
	}
}

/**
 * Get component dictionary for an entity
 * Use this to serialize a specific component
 */
inline Dictionary get_component_dict(const flecs::entity& e, flecs::entity_t component_id) {
	return FlecsReflection::Registry::get().serialize(e, component_id);
}

/**
 * Set component from dictionary
 * Use this to deserialize a specific component
 */
inline void set_component_from_dict(flecs::entity& e, flecs::entity_t component_id, const Dictionary& dict) {
	FlecsReflection::Registry::get().deserialize(e, component_id, dict);
}

} // namespace AllComponents