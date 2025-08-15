#pragma once
#include "core/math/transform_3d.h"
#include "core/templates/vector.h"
#include "ecs/components/worldcomponents.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/transform_3d_component.h"
#include "core/variant/typed_array.h"
#include "ecs/flecs_types/flecs_world.h"

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

class RenderUtility3D : Object {
	GDCLASS(RenderUtility3D, Object)
public:
	RenderUtility3D() = default;
	~RenderUtility3D();
	static flecs::entity _create_mesh_instance(const flecs::world *world, const RID& mesh_id, const Transform3D& transform, const String& name, const RID& scenario_id);
	static flecs::entity _create_mesh_instance(const flecs::world *world, const Transform3D &transform, const RID &scenario_id, const String &name);
	static flecs::entity _create_mesh_instance(const flecs::world *world, MeshInstance3D* mesh_instance_3d);
	static flecs::entity _create_multi_mesh(const flecs::world *world,
			const Transform3D &transform,
			uint32_t size,
			const RID &mesh_id,
			const Vector<RID> &material_ids,
			const RID &scenario_id,
			const String &name,
			bool use_colors = false,
			bool use_custom_data = false, bool use_indirect = false);
	static flecs::entity _create_multi_mesh(const flecs::world *world, MultiMeshInstance3D* multi_mesh_instance);
	static flecs::entity _create_multi_mesh_instance(
		const flecs::world *world,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multi_mesh_id,
		const String &name);
	static Vector<flecs::entity> _create_multi_mesh_instances(
			const flecs::world *world,
			const Vector<Transform3D> &transform,
			const flecs::entity& multi_mesh);
	static flecs::entity _create_particles(
		const flecs::world *world,
		const Transform3D &transform,
		const RID &particles_id,
		const RID &scenario_id,
		const String &name);
	static flecs::entity _create_particles(const flecs::world *world, GPUParticles3D *gpu_particles_3d);
	static flecs::entity _create_reflection_probe(const flecs::world *world, const RID &probe_id, const Transform3D &transform, const String& name) ;
	static flecs::entity _create_reflection_probe(const flecs::world *world, ReflectionProbe *reflection_probe);
	static flecs::entity _create_skeleton(const flecs::world *world, const RID &skeleton_id, const String &name);
	static flecs::entity _create_skeleton(const flecs::world *world, Skeleton3D* skeleton_3d);
	static flecs::entity _create_environment(const flecs::world *world, const RID &environment_id, const String &name);
	static flecs::entity _create_environment(const flecs::world *world, WorldEnvironment* world_environment);
	static flecs::entity _create_camera(const flecs::world *world, const RID &camera_id,const Transform3D &transform, const String& name);
	static flecs::entity _create_camera(const flecs::world *world, const Transform3D &transform, const String &name);
	static flecs::entity _create_camera(const flecs::world *world, Camera3D *camera_3d);
	static flecs::entity _create_compositor(const flecs::world *world, const RID &compositor_id, const String& name);
	static flecs::entity _create_compositor(const flecs::world *world, const Ref<Compositor> &compositor);
	static flecs::entity _create_directional_light(const flecs::world *world, const RID &light_id, const Transform3D& transform, const String &name);
	static flecs::entity _create_directional_light(const flecs::world *world, const Transform3D &transform, const String &name);
	static flecs::entity _create_directional_light(const flecs::world *world, DirectionalLight3D *directional_light);
	static flecs::entity _create_omni_light(const flecs::world *world, const RID &light_id, const Transform3D &transform, const RID &scenario_id);
	static flecs::entity _create_omni_light(const flecs::world *world, const Transform3D &transform, const RID &scenario_id);
	static flecs::entity _create_omni_light(const flecs::world *world, OmniLight3D *omni_light);
	static flecs::entity _create_spot_light(const flecs::world *world, const RID &light_id, const Transform3D &transform, const String &name);
	static flecs::entity _create_spot_light(const flecs::world *world, const Transform3D &transform, const String &name);
	static flecs::entity _create_spot_light(const flecs::world *world, SpotLight3D *spot_light);
	static flecs::entity _create_viewport(const flecs::world *world, const RID &viewport_id, const String &name);
	static flecs::entity _create_viewport(const flecs::world *world, Viewport *viewport);
	static flecs::entity _create_voxel_gi(const flecs::world *world, const RID &voxel_gi_id, const Transform3D& transform, const String &name);
	static flecs::entity _create_voxel_gi(const flecs::world *world, const Transform3D &transform, const String &name);
	static flecs::entity _create_voxel_gi(const flecs::world *world, VoxelGI *voxel_gi);
	static flecs::entity _create_scenario(const flecs::world *world, const RID &scenario_id, const String &name);
	static flecs::entity _create_scenario(const flecs::world *world, const String &name);
	static flecs::entity _create_occluder(const flecs::world *world, const String &name);
	static flecs::entity _create_occluder(const flecs::world *world, const RID& occluder_id, const String &name);
	static flecs::entity _create_occluder(const flecs::world *world, OccluderInstance3D* occluder_instance);
	static bool _bake_material_check(const Ref<Material> &p_material);
	static void _bake_surface(const Transform3D &p_transform, const Array& p_surface_arrays, const Ref<Material> &p_material, float p_simplification_dist, const PackedVector3Array &r_vertices, const PackedInt32Array &r_indices);
	static Ref<FlecsEntity> create_mesh_instance(FlecsWorld* flecs_world, const RID& mesh_id, const Transform3D& transform, const String& name, const RID& scenario_id);
	static Ref<FlecsEntity> create_mesh_instance_with_object(FlecsWorld* flecs_world, MeshInstance3D* mesh_instance_3d);
	static TypedArray<FlecsEntity> create_multi_mesh(FlecsWorld* flecs_world, const Transform3D& transform, uint32_t size, const RID& mesh_id, const TypedArray<RID>& material_ids, const RID& scenario_id, const String& name, bool use_colors = false, bool use_custom_data = false, bool use_indirect = false);
	static TypedArray<FlecsEntity> create_multi_mesh_with_object(FlecsWorld* flecs_world, MultiMeshInstance3D* multi_mesh_instance_3d);
	static Ref<FlecsEntity> create_camera(FlecsWorld* flecs_world, const Transform3D& transform, const String& name);
	static Ref<FlecsEntity> create_camera_with_object(FlecsWorld* flecs_world, Camera3D *camera_3d);
	static Ref<FlecsEntity> create_directional_light(FlecsWorld* flecs_world, const Transform3D& transform, const String& name);
	static Ref<FlecsEntity> create_directional_light_with_object(FlecsWorld* flecs_world, DirectionalLight3D *directional_light_3d);
	static Ref<FlecsEntity> create_voxel_gi(FlecsWorld* flecs_world,const RID &voxel_gi_rid, const Transform3D& transform, const String& name);
	static Ref<FlecsEntity> create_spot_light(FlecsWorld* flecs_world, const RID &light_id, const Transform3D& transform, const String& name);
	static Ref<FlecsEntity> create_spot_light_with_object(FlecsWorld* flecs_world, SpotLight3D *spot_light);
	static Ref<FlecsEntity> create_omni_light(FlecsWorld* flecs_world, const RID &light_id, const Transform3D& transform, const RID& scenario_id);
	static Ref<FlecsEntity> create_omni_light_with_object(FlecsWorld* flecs_world, OmniLight3D *omni_light);
	static Ref<FlecsEntity> create_reflection_probe(FlecsWorld* flecs_world, const RID& probe_id, const Transform3D& transform, const String& name);
	static Ref<FlecsEntity> create_reflection_probe_with_object(FlecsWorld* flecs_world, ReflectionProbe *reflection_probe);
	static Ref<FlecsEntity> create_scenario(FlecsWorld* flecs_world, const RID &scenario_id, const String& name);
	static Ref<FlecsEntity> create_particles(FlecsWorld* flecs_world, const Transform3D& transform,const RID &particles_id, const RID& scenario_id, const String& name);
	static Ref<FlecsEntity> create_particles_with_object(FlecsWorld* flecs_world, GPUParticles3D* gpu_particles_3d);
	static Ref<FlecsEntity> create_viewport(FlecsWorld* flecs_world, const RID& viewport_id, const String& name);
	static Ref<FlecsEntity> create_viewport_with_object(FlecsWorld* flecs_world, Viewport* viewport);
	static Ref<FlecsEntity> create_voxel_gi_with_object(FlecsWorld* flecs_world, VoxelGI* voxel_gi);
	static Ref<FlecsEntity> create_environment(FlecsWorld* flecs_world, const RID& environment_id, const String& name);
	static Ref<FlecsEntity> create_environment_with_object(FlecsWorld* flecs_world, WorldEnvironment* world_environment);
	static Ref<FlecsEntity> create_skeleton(FlecsWorld* flecs_world, const RID& skeleton_id, const String& name);
	static Ref<FlecsEntity> create_skeleton_with_object(FlecsWorld* flecs_world, Skeleton3D* skeleton_3d);
	static Ref<FlecsEntity> create_compositor(FlecsWorld* flecs_world, const RID& compositor_id, const String& name);
	static Ref<FlecsEntity> create_compositor_with_object(FlecsWorld* flecs_world, Compositor *compositor);
	static Ref<FlecsEntity> create_occluder_with_object(FlecsWorld* flecs_world, OccluderInstance3D* occluder_instance);
	static Ref<FlecsEntity> create_occluder(FlecsWorld* flecs_world, const RID& occluder_id, const String& name);
	static void _bind_methods();
	

};
