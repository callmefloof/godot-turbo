#pragma once
#include "core/templates/rid.h"
#include "core/templates/vector.h"

struct MeshComponent
{
	RID mesh_id;
	Vector<RID> material_id;
};

struct MeshInstanceComponent
{
	RID mesh_instance_id;
};

struct MultiMeshComponent
{
	uint32_t instance_count;
};

struct ParticlesComponent
{
	RID particles_id;
};

struct ReflectionProbeComponent
{
	RID reflection_probe_id;
};

struct MeshSkeletonComponent
{
	RID skeleton_id;
};

struct EnvironmentComponent
{
	RID environment_id;
};

struct CameraComponent
{
	RID camera_id;
};

struct CompositorComponent
{
	RID compositor_id;
};

struct DirectionalLightComponent
{
	RID directional_light_id;
};

struct OmniLightComponent
{
	RID omni_light_id;
};

struct SpotLightComponent
{
	RID spot_light_id;
};


struct ViewportComponent
{
	RID viewport_id;
};

struct VoxelGIComponent
{
	RID voxel_gi_id;
};

struct ScenarioComponent
{
	RID scenario_id;
};

struct RenderInstanceComponent
{
	RID instance_id;
};

struct CanvasComponent
{
	RID canvas_id;
};

struct CanvasItemComponent
{
	RID canvas_item_id;
};
