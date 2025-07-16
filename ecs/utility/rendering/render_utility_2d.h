#pragma once
#include "../flecs_types/flecs_world.h"

#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/rendering/rendering_components.h"
#include "../../components/transform_2d_component.h"
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



class RenderUtility2D
{
private:
	RenderUtility2D() = default; // Prevent instantiation
	RenderUtility2D(const RenderUtility2D &) = delete; // Prevent copy
	RenderUtility2D &operator=(const RenderUtility2D &) = delete; // Prevent assignment
	RenderUtility2D(RenderUtility2D &&) = delete; // Prevent move
	RenderUtility2D &operator=(RenderUtility2D &&) = delete; // Prevent move assignment
public:
	static flecs::entity create_mesh_instance(const flecs::world &world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id) {
		Vector<RID> material_ids;
		const int surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
		for (int i = 0; i < surface_count; ++i) {
			if (const RID material_id = RS::get_singleton()->mesh_surface_get_material(mesh_id, i); material_id.is_valid()) {
				material_ids.push_back(material_id);
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		const RID canvas_item = RS::get_singleton()->canvas_item_create();


		const flecs::entity entity = world.entity()
							  .set<MeshComponent>({ mesh_id, material_ids })
							  .set<Transform2DComponent>({ transform }) // Default transform
							  .set<CanvasItemComponent>({ canvas_item, "MeshInstance2D" })
							  .set_name(name.ascii().get_data());
		RS::get_singleton()->canvas_item_add_mesh(canvas_item, mesh_id);
		RS::get_singleton()->canvas_item_set_parent(canvas_item, canvas_id);
		RS::get_singleton()->canvas_item_set_material(canvas_item, material_ids[0]);
		return entity;
	}

	static flecs::entity create_mesh_instance(const flecs::world &world, const Transform2D &transform,  const String &name) {
		Vector<RID> material_ids;
		const RID mesh_id = RS::get_singleton()->mesh_create();
		if (!world.has<World2DComponent>()) {
			ERR_FAIL_COND_V(!world.has<World2DComponent>(), flecs::entity());
		}
		return create_mesh_instance(world, mesh_id, transform, name, world.get<World2DComponent>().canvas_id);
	}

	static Ref<FlecsEntity> create_mesh_instance(const Ref<FlecsWorld>& world, const MeshInstance2D *mesh_instance_2d) {
		if (!world.is_valid() || !world.is_null()) {
			ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), Ref<FlecsEntity>());
		}
		Ref<FlecsEntity> entity = memnew(FlecsEntity);

		create_mesh_instance(&world->get_world());
		//entity->set_entity();

	}

