#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../component_module_base.h"
#include "core/templates/rid.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include <servers/rendering_server.h>
#include "../../utility/object_id_storage.h"

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_id;
	~MeshComponent() {
		for (const RID &mat_id : material_id) {
			if (mat_id.is_valid()) {
				RenderingServer::get_singleton()->free(mat_id);
			}
		}
		if (mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(mesh_id);
		}
	}
};

struct MultiMeshComponent {
	RID multimesh_id;
	uint32_t instance_count;
	~MultiMeshComponent() {
		// Ensure that the RID is released when the component is destroyed
		if (multimesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(multimesh_id);
		}
	}
};

struct MultiMeshInstanceComponent {
	uint32_t index;
};

struct ParticlesComponent {
	RID particles_id;
	~ParticlesComponent() {
		if (particles_id.is_valid()) {
			RenderingServer::get_singleton()->free(particles_id);
		}
	}
};
struct ReflectionProbeComponent {
	RID probe_id;
	~ReflectionProbeComponent() {
		if (probe_id.is_valid()) {
			RenderingServer::get_singleton()->free(probe_id);
		}
	}
};
struct SkeletonComponent {
	RID skeleton_id;
	~SkeletonComponent() {
		if (skeleton_id.is_valid()) {
			RenderingServer::get_singleton()->free(skeleton_id);
		}
	}
};
struct EnvironmentComponent {
	RID environment_id;
	~EnvironmentComponent() {
		if (environment_id.is_valid()) {
			RenderingServer::get_singleton()->free(environment_id);
		}
	}
};
struct CameraComponent {
	RID camera_id;
	~CameraComponent() {
		if (camera_id.is_valid()) {
			RenderingServer::get_singleton()->free(camera_id);
		}
	}
};
struct CompositorComponent {
	RID compositor_id;
	~CompositorComponent() {
		if (compositor_id.is_valid()) {
			RenderingServer::get_singleton()->free(compositor_id);
		}
	}
};
struct DirectionalLight3DComponent {
	RID directional_light_id;
	~DirectionalLight3DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

struct DirectionalLight2DComponent {
	RID directional_light_id;
	~DirectionalLight2DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

struct PointLightComponent
{
	RID point_light_id;
	~PointLightComponent() {
		if (point_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(point_light_id);
		}
	}
};

struct LightOccluderComponent
{
	RID light_occluder_id;
	~LightOccluderComponent()
	{
		if (light_occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(light_occluder_id);
		}
	}
};

struct OmniLightComponent {
	RID omni_light_id;
	~OmniLightComponent() {
		if (omni_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(omni_light_id);
		}
	}
};
struct SpotLightComponent {
	RID spot_light_id;
	~SpotLightComponent() {
		if (spot_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(spot_light_id);
		}
	}
};
struct ViewportComponent {
	RID viewport_id;
};
struct VoxelGIComponent {
	RID voxel_gi_id;
	~VoxelGIComponent() {
		if (voxel_gi_id.is_valid()) {
			RenderingServer::get_singleton()->free(voxel_gi_id);
		}
	}
};
struct ScenarioComponent {
	RID id;
};
struct MainScenarioComonent {
	RID id;
	String entity_name;
	// This is the main scenario, used for the main viewport
	// It is used to set the default environment and camera attributes
	// It is also used to set the default compositor
	// It is used to set the default directional light
	// It is used to set the default omni light
	// It is used to set the default spot light
	// It is used to set the default viewport
};
struct RenderInstanceComponent {
	RID instance_id;
	~RenderInstanceComponent() {
		if (instance_id.is_valid()) {
			RenderingServer::get_singleton()->free(instance_id);
		}
	}
};
struct CanvasComponent {
	RID canvas_id;
	~CanvasComponent() {
		if (canvas_id.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_clear(canvas_id);
			RenderingServer::get_singleton()->free(canvas_id);
		}
	}
};
struct CanvasItemComponent {
	RID canvas_item_id;
	String class_name;
	~CanvasItemComponent() {
		if (canvas_item_id.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_clear(canvas_item_id);
			RenderingServer::get_singleton()->free(canvas_item_id);
		}
	}
};

struct RenderingBaseComponents {
	flecs::component<MeshComponent> mesh;
	flecs::component<MultiMeshComponent> multimesh;
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
	flecs::component<MainScenarioComonent> main_scenario;
	flecs::component<VoxelGIComponent> voxel_gi;
	flecs::component<RenderInstanceComponent> instance;
	flecs::component<CanvasComponent> canvas;
	flecs::component<CanvasItemComponent> canvas_item;

	RenderingBaseComponents(flecs::world &world) :
			mesh(world.component<MeshComponent>("MeshComponent")),
			multimesh(world.component<MultiMeshComponent>("MultiMeshComponent")),
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
			voxel_gi(world.component<VoxelGIComponent>("VoxelGIComponent")),
			scenario(world.component<ScenarioComponent>("ScenarioComponent")),
			main_scenario(world.component<MainScenarioComonent>("MainScenarioComponent")),
			instance(world.component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas(world.component<CanvasComponent>("CanvasComponent")),
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")) {}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
	
