#pragma once

#include "ecs/systems/rendering/occlusion/tile.h"
#include "ecs/components/component_module_base.h"
#include "core/templates/rid.h"
#include "servers/rendering_server.h"
#include "core/os/memory.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/projection.h"

#include "core/object/object.h"

#include "core/variant/typed_array.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/flecs_types/flecs_tag.h"
#include "ecs/flecs_types/flecs_entity.h"
#include "../component_proxy.h"

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_ids;
	MeshComponent() = default;
	~MeshComponent() = default;
	MeshComponent(const RID& id, const Vector<RID>& material_ids) : mesh_id(id) , material_ids(material_ids) {}
};



class MeshComponentRef : public FlecsComponent<MeshComponent> {
	#define MESH_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, mesh_id, MeshComponent)\
	DEFINE_ARRAY_PROPERTY(RID, material_ids, MeshComponent)\


	#define MESH_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, mesh_id, MeshComponentRef)\
	BIND_ARRAY_PROPERTY(RID, material_ids, MeshComponentRef)\


	DEFINE_COMPONENT_PROXY(MeshComponent,
			MESH_COMPONENT_PROPERTIES,
			MESH_COMPONENT_BINDINGS);
};


struct MultiMeshComponent {
	RID multi_mesh_id;
	uint32_t instance_count = 0U;
};



class MultiMeshComponentRef : public FlecsComponent<MultiMeshComponent> {
	#define MULTI_MESH_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, multi_mesh_id, MultiMeshComponent)\
	DEFINE_PROPERTY(uint32_t, instance_count,MultiMeshComponent)\


	#define MULTI_MESH_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, multi_mesh_id, MultiMeshComponentRef)\
	BIND_PROPERTY(uint32_t, instance_count, MultiMeshComponentRef)\


	DEFINE_COMPONENT_PROXY(MultiMeshComponent,
	MULTI_MESH_COMPONENT_PROPERTIES,
	MULTI_MESH_COMPONENT_BINDINGS);
};

struct MultiMeshInstanceComponent {
	uint32_t index;
};
class MultiMeshInstanceComponentRef : public FlecsComponent<MultiMeshInstanceComponent> {
	#define MULTI_MESH_INSTANCE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(uint32_t, index,MultiMeshInstanceComponent )\


	#define MULTI_MESH_INSTANCE_COMPONENT_BINDINGS\
	BIND_PROPERTY(uint32_t, index, MultiMeshInstanceComponentRef)\


	DEFINE_COMPONENT_PROXY(MultiMeshInstanceComponent,
	MULTI_MESH_INSTANCE_COMPONENT_PROPERTIES,
	MULTI_MESH_INSTANCE_COMPONENT_BINDINGS);
};


struct ParticlesComponent {
	RID particles_id;
};

class ParticlesComponentRef : public FlecsComponent<ParticlesComponent> {
	#define PARTICLES_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, particles_id, ParticlesComponent)\

	#define PARTICLES_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, particles_id, ParticlesComponentRef)\

	DEFINE_COMPONENT_PROXY(ParticlesComponent,
	PARTICLES_COMPONENT_PROPERTIES,
	PARTICLES_COMPONENT_BINDINGS);
};


struct ReflectionProbeComponent {
	RID probe_id;
};

class ReflectionProbeComponentRef : public FlecsComponent<ReflectionProbeComponent> {
	#define REFLECTION_PROBE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, probe_id, ReflectionProbeComponent)\


	#define REFLECTION_PROBE_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, probe_id, ReflectionProbeComponentRef)\


	DEFINE_COMPONENT_PROXY(ReflectionProbeComponent,
	REFLECTION_PROBE_COMPONENT_PROPERTIES,
	REFLECTION_PROBE_COMPONENT_BINDINGS);
};

struct SkeletonComponent {
	RID skeleton_id;
};

class SkeletonComponentRef : public FlecsComponent<SkeletonComponent> {
	#define SKELETON_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, skeleton_id,SkeletonComponent)\


	#define SKELETON_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, skeleton_id, SkeletonComponentRef)\

	DEFINE_COMPONENT_PROXY(SkeletonComponent,
	SKELETON_COMPONENT_PROPERTIES,
	SKELETON_COMPONENT_BINDINGS);
};

struct EnvironmentComponent {
	RID environment_id;
};

class EnvironmentComponentRef : public FlecsComponent<EnvironmentComponent> {
	#define ENVIRONMENT_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, environment_id,EnvironmentComponent)\


	#define ENVIRONMENT_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, environment_id, EnvironmentComponentRef)\


