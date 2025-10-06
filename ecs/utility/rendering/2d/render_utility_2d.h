#pragma once
#include "core/variant/typed_array.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/main/canvas_item.h"
#include "scene/resources/mesh.h"
#include <scene/2d/light_2d.h>
#include <scene/2d/light_occluder_2d.h>
#include <scene/2d/mesh_instance_2d.h>


class RenderUtility2D : public Object{
	GDCLASS(RenderUtility2D, Object)
public:
	RenderUtility2D() = default;
	~RenderUtility2D() = default;
    static RID create_mesh_instance_with_id(const RID &world_id, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id);
    static RID create_mesh_instance(const RID &world_id, const Transform2D &transform,  const String &name);
    static RID create_mesh_instance_with_object(const RID &world_id, MeshInstance2D *mesh_instance_2d);
    static RID create_multi_mesh(const RID &world_id,
	    const Transform2D &transform,
	    const uint32_t size,
	    const Ref<Mesh> mesh,
	    const String &name,
	    const RID& texture_id,
	    const bool use_colors = false,
	    const bool use_custom_data = false,
	    const bool use_indirect = false);
    static TypedArray<RID> create_multi_mesh_with_object(const RID &world_id,
	    MultiMeshInstance2D *multi_mesh_instance);
    static RID create_multi_mesh_instance(
	    const RID &world_id,
	    const Transform2D &transform,
	    const uint32_t index,
	    const String &name);
    static TypedArray<RID> create_multi_mesh_instances(
	    const RID &world_id,
	    const TypedArray<Transform2D>& transform,
	    const RID &multi_mesh);
    static RID create_camera_with_id(const RID &world_id, const RID &camera_id, const Transform2D &transform, const String &name);
    static RID create_camera_with_object(const RID &world_id, Camera2D *camera_2d);
    static RID create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform2D &transform,  const String &name);
    static RID create_directional_light(const RID &world_id, const Transform2D &transform, const String &name);
    static RID create_directional_light_with_object(const RID &world_id, DirectionalLight2D *directional_light);
    static RID create_point_light(const RID &world_id,  const Transform2D &transform, const String &name);
    static RID create_point_light_with_id(const RID &world_id,  const RID &light_id, const Transform2D &transform, const String &name);
    static RID create_point_light_with_object(const RID &world_id, PointLight2D *point_light);
    static RID create_canvas_item_with_object(const RID &world_id, CanvasItem *canvas_item);
    static RID create_canvas_item_with_id(const RID &world_id, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name);
    static RID create_skeleton_with_id(const RID &world_id, const RID &skeleton_id, const String &name);
    static RID create_skeleton_with_object(const RID &world_id, Skeleton2D *skeleton_2d);
    static RID create_light_occluder_with_object(const RID &world_id, LightOccluder2D *light_occluder);
    static RID create_light_occluder_with_id(const RID &world_id, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name );
    static RID create_light_occluder(const RID &world_id, const Transform2D &transform, const String &name);
    static RID create_gpu_particles_with_id(const RID &world_id, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform,  const String& name);
    static RID create_gpu_particles_with_object(const RID &world_id, GPUParticles2D* gpu_particles, uint32_t count = 0, const uint32_t max_depth = 10000);
};
