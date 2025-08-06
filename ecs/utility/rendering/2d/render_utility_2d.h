#pragma once
#include "ecs/flecs_types/flecs_world.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/transform_2d_component.h"
#include "modules/godot_turbo/ecs/components/worldcomponents.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/main/canvas_item.h"
#include "scene/resources/mesh.h"
#include "servers/rendering_server.h"
#include <scene/2d/light_2d.h>
#include <scene/2d/light_occluder_2d.h>
#include <scene/2d/mesh_instance_2d.h>
#include "../flecs_types/flecs_world.h"
#include "ecs/utility/object_id_storage.h"


class RenderUtility2D : public Object{
	GDCLASS(RenderUtility2D, Object)
public:
	RenderUtility2D() = default;
	~RenderUtility2D() = default;
	static flecs::entity _create_mesh_instance(const flecs::world &world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id);
	static flecs::entity _create_mesh_instance(const flecs::world &world, const Transform2D &transform,  const String &name);
	static flecs::entity _create_mesh_instance(const flecs::world &world, MeshInstance2D *mesh_instance_2d);
	static flecs::entity _create_multi_mesh(const flecs::world &world,
			const Transform2D &transform,
			const uint32_t size,
			const Ref<Mesh> mesh,
			const String &name,
			const RID& texture_id,
			const bool use_colors = false,
			const bool use_custom_data = false,
			const bool use_indirect = false);
	static flecs::entity _create_multi_mesh(const flecs::world &world,
			MultiMeshInstance2D *multi_mesh_instance);
	static flecs::entity _create_multi_mesh_instance(
			const flecs::world &world,
			const Transform2D &transform,
			const uint32_t index,
			const String &name);
	static Vector<flecs::entity> _create_multi_mesh_instances(
			const flecs::world &world,
			const Vector<Transform2D>& transform,
			const flecs::entity &multi_mesh);
	static flecs::entity _create_camera_2d(const flecs::world &world, const RID &camera_id, const Transform2D &transform, const String &name);
	static flecs::entity _create_camera_2d(const flecs::world &world, Camera2D *camera_2d);
	static flecs::entity _create_directional_light(const flecs::world &world, const RID &light_id, const Transform2D &transform,  const String &name);
	static flecs::entity _create_directional_light(const flecs::world &world, const Transform2D &transform, const String &name);
	static flecs::entity _create_directional_light(const flecs::world &world, DirectionalLight2D *directional_light);
	static flecs::entity _create_point_light(const flecs::world &world,  const Transform2D &transform, const String &name);
	static flecs::entity _create_point_light(const flecs::world &world,  const RID &light_id, const Transform2D &transform, const String &name);
	static flecs::entity _create_point_light(const flecs::world &world, PointLight2D *point_light);
	static flecs::entity _create_canvas_item(const flecs::world &world, CanvasItem *canvas_item);
	static flecs::entity _create_canvas_item(const flecs::world &world, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name);
	static flecs::entity _create_skeleton(const flecs::world &world, const RID &skeleton_id, const String &name);
	static flecs::entity _create_skeleton(const flecs::world &world, Skeleton2D *skeleton_2d);
	static flecs::entity _create_light_occluder(const flecs::world &world, LightOccluder2D *light_occluder);
	static flecs::entity _create_light_occluder(const flecs::world &world, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name );
	static flecs::entity _create_light_occluder(const flecs::world &world, const Transform2D &transform, const String &name);
	static flecs::entity _create_gpu_particles_2d(const flecs::world &world, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform,  const String& name);
	static flecs::entity _create_gpu_particles_2d(const flecs::world &world, GPUParticles2D* gpu_particles, uint32_t count = 0, const uint32_t max_depth = 10000);

	static Ref<FlecsEntity> create_mesh_instance(const Ref<FlecsWorld>& flecs_world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id);
	static Ref<FlecsEntity> create_mesh_instance_with_object(const Ref<FlecsWorld>& flecs_world, MeshInstance2D *mesh_instance_2d);
	static TypedArray<FlecsEntity> create_multi_mesh(FlecsWorld *flecs_world, const Transform2D &transform, uint32_t size, const Ref<Mesh>& mesh, const RID &texture_id, const String &name, bool use_colors, bool use_custom_data, bool use_indirect);
	static TypedArray<FlecsEntity> create_multi_mesh_with_object(const Ref<FlecsWorld>& flecs_world, MultiMeshInstance2D *multi_mesh_instance);
	static Ref<FlecsEntity> create_multi_mesh_instance(const Ref<FlecsWorld>& flecs_world,
			const Transform2D &transform,
			const uint32_t index,
			const String &name);
	static Ref<FlecsEntity> create_camera_2d(const Ref<FlecsWorld>& flecs_world, const RID &camera_id, const Transform2D &transform, const String &name);
	static Ref<FlecsEntity> create_camera_2d_with_object(const Ref<FlecsWorld>& flecs_world, Camera2D *camera_2d);
	static Ref<FlecsEntity> create_directional_light(const Ref<FlecsWorld>& flecs_world, const RID &light_id, const Transform2D &transform,  const String &name);
	static Ref<FlecsEntity> create_directional_light_with_object(const Ref<FlecsWorld>& flecs_world, DirectionalLight2D *directional_light);
	static Ref<FlecsEntity> create_point_light(const Ref<FlecsWorld>& flecs_world,  const RID &light_id, const Transform2D &transform, const String &name);
	static Ref<FlecsEntity> create_point_light_with_object(const Ref<FlecsWorld>& flecs_world, PointLight2D *point_light);
	static Ref<FlecsEntity> create_canvas_item_with_object(const Ref<FlecsWorld>& flecs_world, CanvasItem *canvas_item);
	static Ref<FlecsEntity> create_canvas_item(const Ref<FlecsWorld>& flecs_world, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name);
	static Ref<FlecsEntity> create_skeleton(const Ref<FlecsWorld>& flecs_world, const RID &skeleton_id, const String &name);
	static Ref<FlecsEntity> create_skeleton_with_object(const Ref<FlecsWorld>& flecs_world, Skeleton2D *skeleton_2d);
	static Ref<FlecsEntity> create_light_occluder_with_object(const Ref<FlecsWorld>& flecs_world, LightOccluder2D *light_occluder);
	static Ref<FlecsEntity> create_light_occluder(const Ref<FlecsWorld>& flecs_world, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name );
	static Ref<FlecsEntity> create_gpu_particles_2d(const Ref<FlecsWorld>& flecs_world, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform,  const String& name);
	static Ref<FlecsEntity> create_gpu_particles_2d_with_object(const Ref<FlecsWorld>& flecs_world, GPUParticles2D* gpu_particles, uint32_t count = 0, const uint32_t max_depth = 10000);
};
