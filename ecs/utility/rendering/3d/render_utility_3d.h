#pragma once
#include "core/math/transform_3d.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "core/object/object.h"
#include "core/templates/rid.h"
#include "core/object/class_db.h"

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
	GDCLASS(RenderUtility3D, Object);
public:
	RenderUtility3D() = default;
	~RenderUtility3D();
	static RID create_mesh_instance_with_id(const RID &world_id, const RID& mesh_id, const Transform3D& transform, const String& name, const RID& scenario_id);
	static RID create_mesh_instance(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name);
	static RID create_mesh_instance_with_object(const RID &world_id, MeshInstance3D* mesh_instance_3d);
	static RID create_multi_mesh(const RID &world_id,
			const Transform3D &transform,
			uint32_t size,
			const RID &mesh_id,
			const TypedArray<RID> &material_ids,
			const RID &scenario_id,
			const String &name,
			bool use_colors = false,
			bool use_custom_data = false, bool use_indirect = false);
	static TypedArray<RID> create_multi_mesh_with_object(const RID &world_id, MultiMeshInstance3D* multi_mesh_instance);
	static RID create_multi_mesh_instance(const RID &world_id,const Transform3D &transform,const uint32_t index,const RID &multi_mesh_id,const String &name);
	static TypedArray<RID> create_multi_mesh_instances(const RID &world_id,const TypedArray<Transform3D> &transform, const RID &multi_mesh_id);
	static RID create_particles(const RID &world_id,const Transform3D &transform,const RID &particles_id,const int particle_count,const RID &scenario_id,const String &name);
	static RID create_particles_with_object(const RID &world_id, GPUParticles3D *gpu_particles_3d);
	static RID create_reflection_probe(const RID &world_id, const RID &probe_id, const Transform3D &transform, const String& name) ;
	static RID create_reflection_probe_with_object(const RID &world_id, ReflectionProbe *reflection_probe);
	static RID create_skeleton(const RID &world_id, const RID &skeleton_id, const String &name);
	static RID create_skeleton_with_object(const RID &world_id, Skeleton3D* skeleton_3d);
	static RID create_environment(const RID &world_id, const RID &environment_id, const String &name);
	static RID create_environment_with_object(const RID &world_id, WorldEnvironment* world_environment);
	static RID create_camera_with_id(const RID &world_id, const RID &camera_id,const Transform3D &transform, const String& name);
	static RID create_camera(const RID &world_id, const Transform3D &transform, const String &name);
	static RID create_camera_with_object(const RID &world_id, Camera3D *camera_3d);
	static RID create_compositor(const RID &world_id, const RID &compositor_id, const String& name);
	static RID create_compositor_with_object(const RID &world_id, const Ref<Compositor> &compositor);
	static RID create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform3D& transform, const String &name);
	static RID create_directional_light(const RID &world_id, const Transform3D &transform, const String &name);
	static RID create_directional_light_with_object(const RID &world_id, DirectionalLight3D *directional_light);
	static RID create_omni_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const RID &scenario_id, const String &name);
	static RID create_omni_light(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name);
	static RID create_omni_light_with_object(const RID &world_id, OmniLight3D *omni_light);
	static RID create_spot_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const String &name);
	static RID create_spot_light(const RID &world_id, const Transform3D &transform, const String &name);
	static RID create_spot_light_with_object(const RID &world_id, SpotLight3D *spot_light);
	static RID create_viewport_with_id(const RID &world_id, const RID &viewport_id, const String &name);
	static RID create_viewport_with_object(const RID &world_id, Viewport *viewport);
	static RID create_voxel_gi_with_id(const RID &world_id, const RID &voxel_gi_id, const Transform3D& transform, const String &name);
	static RID create_voxel_gi(const RID &world_id, const Transform3D &transform, const String &name);
	static RID create_voxel_gi_with_object(const RID &world_id, VoxelGI *voxel_gi);
	static RID create_scenario_with_id(const RID &world_id, const RID &scenario_id, const String &name);
	static RID create_scenario(const RID &world_id, const String &name);
	
	static void _bind_methods();
	

};
