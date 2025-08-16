//
// Created by Floof on 29-7-2025.
//

#include "render_utility_3d.h"

#include "core/io/marshalls.h"
#include "core/math/math_funcs.h"
#include "core/math/plane.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/gpu_particles_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/3d/reflection_probe.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/3d/world_environment.h"
#include "scene/resources/compositor.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/multimesh.h"
#include "scene/resources/surface_tool.h"
#include "ecs/systems/rendering/occlusion/occlusion_system.h"

#include "scene/3d/occluder_instance_3d.h"

#include "ecs/utility/ref_storage.h"
#include "ecs/utility/node_storage.h"

#include "core/templates/rid.h"
#include "scene/main/viewport.h"
#include "servers/rendering_server.h"
#include <cassert>

#include "scene/3d/voxel_gi.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/object_instance_component.h"
#include "core/templates/vector.h"
#include "servers/rendering/renderer_scene_occlusion_cull.h"
#include <vector>
#include <algorithm>
#include <execution>
#include "ecs/systems/commands/command.h"
#include "ecs/components/dirty_transform.h"


RenderUtility3D::~RenderUtility3D() {
}

flecs::entity RenderUtility3D::_create_mesh_instance(const flecs::world *world, const RID &mesh_id, const Transform3D &transform, const String &name, const RID &scenario_id) {
	Vector<RID> material_ids;
	const uint32_t surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
	for (uint32_t i = 0; i < surface_count; ++i) {
		const RID material_id = RS::get_singleton()->mesh_surface_get_material(mesh_id, i);
		if (material_id.is_valid()) {
			material_ids.push_back(material_id);
		} else {
			material_ids.push_back(RID()); // Use an empty RID if no material is set
		}
	}
	auto mesh_component = MeshComponent();
	mesh_component.material_ids = material_ids;
	mesh_component.mesh_id = mesh_id;
	mesh_component.custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_id);

	return world->entity()
			.set<MeshComponent>(mesh_component)
			.set<Transform3DComponent>({ transform }) // Default transform
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(mesh_id, scenario_id) })
			.set<VisibilityComponent>({ true })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_mesh_instance(const flecs::world *world, const Transform3D &transform, const RID &scenario_id, const String &name) {
	Vector<RID> material_ids;
	const RID mesh_id = RS::get_singleton()->mesh_create();
	return _create_mesh_instance(world, mesh_id, transform, name, scenario_id);
}

flecs::entity RenderUtility3D::_create_mesh_instance(const flecs::world *world, MeshInstance3D *mesh_instance_3d) {
	Vector<RID> material_ids;
	const Ref<Mesh> mesh = mesh_instance_3d->get_mesh();
	if (!mesh.is_valid()) {
		ERR_FAIL_COND_V(!mesh.is_valid(), flecs::entity());
	}
	RefStorage::add(mesh, mesh->get_rid());
	if (!mesh_instance_3d->get_base().is_valid()) {
		ERR_FAIL_COND_V(!mesh_instance_3d->get_base().is_valid(), flecs::entity());
	}
	const RID &mesh_rid = mesh->get_rid();
	if (!mesh_rid.is_valid()) {
		ERR_FAIL_COND_V(!mesh_rid.is_valid(), flecs::entity());
	}
	if (!mesh_instance_3d->get_instance().is_valid()) {
		ERR_FAIL_COND_V(!mesh_instance_3d->get_instance().is_valid(), flecs::entity());
	}
	const RID &instance_rid = mesh_instance_3d->get_instance();
	if (!instance_rid.is_valid()) {
		ERR_FAIL_COND_V(!instance_rid.is_valid(), flecs::entity());
	}

	const RID &base = mesh_instance_3d->get_base();
	
	if (!base.is_valid()) {
		ERR_FAIL_COND_V(!base.is_valid(), flecs::entity());
	}

	const RID& scenario_id = world->get<World3DComponent>().scenario_id;
	if (!scenario_id.is_valid()) {
		ERR_FAIL_COND_V(!scenario_id.is_valid(), flecs::entity());
	}
	const RID &instance = RS::get_singleton()->instance_create2(mesh_rid, scenario_id);
	if (!instance.is_valid()) {
		ERR_FAIL_COND_V(!instance.is_valid(), flecs::entity());
	}

	for (int i = 0; i < mesh->get_surface_count(); ++i) {
		const Ref<Material> material = mesh->surface_get_material(i);
		if (material.is_valid()) {
			material_ids.push_back(material->get_rid());
		} else {
			material_ids.push_back(RID()); // Use an empty RID if no material is set
		}
	}
	auto mesh_component = MeshComponent();
	mesh_component.material_ids = material_ids;
	mesh_component.mesh_id = base;
	NodeStorage::add(mesh_instance_3d, mesh_instance_3d->get_instance_id());
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = mesh_instance_3d->get_instance_id();
	return world->entity()
			.set<MeshComponent>(mesh_component)
			.set<Transform3DComponent>({ mesh_instance_3d->get_transform() })
			.set<RenderInstanceComponent>({ instance })
			.set<VisibilityComponent>({ true })
			.set<ObjectInstanceComponent>(object_instance_component)
			.add<DirtyTransform>()
			.set_name(String(mesh->get_name()).ascii().get_data());
}

