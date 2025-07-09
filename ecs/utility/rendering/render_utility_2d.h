#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"  
#include "../../components/rendering/rendering_components.h"  
#include "scene/resources/mesh.h"
#include "servers/rendering_server.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/2d/skeleton_2d.h"
#include <scene/2d/mesh_instance_2d.h>
#include <scene/2d/light_2d.h>
#include "../../components/transform_2d_component.h"
#include "scene/main/canvas_item.h"
#include <scene/main/window.h>
#include <scene/2d/light_occluder_2d.h>



class RenderUtility2D
{
private:
	RenderUtility2D() = default; // Prevent instantiation
	RenderUtility2D(const RenderUtility2D &) = delete; // Prevent copy
	RenderUtility2D &operator=(const RenderUtility2D &) = delete; // Prevent assignment
	RenderUtility2D(RenderUtility2D &&) = delete; // Prevent move
	RenderUtility2D &operator=(RenderUtility2D &&) = delete; // Prevent move assignment
public:
	static inline flecs::entity CreateMeshInstance(flecs::world &world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id) {
		Vector<RID> material_ids;
		auto surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
		for (int i = 0; i < surface_count; ++i) {
			RID material_id = RS::get_singleton()->mesh_surface_get_material(mesh_id, i);
			if (material_id.is_valid()) {
				material_ids.push_back(material_id);
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		auto canvas_item = RS::get_singleton()->canvas_item_create();


		auto entity = world.entity()
							  .set<MeshComponent>({ mesh_id, material_ids })
							  .set<Transform2DComponent>({ transform }) // Default transform
							  .set<CanvasItemComponent>({ canvas_item, "MeshInstance2D" })
							  .set_name(name.ascii().get_data());
		RS::get_singleton()->canvas_item_add_mesh(canvas_item, mesh_id);
		RS::get_singleton()->canvas_item_set_parent(canvas_item, canvas_id);
		RS::get_singleton()->canvas_item_set_material(canvas_item, material_ids[0]);
		return entity;
	}

	static inline flecs::entity CreateMeshInstance(flecs::world &world, const Transform2D &transform, const RID &canvas_id, const String &name) {
		Vector<RID> material_ids;
		auto mesh_id = RS::get_singleton()->mesh_create();
		return CreateMeshInstance(world, mesh_id, transform, name, canvas_id);
	}

	static inline flecs::entity CreateMeshInstance(flecs::world &world, MeshInstance2D *mesh_instance_2d) {
		Vector<RID> material_ids;
		auto mesh = mesh_instance_2d->get_mesh();
		auto canvas = mesh_instance_2d->get_canvas();
		auto canvas_item = mesh_instance_2d->get_canvas_item();
		for (int i = 0; i < mesh->get_surface_count(); ++i) {
			Ref<Material> material = mesh->surface_get_material(i);
			if (material.is_valid()) {
				material_ids.push_back(material->get_rid());
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		Node2D *parent = Object::cast_to<Node2D>(mesh_instance_2d->get_parent());
		if (parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
		}

		ObjectIDStorage::add(mesh_instance_2d, mesh_instance_2d->get_canvas_item());
		return world.entity()
				.set<MeshComponent>({ mesh->get_rid(), material_ids })
				.set<CanvasItemComponent>({ canvas_item, mesh_instance_2d->get_class_name() })
				.set<Transform2DComponent>({ mesh_instance_2d->get_transform() })
				.set_name(String(mesh_instance_2d->get_name()).ascii().get_data());
	}

	static inline flecs::entity CreateMultiMesh(flecs::world &world,
			const Transform2D &transform,
			const uint32_t size,
			const RID &mesh_id,
			const RID &canvas_id,
			const String &name,
			const RID& texture_id,
			const bool use_colors = false,
			const bool use_custom_data = false,
			const bool use_indirect = false) {
		RID multmesh_id = RS::get_singleton()->multimesh_create();
		RID canvas_item = RS::get_singleton()->canvas_item_create();
		RS::get_singleton()->multimesh_allocate_data(multmesh_id, size, RS::MULTIMESH_TRANSFORM_2D, use_colors, use_custom_data, use_indirect);
		RS::get_singleton()->canvas_item_add_multimesh(canvas_item, multmesh_id, texture_id);
		RS::get_singleton()->multimesh_set_mesh(multmesh_id, mesh_id);
		auto &entity = world.entity()
							   .set<MultiMeshComponent>({ multmesh_id, size })
							   .set<MeshComponent>({ mesh_id })
							   .set<CanvasItemComponent>({ canvas_item })
							   .set<Transform2DComponent>({ transform })
							   .set_name(name.ascii().get_data());

		return entity;
	}

	static inline flecs::entity CreateMultiMesh(flecs::world &world,
			const Transform2D &transform,
			MultiMeshInstance2D *multimesh_instance,
			const String &name) {
		RID multmesh_id = multimesh_instance->get_multimesh()->get_rid();
		Ref<Mesh> mesh = multimesh_instance->get_multimesh()->get_mesh();
		uint32_t size = multimesh_instance->get_multimesh()->get_instance_count();
		RID canvas_item = multimesh_instance->get_canvas_item();
		auto &entity = world.entity()
							   .set<MultiMeshComponent>({ multmesh_id, size })
							   .set<MeshComponent>({ mesh->get_rid() })
							   .set<CanvasItemComponent>({ canvas_item, "MultiMesh2D" })
							   .set<Transform2DComponent>({ transform })
							   .set_name(name.ascii().get_data());

		Node2D *parent = Object::cast_to<Node2D>(multimesh_instance->get_parent());
		if (parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
		}
		ObjectIDStorage::add(multimesh_instance, canvas_item);

		return entity;
	}

	static inline flecs::entity CreateMultiMeshInstance(
			flecs::world &world,
			const Transform2D &transform,
			const uint32_t index,
			const RID &multimesh_id,
			const String &name) {
		return world.entity()
				.set<MultiMeshInstanceComponent>({ index })
				.set<Transform2DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static inline Vector<flecs::entity> CreateMultimeshInstances(
			flecs::world &world,
			const Transform2D (&transform)[],
			flecs::entity &multimesh) {
		Vector<flecs::entity> instances;

		auto &multimesh_component = multimesh.get<MultiMeshComponent>();

		const uint32_t instance_count = RS::get_singleton()->multimesh_get_instance_count(multimesh_component.multimesh_id);
		
		for (uint32_t i = 0; i < instance_count; ++i) {
			instances.append(CreateMultiMeshInstance(world, transform[i], i, multimesh_component.multimesh_id, multimesh.name() + " - Instance: #" + String::num_int64(i)));
		}
		return instances;
	}

	static inline flecs::entity CreateCamera2D(flecs::world &world, const RID &camera_id, const Transform2D &transform, const String &name) {
		return world.entity()
			.set<CameraComponent>({ camera_id })
			.set<Transform2DComponent>({ transform })
			.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateCamera2D(flecs::world &world, Camera2D *camera_2d) {
		if (camera_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera_id = RS::get_singleton()->camera_create();
		if (!camera_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera = world.entity()
							  .set<Transform2DComponent>({ camera_2d->get_transform() })
							  .set<CameraComponent>({ camera_id })
							  .set_name(String(camera_2d->get_name()).ascii().get_data());
		if (camera_2d->is_inside_tree()) {
			camera_2d->get_parent()->remove_child(camera_2d);
			camera_2d->queue_free();
		}
		return camera;
	}

	static inline flecs::entity CreateDirectional2DLight(flecs::world &world, const RID &light_id, const Transform2D &transform, const RID &canvas_id, const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, canvas_id);
		RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);
		auto directional_light = world.entity()
											.set<DirectionalLight2DComponent>({ light_id })
											.set<Transform2DComponent>({ transform })
											.set_name(name.ascii().get_data());
		return directional_light;
	}

	static inline flecs::entity CreateDirectional2DLight(flecs::world &world, const RID& canvas_id, const Transform2D &transform, const String &name) {
		auto diretional_light_id = RS::get_singleton()->canvas_light_create();
		return CreateDirectional2DLight(world, diretional_light_id, transform, canvas_id, name);
	}

	static inline flecs::entity CreateDirectional2DLight(flecs::world &world, DirectionalLight2D *directional_light) {
		if (directional_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto light_id = RS::get_singleton()->canvas_light_create();
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

		Node2D *parent = Object::cast_to<Node2D>(directional_light->get_parent());
		if (parent != nullptr) {
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

	static inline flecs::entity CreatePointLight(flecs::world &world, const RID &canvas_id, const Transform2D &transform, const String &name) {
		RID light_id = RS::get_singleton()->canvas_light_create();
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return CreatePointLight(world, light_id, transform, name);
	}

	static inline flecs::entity CreatePointLight(flecs::world &world, const RID &canvas_id, const RID &light_id, const Transform2D &transform, const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		RS::get_singleton()->canvas_light_attach_to_canvas(light_id, canvas_id);
		RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);
		auto directional_light = world.entity()
										 .set<DirectionalLight2DComponent>({ light_id })
										 .set<Transform2DComponent>({ transform })
										 .set_name(name.ascii().get_data());
		return directional_light;
	}

	static inline flecs::entity CreatePointLight(flecs::world &world, PointLight2D *point_light) {
		if (point_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto light_id = RS::get_singleton()->canvas_light_create();
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

		Node2D *parent = Object::cast_to<Node2D>(point_light->get_parent());
		if (parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(light_id, parent->get_canvas_item());
		}
		auto &entity = world.entity()
							   .set<PointLightComponent>({ light_id })
							   .set<Transform2DComponent>({ point_light->get_transform() })
							   .set_name(String(point_light->get_name()).ascii().get_data());

		if (point_light->is_inside_tree()) {
			point_light->get_parent()->remove_child(point_light);
			point_light->queue_free();
		}

		
		return entity;
	}

	static inline flecs::entity CreateCanvas(flecs::world &world, const RID &canvas_id, const String &name) {
		return world.entity()
				.set<CanvasComponent>({ canvas_id })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateCanvasItem(flecs::world &world, CanvasItem *canvas_item) {
		if (canvas_item == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(String(canvas_item->get_name()).ascii().get_data())
							   .set<CanvasItemComponent>({ canvas_item->get_canvas_item(), canvas_item->get_class() })
							   .set<Transform2DComponent>({canvas_item->get_transform()});

		Node2D *parent = Object::cast_to<Node2D>(canvas_item->get_parent());
		if (parent != nullptr) {
			RS::get_singleton()->canvas_item_set_parent(canvas_item->get_canvas_item(), parent->get_canvas_item());
		}
		ObjectIDStorage::add(canvas_item, canvas_item->get_canvas_item());

		return entity;
	}

	static inline flecs::entity CreateCanvasItem(flecs::world &world, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name) {
		return world.entity()
				.set<CanvasItemComponent>({ canvas_item_id, class_name })
				.set<Transform2DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateCanvas(flecs::world &world, Camera2D *camera) {
		if (camera == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(String(String(camera->get_name()) + " - Canvas #" + String::num_int64(world.count<CanvasComponent>())).utf8().get_data())
							   .set<CanvasComponent>({ camera->get_canvas() });
		return entity;
	}

	static inline flecs::entity CreateSkeleton(flecs::world &world, const RID &skeleton_id, const String &name) {
		return world.entity()
				.set<SkeletonComponent>({ skeleton_id })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateSkeleton(flecs::world &world, Skeleton2D *skeleton_2d) {
		auto skeleton_id = RS::get_singleton()->skeleton_create();
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

	static inline flecs::entity CreateLightOccluder(flecs::world &world, LightOccluder2D *light_occluder)
	{
		auto name = light_occluder->get_name();
		auto light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
		RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, light_occluder->get_canvas());
		RS::get_singleton()->canvas_light_occluder_set_as_sdf_collision(light_occluder_id, light_occluder->is_set_as_sdf_collision());
		RS::get_singleton()->canvas_light_occluder_set_light_mask(light_occluder_id, light_occluder->get_occluder_light_mask());
		auto polygon = light_occluder->get_occluder_polygon();
		if (polygon.is_valid() && polygon != nullptr)
		{
			RS::get_singleton()->canvas_light_occluder_set_polygon(light_occluder_id, polygon->get_rid());
		}
		RS::get_singleton()->canvas_light_occluder_set_enabled(light_occluder_id, light_occluder->is_enabled());
		RS::get_singleton()->canvas_light_occluder_transform_physics_interpolation(light_occluder_id, light_occluder->get_transform());


		auto entity = world.entity()
								.set<LightOccluderComponent>({})
								.set<Transform2DComponent>({ light_occluder->get_transform() })
								.set_name(String(name).ascii().get_data());

		return entity;
	}

	static inline flecs::entity CreateLightOccluder(flecs::world &world, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name ) {
		auto entity = world.entity()
							  .set<LightOccluderComponent>({ light_occluder_id })
							  .set<Transform2DComponent>({ transform })
							  .set_name(name.ascii().get_data());
		RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, canvas_id);
		return entity;
	}

	static inline flecs::entity CreateLightOccluder(flecs::world &world, const Transform2D &transform, const RID &canvas_id, const String &name) {
		auto light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
		return CreateLightOccluder(world, light_occluder_id, transform,canvas_id, name);
	}
};
