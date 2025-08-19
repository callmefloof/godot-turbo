//
// Created by Floof on 29-7-2025.
//

#include "render_utility_3d.h"

#include "core/error/error_macros.h"
#include "core/io/marshalls.h"
#include "core/math/math_funcs.h"
#include "core/math/rect2i.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "flecs.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/gpu_particles_3d.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/3d/reflection_probe.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/3d/world_environment.h"
#include "scene/resources/compositor.h"
#include "scene/resources/environment.h"
#include "scene/resources/material.h"
#include "scene/resources/mesh.h"
#include "scene/resources/multimesh.h"
#include "scene/resources/surface_tool.h"
#include "core/object/class_db.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/object_instance_component.h"
#include "ecs/components/worldcomponents.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/dirty_transform.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "scene/3d/occluder_instance_3d.h"
#include "ecs/flecs_types/flecs_server.h"
#include "core/templates/rid.h"
#include "scene/main/viewport.h"
#include "servers/rendering_server.h"
#include <cassert>
#include "scene/3d/voxel_gi.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/object_instance_component.h"
#include "core/templates/vector.h"
#include <cstdint>
#include <vector>
#include "ecs/components/dirty_transform.h"
#include "ecs/flecs_types/flecs_server.h"
#include "worldcomponents.h"

RenderUtility3D::~RenderUtility3D() {
}