flecs::entity RenderUtility3D::_create_multi_mesh(const flecs::world *world,
		const Transform3D &transform,
		const uint32_t size,
		const RID &mesh_id,
		const Vector<RID> &material_ids,
		const RID &scenario_id,
		const String &name,
		const bool use_colors,
		const bool use_custom_data,
		const bool use_indirect) {
	const RID multi_mesh_id = RS::get_singleton()->multimesh_create();
	if (!multi_mesh_id.is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	RS::get_singleton()->multimesh_set_mesh(multi_mesh_id, mesh_id);
	RS::get_singleton()->multimesh_allocate_data(multi_mesh_id, size, RS::MULTIMESH_TRANSFORM_3D, use_colors, use_custom_data, use_indirect);
	auto mesh_component = MeshComponent();
	mesh_component.material_ids = material_ids;
	mesh_component.mesh_id = mesh_id;
	const flecs::entity entity = world->entity()
										 .set<MultiMeshComponent>({ multi_mesh_id, size })
										 .set<MeshComponent>(mesh_component)
										 .set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(multi_mesh_id, scenario_id) })
										 .set<Transform3DComponent>({ transform })
										 .add<DirtyTransform>()
										 .set_name(name.ascii().get_data());

	return entity;
}

flecs::entity RenderUtility3D::_create_multi_mesh(const flecs::world *world, MultiMeshInstance3D *multi_mesh_instance) {
	const Ref<MultiMesh> multi_mesh = multi_mesh_instance->get_multimesh();
	if (multi_mesh.is_null()) {
		ERR_FAIL_COND_V(multi_mesh.is_null(), flecs::entity());
	}
	if (!multi_mesh.is_valid()) {
		ERR_FAIL_COND_V(!multi_mesh.is_valid(), flecs::entity());
	}
	RefStorage::add(multi_mesh, multi_mesh->get_rid());



	const RID &multi_mesh_id = multi_mesh->get_rid();
	const RID &mesh_id = multi_mesh->get_mesh()->get_rid();
	if (!multi_mesh_id.is_valid()) {
		ERR_FAIL_COND_V(!multi_mesh_id.is_valid(), flecs::entity());
	}
	if (!mesh_id.is_valid()) {
		ERR_FAIL_COND_V(!mesh_id.is_valid(), flecs::entity());
	}
	auto mesh_ref = multi_mesh_instance->get_multimesh()->get_mesh();
	RefStorage::add(mesh_ref, mesh_ref->get_rid());
	if(!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	const RID &instance_id = RS::get_singleton()->instance_create2(multi_mesh_id, world->get<World3DComponent>().scenario_id);

	const String name = multi_mesh_instance->get_name();
	const Transform3D transform = multi_mesh_instance->get_transform();
	const uint32_t size = multi_mesh_instance->get_multimesh()->get_instance_count();
	const int surface_count = multi_mesh_instance->get_multimesh()->get_mesh()->get_surface_count();
	Vector<RID> material_ids;
	for(int i = 0; i < surface_count; i++){
		if(mesh_ref->surface_get_material(i).is_valid()) {
			Ref<Material> material = mesh_ref->surface_get_material(i);
			if (!material.is_valid()) {
				ERR_PRINT("Material is not valid for surface " + itos(i) + " of MultiMesh.");
				continue;
			}
			if (!material->get_rid().is_valid()) {
				ERR_PRINT("Material RID is not valid for surface " + itos(i) + " of MultiMesh.");
				continue;
			}
			RefStorage::add(material, material->get_rid());
			material_ids.append(material->get_rid());
		} else{
			ERR_PRINT("Material not set.");
		}
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = multi_mesh_instance->get_instance_id();
	const AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_id);
	const flecs::entity entity = world->entity()
										.set<MultiMeshComponent>({ multi_mesh_id, size })
										.set<MeshComponent>({ mesh_id, material_ids, custom_aabb })
										.set<RenderInstanceComponent>({ instance_id })
										.set<Transform3DComponent>({ transform })
										.set<VisibilityComponent>({ true })
										.set<ObjectInstanceComponent>(object_instance_component)
										.add<DirtyTransform>()
										.set_name(name.ascii().get_data());
	
										
	

		// Create a bulk array of entities
	std::vector<flecs::entity_t> entities(size);


	// Bulk assign components
	std::vector<MultiMeshInstanceComponent> mm_components(size);
	std::vector<Transform3DComponent> transforms(size);
	std::vector<VisibilityComponent> vis(size, { true });

	for (uint32_t i = 0; i < size; ++i) {
		mm_components[i].index = i;
		transforms[i].transform = multi_mesh_instance->get_multimesh()->get_instance_transform(i);
		vis[i].visible = true;
	}

	
	std::vector<void*> data(size * 3);
	data[0] = mm_components.data();
	data[1] = transforms.data();
	data[2] = vis.data();
	data[3] = nullptr; // Needed for pair/tag

	
	
	ecs_bulk_desc_t bulk_desc = {0, nullptr, size, {
		world->lookup("MultiMeshInstanceComponent").id(),
		world->lookup("Transform3DComponent").id(),
		world->lookup("VisibilityComponent").id(),
		world->pair(flecs::ChildOf, entity),
	}, data.data()};

	auto entity_ids = ecs_bulk_init(world->c_ptr(), &bulk_desc);


	
	NodeStorage::add(multi_mesh_instance, multi_mesh_instance->get_instance_id());

	return entity;
}

flecs::entity RenderUtility3D::_create_multi_mesh_instance(
		const flecs::world *world,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multi_mesh_id,
		const String &name) {
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<MultiMeshInstanceComponent>({ index })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

Vector<flecs::entity> RenderUtility3D::_create_multi_mesh_instances(
		const flecs::world *world,
		const Vector<Transform3D> &transform,
		const flecs::entity &multi_mesh) {
	Vector<flecs::entity> instances;
	const auto &[multi_mesh_id, instance_count] = multi_mesh.get<MultiMeshComponent>();
	for (uint32_t i = 0; i < instance_count; ++i) {
		instances.append(_create_multi_mesh_instance(world, transform[i], i, multi_mesh_id, multi_mesh.name() + " - Instance: #" + String::num_int64(i)));
	}
	return instances;
}

flecs::entity RenderUtility3D::_create_particles(
		const flecs::world *world,
		const Transform3D &transform,
		const RID &particles_id,
		const RID &scenario_id,
		const String &name) {
	return world->entity()
			.set<ParticlesComponent>({ particles_id })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(particles_id, scenario_id) })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_particles(const flecs::world *world, GPUParticles3D *gpu_particles_3d) {
	if (gpu_particles_3d == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}

	if(!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	const RID& instance_id = RS::get_singleton()->instance_create2(gpu_particles_3d->get_base(), world->get<World3DComponent>().scenario_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = gpu_particles_3d->get_instance_id();
	auto &particles = world->entity()
							.set<ParticlesComponent>({ gpu_particles_3d->get_base() })
							.set<RenderInstanceComponent>({ instance_id })
							.set<Transform3DComponent>({ gpu_particles_3d->get_transform() })
							.set<VisibilityComponent>({ true })
							.set<ObjectInstanceComponent>(object_instance_component)
							.add<DirtyTransform>()
							.set_name(String(gpu_particles_3d->get_name()).ascii().get_data());
	NodeStorage::add(gpu_particles_3d, gpu_particles_3d->get_instance_id());
	return particles;
}

flecs::entity RenderUtility3D::_create_reflection_probe(const flecs::world *world, const RID &probe_id, const Transform3D &transform, const String &name) {
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<ReflectionProbeComponent>({ probe_id })
			.set<Transform3DComponent>({ transform })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(probe_id, world->get<World3DComponent>().scenario_id) })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_reflection_probe(const flecs::world *world, ReflectionProbe *reflection_probe) {
	flecs::entity entity_probe = _create_reflection_probe(world, reflection_probe->get_base(), reflection_probe->get_transform(), reflection_probe->get_name());
	NodeStorage::add(reflection_probe, reflection_probe->get_instance_id());
	ObjectInstanceComponent object_instance_component = ObjectInstanceComponent(reflection_probe->get_instance_id());
	entity_probe.set<ObjectInstanceComponent>(object_instance_component);
	return entity_probe;
}

flecs::entity RenderUtility3D::_create_skeleton(const flecs::world *world, const RID &skeleton_id, const String &name) {
	return world->entity()
			.set<SkeletonComponent>({ skeleton_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_skeleton(const flecs::world *world, Skeleton3D *skeleton_3d) {
	const RID skeleton_id = RS::get_singleton()->skeleton_create();
	if (skeleton_3d == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_3d->get_bone_count(), false);
	for (int i = 0; i < skeleton_3d->get_bone_count(); ++i) {
		RS::get_singleton()->skeleton_bone_set_transform(skeleton_id, i, skeleton_3d->get_bone_global_pose(i));
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = skeleton_3d->get_instance_id();
	NodeStorage::add(skeleton_3d, skeleton_3d->get_instance_id());
	return world->entity()
			.set<SkeletonComponent>({ skeleton_id })
			.set<Transform3DComponent>({ skeleton_3d->get_transform() })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(skeleton_id, world->get<World3DComponent>().scenario_id) })
			.set<ObjectInstanceComponent>(object_instance_component)
			.add<DirtyTransform>()
			.set_name(String(skeleton_3d->get_name()).ascii().get_data());
}

flecs::entity RenderUtility3D::_create_environment(const flecs::world *world, const RID &environment_id, const String &name) {
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<EnvironmentComponent>({ environment_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_environment(const flecs::world *world, WorldEnvironment *world_environment) {
	if (world_environment == nullptr) {
		ERR_FAIL_COND_V(world_environment == nullptr, flecs::entity());
	}
	if (world_environment->get_environment().is_null() || !world_environment->get_environment().is_valid()) {
		ERR_FAIL_COND_V(world_environment->get_environment().is_null() || !world_environment->get_environment().is_valid(), flecs::entity());
	}
	const Ref<Environment> &environment_ref = world_environment->get_environment();
	const RID &environment_id = world_environment->get_environment()->get_rid();
	RefStorage::add(environment_ref, environment_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = world_environment->get_instance_id();
	NodeStorage::add(world_environment, world_environment->get_instance_id());
	return world->entity()
			.set<EnvironmentComponent>({ environment_id })
			.set<ObjectInstanceComponent>(object_instance_component)
			.set_name("WorldEnvironment");
}

flecs::entity RenderUtility3D::_create_camera(const flecs::world *world, const RID &camera_id, const Transform3D &transform, const String &name) {
	return world->entity()
			.set<CameraComponent>({ camera_id })
			.set<Transform3DComponent>({ transform })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_camera(const flecs::world *world, const Transform3D &transform, const String &name) {
	const RID camera_id = RS::get_singleton()->camera_create();
	if (!camera_id.is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	return world->entity()
			.set<CameraComponent>({ camera_id })
			.set<Transform3DComponent>({ transform })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_camera(const flecs::world *world, Camera3D *camera_3d) {
	if (camera_3d == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	Vector2 camera_offset = Vector2(camera_3d->get_h_offset(),camera_3d->get_v_offset());
	CameraComponent camera_component = { 
											camera_3d->get_camera(),
										 	camera_3d->get_frustum(),
											camera_3d->get_position(),
										 	camera_3d->get_far(),
										 	camera_3d->get_near(),
										 	camera_3d->get_camera_projection(),
											camera_offset};
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = camera_3d->get_instance_id();
	NodeStorage::add(camera_3d, camera_3d->get_instance_id());
	const flecs::entity camera = world->entity()
		.set<CameraComponent>(camera_component)
		.set<Transform3DComponent>({ camera_3d->get_camera_transform() })
		.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(camera_3d->get_camera(), world->get<World3DComponent>().scenario_id) })
		.set<ObjectInstanceComponent>(object_instance_component)
		.set_name(String(camera_3d->get_name()).ascii().get_data());
	if (camera_3d->get_compositor().is_null() || camera_3d->get_compositor().is_valid()) {
		Ref<Compositor> compositor_ref = camera_3d->get_compositor();
		RefStorage::add(compositor_ref, compositor_ref->get_rid());
		if (!compositor_ref.is_valid()) {
			ERR_FAIL_COND_V(!compositor_ref.is_valid(), flecs::entity());
		}
		const RID compositor_id = compositor_ref->get_rid();
		flecs::entity compositor_entity = _create_compositor(world, compositor_id, compositor_ref->get_name());
		compositor_entity.child(camera);
	}

	return camera;
}

flecs::entity RenderUtility3D::_create_compositor(const flecs::world *world, const RID &compositor_id, const String &name) {
	return world->entity()
			.set<CompositorComponent>({ compositor_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_compositor(const flecs::world *world, const Ref<Compositor> &compositor) {
	if (compositor == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	const RID compositor_id = compositor->get_rid();
	if (!compositor_id.is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	RefStorage::add(compositor, compositor_id);
	const flecs::entity entity = world->entity()
										 .set<CompositorComponent>({ compositor_id })
										 .set_name(String(compositor->get_name()).ascii().get_data());
										 // we cannot delete this object
	return entity;
}

flecs::entity RenderUtility3D::_create_directional_light(const flecs::world *world, const RID &light_id, const Transform3D &transform, const String &name) {
	if (!light_id.is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<DirectionalLight3DComponent>({ light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world->get<World3DComponent>().scenario_id) })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_directional_light(const flecs::world *world, const Transform3D &transform, const String &name) {
	const RID directional_light_id = RS::get_singleton()->directional_light_create();
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<DirectionalLight3DComponent>({ directional_light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(directional_light_id, world->get<World3DComponent>().scenario_id) })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_directional_light(const flecs::world *world, DirectionalLight3D *directional_light) {
	if (directional_light == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = directional_light->get_instance_id();
	NodeStorage::add(directional_light, directional_light->get_instance_id());
	const flecs::entity entity = world->entity()
										 .set<DirectionalLight3DComponent>({ directional_light->get_base() })
										 .set<Transform3DComponent>({ directional_light->get_transform() })
										 .set<VisibilityComponent>({ true })
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set<RenderInstanceComponent>({ directional_light->get_instance() })
										 .set_name(String(directional_light->get_name()).ascii().get_data());
	return entity;
}

flecs::entity RenderUtility3D::_create_omni_light(const flecs::world *world, const RID &light_id, const Transform3D &transform, const RID &scenario_id) {
	return world->entity("OmniLight")
			.set<OmniLightComponent>({ light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.add<DirtyTransform>()
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, scenario_id) });
}

flecs::entity RenderUtility3D::_create_omni_light(const flecs::world *world, const Transform3D &transform, const RID &scenario_id) {
	const RID omni_light_id = RS::get_singleton()->omni_light_create();
	return world->entity("OmniLight")
			.set<OmniLightComponent>({ omni_light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.add<DirtyTransform>()
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(omni_light_id, scenario_id) });
}

flecs::entity RenderUtility3D::_create_omni_light(const flecs::world *world, OmniLight3D *omni_light) {
	if (omni_light == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = omni_light->get_instance_id();
	NodeStorage::add(omni_light, omni_light->get_instance_id());
	const flecs::entity entity = world->entity()
										 .set<OmniLightComponent>({ omni_light->get_base() })
										 .set<Transform3DComponent>({ omni_light->get_transform() })
										 .set<RenderInstanceComponent>({ omni_light->get_instance() })
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set<VisibilityComponent>({ true })
										 .add<DirtyTransform>()
										 .set_name(String(omni_light->get_name()).ascii().get_data());
	return entity;
}

flecs::entity RenderUtility3D::_create_spot_light(const flecs::world *world, const RID &light_id, const Transform3D &transform, const String &name) {
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<SpotLightComponent>({ light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(light_id, world->get<World3DComponent>().scenario_id) })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_spot_light(const flecs::world *world, const Transform3D &transform, const String &name) {
	const RID spot_light_id = RS::get_singleton()->spot_light_create();
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<SpotLightComponent>({ spot_light_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(spot_light_id, world->get<World3DComponent>().scenario_id) })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_spot_light(const flecs::world *world, SpotLight3D *spot_light) {
	if (spot_light == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	const RID &spot_light_id = spot_light->get_base();
	if (!spot_light_id.is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	if (!spot_light->get_instance().is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	if (!spot_light->get_base().is_valid()) {
		ERR_FAIL_V(flecs::entity());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = spot_light->get_instance_id();
	NodeStorage::add(spot_light, spot_light->get_instance_id());
	const flecs::entity entity = world->entity()
										 .set<SpotLightComponent>({ spot_light->get_base() })
										 .set<Transform3DComponent>({ spot_light->get_transform() })
										 .set<VisibilityComponent>({ true })
										 .add<DirtyTransform>()
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set<RenderInstanceComponent>({ spot_light->get_instance() })
										 .set_name(String(spot_light->get_name()).ascii().get_data());
	return entity;
}

flecs::entity RenderUtility3D::_create_viewport(const flecs::world *world, const RID &viewport_id, const String &name) {
	return world->entity()
			.set<ViewportComponent>({ viewport_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_viewport(const flecs::world *world, Viewport *viewport) {
	if (viewport == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = viewport->get_instance_id();
	NodeStorage::add(viewport, viewport->get_instance_id());
	const flecs::entity entity = world->entity("Viewport")
										 .set<ViewportComponent>({ viewport->get_viewport_rid() })
										 .set<ObjectInstanceComponent>(object_instance_component);
	return entity;
}

flecs::entity RenderUtility3D::_create_voxel_gi(const flecs::world *world, const RID &voxel_gi_id, const Transform3D &transform, const String &name) {
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<VoxelGIComponent>({ voxel_gi_id })
			.set<Transform3DComponent>({ transform })
			.set<VisibilityComponent>({ true })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world->get<World3DComponent>().scenario_id) })
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_voxel_gi(const flecs::world *world, const Transform3D &transform, const String &name) {
	const RID voxel_gi_id = RS::get_singleton()->voxel_gi_create();
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	return world->entity()
			.set<VoxelGIComponent>({ voxel_gi_id })
			.set<Transform3DComponent>({ transform })
			.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(voxel_gi_id, world->get<World3DComponent>().scenario_id) })
			.set<VisibilityComponent>({ true })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_voxel_gi(const flecs::world *world, VoxelGI *voxel_gi) {
	if (voxel_gi == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = voxel_gi->get_instance_id();
	NodeStorage::add(voxel_gi, voxel_gi->get_instance_id());
	const flecs::entity entity = world->entity(String(voxel_gi->get_name()).ascii().get_data())
										 .set<VoxelGIComponent>({ voxel_gi->get_base() })
										 .set<Transform3DComponent>({ voxel_gi->get_transform() })
										 .set<RenderInstanceComponent>({ voxel_gi->get_instance() })
										 .set<VisibilityComponent>({ true })
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set_name(String(voxel_gi->get_name()).ascii().get_data());
	return entity;
}

flecs::entity RenderUtility3D::_create_scenario(const flecs::world *world, const RID &scenario_id, const String &name) {
	return world->entity()
			.set<ScenarioComponent>({ scenario_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_scenario(const flecs::world *world, const String &name) {
	const RID scenario_id = RS::get_singleton()->scenario_create();
	return world->entity()
			.set<ScenarioComponent>({ scenario_id })
			.set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_occluder(const flecs::world *world, const String &name) {
	return _create_occluder(world, RS::get_singleton()->occluder_create(), name);
}

flecs::entity RenderUtility3D::_create_occluder(const flecs::world *world, const RID &occluder_id, const String &name) {
	return world->entity().set<Occluder>({ occluder_id }).set_name(name.ascii().get_data());
}

flecs::entity RenderUtility3D::_create_occluder(const flecs::world *world, OccluderInstance3D *occluder_instance) {
	if (occluder_instance == nullptr) {
		ERR_FAIL_V(flecs::entity());
	}
	const Ref<Occluder3D> occluder = occluder_instance->get_occluder();
	const PackedVector3Array vertices = occluder->get_vertices();
	const PackedInt32Array indices = occluder->get_indices();
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = occluder_instance->get_instance_id();
	NodeStorage::add(occluder_instance, occluder_instance->get_instance_id());
	if(!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), flecs::entity());
	}
	const flecs::entity entity = world->entity()
		.set<RenderInstanceComponent>({ RS::get_singleton()->instance_create2(occluder->get_rid(), world->get<World3DComponent>().scenario_id) })
		.set<Occluder>({ occluder->get_rid(),{},vertices,indices }).set_name(occluder->get_name().ascii().get_data())
		.set<Transform3DComponent>({ occluder_instance->get_transform() })
		.set<ObjectInstanceComponent>(object_instance_component);
	RefStorage::add(occluder, occluder->get_rid());

	return entity;
}

bool RenderUtility3D::_bake_material_check(const Ref<Material> &p_material) {
	const StandardMaterial3D *standard_mat = Object::cast_to<StandardMaterial3D>(p_material.ptr());
	if (standard_mat && standard_mat->get_transparency() != StandardMaterial3D::TRANSPARENCY_DISABLED) {
		return false;
	}
	return true;
}

void RenderUtility3D::_bake_surface(const Transform3D &p_transform, const Array& p_surface_arrays, const Ref<Material> &p_material, float p_simplification_dist, const PackedVector3Array &r_vertices, const PackedInt32Array &r_indices) {
	if (!_bake_material_check(p_material)) {
		return;
	}
	ERR_FAIL_COND_MSG(p_surface_arrays.size() != Mesh::ARRAY_MAX, "Invalid surface array.");

	PackedVector3Array vertices = p_surface_arrays[Mesh::ARRAY_VERTEX];
	PackedInt32Array indices = p_surface_arrays[Mesh::ARRAY_INDEX];

	if (vertices.size() == 0 || indices.size() == 0) {
		return;
	}

	Vector3 *vertices_ptr = vertices.ptrw();
	for (int j = 0; j < vertices.size(); j++) {
		vertices_ptr[j] = p_transform.xform(vertices_ptr[j]);
	}

	if (!Math::is_zero_approx(p_simplification_dist) && SurfaceTool::simplify_func) {
		Vector<float> vertices_f32 = vector3_to_float32_array(vertices.ptr(), vertices.size());

		float error_scale = SurfaceTool::simplify_scale_func(vertices_f32.ptr(), vertices.size(), sizeof(float) * 3);
		float target_error = p_simplification_dist / error_scale;
		float error = -1.0f;
		int target_index_count = MIN(indices.size(), 36);

		const int simplify_options = SurfaceTool::SIMPLIFY_LOCK_BORDER;

		uint32_t index_count = SurfaceTool::simplify_func(
				(unsigned int *)indices.ptrw(),
				(unsigned int *)indices.ptr(),
				indices.size(),
				vertices_f32.ptr(), vertices.size(), sizeof(float) * 3,
				target_index_count, target_error, simplify_options, &error);
		indices.resize(index_count);
	}
}

Ref<FlecsEntity> RenderUtility3D::create_mesh_instance(FlecsWorld *flecs_world, const RID &mesh_id, const Transform3D &transform, const String &name, const RID &scenario_id)  {
	flecs::entity e = _create_mesh_instance(flecs_world->get_world_ref(), mesh_id, transform, name, scenario_id);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);

	MeshComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_mesh_instance_with_object(FlecsWorld *flecs_world, MeshInstance3D *mesh_instance_3d) {
	if (mesh_instance_3d == nullptr) {
		ERR_FAIL_V(Ref<FlecsEntity>());
	}
	flecs::entity e = _create_mesh_instance(flecs_world->get_world_ref(), mesh_instance_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	MeshComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

TypedArray<FlecsEntity> RenderUtility3D::create_multi_mesh(FlecsWorld *flecs_world, const Transform3D &transform, uint32_t size, const RID &mesh_id, const TypedArray<RID> &material_ids, const RID &scenario_id, const String &name, bool use_colors, bool use_custom_data, bool use_indirect)  {
	TypedArray<FlecsEntity> entities;
	Vector<RID> material_ids_vector;
	for (int i = 0; i < material_ids.size(); ++i) {
		material_ids_vector.push_back(material_ids[i]);
	}
	if (material_ids_vector.is_empty()) {
		// If no materials are provided, we can use an empty RID for the mesh component
		material_ids_vector.push_back(RID());
	}
	flecs::world* world = flecs_world->get_world_ref();
	flecs::entity e = _create_multi_mesh(world, transform, size, mesh_id, material_ids_vector, scenario_id, name, use_colors, use_custom_data, use_indirect);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	entities.push_back(flecs_entity);
	MultiMeshComponentRef::create_component(flecs_entity);
	MeshComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);

	Vector<flecs::entity> entity_buffer;
	entity_buffer.resize(e.get<MultiMeshComponent>().instance_count);
	entities.resize(e.get<MultiMeshComponent>().instance_count+1);
	int i = 0;
	e.children([&](flecs::entity child) {
		entity_buffer.write[i] = child;
		i++;
	});
	world->defer_begin();
	i = 1;
	for(auto& child : entity_buffer){
		Ref<FlecsEntity> child_entity = flecs_world->add_entity(child);
		entities.set(i,child_entity);
		MultiMeshInstanceComponentRef::create_component(child_entity);
		Transform3DComponentRef::create_component(child_entity);
		VisibilityComponentRef::create_component(child_entity);
		i++;
	}
	
	world->defer_end();
	return entities;
}

TypedArray<FlecsEntity> RenderUtility3D::create_multi_mesh_with_object(FlecsWorld *flecs_world, MultiMeshInstance3D *multi_mesh_instance_3d) {
	TypedArray<FlecsEntity> entities;
	if (multi_mesh_instance_3d == nullptr) {
		ERR_FAIL_COND_V(multi_mesh_instance_3d == nullptr,entities);
	}
	if(multi_mesh_instance_3d->get_multimesh().is_null()){
		ERR_FAIL_COND_V(multi_mesh_instance_3d->get_multimesh().is_null(),entities);
	}
	flecs::world* world = flecs_world->get_world_ref();
	flecs::entity e = _create_multi_mesh(world, multi_mesh_instance_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	entities.push_back(flecs_entity);
	MultiMeshComponentRef::create_component(flecs_entity);
	MeshComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);

	Vector<flecs::entity> entity_buffer;
	entity_buffer.resize(e.get<MultiMeshComponent>().instance_count);
	int i = 0;
	e.children([&](flecs::entity child) {
		if (child.is_alive() == false) {
			// Skip dead entities
			return;
		}
		entity_buffer.write[i] = child;
		i++;
	});
	world->defer_begin();
	i = 1;

	entities.resize(e.get<MultiMeshComponent>().instance_count+1);
	entities.set(0,flecs_entity);
	for(auto& child : entity_buffer){
		Ref<FlecsEntity> child_entity = flecs_world->add_entity(child);
		entities.set(i,child_entity);
		MultiMeshInstanceComponentRef::create_component(child_entity);
		Transform3DComponentRef::create_component(child_entity);
		VisibilityComponentRef::create_component(child_entity);
		i++;
	}
	
	world->defer_end();
	return entities;
}

Ref<FlecsEntity> RenderUtility3D::create_camera(FlecsWorld *flecs_world, const Transform3D &transform, const String &name)  {
	flecs::entity e = _create_camera(flecs_world->get_world_ref(), transform, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	Ref<FlecsEntity> compositor_entity = flecs_world->add_entity(e.parent());
	CompositorComponentRef::create_component(compositor_entity);
	CameraComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;

}

Ref<FlecsEntity> RenderUtility3D::create_directional_light(FlecsWorld *flecs_world, const Transform3D &transform, const String &name)  {
	flecs::entity e = _create_directional_light(flecs_world->get_world_ref(), transform, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	DirectionalLight3DComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_directional_light_with_object(FlecsWorld *flecs_world, DirectionalLight3D *directional_light_3d) {
	flecs::entity e = _create_directional_light(flecs_world->get_world_ref(), directional_light_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	DirectionalLight3DComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_camera_with_object(FlecsWorld *flecs_world, Camera3D *camera_3d) {
	flecs::entity e = _create_camera(flecs_world->get_world_ref(), camera_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	CameraComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_voxel_gi(FlecsWorld *flecs_world, const RID& voxel_gi_rid, const Transform3D &transform, const String &name)  {
	flecs::entity e = _create_voxel_gi(flecs_world->get_world_ref(),voxel_gi_rid, transform, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	VoxelGIComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_spot_light(FlecsWorld *flecs_world, const RID &light_id, const Transform3D &transform, const String &name)  {
	flecs::entity e = _create_spot_light(flecs_world->get_world_ref(),light_id, transform, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	SpotLightComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_spot_light_with_object(FlecsWorld *flecs_world, SpotLight3D *spot_light) {
	flecs::entity e = _create_spot_light(flecs_world->get_world_ref(), spot_light);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	SpotLightComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_omni_light(FlecsWorld *flecs_world, const RID &light_id, const Transform3D &transform, const RID &scenario_id)  {
	flecs::entity e = _create_omni_light(flecs_world->get_world_ref(),light_id, transform, scenario_id);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	OmniLightComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_omni_light_with_object(FlecsWorld *flecs_world, OmniLight3D *omni_light) {
	flecs::entity e = _create_omni_light(flecs_world->get_world_ref(), omni_light);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	OmniLightComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_reflection_probe(FlecsWorld *flecs_world, const RID &probe_id, const Transform3D &transform, const String &name)  {
	flecs::entity e = _create_reflection_probe(flecs_world->get_world_ref(), probe_id, transform, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ReflectionProbeComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_scenario(FlecsWorld *flecs_world,const RID &scenario_id, const String &name)  {
	flecs::entity e = _create_scenario(flecs_world->get_world_ref(),scenario_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ScenarioComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_reflection_probe_with_object(FlecsWorld *flecs_world, ReflectionProbe *reflection_probe) {
	flecs::entity e = _create_reflection_probe(flecs_world->get_world_ref(), reflection_probe);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ReflectionProbeComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_particles(FlecsWorld *flecs_world, const Transform3D &transform, const RID& particles_id, const RID &scenario_id, const String &name)  {
	flecs::entity e = _create_particles(flecs_world->get_world_ref(), transform, particles_id, scenario_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ParticlesComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_particles_with_object(FlecsWorld *flecs_world, GPUParticles3D *gpu_particles_3d)  {
	flecs::entity e = _create_particles(flecs_world->get_world_ref(), gpu_particles_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ParticlesComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_viewport(FlecsWorld *flecs_world, const RID &viewport_id, const String &name)  {
	flecs::entity e = _create_viewport(flecs_world->get_world_ref(), viewport_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ViewportComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_viewport_with_object(FlecsWorld *flecs_world, Viewport *viewport)  {
	flecs::entity e = _create_viewport(flecs_world->get_world_ref(), viewport);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	ViewportComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_voxel_gi_with_object(FlecsWorld *flecs_world, VoxelGI *voxel_gi)  {
	flecs::entity e = _create_voxel_gi(flecs_world->get_world_ref(), voxel_gi);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	VoxelGIComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_environment(FlecsWorld *flecs_world, const RID &environment_id, const String &name)  {
	flecs::entity e = _create_environment(flecs_world->get_world_ref(), environment_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	EnvironmentComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);

	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_environment_with_object(FlecsWorld *flecs_world, WorldEnvironment *world_environment)  {
	flecs::entity e = _create_environment(flecs_world->get_world_ref(), world_environment);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	EnvironmentComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_skeleton(FlecsWorld *flecs_world, const RID &skeleton_id, const String &name)  {
	flecs::entity e = _create_skeleton(flecs_world->get_world_ref(), skeleton_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	SkeletonComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	// Add the skeleton component to the entity
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_skeleton_with_object(FlecsWorld *flecs_world, Skeleton3D *skeleton_3d)  {
	flecs::entity e = _create_skeleton(flecs_world->get_world_ref(), skeleton_3d);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	SkeletonComponentRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);

	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_compositor(FlecsWorld *flecs_world, const RID &compositor_id, const String &name)  {
	flecs::entity e = _create_compositor(flecs_world->get_world_ref(), compositor_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	CompositorComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_compositor_with_object(FlecsWorld *flecs_world, Compositor *compositor)  {
	flecs::entity e = _create_compositor(flecs_world->get_world_ref(), compositor);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	CompositorComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_occluder_with_object(FlecsWorld *flecs_world, OccluderInstance3D *occluder_instance)  {
	if (occluder_instance == nullptr) {
		ERR_FAIL_V(Ref<FlecsEntity>());
	}
	flecs::entity e = _create_occluder(flecs_world->get_world_ref(), occluder_instance);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	OccluderRef::create_component(flecs_entity);
	Transform3DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
	ObjectInstanceComponentRef::create_component(flecs_entity);
	VisibilityComponentRef::create_component(flecs_entity);
	return flecs_entity;
}

Ref<FlecsEntity> RenderUtility3D::create_occluder(FlecsWorld *flecs_world, const RID &occluder_id, const String &name)  {
	flecs::entity e = _create_occluder(flecs_world->get_world_ref(), occluder_id, name);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	return flecs_entity;
}

void RenderUtility3D::_bind_methods(){
	ClassDB::bind_static_method(get_class_static(), "create_particles",
		&RenderUtility3D::create_particles, "flecs_world", "transform", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_particles_with_object",
		&RenderUtility3D::create_particles_with_object, "flecs_world", "gpu_particles_3d");
	ClassDB::bind_static_method(get_class_static(), "create_mesh_instance",
		&RenderUtility3D::create_mesh_instance, "flecs_world", "mesh_id", "transform", "name", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_multi_mesh",
	&RenderUtility3D::create_multi_mesh, "flecs_world", "transform", "size", "mesh_id", "material_ids");
	ClassDB::bind_static_method(get_class_static(), "create_camera",
		&RenderUtility3D::create_camera, "flecs_world", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_directional_light",
		&RenderUtility3D::create_directional_light, "flecs_world", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_voxel_gi",
		&RenderUtility3D::create_voxel_gi, "flecs_world", "voxel_gi_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_spot_light",
		&RenderUtility3D::create_spot_light, "flecs_world","light_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_spot_light_with_object",
		&RenderUtility3D::create_spot_light_with_object, "flecs_world", "spot_light");
	ClassDB::bind_static_method(get_class_static(), "create_mesh_instance_with_object",
		&RenderUtility3D::create_mesh_instance_with_object, "flecs_world", "mesh_instance_3d");
	ClassDB::bind_static_method(get_class_static(), "create_omni_light",
		&RenderUtility3D::create_omni_light, "flecs_world","light_id", "transform", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_omni_light_with_object",
		&RenderUtility3D::create_omni_light_with_object, "flecs_world", "omni_light");
	ClassDB::bind_static_method(get_class_static(), "create_reflection_probe",
		&RenderUtility3D::create_reflection_probe, "flecs_world", "probe_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_scenario",
		&RenderUtility3D::create_scenario, "flecs_world", "name");
	ClassDB::bind_static_method(get_class_static(), "create_viewport",
		&RenderUtility3D::create_viewport, "flecs_world", "viewport_id", "name");
	ClassDB::bind_static_method(get_class_static(), "create_viewport_with_object",
		&RenderUtility3D::create_viewport_with_object, "flecs_world", "viewport");
	ClassDB::bind_static_method(get_class_static(), "create_voxel_gi_with_object",
		&RenderUtility3D::create_voxel_gi_with_object, "flecs_world", "voxel_gi");
	ClassDB::bind_static_method(get_class_static(), "create_environment",
		&RenderUtility3D::create_environment, "flecs_world", "environment_id", "name");
	ClassDB::bind_static_method(get_class_static(), "create_environment_with_object",
		&RenderUtility3D::create_environment_with_object, "flecs_world", "world_environment");
	ClassDB::bind_static_method(get_class_static(), "create_skeleton",
		&RenderUtility3D::create_skeleton, "flecs_world", "skeleton_id", "name");
	ClassDB::bind_static_method(get_class_static(), "create_skeleton_with_object",
		&RenderUtility3D::create_skeleton_with_object, "flecs_world", "skeleton_3d");
	ClassDB::bind_static_method(get_class_static(), "create_compositor",
		&RenderUtility3D::create_compositor, "flecs_world", "compositor_id", "name");
	ClassDB::bind_static_method(get_class_static(), "create_compositor_with_object",
		&RenderUtility3D::create_compositor_with_object, "flecs_world", "compositor");
	ClassDB::bind_static_method(get_class_static(), "create_occluder_with_object",
		&RenderUtility3D::create_occluder_with_object, "flecs_world", "occluder_instance");
	ClassDB::bind_static_method(get_class_static(), "create_occluder",
		&RenderUtility3D::create_occluder, "flecs_world", "occluder_id", "name");
	ClassDB::bind_static_method(get_class_static(), "bake_material_check",
		&RenderUtility3D::_bake_material_check, "p_material");
	ClassDB::bind_static_method(get_class_static(), "bake_surface",
		&RenderUtility3D::_bake_surface, "p_transform", "p_surface_arrays", "p_material", "p_simplification_dist", "r_vertices", "r_indices");
}