	static flecs::entity create_mesh_instance(const flecs::world &world, MeshInstance2D *mesh_instance_2d) {
		Vector<RID> material_ids;
		const Ref<Mesh> mesh = mesh_instance_2d->get_mesh();
		const RID canvas_item = mesh_instance_2d->get_canvas_item();
		for (int i = 0; i < mesh->get_surface_count(); ++i) {
			if (Ref<Material> material = mesh->surface_get_material(i); material.is_valid()) {
				material_ids.push_back(material->get_rid());
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		if (const Node2D *parent = Object::cast_to<Node2D>(mesh_instance_2d->get_parent()); parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
		}

		ObjectIDStorage::add(mesh_instance_2d, mesh_instance_2d->get_canvas_item());
		return world.entity()
				.set<MeshComponent>({ mesh->get_rid(), material_ids })
				.set<CanvasItemComponent>({ canvas_item, mesh_instance_2d->get_class_name() })
				.set<Transform2DComponent>({ mesh_instance_2d->get_transform() })
				.set_name(String(mesh_instance_2d->get_name()).ascii().get_data());
	}

	static flecs::entity create_multi_mesh(const flecs::world &world,
			const Transform2D &transform,
			const uint32_t size,
			const RID &mesh_id,
			const String &name,
			const RID& texture_id,
			const bool use_colors = false,
			const bool use_custom_data = false,
			const bool use_indirect = false) {
		const RID multi_mesh_id = RS::get_singleton()->multimesh_create();
		const RID canvas_item = RS::get_singleton()->canvas_item_create();
		RS::get_singleton()->multimesh_allocate_data(multi_mesh_id, size, RS::MULTIMESH_TRANSFORM_2D, use_colors, use_custom_data, use_indirect);
		RS::get_singleton()->canvas_item_add_multimesh(canvas_item, multi_mesh_id, texture_id);
		RS::get_singleton()->multimesh_set_mesh(multi_mesh_id, mesh_id);
		const flecs::entity entity = world.entity()
							  .set<MultiMeshComponent>({ multi_mesh_id, size })
							  .set<MeshComponent>({ mesh_id })
							  .set<CanvasItemComponent>({ canvas_item })
							  .set<Transform2DComponent>({ transform })
							  .set_name(name.ascii().get_data());

		return entity;
	}

	static flecs::entity create_multi_mesh(const flecs::world &world,
			MultiMeshInstance2D *multi_mesh_instance) {
		const RID mult_mesh_id = multi_mesh_instance->get_multimesh()->get_rid();
		const Ref<Mesh> mesh = multi_mesh_instance->get_multimesh()->get_mesh();
		const RID canvas_item = multi_mesh_instance->get_canvas_item();
		const Transform2D transform = multi_mesh_instance->get_transform();
		const String name = multi_mesh_instance->get_name();
		const flecs::entity entity = world.entity()
							  .set<MultiMeshComponent>({ mult_mesh_id, static_cast<uint32_t>(multi_mesh_instance->get_multimesh()->get_instance_count()) })
							  .set<MeshComponent>({ mesh->get_rid() })
							  .set<CanvasItemComponent>({ canvas_item, "MultiMesh2D" })
							  .set<Transform2DComponent>({ transform })
							  .set_name(name.ascii().get_data());

		if (const Node2D *parent = Object::cast_to<Node2D>(multi_mesh_instance->get_parent()); parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
		}
		ObjectIDStorage::add(multi_mesh_instance, canvas_item);

		return entity;
	}


	static flecs::entity create_multi_mesh_instance(
			const flecs::world &world,
			const Transform2D &transform,
			const uint32_t index,
			const String &name) {
		return world.entity()
				.set<MultiMeshInstanceComponent>({ index })
				.set<Transform2DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static Vector<flecs::entity> create_multi_mesh_instances(
			const flecs::world &world,
			const Vector<Transform2D>& transform,
			const flecs::entity &multi_mesh) {
		Vector<flecs::entity> instances;

		const auto &[multi_mesh_id, instance_count] = multi_mesh.get<MultiMeshComponent>();
		for (uint32_t i = 0; i < instance_count; ++i) {
			instances.append(create_multi_mesh_instance(world, transform[i], i, multi_mesh.name() + " - Instance: #" + String::num_int64(i)));
		}
		return instances;
	}

	static flecs::entity create_camera_2d(const flecs::world &world, const RID &camera_id, const Transform2D &transform, const String &name) {
		return world.entity()
			.set<CameraComponent>({ camera_id })
			.set<Transform2DComponent>({ transform })
			.set_name(name.ascii().get_data());
	}

	static flecs::entity create_camera_2d(const flecs::world &world, Camera2D *camera_2d) {
		if (camera_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID camera_id = RS::get_singleton()->camera_create();
		if (!camera_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity camera = world.entity()
							  .set<Transform2DComponent>({ camera_2d->get_transform() })
							  .set<CameraComponent>({ camera_id })
							  .set_name(String(camera_2d->get_name()).ascii().get_data());
		if (camera_2d->is_inside_tree()) {
			camera_2d->get_parent()->remove_child(camera_2d);
			camera_2d->queue_free();
		}
		return camera;
	}

	static flecs::entity create_directional_light(const flecs::world &world, const RID &light_id, const Transform2D &transform,  const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		if (!world.has<World2DComponent>()) {
			ERR_FAIL_COND_V(!world.has<World2DComponent>(),flecs::entity());
		}
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world.get<World2DComponent>().canvas_id);
		RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);
		const flecs::entity directional_light = world.entity()
											.set<DirectionalLight2DComponent>({ light_id })
											.set<Transform2DComponent>({ transform })
											.set_name(name.ascii().get_data());
		return directional_light;
	}

	static flecs::entity create_directional_light(const flecs::world &world, const Transform2D &transform, const String &name) {
		const RID directional_light_id = RS::get_singleton()->canvas_light_create();
		return create_directional_light(world, directional_light_id, transform, name);
	}

	static flecs::entity create_directional_light(const flecs::world &world, DirectionalLight2D *directional_light) {
		if (directional_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID light_id = RS::get_singleton()->canvas_light_create();
		//best attempt to copy settings
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, directional_light->get_canvas());
		RS::get_singleton()->canvas_item_set_light_mask(light_id, directional_light->get_light_mask());
		RS::get_singleton()->canvas_light_set_color(light_id, directional_light->get_color());
		RS::get_singleton()->canvas_light_set_energy(light_id, directional_light->get_energy());
		RS::get_singleton()->canvas_light_set_enabled(light_id, directional_light->is_enabled());
		RS::get_singleton()->canvas_light_set_z_range(light_id, directional_light->get_z_range_min(), directional_light->get_z_range_max());
		RS::get_singleton()->canvas_light_set_layer_range(light_id, directional_light->get_layer_range_min(), directional_light->get_layer_range_max());
		RS::get_singleton()->canvas_light_set_item_cull_mask(light_id, directional_light->get_item_cull_mask());
		RS::get_singleton()->canvas_light_set_item_shadow_cull_mask(light_id, directional_light->get_item_shadow_cull_mask());
		RS::get_singleton()->canvas_light_set_directional_distance(light_id, directional_light->get_max_distance());
		RS::get_singleton()->canvas_light_set_blend_mode(light_id, static_cast<RS::CanvasLightBlendMode>(directional_light->get_blend_mode()));
		RS::get_singleton()->canvas_light_set_shadow_enabled(light_id, directional_light->is_shadow_enabled());
		RS::get_singleton()->canvas_light_set_shadow_filter(light_id, static_cast<RS::CanvasLightShadowFilter>(directional_light->get_shadow_filter()));
		RS::get_singleton()->canvas_light_set_shadow_color(light_id, directional_light->get_shadow_color());
		RS::get_singleton()->canvas_light_set_shadow_smooth(light_id, directional_light->get_shadow_smooth());
		RS::get_singleton()->canvas_light_set_transform(light_id, directional_light->get_transform());
		RS::get_singleton()->canvas_light_set_interpolated(light_id, directional_light->is_physics_interpolated());
		RS::get_singleton()->canvas_light_reset_physics_interpolation(light_id);
		RS::get_singleton()->canvas_light_transform_physics_interpolation(light_id, directional_light->get_transform());
		RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);

		if (const Node2D *parent = Object::cast_to<Node2D>(directional_light->get_parent()); parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(light_id, parent->get_canvas_item());
		}

		auto &entity = world.entity()
							  .set<DirectionalLight2DComponent>({ light_id })
							  .set<Transform2DComponent>({ directional_light->get_transform() })
							  .set_name(String(directional_light->get_name()).ascii().get_data());
		if (directional_light->is_inside_tree()) {
			directional_light->get_parent()->remove_child(directional_light);
			directional_light->queue_free();
		}
		return entity;
	}

	static flecs::entity create_point_light(const flecs::world &world,  const Transform2D &transform, const String &name) {
		const RID light_id = RS::get_singleton()->canvas_light_create();
		if (!light_id.is_valid()) {
			ERR_FAIL_COND_V(!light_id.is_valid(),flecs::entity());
		}
		return create_point_light(world, light_id, transform, name);
	}

	static flecs::entity create_point_light(const flecs::world &world,  const RID &light_id, const Transform2D &transform, const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_COND_V(!light_id.is_valid(),flecs::entity());
		}
		if (!world.has<World2DComponent>()) {
			ERR_FAIL_COND_V(!world.has<World2DComponent>(),flecs::entity());

		}
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world.get<World2DComponent>().canvas_id);
		RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);
		const flecs::entity directional_light = world.entity()
										 .set<DirectionalLight2DComponent>({ light_id })
										 .set<Transform2DComponent>({ transform })
										 .set_name(name.ascii().get_data());
		return directional_light;
	}

	static flecs::entity create_point_light(const flecs::world &world, PointLight2D *point_light) {
		if (point_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const RID light_id = RS::get_singleton()->canvas_light_create();
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}

		//best attempt to copy settings
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, point_light->get_canvas());
		RS::get_singleton()->canvas_item_set_light_mask(light_id, point_light->get_light_mask());
		RS::get_singleton()->canvas_light_set_color(light_id, point_light->get_color());
		RS::get_singleton()->canvas_light_set_energy(light_id, point_light->get_energy());
		RS::get_singleton()->canvas_light_set_enabled(light_id, point_light->is_enabled());
		RS::get_singleton()->canvas_light_set_z_range(light_id, point_light->get_z_range_min(), point_light->get_z_range_max());
		RS::get_singleton()->canvas_light_set_layer_range(light_id, point_light->get_layer_range_min(), point_light->get_layer_range_max());
		RS::get_singleton()->canvas_light_set_item_cull_mask(light_id, point_light->get_item_cull_mask());
		RS::get_singleton()->canvas_light_set_item_shadow_cull_mask(light_id, point_light->get_item_shadow_cull_mask());
		RS::get_singleton()->canvas_light_set_blend_mode(light_id, static_cast<RenderingServer::CanvasLightBlendMode>(point_light->get_blend_mode()));
		RS::get_singleton()->canvas_light_set_shadow_enabled(light_id, point_light->is_shadow_enabled());
		RS::get_singleton()->canvas_light_set_shadow_filter(light_id, static_cast<RenderingServer::CanvasLightShadowFilter>(point_light->get_shadow_filter()));
		RS::get_singleton()->canvas_light_set_shadow_color(light_id, point_light->get_shadow_color());
		RS::get_singleton()->canvas_light_set_shadow_smooth(light_id, point_light->get_shadow_smooth());
		RS::get_singleton()->canvas_light_set_transform(light_id, point_light->get_transform());
		RS::get_singleton()->canvas_light_set_interpolated(light_id, point_light->is_physics_interpolated());
		RS::get_singleton()->canvas_light_reset_physics_interpolation(light_id);
		RS::get_singleton()->canvas_light_transform_physics_interpolation(light_id, point_light->get_transform());
		RS::get_singleton()->canvas_light_set_mode(light_id, RenderingServer::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);

		if (const Node2D *parent = Object::cast_to<Node2D>(point_light->get_parent()); parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(light_id, parent->get_canvas_item());
		}
		const flecs::entity entity = world.entity()
							  .set<PointLightComponent>({ light_id })
							  .set<Transform2DComponent>({ point_light->get_transform() })
							  .set_name(String(point_light->get_name()).ascii().get_data());

		if (point_light->is_inside_tree()) {
			point_light->get_parent()->remove_child(point_light);
			point_light->queue_free();
		}
		
		return entity;
	}


	static flecs::entity create_canvas_item(const flecs::world &world, CanvasItem *canvas_item) {
		if (canvas_item == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		const flecs::entity entity = world.entity(String(canvas_item->get_name()).ascii().get_data())
							  .set<CanvasItemComponent>({ canvas_item->get_canvas_item(), canvas_item->get_class() })
							  .set<Transform2DComponent>({canvas_item->get_transform()});

		if (const Node2D *parent = Object::cast_to<Node2D>(canvas_item->get_parent()); parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item->get_canvas_item(), parent->get_canvas_item());
		}
		ObjectIDStorage::add(canvas_item, canvas_item->get_canvas_item());

		return entity;
	}

	static flecs::entity create_canvas_item(const flecs::world &world, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name) {
		return world.entity()
				.set<CanvasItemComponent>({ canvas_item_id, class_name })
				.set<Transform2DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}



	static flecs::entity create_skeleton(const flecs::world &world, const RID &skeleton_id, const String &name) {
		return world.entity()
				.set<SkeletonComponent>({ skeleton_id })
				.set_name(name.ascii().get_data());
	}

	static flecs::entity create_skeleton(const flecs::world &world, Skeleton2D *skeleton_2d) {
		const RID skeleton_id = RS::get_singleton()->skeleton_create();
		if (skeleton_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		if (!skeleton_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_2d->get_bone_count(), false);
		for (int i = 0; i < skeleton_2d->get_bone_count(); ++i) {
			RS::get_singleton()->skeleton_bone_set_transform_2d(skeleton_id, i, skeleton_2d->get_bone(i)->get_transform());
		}
		if (skeleton_2d->is_inside_tree())
		{
			skeleton_2d->get_parent()->remove_child(skeleton_2d);
			skeleton_2d->queue_free();
		}
		return world.entity(String(skeleton_2d->get_name()).ascii().get_data())
				.set<SkeletonComponent>({ skeleton_id });
	}

	static flecs::entity create_light_occluder(const flecs::world &world, LightOccluder2D *light_occluder)
	{
		const String name = light_occluder->get_name();
		const RID light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
		RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, light_occluder->get_canvas());
		RS::get_singleton()->canvas_light_occluder_set_as_sdf_collision(light_occluder_id, light_occluder->is_set_as_sdf_collision());
		RS::get_singleton()->canvas_light_occluder_set_light_mask(light_occluder_id, light_occluder->get_occluder_light_mask());
		if (const auto polygon = light_occluder->get_occluder_polygon(); polygon.is_valid() && polygon != nullptr)
		{
			RS::get_singleton()->canvas_light_occluder_set_polygon(light_occluder_id, polygon->get_rid());
		}
		RS::get_singleton()->canvas_light_occluder_set_enabled(light_occluder_id, light_occluder->is_enabled());
		RS::get_singleton()->canvas_light_occluder_transform_physics_interpolation(light_occluder_id, light_occluder->get_transform());

		if (light_occluder->is_inside_tree()) {
			light_occluder->get_parent()->remove_child(light_occluder);
			light_occluder->call_deferred("queue_free");
		}
		const flecs::entity entity = world.entity()
								.set<LightOccluderComponent>({})
								.set<Transform2DComponent>({ light_occluder->get_transform() })
								.set_name(name.ascii().get_data());

		return entity;
	}

	static flecs::entity create_light_occluder(const flecs::world &world, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name ) {
		const auto entity = world.entity()
							  .set<LightOccluderComponent>({ light_occluder_id })
							  .set<Transform2DComponent>({ transform })
							  .set_name(name.ascii().get_data());
		RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, canvas_id);
		return entity;
	}

	static flecs::entity create_light_occluder(const flecs::world &world, const Transform2D &transform, const String &name) {
		const auto light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
		if (!world.has<World2DComponent>()
			&& world.get<World2DComponent>().is_valid()
			&& !world.get<World2DComponent>().is_null()) {
			ERR_FAIL_COND_V(!world.has<World2DComponent>(), flecs::entity());
		}
		return create_light_occluder(world, light_occluder_id, transform,world.get<World2DComponent>().canvas_id, name);
	}

	static flecs::entity create_gpu_particles_2d(const flecs::world &world, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform,  const String& name) {
		const auto entity = world.entity()
		.set<ParticlesComponent>({ particles_id })
		.set<Transform2DComponent>({ transform })
		.set_name(name.ascii().get_data());
		if (!world.has<World2DComponent>()
				&& world.get<World2DComponent>().is_valid()
				&& !world.get<World2DComponent>().is_null()) {
			ERR_FAIL_COND_V(!world.has<World2DComponent>(), flecs::entity());
				}

		RS::get_singleton()->canvas_item_set_parent(canvas_item_id, world.get<World2DComponent>().canvas_id);
		RS::get_singleton()->canvas_item_add_particles(canvas_item_id, particles_id, texture_id);
		return entity;
	}

	static flecs::entity create_gpu_particles_2d(const flecs::world &world, GPUParticles2D* gpu_particles, uint32_t count = 0, const uint32_t max_depth = 10000) {
		count++;
		if (gpu_particles == nullptr) {
			ERR_FAIL_COND_V(gpu_particles == nullptr, flecs::entity());
		}
		if (count > max_depth) {
			ERR_FAIL_COND_V(count <= max_depth, flecs::entity());
		}
		const RID new_particles_id = RS::get_singleton()->particles_create();
		const RID canvas_item_id = RS::get_singleton()->canvas_item_create();

		//Setting parameters manually, copied from GPUParticles2D class
		RS::get_singleton()->particles_set_emitting(new_particles_id,gpu_particles->is_emitting());
		RS::get_singleton()->particles_set_amount(new_particles_id, gpu_particles->get_amount());
		RS::get_singleton()->particles_set_lifetime(new_particles_id, gpu_particles->get_lifetime());
		RS::get_singleton()->particles_set_one_shot(new_particles_id,gpu_particles->get_one_shot());
		RS::get_singleton()->particles_set_pre_process_time(new_particles_id,gpu_particles->get_pre_process_time());
		RS::get_singleton()->particles_set_explosiveness_ratio(new_particles_id,gpu_particles->get_explosiveness_ratio());
		RS::get_singleton()->particles_set_randomness_ratio(new_particles_id,gpu_particles->get_randomness_ratio());


		//taken from set_visibility_rect in GPUParticles2D
		const Rect2 visibility_rect = gpu_particles->get_visibility_rect();
		AABB aabb;
		aabb.position.x = visibility_rect.position.x;
		aabb.position.y = visibility_rect.position.y;
		aabb.size.x = visibility_rect.size.x;
		aabb.size.y = visibility_rect.size.y;
		RS::get_singleton()->particles_set_custom_aabb(new_particles_id, aabb);
		// end of set_visibility_rect


		RS::get_singleton()->particles_set_use_local_coordinates(new_particles_id,gpu_particles->get_use_local_coordinates());

		//check if we can set a process_material, if not, skip and fail silently
		if (const Ref<Material> process_material = gpu_particles->get_process_material(); process_material.is_valid() && !process_material.is_null()) {
			RS::get_singleton()->particles_set_process_material(new_particles_id,process_material->get_rid());
		}

		RS::get_singleton()->particles_set_speed_scale(new_particles_id,gpu_particles->get_speed_scale());
		RS::get_singleton()->particles_set_collision_base_size(new_particles_id,gpu_particles->get_collision_base_size());
		RS::get_singleton()->particles_set_trails(new_particles_id,gpu_particles->is_trail_enabled(), static_cast<float>(gpu_particles->get_trail_lifetime()));

		const RID mesh_id = RS::get_singleton()->mesh_create();
		const Ref<Texture2D> texture = gpu_particles->get_texture();
		// takes from GPUParticles2D::_notification
		if (gpu_particles->is_trail_enabled()){
			PackedVector2Array points;
			PackedVector2Array uvs;
			PackedInt32Array bone_indices;
			PackedFloat32Array bone_weights;
			PackedInt32Array indices;
			const int trail_sections = gpu_particles->get_trail_sections();
			const int trail_section_subdivisions = gpu_particles->get_trail_section_subdivisions();
			const Vector2 size = texture.is_valid()
									&& !texture.is_null()
									? texture->get_size()
									: Vector2();

			const int total_segments = trail_sections * trail_section_subdivisions;
			real_t depth = size.height * trail_sections;

			for (int j = 0; j <= total_segments; j++) {
				real_t v = j;
				v /= total_segments;

				real_t y = depth * v;
				//y = (depth * 0.5) - y;

				int bone = j / trail_section_subdivisions;
				real_t blend = 1.0 - real_t(j % trail_section_subdivisions) / real_t(trail_section_subdivisions);

				real_t s = size.width;

				points.push_back(Vector2(-s * 0.5, 0));
				points.push_back(Vector2(+s * 0.5, 0));

				uvs.push_back(Vector2(0, v));
				uvs.push_back(Vector2(1, v));

				for (int i = 0; i < 2; i++) {
					bone_indices.push_back(bone);
					bone_indices.push_back(MIN(trail_sections, bone + 1));
					bone_indices.push_back(0);
					bone_indices.push_back(0);

					bone_weights.push_back(blend);
					bone_weights.push_back(1.0 - blend);
					bone_weights.push_back(0);
					bone_weights.push_back(0);
				}

				if (j > 0) {
					int base = j * 2 - 2;
					indices.push_back(base + 0);
					indices.push_back(base + 1);
					indices.push_back(base + 2);

					indices.push_back(base + 1);
					indices.push_back(base + 3);
					indices.push_back(base + 2);
				}
				}

				Array arr;
				arr.resize(RS::ARRAY_MAX);
				arr[RS::ARRAY_VERTEX] = points;
				arr[RS::ARRAY_TEX_UV] = uvs;
				arr[RS::ARRAY_BONES] = bone_indices;
				arr[RS::ARRAY_WEIGHTS] = bone_weights;
				arr[RS::ARRAY_INDEX] = indices;

				RS::get_singleton()->mesh_add_surface_from_arrays(mesh_id, RS::PRIMITIVE_TRIANGLES, arr, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);

				Vector<Transform3D> xforms;
				for (int i = 0; i <= trail_sections; i++) {
					Transform3D xform;
					/*
					xform.origin.y = depth / 2.0 - size.height * real_t(i);
					xform.origin.y = -xform.origin.y; //bind is an inverse transform, so negate y */
					xforms.push_back(xform);
				}

				RS::get_singleton()->particles_set_trail_bind_poses(new_particles_id, xforms);
		}
		//end segment

		RS::get_singleton()->particles_set_interp_to_end(new_particles_id,gpu_particles->get_interp_to_end());
		RS::get_singleton()->particles_set_fixed_fps(new_particles_id,gpu_particles->get_fixed_fps());
		RS::get_singleton()->particles_set_fractional_delta(new_particles_id,gpu_particles->get_fractional_delta());
		RS::get_singleton()->particles_set_interpolate(new_particles_id,gpu_particles->get_interpolate());
		RS::get_singleton()->particles_set_draw_order(new_particles_id, static_cast<RS::ParticlesDrawOrder>(gpu_particles->get_draw_order()));

		RID texture_id = RID();
		if (texture.is_valid() && texture.is_null()) {
			texture_id = texture->get_rid();
		}
		RS::get_singleton()->particles_set_amount_ratio(new_particles_id, gpu_particles->get_amount_ratio());

		//cannot be ported due to it being directly associated with the Node2D type
		//PackedStringArray get_configuration_warnings() const override;

		RS::get_singleton()->particles_set_seed(new_particles_id, gpu_particles->get_seed());
		flecs::entity new_gpu_particle_entity = create_gpu_particles_2d(world, canvas_item_id, new_particles_id,texture_id ,gpu_particles->get_transform(), String(gpu_particles->get_name()));

		//copied from GPUParticles2D::_attach_sub_emitter
		const NodePath sub_emitter_path = gpu_particles->get_sub_emitter();
		if (Node *n = gpu_particles->get_node_or_null(sub_emitter_path)) {
			if (GPUParticles2D *sen = Object::cast_to<GPUParticles2D>(n); sen && sen != gpu_particles) {
				flecs::entity particle_child = create_gpu_particles_2d(world, sen, count);

				RS::get_singleton()->particles_set_subemitter(new_particles_id, particle_child.get<ParticlesComponent>().particles_id);
				particle_child.child(new_gpu_particle_entity);
			}
		}
		// end copy

		//destroy scene instance
		gpu_particles->get_parent()->remove_child(gpu_particles);
		gpu_particles->call_deferred("queue_free");

		return new_gpu_particle_entity;
	}
};
