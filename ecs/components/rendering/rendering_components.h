#pragma once
#include "../../../../../core/templates/rid.h"
#include "../../../../../servers/rendering_server.h"
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../flecs_types/flecs_entity.h"
#include "../component_module_base.h"
#include "../../../../../core/os/memory.h"
#include "../../../../../core/math/transform_3d.h"
#include "../../../../../core/math/vector2.h"
#include "../../../../../core/math/projection.h"

#include "../../../../../core/object/object.h"

#include "../../../../../core/variant/typed_array.h"
#include "../../../../../core/string/ustring.h"
#include "../../../../../core/templates/vector.h"
#include "../../flecs_types/flecs_component.h"
#include "../component_proxy.h"

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_ids;
	MeshComponent() = default;
	MeshComponent(const RID& id, const Vector<RID>& material_ids) : mesh_id(id) , material_ids(material_ids) {}
	~MeshComponent() {
		for (const RID &mat_id : material_ids) {
			if (mat_id.is_valid()) {
				RenderingServer::get_singleton()->free(mat_id);
			}
		}
		if (mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(mesh_id);
		}
	}
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
			MESH_COMPONENT_BINDINGS)
};


struct MultiMeshComponent {
	RID multi_mesh_id;
	uint32_t instance_count = 0U;
	~MultiMeshComponent() {
		// Ensure that the RID is released when the component is destroyed
		if (multi_mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(multi_mesh_id);
		}
	}
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
	~ParticlesComponent() {
		if (particles_id.is_valid()) {
			RenderingServer::get_singleton()->free(particles_id);
		}
	}
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
	~ReflectionProbeComponent() {
		if (probe_id.is_valid()) {
			RenderingServer::get_singleton()->free(probe_id);
		}
	}
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
	~SkeletonComponent() {
		if (skeleton_id.is_valid()) {
			RenderingServer::get_singleton()->free(skeleton_id);
		}
	}
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
	~EnvironmentComponent() {
		if (environment_id.is_valid()) {
			RenderingServer::get_singleton()->free(environment_id);
		}
	}
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
	~CameraComponent() {
		if (camera_id.is_valid()) {
			RenderingServer::get_singleton()->free(camera_id);
		}
	}
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
	~CompositorComponent() {
		if (compositor_id.is_valid()) {
			RenderingServer::get_singleton()->free(compositor_id);
		}
	}
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
	~DirectionalLight3DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
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
	~DirectionalLight2DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
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
	~PointLightComponent() {
		if (point_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(point_light_id);
		}
	}
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
	~LightOccluderComponent() {
		if (light_occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(light_occluder_id);
		}
	}
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
	~OmniLightComponent() {
		if (omni_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(omni_light_id);
		}
	}
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
	~SpotLightComponent() {
		if (spot_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(spot_light_id);
		}
	}
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
	~VoxelGIComponent() {
		if (voxel_gi_id.is_valid()) {
			RenderingServer::get_singleton()->free(voxel_gi_id);
		}
	}
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
	~RenderInstanceComponent() {
		if (render_instance_id.is_valid()) {
			RenderingServer::get_singleton()->free(render_instance_id);
		}
	}
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
	~CanvasItemComponent() {
		if (canvas_item_id.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_clear(canvas_item_id);
			RenderingServer::get_singleton()->free(canvas_item_id);
		}
	}
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





struct ScreenTriangle;

struct FrustumCulled { /* tag component */ };
struct Occluded { /* tag component */ };
struct Occluder {
	RID occluder_id;
	Vector<ScreenTriangle> screen_triangles;
	PackedVector3Array vertices;
	PackedInt32Array indices;
};
struct Occludee {
	AABB worldAABB;
	AABB aabb;
	Occludee() = default;
	~Occludee() = default;
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

	explicit RenderingBaseComponents(const flecs::world &world) :
			mesh(world.component<MeshComponent>("MeshComponent")),
			multi_mesh(world.component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world.component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world.component<ParticlesComponent>("ParticlesComponent")),
			probe(world.component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world.component<SkeletonComponent>("SkeletonComponent")),
			environment(world.component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world.component<CameraComponent>("CameraComponent")),
			compositor(world.component<CompositorComponent>("CompositorComponent")),
			directional_light(world.component<DirectionalLight3DComponent>("DirectionalLightComponent")),
			directional_light_2d(world.component<DirectionalLight2DComponent>("DirectionalLight2DComponent")),
			point_light(world.component<PointLightComponent>("PointLightComponent")),
			omni_light(world.component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world.component<SpotLightComponent>("SpotLightComponent")),
			viewport(world.component<ViewportComponent>("ViewportComponent")),
			scenario(world.component<ScenarioComponent>("ScenarioComponent")),
			voxel_gi(world.component<VoxelGIComponent>("VoxelGIComponent")),
			instance(world.component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")),
			occluder(world.component<Occluder>("Occluder")),
			occludee(world.component<Occludee>("Occludee")),
			frustum_culled(world.component<FrustumCulled>("FrustumCulled")),
			occluded(world.component<Occluded>("Occluded"))
			{}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
