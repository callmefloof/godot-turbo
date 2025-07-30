#pragma once
#include "../../../../../core/math/transform_3d.h"
#include "../../../../../core/templates/vector.h"
#include "../../../ecs/components/worldcomponents.h"
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/rendering/rendering_components.h"
#include "../../components/transform_3d_component.h"

class VoxelGI;
class OccluderInstance3D;
class Viewport;
class SpotLight3D;
class OmniLight3D;
class DirectionalLight3D;
class Compositor;
class Camera3D;
class WorldEnvironment;
class Skeleton3D;
class ReflectionProbe;
class GPUParticles3D;
class MultiMeshInstance3D;
class MeshInstance3D;
class String;
struct Transform3D;
class RID;
struct Plane;
class Material;

class RenderUtility3D {
	RenderUtility3D() = default; // Prevent instantiation
	RenderUtility3D(const RenderUtility3D &) = delete; // Prevent copy
	RenderUtility3D &operator=(const RenderUtility3D &) = delete; // Prevent assignment
	RenderUtility3D(RenderUtility3D &&) = delete; // Prevent move
	RenderUtility3D &operator=(RenderUtility3D &&) = delete; // Prevent move assignment

public:
	static flecs::entity create_mesh_instance(const flecs::world &world, const RID& mesh_id, const Transform3D& transform, const String& name, const RID& scenario_id);
	static flecs::entity create_mesh_instance(const flecs::world &world, const Transform3D &transform, const RID &scenario_id, const String &name);
	static flecs::entity create_mesh_instance(const flecs::world &world, MeshInstance3D* mesh_instance_3d);
	static flecs::entity create_multi_mesh(const flecs::world &world,
			const Transform3D &transform,
			uint32_t size,
			const RID &mesh_id,
			const Vector<RID> &material_ids,
			const RID &scenario_id,
			const String &name,
			bool use_colors = false,
			bool use_custom_data = false, bool use_indirect = false);
	static flecs::entity create_multi_mesh(const flecs::world &world, MultiMeshInstance3D* multi_mesh_instance);
	static flecs::entity create_multi_mesh_instance(
		const flecs::world &world,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multi_mesh_id,
		const String &name);
	static Vector<flecs::entity> create_multi_mesh_instances(
			const flecs::world &world,
			const Vector<Transform3D> &transform,
			const flecs::entity& multi_mesh);
	static flecs::entity create_particles(
		const flecs::world &world,
		const Transform3D &transform,
		const RID &scenario_id,
		const String &name);
	static flecs::entity create_particles(const flecs::world &world, GPUParticles3D *gpu_particles_3d);
	static flecs::entity create_reflection_probe(const flecs::world &world, const RID &probe_id, const Transform3D &transform, const String& name) ;
	static flecs::entity create_reflection_probe(const flecs::world &world, ReflectionProbe *reflection_probe);
	static flecs::entity create_skeleton(const flecs::world &world, const RID &skeleton_id, const String &name);
	static flecs::entity create_skeleton(const flecs::world &world, Skeleton3D* skeleton_3d);
	static flecs::entity create_environment(const flecs::world &world, const RID &environment_id, const String &name);
	static flecs::entity create_environment(const flecs::world &world, WorldEnvironment* world_environment);
	static flecs::entity create_camera(const flecs::world &world, const RID &camera_id,const Transform3D &transform, const String& name);
	static flecs::entity create_camera(const flecs::world &world, const Transform3D &transform, const String &name);
	static flecs::entity create_camera(const flecs::world &world, Camera3D *camera_3d);
	static flecs::entity create_compositor(const flecs::world &world, const RID &compositor_id, const String& name);
	static flecs::entity create_compositor(const flecs::world &world, Compositor *compositor);
	static flecs::entity create_directional_light(const flecs::world &world, const RID &light_id, const Transform3D& transform, const String &name);
	static flecs::entity create_directional_light(const flecs::world &world, const Transform3D &transform, const String &name);
	static flecs::entity create_directional_light(const flecs::world &world, DirectionalLight3D *directional_light);
	static flecs::entity create_omni_light(const flecs::world &world, const RID &light_id, const Transform3D &transform, const RID &scenario_id);
	static flecs::entity create_omni_light(const flecs::world &world, const Transform3D &transform, const RID &scenario_id);
	static flecs::entity create_omni_light(const flecs::world &world, OmniLight3D *omni_light);
	static flecs::entity create_spot_light(const flecs::world &world, const RID &light_id, const Transform3D &transform, const String &name);
	static flecs::entity create_spot_light(const flecs::world &world, const Transform3D &transform, const String &name);
	static flecs::entity create_spot_light(const flecs::world &world, SpotLight3D *spot_light);
	static flecs::entity create_viewport(const flecs::world &world, const RID &viewport_id, const String &name);
	static flecs::entity create_viewport(const flecs::world &world, Viewport *viewport);
	static flecs::entity create_voxel_gi(const flecs::world &world, const RID &voxel_gi_id, const Transform3D& transform, const String &name);
	static flecs::entity_t create_voxel_gi(const flecs::world &world, const Transform3D &transform, const String &name);
	static flecs::entity create_voxel_gi(const flecs::world &world, VoxelGI *voxel_gi);
	static flecs::entity create_scenario(const flecs::world &world, const RID &scenario_id, const String &name);
	static flecs::entity create_scenario(const flecs::world &world, const String &name);
	static flecs::entity create_occluder(const flecs::world& world, const String &name);
	static flecs::entity create_occluder(const flecs::world &world, const RID& occluder_id, const String &name);
	static flecs::entity create_occluder(const flecs::world &world, OccluderInstance3D* occluder_instance);
	static bool _bake_material_check(const Ref<Material> &p_material);
	static void _bake_surface(const Transform3D &p_transform, Array p_surface_arrays, const Ref<Material> &p_material, float p_simplification_dist, PackedVector3Array &r_vertices, PackedInt32Array &r_indices);
};