	DEFINE_COMPONENT_PROXY(EnvironmentComponent,
	ENVIRONMENT_COMPONENT_PROPERTIES,
	ENVIRONMENT_COMPONENT_BINDINGS);
};

struct CameraComponent {
	RID camera_id;
	Vector<Plane> frustum;
	Vector3 position;
	float far;
	float near;
	Projection projection;
	Vector2 camera_offset;
	CameraComponent() = default;
	~CameraComponent() = default;
};

class CameraComponentRef : public FlecsComponent<CameraComponent> {

	#define CAMERA_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, camera_id,CameraComponent)\
	DEFINE_ARRAY_PROPERTY(Plane,frustum,CameraComponent)\
	DEFINE_PROPERTY(Vector3,position,CameraComponent)\
	DEFINE_PROPERTY(float,far,CameraComponent)\
	DEFINE_PROPERTY(float,near,CameraComponent)\
	DEFINE_PROPERTY(Projection,projection,CameraComponent)\
	DEFINE_PROPERTY(Vector2,camera_offset,CameraComponent)\


	#define CAMERA_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, camera_id, CameraComponentRef)\
	BIND_ARRAY_PROPERTY(Plane,frustum,CameraComponentRef)\
	BIND_PROPERTY(Vector3,position,CameraComponentRef)\
	BIND_PROPERTY(float,far,CameraComponentRef)\
	BIND_PROPERTY(float,near,CameraComponentRef)\
	BIND_PROPERTY(Projection,projection,CameraComponentRef)\
	BIND_PROPERTY(Vector2,camera_offset,CameraComponentRef)\


	DEFINE_COMPONENT_PROXY(CameraComponent,
	CAMERA_COMPONENT_PROPERTIES,
	CAMERA_COMPONENT_BINDINGS);
};

struct CompositorComponent {
	RID compositor_id;
};

class CompositorComponentRef : public FlecsComponent<CompositorComponent> {
	#define COMPOSITOR_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, compositor_id,CompositorComponent)\


	#define COMPOSITOR_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, compositor_id, CompositorComponentRef)\

	DEFINE_COMPONENT_PROXY(CompositorComponent,
	COMPOSITOR_COMPONENT_PROPERTIES,
	COMPOSITOR_COMPONENT_BINDINGS);
};


struct DirectionalLight3DComponent {
	RID directional_light_id;
};

class DirectionalLight3DComponentRef : public FlecsComponent<DirectionalLight3DComponent> {
	#define DIRECTIONAL_LIGHT_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, directional_light_id,DirectionalLight3DComponent)\


	#define DIRECTIONAL_LIGHT_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, directional_light_id, DirectionalLight3DComponentRef)\

	DEFINE_COMPONENT_PROXY(DirectionalLight3DComponent,
	DIRECTIONAL_LIGHT_3D_COMPONENT_PROPERTIES,
	DIRECTIONAL_LIGHT_3D_COMPONENT_BINDINGS);
};

struct DirectionalLight2DComponent {
	RID directional_light_id;
};

class DirectionalLight2DComponentRef : public FlecsComponent<DirectionalLight2DComponent> {
	#define DIRECTIONAL_LIGHT_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, directional_light_id,DirectionalLight3DComponent)\


	#define DIRECTIONAL_LIGHT_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, directional_light_id, DirectionalLight2DComponentRef)\

	DEFINE_COMPONENT_PROXY(DirectionalLight2DComponent,
	DIRECTIONAL_LIGHT_2D_COMPONENT_PROPERTIES,
	DIRECTIONAL_LIGHT_2D_COMPONENT_BINDINGS);
};

struct PointLightComponent {
	RID point_light_id;
};

class PointLightComponentRef : public FlecsComponent<PointLightComponent> {
	#define POINT_LIGHT_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, point_light_id,PointLightComponent)\


	#define POINT_LIGHT_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, point_light_id, PointLightComponentRef)\

	DEFINE_COMPONENT_PROXY(PointLightComponent,
	POINT_LIGHT_COMPONENT_PROPERTIES,
	POINT_LIGHT_COMPONENT_BINDINGS);
};

struct LightOccluderComponent {
	RID light_occluder_id;
};

class LightOccluderComponentRef : public FlecsComponent<LightOccluderComponent> {
	#define LIGHT_OCCLUDER_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, light_occluder_id,LightOccluderComponent)\


	#define LIGHT_OCCLUDER_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, light_occluder_id, LightOccluderComponentRef)\

	DEFINE_COMPONENT_PROXY(LightOccluderComponent,
	LIGHT_OCCLUDER_COMPONENT_PROPERTIES,
	LIGHT_OCCLUDER_COMPONENT_BINDINGS);
};

