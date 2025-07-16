#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../component_module_base.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "../../../../core/templates/vector.h"
#include "modules/godot_turbo/ecs/components/component_proxy.h"
#include <servers/rendering_server.h>

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_ids;
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

#define MESH_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, mesh_id)\
DEFINE_PROPERTY_ARRAY(RID, material_ids)\


#define MESH_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, mesh_id, MeshComponentRef)\
BIND_VECTOR_PROPERTY(RID, material_ids, MeshComponentRef)\

DEFINE_COMPONENT_PROXY(MeshComponentRef, MeshComponent,
MESH_COMPONENT_PROPERTIES,
MESH_COMPONENT_BINDINGS);

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

#define MULTI_MESH_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, multi_mesh_id)\
DEFINE_PROPERTY(uint32_t, instance_count)\


#define MULTI_MESH_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, multi_mesh_id, MultiMeshComponentRef)\
BIND_PROPERTY(uint32_t, instance_count, MultiMeshComponentRef)\


DEFINE_COMPONENT_PROXY(MultiMeshComponentRef, MultiMeshComponent,
MULTI_MESH_COMPONENT_PROPERTIES,
MULTI_MESH_COMPONENT_BINDINGS);


struct MultiMeshInstanceComponent {
	uint32_t index;
};

#define MULTI_MESH_INSTANCE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(uint32_t, index)\


#define MULTI_MESH_INSTANCE_COMPONENT_BINDINGS\
BIND_PROPERTY(uint32_t, index, MultiMeshInstanceComponentRef)\


DEFINE_COMPONENT_PROXY(MultiMeshInstanceComponentRef, MultiMeshInstanceComponent,
MULTI_MESH_INSTANCE_COMPONENT_PROPERTIES,
MULTI_MESH_INSTANCE_COMPONENT_BINDINGS);

struct ParticlesComponent {
	RID particles_id;
	~ParticlesComponent() {
		if (particles_id.is_valid()) {
			RenderingServer::get_singleton()->free(particles_id);
		}
	}
};

#define PARTICLES_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, particles_id)\


#define PARTICLES_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, particles_id, ParticlesComponentRef)\

#define PARTICLES_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, particles_id))\

#define PARTICLES_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(particles_id)\

DEFINE_COMPONENT_PROXY(ParticlesComponentRef, ParticlesComponent,
PARTICLES_COMPONENT_PROPERTIES,
PARTICLES_COMPONENT_BINDINGS);

struct ReflectionProbeComponent {
	RID probe_id;
	~ReflectionProbeComponent() {
		if (probe_id.is_valid()) {
			RenderingServer::get_singleton()->free(probe_id);
		}
	}
};

#define REFLECTION_PROBE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, probe_id)\


#define REFLECTION_PROBE_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, probe_id, ReflectionProbeComponentRef)\

#define REFLECTION_PROBE_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, probe_id))\

#define REFLECTION_PROBE_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(probe_id)\

DEFINE_COMPONENT_PROXY(ReflectionProbeComponentRef, ReflectionProbeComponent,
REFLECTION_PROBE_COMPONENT_PROPERTIES,
REFLECTION_PROBE_COMPONENT_BINDINGS);

struct SkeletonComponent {
	RID skeleton_id;
	~SkeletonComponent() {
		if (skeleton_id.is_valid()) {
			RenderingServer::get_singleton()->free(skeleton_id);
		}
	}
};

#define SKELETON_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, skeleton_id)\


#define SKELETON_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, skeleton_id, SkeletonComponentRef)\

#define SKELETON_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, skeleton_id))\

#define SKELETON_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(skeleton_id)\

DEFINE_COMPONENT_PROXY(SkeletonComponentRef, SkeletonComponent,
SKELETON_COMPONENT_PROPERTIES,
SKELETON_COMPONENT_BINDINGS);

struct EnvironmentComponent {
	RID environment_id;
	~EnvironmentComponent() {
		if (environment_id.is_valid()) {
			RenderingServer::get_singleton()->free(environment_id);
		}
	}
};

#define ENVIRONMENT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, environment_id)\


#define ENVIRONMENT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, environment_id, EnvironmentComponentRef)\

#define ENVIRONMENT_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, environment_id))\

#define ENVIRONMENT_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(environment_id)\

DEFINE_COMPONENT_PROXY(EnvironmentComponentRef, EnvironmentComponent,
ENVIRONMENT_COMPONENT_PROPERTIES,
ENVIRONMENT_COMPONENT_BINDINGS);

struct CameraComponent {
	RID camera_id;
	~CameraComponent() {
		if (camera_id.is_valid()) {
			RenderingServer::get_singleton()->free(camera_id);
		}
	}
};

#define CAMERA_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, camera_id)\


#define CAMERA_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, camera_id, CameraComponentRef)\

#define CAMERA_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, camera_id))\

#define CAMERA_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(camera_id)\

DEFINE_COMPONENT_PROXY(CameraComponentRef, CameraComponent,
CAMERA_COMPONENT_PROPERTIES,
CAMERA_COMPONENT_BINDINGS);

struct CompositorComponent {
	RID compositor_id;
	~CompositorComponent() {
		if (compositor_id.is_valid()) {
			RenderingServer::get_singleton()->free(compositor_id);
		}
	}
};

#define COMPOSITOR_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, compositor_id)\


#define COMPOSITOR_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, compositor_id, CompositorComponentRef)\

DEFINE_COMPONENT_PROXY(CompositorComponentRef, CompositorComponent,
COMPOSITOR_COMPONENT_PROPERTIES,
COMPOSITOR_COMPONENT_BINDINGS);