RID RenderUtility3D::create_mesh_instance_with_id(const RID &world_id, const RID &mesh_id, const Transform3D &transform, const String &name, const RID &scenario_id) {
	Vector<RID> material_ids;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	
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
	auto transform_component = Transform3DComponent();
	transform_component.transform = transform;
	RID render_instance_id = RS::get_singleton()->instance_create2(mesh_id, scenario_id);
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = render_instance_id;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	flecs::entity e = world->entity()
			.set<MeshComponent>(mesh_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_mesh_instance(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name) {
	TypedArray<RID> material_ids;
	const RID mesh_id = RS::get_singleton()->mesh_create();
	return create_mesh_instance_with_id(world_id, mesh_id, transform, name, scenario_id);
}

RID RenderUtility3D::create_mesh_instance_with_object(const RID &world_id, MeshInstance3D *mesh_instance_3d) {
	ERR_FAIL_COND_V(!mesh_instance_3d, RID());
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	ERR_FAIL_COND_V(!world, RID());
	Vector<RID> material_ids;
	const Ref<Mesh> mesh = mesh_instance_3d->get_mesh();
	if (!mesh.is_valid()) {
		ERR_FAIL_COND_V(!mesh.is_valid(), RID());
	}
	FlecsServer::get_singleton()->add_to_ref_storage(mesh, world_id);
	if (!mesh_instance_3d->get_base().is_valid()) {
		ERR_FAIL_COND_V(!mesh_instance_3d->get_base().is_valid(), RID());
	}
	const RID &mesh_rid = mesh->get_rid();
	if (!mesh_rid.is_valid()) {
		ERR_FAIL_COND_V(!mesh_rid.is_valid(), RID());
	}
	if (!mesh_instance_3d->get_instance().is_valid()) {
		ERR_FAIL_COND_V(!mesh_instance_3d->get_instance().is_valid(), RID());
	}
	const RID &instance_rid = mesh_instance_3d->get_instance();
	if (!instance_rid.is_valid()) {
		ERR_FAIL_COND_V(!instance_rid.is_valid(), RID());
	}

	const RID &base = mesh_instance_3d->get_base();
	
	if (!base.is_valid()) {
		ERR_FAIL_COND_V(!base.is_valid(), RID());
	}

	const RID& scenario_id = world->get<World3DComponent>().scenario_id;
	if (!scenario_id.is_valid()) {
		ERR_FAIL_COND_V(!scenario_id.is_valid(), RID());
	}
	const RID &instance = RS::get_singleton()->instance_create2(mesh_rid, scenario_id);
	if (!instance.is_valid()) {
		ERR_FAIL_COND_V(!instance.is_valid(), RID());
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
	FlecsServer::get_singleton()->add_to_node_storage(mesh_instance_3d, world_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = mesh_instance_3d->get_instance_id();
	Transform3DComponent transform_component;
	transform_component.transform = mesh_instance_3d->get_transform();
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = instance;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	flecs::entity e = world->entity()
			.set<MeshComponent>(mesh_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<VisibilityComponent>(visibility_component)
			.set<ObjectInstanceComponent>(object_instance_component)
			.add<DirtyTransform>()
			.set_name(String(mesh->get_name()).ascii().get_data());
	FlecsServer::get_singleton()->add_to_node_storage(mesh_instance_3d, world_id);
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_multi_mesh(const RID &world_id,
		const Transform3D &transform,
		const uint32_t size,
		const RID &mesh_id,
		const TypedArray<RID> &material_ids,
		const RID &scenario_id,
		const String &name,
		const bool use_colors,
		const bool use_custom_data,
		const bool use_indirect) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	const RID multi_mesh_id = RS::get_singleton()->multimesh_create();
	if (!multi_mesh_id.is_valid()) {
		ERR_FAIL_V(RID());
	}
	RS::get_singleton()->multimesh_set_mesh(multi_mesh_id, mesh_id);
	RS::get_singleton()->multimesh_allocate_data(multi_mesh_id, size, RS::MULTIMESH_TRANSFORM_3D, use_colors, use_custom_data, use_indirect);
	auto mesh_component = MeshComponent();
	Vector<RID> material_ids_vector;
	for (int i = 0; i < material_ids.size(); i++) {
		material_ids_vector.push_back(material_ids[i]);
	}
	mesh_component.material_ids = material_ids_vector;
	mesh_component.mesh_id = mesh_id;
	auto multi_mesh_component = MultiMeshComponent();
	multi_mesh_component.multi_mesh_id = multi_mesh_id;
	multi_mesh_component.instance_count = size;
	RID instance_id = RS::get_singleton()->instance_create2(multi_mesh_id, scenario_id);
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = instance_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;


	flecs::entity e = world->entity()
			.set<MultiMeshComponent>(multi_mesh_component)
			.set<MeshComponent>(mesh_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<Transform3DComponent>(transform_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	const RID entity = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);

	return entity;
}

TypedArray<RID> RenderUtility3D::create_multi_mesh_with_object(const RID &world_id, MultiMeshInstance3D *multi_mesh_instance) {
	const Ref<MultiMesh> multi_mesh = multi_mesh_instance->get_multimesh();
	TypedArray<RID> entities;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (multi_mesh.is_null()) {
		ERR_FAIL_COND_V(multi_mesh.is_null(), TypedArray<RID>());
	}
	if (!multi_mesh.is_valid()) {
		ERR_FAIL_COND_V(!multi_mesh.is_valid(), TypedArray<RID>());
	}
	FlecsServer::get_singleton()->add_to_ref_storage(multi_mesh, world_id);



	const RID &multi_mesh_id = multi_mesh->get_rid();
	const RID &mesh_id = multi_mesh->get_mesh()->get_rid();
	if (!multi_mesh_id.is_valid()) {
		ERR_FAIL_COND_V(!multi_mesh_id.is_valid(), TypedArray<RID>());
	}
	if (!mesh_id.is_valid()) {
		ERR_FAIL_COND_V(!mesh_id.is_valid(), TypedArray<RID>());
	}
	auto mesh_ref = multi_mesh_instance->get_multimesh()->get_mesh();
	FlecsServer::get_singleton()->add_to_ref_storage(mesh_ref, world_id);
	if(!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), TypedArray<RID>());
	}
	const RID &instance_id = RS::get_singleton()->instance_create2(multi_mesh_id, world->get<World3DComponent>().scenario_id);

	const String name = multi_mesh_instance->get_name();
	const Transform3D transform = multi_mesh_instance->get_transform();
	const uint32_t size = multi_mesh_instance->get_multimesh()->get_instance_count();
	entities.resize(size+1);
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
			FlecsServer::get_singleton()->add_to_ref_storage(material, world_id);
			material_ids.append(material->get_rid());
		} else{
			ERR_PRINT("Material not set.");
		}
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = multi_mesh_instance->get_instance_id();
	const AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_id);
	MultiMeshComponent multi_mesh_component;
	multi_mesh_component.multi_mesh_id = multi_mesh_id;
	multi_mesh_component.instance_count = size;
	MeshComponent mesh_component;
	mesh_component.mesh_id = mesh_id;
	mesh_component.material_ids = material_ids;
	mesh_component.custom_aabb = custom_aabb;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = instance_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	flecs::entity e = world->entity()
		.set<MultiMeshComponent>(multi_mesh_component)
		.set<MeshComponent>(mesh_component)
		.set<RenderInstanceComponent>(render_instance_component)
		.set<Transform3DComponent>(transform_component)
		.set<VisibilityComponent>(visibility_component)
		.set<ObjectInstanceComponent>(object_instance_component)
		.add<DirtyTransform>()
		.set_name(name.ascii().get_data());
	const RID entity = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
	entities[0] = entity;
	TypedArray<Transform3D> transforms;
	transforms.resize(size);
	for (uint32_t i = 0; i < size; ++i) {
		transforms[i] = multi_mesh_instance->get_transform();
	}
	entities.append_array(create_multi_mesh_instances(world_id, transforms, entity));



	return entities;
}

TypedArray<RID> RenderUtility3D::create_multi_mesh_instances(const RID &world_id,const TypedArray<Transform3D> &transforms, const RID &multi_mesh_entity_id) {
	TypedArray<RID> entities;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	flecs::entity mm_entity = FlecsServer::get_singleton()->_get_entity(multi_mesh_entity_id, world_id);
	uint32_t instance_count = mm_entity.get<MultiMeshComponent>().instance_count;


	// Bulk assign components
	std::vector<MultiMeshInstanceComponent> mm_components(instance_count);
	std::vector<Transform3DComponent> transform_components(instance_count);
	std::vector<VisibilityComponent> visibility_components(instance_count);


	for (uint32_t i = 0; i < instance_count; ++i) {
		mm_components[i].index = i;
		transform_components[i].transform = transforms[i];
		visibility_components[i].visible = true;
	}


	std::vector<void*> data(instance_count * 3);
	data[0] = mm_components.data();
	data[1] = transform_components.data();
	data[2] = visibility_components.data();
	data[3] = nullptr; // Needed for pair/tag

	if(instance_count > std::numeric_limits<int32_t>::max()) {
		ERR_PRINT("Instance count exceeds maximum limit of uint32_t.");
		return entities;
	}

	ecs_bulk_desc_t bulk_desc = {0, nullptr, static_cast<int32_t>(instance_count), {
		world->lookup("MultiMeshInstanceComponent").id(),
		world->lookup("Transform3DComponent").id(),
		world->lookup("VisibilityComponent").id(),
		world->pair(flecs::ChildOf, mm_entity),
	}, data.data()};

	auto entity_ids = ecs_bulk_init(world->c_ptr(), &bulk_desc);

	return entities;
}

RID RenderUtility3D::create_multi_mesh_instance(
		const RID &world_id,
		const Transform3D &transform,
		const uint32_t index,
		const RID &multi_mesh_id,
		const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	MultiMeshInstanceComponent multi_mesh_instance_component;
	multi_mesh_instance_component.index = index;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	flecs::entity e = world->entity()
			.set<MultiMeshInstanceComponent>(multi_mesh_instance_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_particles(
		const RID &world_id,
		const Transform3D &transform,
		const RID &particles_id,
		const int particle_count,
		const RID &scenario_id,
		const String &name) {
	ParticlesComponent particles_component;
	particles_component.particles_id = particles_id;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(particles_id, scenario_id);
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	world->entity()
			.set<ParticlesComponent>(particles_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
		
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, world->entity());
}

RID RenderUtility3D::create_particles_with_object(const RID &world_id, GPUParticles3D *gpu_particles_3d) {
	if (gpu_particles_3d == nullptr) {
		ERR_FAIL_V(RID());
	}
	flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);

	if(!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	const RID& instance_id = RS::get_singleton()->instance_create2(gpu_particles_3d->get_base(), world->get<World3DComponent>().scenario_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = gpu_particles_3d->get_instance_id();
	ParticlesComponent particles_component;
	particles_component.particles_id = gpu_particles_3d->get_base();
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = instance_id;
	Transform3DComponent transform_component;
	transform_component.transform = gpu_particles_3d->get_transform();
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	auto &particles = world->entity()
							.set<ParticlesComponent>(particles_component)
							.set<RenderInstanceComponent>(render_instance_component)
							.set<Transform3DComponent>(transform_component)
							.set<VisibilityComponent>(visibility_component)
							.set<ObjectInstanceComponent>(object_instance_component)
							.add<DirtyTransform>()
							.set_name(String(gpu_particles_3d->get_name()).ascii().get_data());
	FlecsServer::get_singleton()->add_to_node_storage(gpu_particles_3d, world_id);
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, particles);
}

RID RenderUtility3D::create_reflection_probe(const RID &world_id, const RID &probe_id, const Transform3D &transform, const String &name) {
	flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	ReflectionProbeComponent reflection_probe_component;
	reflection_probe_component.probe_id = probe_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(probe_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
			.set<ReflectionProbeComponent>(reflection_probe_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_reflection_probe_with_object(const RID &world_id, ReflectionProbe *reflection_probe) {
	RID entity_probe = create_reflection_probe(world_id, reflection_probe->get_base(), reflection_probe->get_transform(), reflection_probe->get_name());
	FlecsServer::get_singleton()->add_to_node_storage(reflection_probe, world_id);
	ObjectInstanceComponent object_instance_component = ObjectInstanceComponent(reflection_probe->get_instance_id());
	flecs::entity e = FlecsServer::get_singleton()->_get_entity(entity_probe, world_id);
	e.set<ObjectInstanceComponent>(object_instance_component);
	return entity_probe;
}

RID RenderUtility3D::create_skeleton(const RID &world_id, const RID &skeleton_id, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	SkeletonComponent skeleton_component;
	skeleton_component.skeleton_id = skeleton_id;
	flecs::entity e = world->entity()
			.set<SkeletonComponent>(skeleton_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_skeleton_with_object(const RID &world_id, Skeleton3D *skeleton_3d) {
	const RID skeleton_id = RS::get_singleton()->skeleton_create();
	if (skeleton_3d == nullptr) {
		ERR_FAIL_V(RID());
	}
	RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_3d->get_bone_count(), false);
	for (int i = 0; i < skeleton_3d->get_bone_count(); ++i) {
		RS::get_singleton()->skeleton_bone_set_transform(skeleton_id, i, skeleton_3d->get_bone_global_pose(i));
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = skeleton_3d->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(skeleton_3d, world_id);
	SkeletonComponent skeleton_component;
	skeleton_component.skeleton_id = skeleton_id;
	Transform3DComponent transform_component;
	transform_component.transform = skeleton_3d->get_transform();
	RID render_instance_id = RS::get_singleton()->instance_create2(skeleton_id, world->get<World3DComponent>().scenario_id);
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = render_instance_id;
	flecs::entity e = world->entity()
			.set<SkeletonComponent>(skeleton_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<ObjectInstanceComponent>(object_instance_component)
			.add<DirtyTransform>()
			.set_name(String(skeleton_3d->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_environment(const RID &world_id, const RID &environment_id, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	EnvironmentComponent environment_component;
	environment_component.environment_id = environment_id;
	flecs::entity e = world->entity()
			.set<EnvironmentComponent>(environment_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_environment_with_object(const RID &world_id, WorldEnvironment *world_environment) {
	if (world_environment == nullptr) {
		ERR_FAIL_COND_V(world_environment == nullptr, RID());
	}
	if (world_environment->get_environment().is_null() || !world_environment->get_environment().is_valid()) {
		ERR_FAIL_COND_V(world_environment->get_environment().is_null() || !world_environment->get_environment().is_valid(), RID());
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	const Ref<Environment> &environment_ref = world_environment->get_environment();
	const RID &environment_id = world_environment->get_environment()->get_rid();
	FlecsServer::get_singleton()->add_to_ref_storage(environment_ref, world_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = world_environment->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(world_environment, world_id);
	EnvironmentComponent environment_component;
	environment_component.environment_id = environment_id;
	flecs::entity e = world->entity()
			.set<EnvironmentComponent>(environment_component)
			.set<ObjectInstanceComponent>(object_instance_component)
			.set_name("WorldEnvironment");
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_camera_with_id(const RID &world_id, const RID &camera_id, const Transform3D &transform, const String &name) {
	CameraComponent camera_component;
	camera_component.camera_id = camera_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	flecs::entity e = world->entity()
			.set<CameraComponent>(camera_component)
			.set<Transform3DComponent>(transform_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_camera(const RID &world_id, const Transform3D &transform, const String &name) {
	const RID camera_id = RS::get_singleton()->camera_create();
	if (!camera_id.is_valid()) {
		ERR_FAIL_V(RID());
	}
	CameraComponent camera_component;
	camera_component.camera_id = camera_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	flecs::entity e = world->entity()
			.set<CameraComponent>(camera_component)
			.set<Transform3DComponent>(transform_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_camera_with_object(const RID &world_id, Camera3D *camera_3d) {
	if (camera_3d == nullptr) {
		ERR_FAIL_V(RID());
	}
	Vector2 camera_offset = Vector2(camera_3d->get_h_offset(),camera_3d->get_v_offset());
	CameraComponent camera_component;
	camera_component.camera_id = camera_3d->get_camera();
	camera_component.frustum = camera_3d->get_frustum();
	camera_component.position = camera_3d->get_position();
	camera_component.far = camera_3d->get_far();
	camera_component.near = camera_3d->get_near();
	camera_component.projection = camera_3d->get_camera_projection();
	camera_component.camera_offset = camera_offset;
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = camera_3d->get_instance_id();
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	Transform3DComponent transform_component;
	transform_component.transform = camera_3d->get_camera_transform();
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(camera_3d->get_camera(), world->get<World3DComponent>().scenario_id);


	FlecsServer::get_singleton()->add_to_node_storage(camera_3d, world_id);
	const flecs::entity e = world->entity()
		.set<CameraComponent>(camera_component)
		.set<Transform3DComponent>(transform_component)
		.set<RenderInstanceComponent>(render_instance_component)
		.set<ObjectInstanceComponent>(object_instance_component)
		.set_name(String(camera_3d->get_name()).ascii().get_data());
	const RID camera = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);

	if (camera_3d->get_compositor().is_null() || camera_3d->get_compositor().is_valid()) {
		Ref<Compositor> compositor_ref = camera_3d->get_compositor();
		FlecsServer::get_singleton()->add_to_ref_storage(compositor_ref, world_id);
		if (!compositor_ref.is_valid()) {
			ERR_FAIL_COND_V(!compositor_ref.is_valid(), RID());
		}
		const RID compositor_id = compositor_ref->get_rid();
		RID compositor_entity = create_compositor(world_id, compositor_id, compositor_ref->get_name());
		FlecsServer::get_singleton()->add_child(camera, compositor_entity);
	}

	return camera;
}

RID RenderUtility3D::create_compositor(const RID &world_id, const RID &compositor_id, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	CompositorComponent compositor_component;
	compositor_component.compositor_id = compositor_id;
	flecs::entity e = world->entity()
			.set<CompositorComponent>(compositor_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_compositor_with_object(const RID &world_id, const Ref<Compositor> &compositor) {
	if (compositor == nullptr) {
		ERR_FAIL_V(RID());
	}
	const RID compositor_id = compositor->get_rid();
	if (!compositor_id.is_valid()) {
		ERR_FAIL_V(RID());
	}
	FlecsServer::get_singleton()->add_to_ref_storage(compositor, world_id);
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	CompositorComponent compositor_component;
	compositor_component.compositor_id = compositor_id;
	flecs::entity e = world->entity()
			.set<CompositorComponent>(compositor_component)
			.set_name(String(compositor->get_name()).ascii().get_data());
	const RID entity = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
	return entity;
}

RID RenderUtility3D::create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const String &name) {
	if (!light_id.is_valid()) {
		ERR_FAIL_V(RID());
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	DirectionalLight3DComponent directional_light_component;
	directional_light_component.light_id = light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(light_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
			.set<DirectionalLight3DComponent>(directional_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set<RenderInstanceComponent>(render_instance_component)
			.set_name(name.ascii().get_data());


	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_directional_light(const RID &world_id, const Transform3D &transform, const String &name) {
	const RID directional_light_id = RS::get_singleton()->directional_light_create();
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	DirectionalLight3DComponent directional_light_component;
	directional_light_component.light_id = directional_light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(directional_light_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
			.set<DirectionalLight3DComponent>(directional_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.set<RenderInstanceComponent>(render_instance_component);
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_directional_light_with_object(const RID &world_id, DirectionalLight3D *directional_light) {
	if (directional_light == nullptr) {
		ERR_FAIL_V(RID());
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = directional_light->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(directional_light, world_id);
	DirectionalLight3DComponent directional_light_component;
	directional_light_component.light_id = directional_light->get_base();
	Transform3DComponent transform_component;
	transform_component.transform = directional_light->get_transform();
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = directional_light->get_instance();

	const flecs::entity e = world->entity()
										 .set<DirectionalLight3DComponent>(directional_light_component)
										 .set<Transform3DComponent>(transform_component)
										 .set<VisibilityComponent>(visibility_component)
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set<RenderInstanceComponent>(render_instance_component)
										 .set_name(String(directional_light->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_omni_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const RID &scenario_id, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	OmniLightComponent omni_light_component;
	omni_light_component.light_id = light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(light_id, scenario_id);
	flecs::entity e = world->entity()
			.set<OmniLightComponent>(omni_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set<RenderInstanceComponent>(render_instance_component)
			.set_name(name.ascii().get_data());

	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_omni_light(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name) {
	const RID omni_light_id = RS::get_singleton()->omni_light_create();
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	OmniLightComponent omni_light_component;
	omni_light_component.light_id = omni_light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(omni_light_id, scenario_id);
	flecs::entity e = world->entity()
			.set<OmniLightComponent>(omni_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.add<DirtyTransform>()
			.set<RenderInstanceComponent>(render_instance_component);

	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}
//FlecsServer::get_singleton()->_get_world(world_id)
RID RenderUtility3D::create_omni_light_with_object(const RID &world_id, OmniLight3D *omni_light) {
	if (omni_light == nullptr) {
		ERR_FAIL_V(RID());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = omni_light->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(omni_light, world_id);
	OmniLightComponent omni_light_component;
	omni_light_component.light_id = omni_light->get_base();
	Transform3DComponent transform_component;
	transform_component.transform = omni_light->get_transform();
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = omni_light->get_instance();
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	

	const flecs::entity e = FlecsServer::get_singleton()->_get_world(world_id)->entity()
										 .set<OmniLightComponent>(omni_light_component)
										 .set<Transform3DComponent>(transform_component)
										 .set<RenderInstanceComponent>(render_instance_component)
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set<VisibilityComponent>(visibility_component)
										 .add<DirtyTransform>()
										 .set_name(String(omni_light->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_spot_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	SpotLightComponent spot_light_component;
	spot_light_component.light_id = light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(light_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
			.set<SpotLightComponent>(spot_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_spot_light(const RID &world_id, const Transform3D &transform, const String &name) {
	const RID spot_light_id = RS::get_singleton()->spot_light_create();
	flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	SpotLightComponent spot_light_component;
	spot_light_component.light_id = spot_light_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(spot_light_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
			.set<SpotLightComponent>(spot_light_component)
			.set<Transform3DComponent>(transform_component)
			.set<VisibilityComponent>(visibility_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
			
}

RID RenderUtility3D::create_spot_light_with_object(const RID &world_id, SpotLight3D *spot_light) {
	if (spot_light == nullptr) {
		ERR_FAIL_V(RID());
	}
	const RID &spot_light_id = spot_light->get_base();
	if (!spot_light_id.is_valid()) {
		ERR_FAIL_V(RID());
	}
	if (!spot_light->get_instance().is_valid()) {
		ERR_FAIL_V(RID());
	}
	if (!spot_light->get_base().is_valid()) {
		ERR_FAIL_V(RID());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = spot_light->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(spot_light, world_id);
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	ERR_FAIL_COND_V(world == nullptr, RID());
	SpotLightComponent spot_light_component;
	spot_light_component.light_id = spot_light->get_base();
	Transform3DComponent transform_component;
	transform_component.transform = spot_light->get_transform();
	VisibilityComponent visibility_component;
	visibility_component.visible = true;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(spot_light_component.light_id, world->get<World3DComponent>().scenario_id);
	flecs::entity e = world->entity()
										 .set<SpotLightComponent>(spot_light_component)
										 .set<Transform3DComponent>(transform_component)
										 .set<VisibilityComponent>(visibility_component)
										 .set<RenderInstanceComponent>(render_instance_component)
										 .add<DirtyTransform>()
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set_name(String(spot_light->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_viewport_with_id(const RID &world_id, const RID &viewport_id, const String &name) {
	flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
	ViewportComponent viewport_component;
	viewport_component.viewport_id = viewport_id;
	flecs::entity e = world->entity()
			.set<ViewportComponent>(viewport_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_viewport_with_object(const RID &world_id, Viewport *viewport) {
	if (viewport == nullptr) {
		ERR_FAIL_V(RID());
	}
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = viewport->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(viewport, world_id);
	ViewportComponent viewport_component;
	viewport_component.viewport_id = viewport->get_viewport_rid();
	const flecs::entity e = FlecsServer::get_singleton()->_get_world(world_id)->entity()
										 .set<ViewportComponent>(viewport_component)
										 .set<ObjectInstanceComponent>(object_instance_component).set_name(String(viewport->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_voxel_gi_with_id(const RID &world_id, const RID &voxel_gi_id, const Transform3D &transform, const String &name) {
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	VoxelGIComponent voxel_gi_component;
	voxel_gi_component.voxel_gi_id = voxel_gi_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(voxel_gi_id, world->get<World3DComponent>().scenario_id);
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	const flecs::entity e = world->entity()
			.set<VoxelGIComponent>(voxel_gi_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<VisibilityComponent>(visibility_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_voxel_gi(const RID &world_id, const Transform3D &transform, const String &name) {
	const RID voxel_gi_id = RS::get_singleton()->voxel_gi_create();
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	if (!world->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!world->has<World3DComponent>(), RID());
	}
	VoxelGIComponent voxel_gi_component;
	voxel_gi_component.voxel_gi_id = voxel_gi_id;
	Transform3DComponent transform_component;
	transform_component.transform = transform;
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(voxel_gi_id, world->get<World3DComponent>().scenario_id);
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	flecs::entity e =world->entity()
			.set<VoxelGIComponent>(voxel_gi_component)
			.set<Transform3DComponent>(transform_component)
			.set<RenderInstanceComponent>(render_instance_component)
			.set<VisibilityComponent>(visibility_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_voxel_gi_with_object(const RID &world_id, VoxelGI *voxel_gi) {
	if (voxel_gi == nullptr) {
		ERR_FAIL_V(RID());
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = voxel_gi->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(voxel_gi, world_id);
	VoxelGIComponent voxel_gi_component;
	voxel_gi_component.voxel_gi_id = voxel_gi->get_base();
	Transform3DComponent transform_component;
	transform_component.transform = voxel_gi->get_transform();
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(voxel_gi->get_base(), world->get<World3DComponent>().scenario_id);
	VisibilityComponent visibility_component;
	visibility_component.visible = true;

	const flecs::entity e = world->entity(String(voxel_gi->get_name()).ascii().get_data())
										 .set<VoxelGIComponent>(voxel_gi_component)
										 .set<Transform3DComponent>(transform_component)
										 .set<RenderInstanceComponent>(render_instance_component)
										 .set<VisibilityComponent>(visibility_component)
										 .set<ObjectInstanceComponent>(object_instance_component)
										 .set_name(String(voxel_gi->get_name()).ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_scenario_with_id(const RID &world_id, const RID &scenario_id, const String &name) {
	ScenarioComponent scenario_component;
	scenario_component.scenario_id = scenario_id;
	flecs::entity e = FlecsServer::get_singleton()->_get_world(world_id)->entity()
			.set<ScenarioComponent>(scenario_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_scenario(const RID &world_id, const String &name) {
	const RID scenario_id = RS::get_singleton()->scenario_create();
	ScenarioComponent scenario_component;
	scenario_component.scenario_id = scenario_id;
	flecs::entity e = FlecsServer::get_singleton()->_get_world(world_id)->entity()
			.set<ScenarioComponent>(scenario_component)
			.set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_occluder(const RID &world_id, const String &name) {
	return create_occluder_with_id(world_id, RS::get_singleton()->occluder_create(), name);
}

RID RenderUtility3D::create_occluder_with_id(const RID &world_id, const RID &occluder_id, const String &name) {
	flecs::world* world = FlecsServer::get_singleton()->_get_world(world_id);
	Occluder occluder = {};
	occluder.occluder_id = occluder_id;

	flecs::entity e = world->entity().set<Occluder>(occluder).set_name(name.ascii().get_data());
	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility3D::create_occluder_with_object(const RID &world_id, OccluderInstance3D *occluder_instance) {
	if (occluder_instance == nullptr) {
		ERR_FAIL_V(RID());
	}
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	const Ref<Occluder3D> occluder = occluder_instance->get_occluder();
	const PackedVector3Array vertices = occluder->get_vertices();
	const PackedInt32Array indices = occluder->get_indices();
	ObjectInstanceComponent object_instance_component;
	object_instance_component.object_instance_id = occluder_instance->get_instance_id();
	FlecsServer::get_singleton()->add_to_node_storage(occluder_instance, world_id);
	if(!FlecsServer::get_singleton()->_get_world(world_id)->has<World3DComponent>()) {
		ERR_FAIL_COND_V(!FlecsServer::get_singleton()->_get_world(world_id)->has<World3DComponent>(), RID());
	}
	RenderInstanceComponent render_instance_component;
	render_instance_component.instance_id = RS::get_singleton()->instance_create2(occluder->get_rid(), FlecsServer::get_singleton()->_get_world(world_id)->get<World3DComponent>().scenario_id);
	Occluder occluder_component;
	occluder_component.occluder_id = occluder->get_rid();
	occluder_component.vertices = vertices;
	occluder_component.indices = indices;
	Transform3DComponent transform_component;
	transform_component.transform = occluder_instance->get_transform();

	const flecs::entity e = world->entity()
		.set<RenderInstanceComponent>(render_instance_component)
		.set<Occluder>(occluder_component).set_name(occluder->get_name().ascii().get_data())
		.set<Transform3DComponent>(transform_component)
		.set<ObjectInstanceComponent>(object_instance_component);
	FlecsServer::get_singleton()->add_to_ref_storage(occluder, world_id);

	return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

bool RenderUtility3D::bake_material_check(const Ref<Material> &p_material) {
	const StandardMaterial3D *standard_mat = Object::cast_to<StandardMaterial3D>(p_material.ptr());
	if (standard_mat && standard_mat->get_transparency() != StandardMaterial3D::TRANSPARENCY_DISABLED) {
		return false;
	}
	return true;
}

void RenderUtility3D::bake_surface(const Transform3D &p_transform, const Array& p_surface_arrays, const Ref<Material> &p_material, float p_simplification_dist, const PackedVector3Array &r_vertices, const PackedInt32Array &r_indices) {
	if (!bake_material_check(p_material)) {
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

void RenderUtility3D::_bind_methods(){
	ClassDB::bind_static_method(get_class_static(), "create_particles",
		&RenderUtility3D::create_particles, "flecs_world", "transform", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_particles_with_object",
		&RenderUtility3D::create_particles_with_object, "flecs_world", "gpu_particles_3d");
	ClassDB::bind_static_method(get_class_static(), "create_mesh_instance",
		&RenderUtility3D::create_mesh_instance, "flecs_world", "mesh_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_mesh_instance_with_id",
		&RenderUtility3D::create_mesh_instance_with_id, "flecs_world", "mesh_instance_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_multi_mesh",
		&RenderUtility3D::create_multi_mesh, "flecs_world", "transform", "name", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_camera",
		&RenderUtility3D::create_camera_with_id, "flecs_world", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_directional_light",
		&RenderUtility3D::create_directional_light, "flecs_world", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_directional_light_with_id",
		&RenderUtility3D::create_directional_light_with_id, "flecs_world", "light_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_directional_light_with_object",
		&RenderUtility3D::create_directional_light_with_object, "flecs_world", "directional_light");

	ClassDB::bind_static_method(get_class_static(), "create_voxel_gi",
		&RenderUtility3D::create_voxel_gi, "flecs_world", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_voxel_gi_with_id",
		&RenderUtility3D::create_voxel_gi_with_id, "flecs_world", "voxel_gi_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_spot_light",
		&RenderUtility3D::create_spot_light, "flecs_world", "transform", "name");


	ClassDB::bind_static_method(get_class_static(), "create_spot_light_with_id",
		&RenderUtility3D::create_spot_light_with_id, "flecs_world", "light_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_spot_light_with_object",
		&RenderUtility3D::create_spot_light_with_object, "flecs_world", "spot_light");
	ClassDB::bind_static_method(get_class_static(), "create_mesh_instance_with_object",
		&RenderUtility3D::create_mesh_instance_with_object, "flecs_world", "mesh_instance_3d");
	ClassDB::bind_static_method(get_class_static(), "create_omni_light",
		&RenderUtility3D::create_omni_light_with_id, "flecs_world","light_id", "transform", "scenario_id");
	ClassDB::bind_static_method(get_class_static(), "create_omni_light_with_object",
		&RenderUtility3D::create_omni_light_with_object, "flecs_world", "omni_light");
	
	ClassDB::bind_static_method(get_class_static(), "create_reflection_probe",
		&RenderUtility3D::create_reflection_probe, "flecs_world", "probe_id", "transform", "name");
	ClassDB::bind_static_method(get_class_static(), "create_scenario",
		&RenderUtility3D::create_scenario, "flecs_world", "name");
	ClassDB::bind_static_method(get_class_static(), "create_viewport_with_id",
		&RenderUtility3D::create_viewport_with_id, "flecs_world", "viewport_id", "name");
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
		&RenderUtility3D::create_occluder, "world_id", "name");
	ClassDB::bind_static_method(get_class_static(), "create_occluder_with_id",
		&RenderUtility3D::create_occluder_with_id, "world_id", "occluder_id", "name");
	ClassDB::bind_static_method(get_class_static(), "bake_material_check",
		&RenderUtility3D::bake_material_check, "p_material");
	ClassDB::bind_static_method(get_class_static(), "bake_surface",
		&RenderUtility3D::bake_surface, "p_transform", "p_surface_arrays", "p_material", "p_simplification_dist", "r_vertices", "r_indices");
}