struct OmniLightComponent {
	RID omni_light_id;
};

class OmniLightComponentRef : public FlecsComponent<OmniLightComponent> {
	#define OMNI_LIGHT_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, omni_light_id,OmniLightComponent)\


	#define OMNI_LIGHT_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, omni_light_id, OmniLightComponentRef)\

	DEFINE_COMPONENT_PROXY(OmniLightComponent,
	OMNI_LIGHT_COMPONENT_PROPERTIES,
	OMNI_LIGHT_COMPONENT_BINDINGS);
};


struct SpotLightComponent {
	RID spot_light_id;
};

class SpotLightComponentRef : public FlecsComponent<SpotLightComponent> {
	#define SPOT_LIGHT_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, spot_light_id,SpotLightComponent)\


	#define SPOT_LIGHT_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, spot_light_id, SpotLightComponentRef)\

	DEFINE_COMPONENT_PROXY(SpotLightComponent,
	SPOT_LIGHT_COMPONENT_PROPERTIES,
	SPOT_LIGHT_COMPONENT_BINDINGS);
};

struct ViewportComponent {
	RID viewport_id;
};

class ViewportComponentRef : public FlecsComponent<ViewportComponent> {
	#define VIEWPORT_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, viewport_id,ViewportComponent)\


	#define VIEWPORT_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, viewport_id, ViewportComponentRef)\

	DEFINE_COMPONENT_PROXY(ViewportComponent,
	VIEWPORT_COMPONENT_PROPERTIES,
	VIEWPORT_COMPONENT_BINDINGS);
};

struct VoxelGIComponent {
	RID voxel_gi_id;
};

class VoxelGIComponentRef : public FlecsComponent<VoxelGIComponent> {
	#define VOXEL_GI_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, voxel_gi_id,VoxelGIComponent)\


	#define VOXEL_GI_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, voxel_gi_id, VoxelGIComponentRef)\

	DEFINE_COMPONENT_PROXY(VoxelGIComponent,
	VOXEL_GI_COMPONENT_PROPERTIES,
	VOXEL_GI_COMPONENT_BINDINGS);
};

struct ScenarioComponent {
	RID id;
};

class ScenarioComponentRef : public FlecsComponent<ScenarioComponent> {
	#define SCENARIO_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, id,ScenarioComponent)\


	#define SCENARIO_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, id, ScenarioComponentRef)\

	DEFINE_COMPONENT_PROXY(ScenarioComponent,
	SCENARIO_COMPONENT_PROPERTIES,
	SCENARIO_COMPONENT_BINDINGS);
};

struct RenderInstanceComponent {
	RID render_instance_id;
};

class RenderInstanceComponentRef : public FlecsComponent<RenderInstanceComponent> {
	#define RENDER_INSTANCE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, render_instance_id,RenderInstanceComponent)\


	#define RENDER_INSTANCE_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, render_instance_id, RenderInstanceComponentRef)\

	DEFINE_COMPONENT_PROXY(RenderInstanceComponent,
	RENDER_INSTANCE_COMPONENT_PROPERTIES,
	RENDER_INSTANCE_COMPONENT_BINDINGS);
};

struct CanvasItemComponent {
	RID canvas_item_id;
	StringName class_name;
};

class CanvasItemComponentRef : public FlecsComponent<CanvasItemComponent> {
	#define CANVAS_ITEM_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, canvas_item_id,CanvasItemComponent)\
	DEFINE_PROPERTY(StringName, class_name,CanvasItemComponent)\


	#define CANVAS_ITEM_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, canvas_item_id, CanvasItemComponentRef)\
	BIND_PROPERTY(StringName, class_name, CanvasItemComponentRef)\

	DEFINE_COMPONENT_PROXY(CanvasItemComponent,
	CANVAS_ITEM_COMPONENT_PROPERTIES,
	CANVAS_ITEM_COMPONENT_BINDINGS);
};

struct FrustumCulled { /* tag component */ };

struct FrustumCulledRef : public FlecsTag<FrustumCulled> {
	DEFINE_TAG_PROXY(FrustumCulled);
};

struct Occluded { /* tag component */ };

class OccludedRef : public FlecsTag<Occluded> {
	DEFINE_TAG_PROXY(Occluded);
};

struct MainCamera { /* tag component */ };

class MainCameraRef : public FlecsTag<MainCamera> {
	DEFINE_TAG_PROXY(MainCamera);
};


struct Occluder {
	RID occluder_id;
	Vector<Ref<ScreenTriangle>> screen_triangles;
	PackedVector3Array vertices;
	PackedInt32Array indices;
};