struct DirectionalLight3DComponent {
	RID directional_light_id;
	~DirectionalLight3DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

#define DIRECTIONAL_LIGHT_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, directional_light_id)\


#define DIRECTIONAL_LIGHT_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, directional_light_id, DirectionalLight3DComponentRef)\

DEFINE_COMPONENT_PROXY(DirectionalLight3DComponentRef, DirectionalLight3DComponent,
DIRECTIONAL_LIGHT_3D_COMPONENT_PROPERTIES,
DIRECTIONAL_LIGHT_3D_COMPONENT_BINDINGS);

struct DirectionalLight2DComponent {
	RID directional_light_id;
	~DirectionalLight2DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

#define DIRECTIONAL_LIGHT_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, directional_light_id)\


#define DIRECTIONAL_LIGHT_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, directional_light_id, DirectionalLight2DComponentRef)\

DEFINE_COMPONENT_PROXY(DirectionalLight2DComponentRef, DirectionalLight3DComponent,
DIRECTIONAL_LIGHT_2D_COMPONENT_PROPERTIES,
DIRECTIONAL_LIGHT_2D_COMPONENT_BINDINGS);

struct PointLightComponent {
	RID point_light_id;
	~PointLightComponent() {
		if (point_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(point_light_id);
		}
	}
};

#define POINT_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, point_light_id)\


#define POINT_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, point_light_id, PointLightComponentRef)\

DEFINE_COMPONENT_PROXY(PointLightComponentRef, PointLightComponent,
POINT_LIGHT_COMPONENT_PROPERTIES,
POINT_LIGHT_COMPONENT_BINDINGS);

struct LightOccluderComponent {
	RID light_occluder_id;
	~LightOccluderComponent() {
		if (light_occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(light_occluder_id);
		}
	}
};

#define LIGHT_OCCLUDER_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, light_occluder_id)\


#define LIGHT_OCCLUDER_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, light_occluder_id, LightOccluderComponentRef)\

DEFINE_COMPONENT_PROXY(LightOccluderComponentRef, LightOccluderComponent,
LIGHT_OCCLUDER_COMPONENT_PROPERTIES,
LIGHT_OCCLUDER_COMPONENT_BINDINGS);


struct OmniLightComponent {
	RID omni_light_id;
	~OmniLightComponent() {
		if (omni_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(omni_light_id);
		}
	}
};

#define OMNI_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, omni_light_id)\


#define OMNI_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, omni_light_id, OmniLightComponentRef)\

DEFINE_COMPONENT_PROXY(OmniLightComponentRef, OmniLightComponent,
OMNI_LIGHT_COMPONENT_PROPERTIES,
OMNI_LIGHT_COMPONENT_BINDINGS);

struct SpotLightComponent {
	RID spot_light_id;
	~SpotLightComponent() {
		if (spot_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(spot_light_id);
		}
	}
};

#define SPOT_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, spot_light_id)\


#define SPOT_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, spot_light_id, SpotLightComponentRef)\

DEFINE_COMPONENT_PROXY(SpotLightComponentRef, SpotLightComponent,
SPOT_LIGHT_COMPONENT_PROPERTIES,
SPOT_LIGHT_COMPONENT_BINDINGS);

struct ViewportComponent {
	RID viewport_id;
};

#define VIEWPORT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, viewport_id)\


#define VIEWPORT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, viewport_id, ViewportComponentRef)\

DEFINE_COMPONENT_PROXY(ViewportComponentRef, ViewportComponent,
VIEWPORT_COMPONENT_PROPERTIES,
VIEWPORT_COMPONENT_BINDINGS);

struct VoxelGIComponent {
	RID voxel_gi_id;
	~VoxelGIComponent() {
		if (voxel_gi_id.is_valid()) {
			RenderingServer::get_singleton()->free(voxel_gi_id);
		}
	}
};

#define VOXEL_GI_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, voxel_gi_id)\


#define VOXEL_GI_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, voxel_gi_id, VoxelGIComponentRef)\

DEFINE_COMPONENT_PROXY(VoxelGIComponentRef, VoxelGIComponent,
VOXEL_GI_COMPONENT_PROPERTIES,
VOXEL_GI_COMPONENT_BINDINGS);

struct ScenarioComponent {
	RID id;
};

#define SCENARIO_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, id)\


#define SCENARIO_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, id, ScenarioComponentRef)\

DEFINE_COMPONENT_PROXY(ScenarioComponentRef, ScenarioComponent,
SCENARIO_COMPONENT_PROPERTIES,
SCENARIO_COMPONENT_BINDINGS);

struct RenderInstanceComponent {
	RID instance_id;
	~RenderInstanceComponent() {
		if (instance_id.is_valid()) {
			RenderingServer::get_singleton()->free(instance_id);
		}
	}
};

#define RENDER_INSTANCE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, instance_id)\


#define RENDER_INSTANCE_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, instance_id, RenderInstanceComponentRef)\

DEFINE_COMPONENT_PROXY(RenderInstanceComponentRef, RenderInstanceComponent,
RENDER_INSTANCE_COMPONENT_PROPERTIES,
RENDER_INSTANCE_COMPONENT_BINDINGS);

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

#define CANVAS_ITEM_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, canvas_item_id)\
DEFINE_PROPERTY(StringName, class_name)\


#define CANVAS_ITEM_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, canvas_item_id, CanvasItemComponentRef)\
BIND_PROPERTY(StringName, class_name, CanvasItemComponentRef)\

DEFINE_COMPONENT_PROXY(CanvasItemComponentRef, CanvasItemComponent,
CANVAS_ITEM_COMPONENT_PROPERTIES,
CANVAS_ITEM_COMPONENT_BINDINGS);

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
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")) {}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
