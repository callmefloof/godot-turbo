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
#include "../object_id_storage.h"
#include <core/templates/rid.h>
#include <core/templates/vector.h>
#include <core/templates/rid_owner.h>
#include <scene/main/viewport.h>
#include <cassert>
#include "../../components/transform_3d_component.h"
#include <scene/main/window.h>
#include <scene/3d/voxel_gi.h>
#include <scene/3d/mesh_instance_3d.h>


class RenderUtility3D {
private:
	RenderUtility3D() = default; // Prevent instantiation
	RenderUtility3D(const RenderUtility3D &) = delete; // Prevent copy
	RenderUtility3D &operator=(const RenderUtility3D &) = delete; // Prevent assignment
	RenderUtility3D(RenderUtility3D &&) = delete; // Prevent move
	RenderUtility3D &operator=(RenderUtility3D &&) = delete; // Prevent move assignment

public:
	static inline flecs::entity CreateMeshInstance3D(flecs::world &world, const RID& mesh_id, const Transform3D& transform, const String& name, const RID& scenario_id) {  
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
		return world.entity()
			.set<MeshComponent>({ mesh_id, material_ids })
			.set<Transform3DComponent>({ transform }) // Default transform
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(mesh_id, scenario_id) })
			.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateMeshInstance3D(flecs::world &world, const Transform3D &transform, const RID &scenario_id, const String &name) {
		Vector<RID> material_ids;
		auto mesh_id = RS::get_singleton()->mesh_create();
		auto surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
		return CreateMeshInstance3D(world, mesh_id, transform, name, scenario_id);
	}

	static inline flecs::entity CreateMeshInstance3D(flecs::world& world, MeshInstance3D* mesh_instance_3d)
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
		return world.entity()
				.set<MeshComponent>({ base, material_ids })
				.set<Transform3DComponent>({ mesh_instance_3d->get_transform() })
				.set<RenderInstanceComponent>({ instance })
				.set_name(String(mesh->get_name()).ascii().get_data());
	}

	static inline flecs::entity CreateMultiMesh3D(flecs::world &world,
			const Transform3D &transform,
			const uint32_t size,
			const RID &mesh_id,
			const RID &scenario_id,
			const String &name,
			const bool use_colors = false,
			const bool use_custom_data = false,
			const bool use_indirect = false
			) {
		RID multmesh_id = RS::get_singleton()->multimesh_create();
		if (!multmesh_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		RS::get_singleton()->multimesh_set_mesh(multmesh_id, mesh_id);
		RS::get_singleton()->multimesh_allocate_data(multmesh_id, size, RS::MULTIMESH_TRANSFORM_3D, use_colors,use_custom_data, use_indirect);
		auto& entity = world.entity()
							  .set<MultiMeshComponent>({ multmesh_id, size })
							  .set<MeshComponent>({ mesh_id })
							  .set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(multmesh_id, scenario_id) })
							  .set<Transform3DComponent>({ transform })
							  .set_name(name.ascii().get_data());

		return entity;
	}

	static inline flecs::entity_t CreateMultiMesh3D(flecs::world &world,
		const Transform3D &transform,
		MultiMeshInstance3D* multimesh_instance,
		const String &name
		) {
		RID multmesh_id = multimesh_instance->get_multimesh()->get_rid();
		Ref<Mesh> mesh = multimesh_instance->get_multimesh()->get_mesh();
		uint32_t size = multimesh_instance->get_multimesh()->get_instance_count();
		RID instance_id = multimesh_instance->get_instance();
		auto &entity = world.entity()
							   .set<MultiMeshComponent>({ multmesh_id, size })
							   .set<MeshComponent>({ mesh->get_rid() })
							   .set<RenderInstanceComponent>({ instance_id })
							   .set<Transform3DComponent>({ transform })
							   .set_name(name.ascii().get_data());
		ObjectIDStorage::add(multimesh_instance, instance_id);
		return entity;
	}	 

	static inline flecs::entity CreateMultiMeshInstance3D(
		flecs::world &world,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multimesh_id,
		const String &name) {  
		return world.entity()  
				.set<MultiMeshInstanceComponent>({ index })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(multimesh_id, world.get<MainScenarioComonent>().id) })
				.set<Transform3DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static inline Vector<flecs::entity> CreateMultimeshInstances3D(
			flecs::world &world,
			const Transform3D (&transform)[],
			flecs::entity& multimesh) {
		Vector<flecs::entity> instances;
		
		auto& multimesh_component = multimesh.get<MultiMeshComponent>();

		const uint32_t instance_count = RS::get_singleton()->multimesh_get_instance_count(multimesh_component.multimesh_id);

		for (uint32_t i = 0; i < instance_count; ++i) {
			instances.append(CreateMultiMeshInstance3D(world, transform[i], i, multimesh_component.multimesh_id, multimesh.name() + " - Instance: #" + String::num_int64(i)));
		}
		return instances;
	}


	static inline flecs::entity CreateParticles(
		flecs::world &world,
		const Transform3D &transform,
		const RID &scenario_id,
		const String &name) {  
		RID particles_id = RS::get_singleton()->particles_create();
		return world.entity()  
				.set<ParticlesComponent>({ particles_id })  
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(particles_id, scenario_id) })
				.set<Transform3DComponent>({ transform })
				.set_name(name.ascii().get_data());  
	}

	static inline flecs::entity CreateParticles(flecs::world &world, GPUParticles3D *gpu_particles_3d) {
		if (gpu_particles_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		if (gpu_particles_3d->is_inside_tree() && gpu_particles_3d->is_inside_world())
		{
			gpu_particles_3d->get_parent()->remove_child(gpu_particles_3d);

		}
		auto &particles = world.entity()
								 .set<ParticlesComponent>({ gpu_particles_3d->get_base() })
								 .set<RenderInstanceComponent>({ gpu_particles_3d->get_instance() })
								 .set<Transform3DComponent>({ gpu_particles_3d->get_transform() })
								 .set_name(String(gpu_particles_3d->get_name()).ascii().get_data());
		ObjectIDStorage::add(gpu_particles_3d,gpu_particles_3d->get_instance());
		return particles;
	}

	static inline flecs::entity CreateReflectionProbe(flecs::world &world, const RID &probe_id, const Transform3D &transform, const String& name) {  
		return world.entity()  
				.set<ReflectionProbeComponent>({ probe_id })
				.set<Transform3DComponent>({transform})
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(probe_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());  
	}

	static inline flecs::entity CreateReflectionProbe(flecs::world &world, ReflectionProbe *reflection_probe) {

		auto &entity_probe = world.entity()
									.set<ReflectionProbeComponent>({ reflection_probe->get_base() })
									.set<Transform3DComponent>({ reflection_probe->get_transform() })
									.set<RenderInstanceComponent>({ reflection_probe->get_instance() })
									.set_name(String(reflection_probe->get_name()).ascii().get_data());
		ObjectIDStorage::add(reflection_probe, reflection_probe->get_instance());
		return entity_probe;
	}
	

	static inline flecs::entity CreateSkeleton(flecs::world &world, const RID &skeleton_id, const String &name) {  
		return world.entity()  
				.set<SkeletonComponent>({ skeleton_id })
				.set_name(name.ascii().get_data());
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
		if (skeleton_3d->is_inside_tree() && skeleton_3d->is_inside_world()) {
			skeleton_3d->get_parent()->remove_child(skeleton_3d);
			skeleton_3d->queue_free();
		}
		return world.entity()
				.set<SkeletonComponent>({ skeleton_id })
				.set_name(String(skeleton_3d->get_name()).ascii().get_data());
	}

	static inline flecs::entity CreateEnvironment(flecs::world &world, const RID &environment_id, const Transform3D &transform, const String &name) {  
		return world.entity()  
				.set<EnvironmentComponent>({ environment_id })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(environment_id, world.get<MainScenarioComonent>().id) })
				.set<Transform3DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}  

	static inline flecs::entity CreateCamera3D(flecs::world &world, const RID &camera_id,const Transform3D &transform, const String& name) {  
		return world.entity()  
				.set<CameraComponent>({ camera_id })
				.set<Transform3DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateCamera3D(flecs::world &world, const Transform3D &transform, const String &name) {
		RID camera_id = RS::get_singleton()->camera_create();
		if (!camera_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return world.entity()
				.set<CameraComponent>({ camera_id })
				.set<Transform3DComponent>({ transform })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateCamera3D(flecs::world &world, Camera3D *camera_3d) {
		if (camera_3d == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto camera = world.entity(String(camera_3d->get_name()).ascii().get_data())
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

	static inline flecs::entity CreateCompositor(flecs::world &world, const RID &compositor_id, const String& name) {
		return world.entity()
				.set<CompositorComponent>({ compositor_id })
				.set_name(name.ascii().get_data());
	}


	static inline flecs::entity CreateCompositor(flecs::world &world, Compositor *compositor) {
		if (compositor == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto compositor_id = compositor->get_rid();
		if (!compositor_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		auto entity = world.entity()
							  .set<CompositorComponent>({ compositor_id })
							  .set_name(String(compositor->get_name()).ascii().get_data());
		ObjectIDStorage::add(compositor, compositor_id);
		return entity;
	}


	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, const RID &light_id, const Transform3D& transform, const String &name) {
		if (!light_id.is_valid()) {
			ERR_FAIL_V(flecs::entity());
		}
		return world.entity()
				.set<DirectionalLight3DComponent>({ light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, const Transform3D &transform, const String &name) {
		auto diretional_light_id = RS::get_singleton()->directional_light_create();
		return world.entity()
				.set<DirectionalLight3DComponent>({ diretional_light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(diretional_light_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateDirectional3DLight(flecs::world &world, DirectionalLight3D *directional_light) {
		if (directional_light == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity()
							  .set<DirectionalLight3DComponent>({ directional_light->get_base() })
							  .set<Transform3DComponent>({ directional_light->get_transform() })
							  .set<RenderInstanceComponent>({ directional_light->get_instance() })
							  .set_name(String(directional_light->get_name()).ascii().get_data());
		ObjectIDStorage::add(directional_light, directional_light->get_instance());
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
		return world.entity()
				.set<SpotLightComponent>({ light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateSpotLight(flecs::world &world, const Transform3D &transform, const String &name) {
		RID spot_light_id = RS::get_singleton()->spot_light_create();
		return world.entity()
				.set<SpotLightComponent>({ spot_light_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(spot_light_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
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
		return world.entity()
				.set<ViewportComponent>({ viewport_id })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity_t CreateViewport(flecs::world &world, Viewport *viewport) {
		if (viewport == nullptr) {
			ERR_FAIL_V(flecs::entity_t());
		}
		auto &entity = world.entity(String(viewport->get_name()).ascii().get_data())
							  .set<ViewportComponent>({ viewport->get_viewport_rid() });
		ObjectIDStorage::add(viewport, viewport->get_viewport_rid());
		return entity;
	}

	static inline flecs::entity CreateVoxelGI(flecs::world &world, const RID &voxel_gi_id, const Transform3D& transform, const String &name) {
		return world.entity()
				.set<VoxelGIComponent>({ voxel_gi_id })
				.set<Transform3DComponent>({transform})
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity_t CreateVoxelGI(flecs::world &world, const Transform3D &transform, const String &name) {
		RID voxel_gi_id = RS::get_singleton()->voxel_gi_create();
		return world.entity()
				.set<VoxelGIComponent>({ voxel_gi_id })
				.set<Transform3DComponent>({ transform })
				.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world.get<MainScenarioComonent>().id) })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateVoxelGI(flecs::world &world, VoxelGI *voxel_gi) {
		if (voxel_gi == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}
		auto &entity = world.entity(String(voxel_gi->get_name()).ascii().get_data())
							  .set<VoxelGIComponent>({ voxel_gi->get_base() })
							  .set<Transform3DComponent>({ voxel_gi->get_transform() })
							  .set<RenderInstanceComponent>({ voxel_gi->get_instance() })
							  .set_name(String(voxel_gi->get_name()).ascii().get_data());
		ObjectIDStorage::add(voxel_gi, voxel_gi->get_instance());
		return entity;
	}

	static inline flecs::entity CreateScenario(flecs::world &world, const RID &scenario_id, const String &name) {
		return world.entity()
				.set<ScenarioComponent>({ scenario_id })
				.set_name(name.ascii().get_data());
	}

	static inline flecs::entity CreateScenario(flecs::world &world, const String &name) {
		RID scenario_id = RS::get_singleton()->scenario_create();
		return world.entity()
				.set<ScenarioComponent>({ scenario_id })
				.set_name(name.ascii().get_data());
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
			world.entity()
					.set<ScenarioComponent>({ world.get<MainScenarioComonent>().id })
					.set_name(world.get<MainScenarioComonent>().entity_name.ascii().get_data());
			world.get_mut<MainScenarioComonent>().id = scenario_id;
			world.get_mut<MainScenarioComonent>().entity_name = name;

		} else {
			world.set<MainScenarioComonent>({ scenario_id });
		}
	}
};
