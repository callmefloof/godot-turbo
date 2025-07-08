#pragma once  
#include "../../../thirdparty/flecs/distr/flecs.h"  
#include "../../components/rendering/rendering_components.h"  
#include "scene/resources/mesh.h"  
#include "servers/rendering_server.h"
#include "scene/3d/gpu_particles_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/resources/multimesh.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/3d/reflection_probe.h"
#include "scene/3d/world_environment.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/reflection_probe.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/main/canvas_item.h"
#include "../object_id_storage.h"
#include <core/templates/rid.h>
#include <core/templates/vector.h>
#include <core/templates/rid_owner.h>
#include <scene/main/viewport.h>
#include <cassert>
#include <servers/rendering/renderer_rd/renderer_scene_render_rd.h>
#include "../../components/transform_3d_component.h"
#include "../../components/transform_2d_component.h"

#include <scene/main/window.h>
#include <scene/3d/voxel_gi.h>
#include <scene/2d/light_2d.h>


using namespace godot_turbo::components::rendering;  
using namespace godot_turbo::components;
namespace godot_turbo::utility::rendering  
{  
class RenderUtility {
private:
	RenderUtility() = default; // Prevent instantiation
	RenderUtility(const RenderUtility &) = delete; // Prevent copy
	RenderUtility &operator=(const RenderUtility &) = delete; // Prevent assignment
	RenderUtility(RenderUtility &&) = delete; // Prevent move
	RenderUtility &operator=(RenderUtility &&) = delete; // Prevent move assignment

public:
	static inline flecs::entity_t CreateMeshInstance(flecs::world &world, const RID& mesh_id, const String& name, const RID& scenario_id) {  
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
		return world.entity(name)
			.set<MeshComponent>({ mesh_id, material_ids })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(mesh_id, scenario_id) });
	}

	static inline flecs::entity_t CreateMeshInstance(flecs::world &world, const RID &scenario_id, const String &name) {
		Vector<RID> material_ids;
		auto mesh_id = RS::get_singleton()->mesh_create();
		auto surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
		for (int i = 0; i < surface_count; ++i) {
			auto material_id = RS::get_singleton()->mesh_surface_get_material(mesh_id, i);
			if (material_id.is_valid()) {
				material_ids.push_back(material_id);
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		return world.entity(name)
				.set<MeshComponent>({ mesh_id, material_ids })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(mesh_id, scenario_id) });
	}

	static inline flecs::entity_t CreateMeshInstance(flecs::world& world, MeshInstance3D* mesh_instance_3d)
	{
		Vector<RID> material_ids;
		auto mesh = mesh_instance_3d->get_mesh();
		auto base = mesh_instance_3d->get_base();
		auto instance = mesh_instance_3d->get_instance();
		for (int i = 0; i < mesh->get_surface_count(); ++i) {
			Ref<Material> material = mesh->surface_get_material(i);
			if (material.is_valid()) {
				material_ids.push_back(material->get_rid());
			} else {
				material_ids.push_back(RID()); // Use an empty RID if no material is set
			}
		}
		ObjectIDStorage::add(mesh_instance_3d,mesh_instance_3d->get_instance());
		return world.entity(mesh->get_name())
				.set<MeshComponent>({ base, material_ids })
				.set<RenderInstanceComponent>({ instance });
	}

	static inline flecs::entity_t CreateMultiMesh(flecs::world &world,
			const Transform3D &transform,
			const uint32_t size,
			const RID &mesh_id,
			const RID &scenario_id,
			const String &name) {
		RID multmesh_id = RS::get_singleton()->multimesh_create();
		auto& entity = world.entity(name)
							  .set<MultiMeshComponent>({ multmesh_id, size })
							  .set<MeshComponent>({ mesh_id })
							  .set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(multmesh_id, scenario_id) })
							  .set<Transform3DComponent>({ transform });

		

		return entity;
	}

	static inline flecs::entity_t CreateMultiMesh(flecs::world &world,
		const Transform3D &transform,
		MultiMeshInstance3D* multimesh_instance,
		const String &name
		) {
		RID multmesh_id = multimesh_instance->get_multimesh()->get_rid();
		Ref<Mesh> mesh = multimesh_instance->get_multimesh()->get_mesh();
		uint32_t size = multimesh_instance->get_multimesh()->get_instance_count();
		RID instance_id = multimesh_instance->get_instance();
		auto& entity = world.entity(name)
								.set<MultiMeshComponent>({ multmesh_id, size })
								.set<MeshComponent>({ mesh->get_rid() })
								.set<RenderInstanceComponent>({ instance_id })
								.set<Transform3DComponent>({ transform });
		return entity;
	}	 

	static inline flecs::entity CreateMultiMeshInstance(
		flecs::world &world,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multimesh_id,
		const String &name) {  
		return world.entity(name)  
				.set<MultiMeshInstanceComponent>({ index })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(multimesh_id, world.get<MainScenarioComonent>().id) })
				.set<Transform3DComponent>({ transform });
	}

	static inline Vector<flecs::entity> CreateMultimeshInstances(
			flecs::world &world,
			const Transform3D (&transform)[],
			flecs::entity& multimesh) {
		Vector<flecs::entity> instances;
		
		auto&& multimesh_component = multimesh.get<MultiMeshComponent>();

		const uint32_t instance_count = RS::get_singleton()->multimesh_get_instance_count(multimesh_component.multimesh_id);

		for (uint32_t i = 0; i < instance_count; ++i) {
			instances.append(CreateMultiMeshInstance(world, transform[i], i, multimesh_component.multimesh_id, multimesh.name() + " - Instance: " + String::num_int64(i)));
		}
		return instances;
	}

	static inline flecs::entity CreateParticles(
		flecs::world &world,
		const Transform3D &transform,
		const RID &scenario_id,
		const String &name) {  
		RID particles_id = RS::get_singleton()->particles_create();
		return world.entity(name)  
				.set<ParticlesComponent>({ particles_id })  
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(particles_id, scenario_id) })
				.set<Transform3DComponent>({ transform });  
	}

	static inline flecs::entity CreateParticles(flecs::world &world, GPUParticles3D *gpu_particles_3d) {
		if (gpu_particles_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		if (gpu_particles_3d->is_inside_tree() && gpu_particles_3d->is_inside_world())
		{
			gpu_particles_3d->get_tree()->get_current_scene()->remove_child(gpu_particles_3d);

		}
		auto& particles = world.entity(gpu_particles_3d->get_name())
								 .set<ParticlesComponent>({ gpu_particles_3d->get_base() })
								 .set<RenderInstanceComponent>({ gpu_particles_3d->get_instance() })
								 .set<Transform3DComponent>({ gpu_particles_3d->get_transform() });
		ObjectIDStorage::add(gpu_particles_3d,gpu_particles_3d->get_instance());
		return particles;
	}

	static inline flecs::entity CreateReflectionProbe(flecs::world &world, const RID &probe_id, const Transform3D &transform, const String& name) {  
		return world.entity(name)  
				.set<ReflectionProbeComponent>({ probe_id })
				.set<Transform3DComponent>({transform})
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(probe_id, world.get<MainScenarioComonent>().id) });  
	}

	static inline flecs::entity CreateReflectionProbe(flecs::world &world, ReflectionProbe *reflection_probe) {

		auto& entity_probe = world.entity(reflection_probe->get_name())
									.set<ReflectionProbeComponent>({ reflection_probe->get_base() })
									.set<Transform3DComponent>({ reflection_probe->get_transform() })
									.set<RenderInstanceComponent>({ reflection_probe->get_instance() });
		ObjectIDStorage::add(reflection_probe, reflection_probe->get_instance());
		return entity_probe;
	}
	

	static inline flecs::entity CreateSkeleton(flecs::world &world, const RID &skeleton_id, const String &name) {  
		return world.entity(name)  
				.set<SkeletonComponent>({ skeleton_id });  
	}  

	static inline flecs::entity CreateSkeleton(flecs::world &world, Skeleton3D* skeleton_3d) {

		auto skeleton_id = RS::get_singleton()->skeleton_create();
		if (skeleton_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_3d->get_bone_count(), false);
		for (int i = 0; i < skeleton_3d->get_bone_count(); ++i) {
			RS::get_singleton()->skeleton_bone_set_transform(skeleton_id, i, skeleton_3d->get_bone_global_pose(i));
		}

		return world.entity(skeleton_3d->get_name())
				.set<SkeletonComponent>({ skeleton_id });
	}

	static inline flecs::entity CreateEnvironment(flecs::world &world, const RID &environment_id, const Transform3D &transform, const String &name) {  
		return world.entity(name)  
				.set<EnvironmentComponent>({ environment_id })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(environment_id, world.get<MainScenarioComonent>().id) })
				.set<Transform3DComponent>({ transform });  
	}  

	static inline flecs::entity CreateCamera3D(flecs::world &world, const RID &camera_id,const Transform3D &transform, const String& name) {  
		return world.entity(name)  
				.set<CameraComponent>({ camera_id })
				.set<Transform3DComponent>({ transform });  
	}

	static inline flecs::entity CreateCamera3D(flecs::world &world, const Transform3D &transform, const String &name) {
		RID camera_id = RS::get_singleton()->camera_create();
		if (!camera_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return world.entity(name)
				.set<CameraComponent>({ camera_id })
				.set<Transform3DComponent>({ transform });
	}

	static inline flecs::entity CreateCamera3D(flecs::world &world, Camera3D *camera_3d) {
		if (camera_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera = world.entity(camera_3d->get_name())
							  .set<CameraComponent>({ camera_3d->get_camera() })
							  .set<Transform3DComponent>({ camera_3d->get_transform() });
		ObjectIDStorage::add(camera_3d, camera_3d->get_camera());
		if (camera_3d->get_compositor().is_null() || camera_3d->get_compositor().is_valid()) {
			auto compositor_id = camera_3d->get_compositor()->get_rid();
			auto compositor_entity = CreateCompositor(world, compositor_id, camera_3d->get_compositor()->get_name());
			camera.child(compositor_entity);
		};

		return camera;
	}



	static inline flecs::entity CreateCamera2D(flecs::world &world, const RID &camera_id, const Transform2D &transform, const String &name) {
		return world.entity(name)
				.set<CameraComponent>({ camera_id })
				.set<Transform2DComponent>({ transform });
	}

	static inline flecs::entity CreateCamera2D(flecs::world &world, Camera2D *camera_2d) {
		if (camera_2d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera_id = RS::get_singleton()->camera_create();
		if (!camera_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera = world.entity(camera_2d->get_name())
			.set<Transform2DComponent>({ camera_2d->get_transform() })
			.set<CameraComponent>({ camera_id });
		if (camera_2d->is_inside_tree()) {
			camera_2d->get_tree()->get_current_scene()->remove_child(camera_2d);
			camera_2d->queue_free();
		}
		return camera;
	}

	static inline flecs::entity CreateCompositor(flecs::world &world, const RID &compositor_id, const String& name) {
		return world.entity(name)
				.set<CompositorComponent>({ compositor_id });
	}


	static inline flecs::entity CreateCompositor(flecs::world &world, Compositor *compositor) {
		if (compositor == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto compositor_id = compositor->get_rid();
		if (!compositor_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity(compositor->get_name())
							  .set<CompositorComponent>({ compositor_id });
		ObjectIDStorage::add(compositor, compositor_id);
		return entity;
	}


	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, const RID &light_id, const Transform3D& transform, const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return world.entity(name)
				.set<DirectionalLight3DComponent>({ light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, const Transform3D &transform, const String &name) {
		auto diretional_light_id = RS::get_singleton()->directional_light_create();
		return world.entity(name)
				.set<DirectionalLight3DComponent>({ diretional_light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(diretional_light_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, DirectionalLight3D *directional_light) {
		if (directional_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto& entity = world.entity(directional_light->get_name())
							  .set<DirectionalLight3DComponent>({ directional_light->get_base() })
							  .set<Transform3DComponent>({ directional_light->get_transform() })
							  .set<RenderInstanceComponent>({ directional_light->get_instance() });
		ObjectIDStorage::add(directional_light, directional_light->get_instance());
		return entity;
	}

	static inline flecs::entity CreateDirectional2DLight(flecs::world &world, const RID &light_id, const Transform2D &transform,const RID& canvas_id, const String &name) {
		world.entity(name)
				.set<DirectionalLight3DComponent>({ light_id })
				.set<Transform2DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) });
				
	}

	static inline flecs::entity CreateDirectional2DLight(flecs::world &world, const Transform2D &transform, const String &name) {
		auto diretional_light_id = RS::get_singleton()->canvas_light_create();
		if (!diretional_light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}

		auto directional_light = world.entity(name)
										 .set<DirectionalLight2DComponent>({ diretional_light_id })
										 .set<Transform2DComponent>({ transform });
		return directional_light;
	}

	static inline flecs::entity_t CreateDirectional2DLight(flecs::world &world, DirectionalLight2D *directional_light) {
		if (directional_light == nullptr) {
			ERR_FAIL_V(flecs::entity_t());
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
		RS::get_singleton()->canvas_light_set_blend_mode(light_id, static_cast<RenderingServer::CanvasLightBlendMode>(directional_light->get_blend_mode()));
		RS::get_singleton()->canvas_light_set_shadow_enabled(light_id, directional_light->is_shadow_enabled());
		RS::get_singleton()->canvas_light_set_shadow_filter(light_id, static_cast<RenderingServer::CanvasLightShadowFilter>(directional_light->get_shadow_filter()));
		RS::get_singleton()->canvas_light_set_shadow_color(light_id, directional_light->get_shadow_color());
		RS::get_singleton()->canvas_light_set_shadow_smooth(light_id, directional_light->get_shadow_smooth());
		RS::get_singleton()->canvas_light_set_transform(light_id, directional_light->get_transform());
		RS::get_singleton()->canvas_light_set_interpolated(light_id, directional_light->is_physics_interpolated());
		RS::get_singleton()->canvas_light_reset_physics_interpolation(light_id);
		RS::get_singleton()->canvas_light_transform_physics_interpolation(light_id, directional_light->get_transform());
		RS::get_singleton()->canvas_light_set_mode(light_id, RenderingServer::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);

		auto &entity = world.entity(directional_light->get_name())
							   .set<DirectionalLight2DComponent>({ light_id })
							   .set<Transform2DComponent>({ directional_light->get_transform() });
		if (directional_light->is_inside_tree()) {
			directional_light->get_tree()->get_current_scene()->remove_child(directional_light);
			directional_light->queue_free();
		}
		return entity;
	}

	static inline flecs::entity CreatePointLight(flecs::world &world, const Transform2D &transform, const String &name)
	{
		RID light_id = RS::get_singleton()->canvas_light_create();
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return CreatePointLight(world, light_id,transform, name);
	}

	static inline flecs::entity CreatePointLight(flecs::world &world, const RID &light_id, const Transform2D &transform, const String &name) {
		return world.entity(name)
				.set<PointLightComponent>({ light_id })
				.set<Transform2DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) });
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

		if (point_light->is_inside_tree()) {
			point_light->get_tree()->get_current_scene()->remove_child(point_light);
			point_light->queue_free();
		}

		auto& entity = world.entity(point_light->get_name())
							  .set<PointLightComponent>({ light_id })
							  .set<Transform2DComponent>({ point_light->get_transform() });
		return entity;
	}
	static inline flecs::entity CreateOmniLight(flecs::world &world, const RID &light_id, const Transform3D &transform, const RID &scenario_id) {
		return world.entity("OmniLight")
				.set<OmniLightComponent>({ light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, scenario_id) });
	}

	static inline flecs::entity CreateOmniLight(flecs::world &world, const Transform3D &transform, const RID &scenario_id) {
		RID omli_light_id = RS::get_singleton()->omni_light_create();
		return world.entity("OmniLight")
				.set<OmniLightComponent>({ omli_light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(omli_light_id, scenario_id) });
	}

	static inline flecs::entity CreateOmniLight(flecs::world &world, OmniLight3D *omni_light) {
		if (omni_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto& entity = world.entity("OmniLight")
							  .set<OmniLightComponent>({ omni_light->get_base() })
							  .set<Transform3DComponent>({ omni_light->get_transform() })
							  .set<RenderInstanceComponent>({ omni_light->get_instance() });
		ObjectIDStorage::add(omni_light, omni_light->get_instance());
		return entity;
	}

	static inline flecs::entity CreateSpotLight(flecs::world &world, const RID &light_id, const Transform3D &transform, const String &name) {
		return world.entity(name)
				.set<SpotLightComponent>({ light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity CreateSpotLight(flecs::world &world, const Transform3D &transform, const String &name) {
		RID spot_light_id = RS::get_singleton()->spot_light_create();
		return world.entity(name)
				.set<SpotLightComponent>({ spot_light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(spot_light_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity CreateSpotLight(flecs::world &world, SpotLight3D *spot_light) {
		if (spot_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto& entity = world.entity("SpotLight")
							  .set<SpotLightComponent>({ spot_light->get_base() })
							  .set<Transform3DComponent>({ spot_light->get_transform() })
							  .set<RenderInstanceComponent>({ spot_light->get_instance() });
		ObjectIDStorage::add(spot_light, spot_light->get_instance());
		return entity;
	}



	static inline flecs::entity CreateViewport(flecs::world &world, const RID &viewport_id, const String &name) {
		return world.entity(name)
				.set<ViewportComponent>({ viewport_id });
	}

	static inline flecs::entity_t CreateViewport(flecs::world &world, Viewport *viewport) {
		if (viewport == nullptr) {
			ERR_FAIL_V(flecs::entity_t());
		}
		auto& entity = world.entity(viewport->get_name())
							  .set<ViewportComponent>({ viewport->get_viewport_rid() });
		ObjectIDStorage::add(viewport, viewport->get_viewport_rid());
		return entity;
	}

	static inline flecs::entity CreateVoxelGI(flecs::world &world, const RID &voxel_gi_id, const Transform3D& transform, const String &name) {
		return world.entity(name)
				.set<VoxelGIComponent>({ voxel_gi_id })
				.set<Transform3DComponent>({transform})
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity_t CreateVoxelGI(flecs::world &world, const Transform3D &transform, const String &name) {
		RID voxel_gi_id = RS::get_singleton()->voxel_gi_create();
		return world.entity(name)
				.set<VoxelGIComponent>({ voxel_gi_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world.get<MainScenarioComonent>().id) });
	}

	static inline flecs::entity CreateVoxelGI(flecs::world &world, VoxelGI *voxel_gi) {
		if (voxel_gi == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto& entity = world.entity(voxel_gi->get_name())
							  .set<VoxelGIComponent>({ voxel_gi->get_base() })
							  .set<Transform3DComponent>({ voxel_gi->get_transform() })
							  .set<RenderInstanceComponent>({ voxel_gi->get_instance() });
		ObjectIDStorage::add(voxel_gi, voxel_gi->get_instance());
		return entity;
	}

	static inline flecs::entity CreateScenario(flecs::world &world, const RID &scenario_id, const String &name) {
		return world.entity(name)
				.set<ScenarioComponent>({ scenario_id });
	}

	static inline flecs::entity CreateScenario(flecs::world &world, const String &name) {
		RID scenario_id = RS::get_singleton()->scenario_create();
		return world.entity(name)
				.set<ScenarioComponent>({ scenario_id });
	}

	static inline void CreateMainScenarioInstance(flecs::world &world, const String &name) {
		RID scenario_id = RS::get_singleton()->scenario_create();
		CreateOrSetMainScenarioInstance(world, scenario_id, name);
	}

	static inline void CreateOrSetMainScenarioInstance(flecs::world &world, const RID &scenario_id, const String &name) {
		if (!scenario_id.is_valid()) {
			ERR_PRINT("Failed to create main scenario instance.");
			return;
		}
		if (world.has<MainScenarioComonent>()) {
			world.entity(world.get<MainScenarioComonent>().entity_name)
					.set<ScenarioComponent>({ world.get<MainScenarioComonent>().id });
			world.get_mut<MainScenarioComonent>().id = scenario_id;
			world.get_mut<MainScenarioComonent>().entity_name = name;

		} else {
			world.set<MainScenarioComonent>({ scenario_id });
		}
	}

	static inline flecs::entity CreateCanvas(flecs::world &world, const RID &canvas_id, const String &name) {
		return world.entity(name)
				.set<CanvasComponent>({ canvas_id });
	}

	static inline flecs::entity CreateCanvasItem(flecs::world &world, CanvasItem  *canvas_item) {
		if (canvas_item == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(canvas_item->get_name())
							   .set<CanvasItemComponent>({ canvas_item->get_canvas_item() });
		ObjectIDStorage::add(canvas_item, canvas_item->get_canvas_item());
		return entity;
	}

	static inline flecs::entity CreateCanvasItem(flecs::world &world, const RID &canvas_item_id, const String &name) {
		return world.entity(name)
				.set<CanvasItemComponent>({ canvas_item_id });
	}

	static inline flecs::entity CreateCanvas(flecs::world &world, Camera2D* camera) {
		if (camera == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(camera->get_name())
							   .set<CanvasComponent>({ camera->get_canvas() });
		return entity;
	}

	static inline flecs::entity CreateCanvas(flecs::world &world, Camera2D *camera) {
		if (camera == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(camera->get_name())
							   .set<CanvasComponent>({ camera->get_canvas() });
		return entity;
	}
};

};
