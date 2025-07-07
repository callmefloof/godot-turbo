#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../component_module_base.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"

namespace godot_turbo::components::rendering {

using namespace godot_turbo::components;

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_id;
};
struct MultiMeshComponent {
	uint32_t instance_count;
};
struct MultiMeshInstanceComponent {
	uint32_t index;
};
struct ParticlesComponent {
	RID particles_id;
};
struct ReflectionProbeComponent {
	RID reflection_probe_id;
};
struct SkeletonComponent {
	RID skeleton_id;
};
struct EnvironmentComponent {
	RID environment_id;
};
struct CameraComponent {
	RID camera_id;
};
struct CompositorComponent {
	RID compositor_id;
};
struct DirectionalLightComponent {
	RID directional_light_id;
};
struct OmniLightComponent {
	RID omni_light_id;
};
struct SpotLightComponent {
	RID spot_light_id;
};
struct ViewportComponent {
	RID viewport_id;
};
struct VoxelGIComponent {
	RID voxel_gi_id;
};
struct ScenarioComponent {
	RID scenario_id;
};
struct RenderInstanceComponent {
	RID instance_id;
};
struct CanvasComponent {
	RID canvas_id;
};
struct CanvasItemComponent {
	RID canvas_item_id;
};

struct RenderingBaseComponents {
	flecs::component<MeshComponent> mesh;
	flecs::component<MultiMeshComponent> multimesh;
	flecs::component<MultiMeshInstanceComponent> mesh_instance;
	flecs::component<ParticlesComponent> particles;
	flecs::component<ReflectionProbeComponent> reflection_probe;
	flecs::component<SkeletonComponent> skeleton;
	flecs::component<EnvironmentComponent> environment;
	flecs::component<CameraComponent> camera;
	flecs::component<CompositorComponent> compositor;
	flecs::component<DirectionalLightComponent> directional_light;
	flecs::component<OmniLightComponent> omni_light;
	flecs::component<SpotLightComponent> spot_light;
	flecs::component<ViewportComponent> viewport;
	flecs::component<ScenarioComponent> scenario;
	flecs::component<RenderInstanceComponent> instance;
	flecs::component<CanvasComponent> canvas;
	flecs::component<CanvasItemComponent> canvas_item;

	RenderingBaseComponents(flecs::world &world) :
			mesh(world.component<MeshComponent>("MeshComponent")),
			multimesh(world.component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world.component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world.component<ParticlesComponent>("ParticlesComponent")),
			reflection_probe(world.component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world.component<SkeletonComponent>("SkeletonComponent")),
			environment(world.component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world.component<CameraComponent>("CameraComponent")),
			compositor(world.component<CompositorComponent>("CompositorComponent")),
			directional_light(world.component<DirectionalLightComponent>("DirectionalLightComponent")),
			omni_light(world.component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world.component<SpotLightComponent>("SpotLightComponent")),
			viewport(world.component<ViewportComponent>("ViewportComponent")),
			scenario(world.component<ScenarioComponent>("ScenarioComponent")),
			instance(world.component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas(world.component<CanvasComponent>("CanvasComponent")),
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")) {}
};

class RenderingComponents : public MultiComponentModule<RenderingComponents, RenderingBaseComponents> {};

} // namespace godot_turbo::components::rendering