class OccluderRef : public FlecsComponent<Occluder> {
	#define OCCLUDER_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, occluder_id, Occluder)\
	DEFINE_ARRAY_PROPERTY(ScreenTriangle, screen_triangles, Occluder)\
	DEFINE_ARRAY_PROPERTY(Vector3, vertices, Occluder)\
	DEFINE_ARRAY_PROPERTY(int32_t, indices, Occluder)\

	#define OCCLUDER_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, occluder_id, OccluderRef)\
	BIND_ARRAY_PROPERTY(ScreenTriangle, screen_triangles, OccluderRef)\
	BIND_ARRAY_PROPERTY(Vector3, vertices, OccluderRef)\
	BIND_ARRAY_PROPERTY(int32_t, indices, OccluderRef)\

	DEFINE_COMPONENT_PROXY(Occluder,
	OCCLUDER_COMPONENT_PROPERTIES,
	OCCLUDER_COMPONENT_BINDINGS);
};

struct Occludee {
	AABB worldAABB;
	AABB aabb;
	Occludee() = default;
	~Occludee() = default;
};

class OccludeeRef : public FlecsComponent<Occludee> {
	#define OCCLUDEE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(AABB, worldAABB, Occludee)\
	DEFINE_PROPERTY(AABB, aabb, Occludee)\

	#define OCCLUDEE_COMPONENT_BINDINGS\
	BIND_PROPERTY(AABB, worldAABB, OccludeeRef)\
	BIND_PROPERTY(AABB, aabb, OccludeeRef)\

	DEFINE_COMPONENT_PROXY(Occludee,
	OCCLUDEE_COMPONENT_PROPERTIES,
	OCCLUDEE_COMPONENT_BINDINGS);
};

struct RenderingBaseComponents{
	flecs::component<MeshComponent> mesh;
	flecs::component<MultiMeshComponent> multi_mesh;
	flecs::component<MultiMeshInstanceComponent> mesh_instance;
	flecs::component<ParticlesComponent> particles;
	flecs::component<ReflectionProbeComponent> probe;
	flecs::component<SkeletonComponent> skeleton;
	flecs::component<EnvironmentComponent> environment;
	flecs::component<CameraComponent> camera;
	flecs::component<MainCamera> main_camera;
	flecs::component<CompositorComponent> compositor;
	flecs::component<DirectionalLight3DComponent> directional_light;
	flecs::component<DirectionalLight2DComponent> directional_light_2d;
	flecs::component<PointLightComponent> point_light;
	flecs::component<OmniLightComponent> omni_light;
	flecs::component<SpotLightComponent> spot_light;
	flecs::component<ViewportComponent> viewport;
	flecs::component<ScenarioComponent> scenario;
	flecs::component<VoxelGIComponent> voxel_gi;
	flecs::component<RenderInstanceComponent> instance;
	flecs::component<CanvasItemComponent> canvas_item;
	flecs::component<Occluder> occluder;
	flecs::component<Occludee> occludee;
	flecs::component<FrustumCulled> frustum_culled;
	flecs::component<Occluded> occluded;

	explicit RenderingBaseComponents(const flecs::world *world) :
			mesh(world->component<MeshComponent>("MeshComponent")),
			multi_mesh(world->component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world->component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world->component<ParticlesComponent>("ParticlesComponent")),
			probe(world->component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world->component<SkeletonComponent>("SkeletonComponent")),
			environment(world->component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world->component<CameraComponent>("CameraComponent")),
			main_camera(world->component<MainCamera>("MainCamera")),
			compositor(world->component<CompositorComponent>("CompositorComponent")),
			directional_light(world->component<DirectionalLight3DComponent>("DirectionalLightComponent")),
			directional_light_2d(world->component<DirectionalLight2DComponent>("DirectionalLight2DComponent")),
			point_light(world->component<PointLightComponent>("PointLightComponent")),
			omni_light(world->component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world->component<SpotLightComponent>("SpotLightComponent")),
			viewport(world->component<ViewportComponent>("ViewportComponent")),
			scenario(world->component<ScenarioComponent>("ScenarioComponent")),
			voxel_gi(world->component<VoxelGIComponent>("VoxelGIComponent")),
			instance(world->component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas_item(world->component<CanvasItemComponent>("CanvasItemComponent")),
			occluder(world->component<Occluder>("Occluder")),
			occludee(world->component<Occludee>("Occludee")),
			frustum_culled(world->component<FrustumCulled>("FrustumCulled")),
			occluded(world->component<Occluded>("Occluded"))
			{}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
