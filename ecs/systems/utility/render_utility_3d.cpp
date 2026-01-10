//
// Created by Floof on 29-7-2025.
//

#include "render_utility_3d.h"

#include "core/error/error_macros.h"
#include "core/math/aabb.h"
#include "core/math/rect2i.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
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
#include "core/object/class_db.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "core/templates/rid.h"
#include "scene/main/viewport.h"
#include "servers/rendering_server.h"
#include <cassert>
#include "scene/3d/voxel_gi.h"
#include "core/templates/vector.h"
#include <cstdint>
#include <vector>
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

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
	multi_mesh_component.has_color = use_colors;
	multi_mesh_component.has_data = use_custom_data;
	multi_mesh_component.is_instanced = use_indirect;
	multi_mesh_component.transform_format = RS::MultimeshTransformFormat::MULTIMESH_TRANSFORM_3D;


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
	//const RID &instance_id = RS::get_singleton()->instance_create2(multi_mesh_id, world->get<World3DComponent>().scenario_id);
	const RID &instance_id = multi_mesh_instance->get_instance();
	const String name = multi_mesh_instance->get_name();
	const Transform3D transform = multi_mesh_instance->get_transform();
	const uint32_t size = multi_mesh_instance->get_multimesh()->get_instance_count();
	//entities.resize(size+1);
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
	multi_mesh_component.transform_format = RS::MultimeshTransformFormat::MULTIMESH_TRANSFORM_3D;
	MeshComponent mesh_component;
	mesh_component.mesh_id = mesh_id;
	mesh_component.material_ids = material_ids;
	mesh_component.custom_aabb = custom_aabb;
	multi_mesh_component.has_color = multi_mesh_instance->get_multimesh()->is_using_colors();
	multi_mesh_component.has_data = multi_mesh_instance->get_multimesh()->is_using_custom_data();
	multi_mesh_component.is_instanced = false; //couldn't figure out what this should default to
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
	entities.append(entity);
	TypedArray<Transform3D> transforms;
	transforms.resize(size);
	for (uint32_t i = 0; i < size; ++i) {
		transforms[i] = multi_mesh_instance->get_transform();
	}
	auto mm_instances = create_multi_mesh_instances(world_id, transforms, entity);
	entities.append_array(mm_instances);



	return entities;
}

TypedArray<RID> RenderUtility3D::create_multi_mesh_instances(const RID &world_id,const TypedArray<Transform3D> &transforms, const RID &multi_mesh_entity_id) {
	TypedArray<RID> entities;
	flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
	flecs::entity mm_entity = FlecsServer::get_singleton()->_get_entity(multi_mesh_entity_id, world_id);
	uint32_t instance_count = mm_entity.get<MultiMeshComponent>().instance_count;

	const RID& mesh_id = RS::get_singleton()->multimesh_get_mesh(mm_entity.get<MultiMeshComponent>().multi_mesh_id);
	const AABB& custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_id);
	const bool mm_use_colors = mm_entity.get<MultiMeshComponent>().has_color;
	const bool mm_use_data = mm_entity.get<MultiMeshComponent>().has_data;


	// Bulk assign components
	std::vector<MultiMeshInstanceComponent> mm_components(instance_count);
	std::vector<Transform3DComponent> transform_components(instance_count);
	std::vector<VisibilityComponent> visibility_components(instance_count);


	const int base_offset = 12; // this is for Transform3D (12 floats)
	std::vector<MultiMeshInstanceDataComponent> mm_data_components(instance_count);
	Vector<float> mm_buffer = RS::get_singleton()->multimesh_get_buffer(mm_entity.get<MultiMeshComponent>().multi_mesh_id);

	for (uint32_t i = 0; i < instance_count; ++i) {
		mm_components[i].index = i;
		mm_components[i].custom_aabb = custom_aabb;
		transform_components[i].transform = transforms[i];
		if(mm_use_colors || mm_use_data) {
			if(mm_use_colors && mm_use_data){
				mm_data_components[i].color = Color(mm_buffer[i * base_offset], mm_buffer[i * base_offset + 1], mm_buffer[i * base_offset + 2], mm_buffer[i * base_offset + 3]);
				mm_data_components[i].data = Vector4(mm_buffer[i * base_offset + 8], mm_buffer[i * base_offset + 9], mm_buffer[i * base_offset + 10], mm_buffer[i * base_offset + 11]);

			}else if(mm_use_colors){
				mm_data_components[i].color = Color(mm_buffer[i * base_offset], mm_buffer[i * base_offset + 1], mm_buffer[i * base_offset + 2], mm_buffer[i * base_offset + 3]);

			}else if(mm_use_data){
				mm_data_components[i].data = Vector4(mm_buffer[i * base_offset + 8], mm_buffer[i * base_offset + 9], mm_buffer[i * base_offset + 10], mm_buffer[i * base_offset + 11]);
			}
		}
	}
	std::vector<void*> data;
	data.push_back(mm_components.data());
	data.push_back(transform_components.data());
	data.push_back(visibility_components.data());
	data.push_back(nullptr); // Needed for tag
	data.push_back(nullptr); // Needed for pair
	data.push_back(mm_data_components.data()); // Needed for pair


	if(mm_use_colors || mm_use_data) {
		data.push_back(mm_data_components.data());

		if(instance_count > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
			ERR_PRINT("Instance count exceeds maximum limit of uint32_t.");
			return entities;
		}

		ecs_bulk_desc_t bulk_desc = {0, nullptr, static_cast<int32_t>(instance_count), {
			world->component<MultiMeshInstanceComponent>(),
			world->component<Transform3DComponent>(),
			world->component<VisibilityComponent>(),
			world->component<DirtyTransform>(),
			ecs_pair(flecs::ChildOf, mm_entity),
			world->component<MultiMeshInstanceDataComponent>()

		}, data.data()};

		print_line(itos(mm_entity.id()));

		auto flecs_entities = ecs_bulk_init(world->c_ptr(), &bulk_desc);

		entities.resize(instance_count);
		for (uint32_t i = 0; i < instance_count; i++) {
			const flecs::entity entity = world->get_alive(flecs_entities[i]);
			//entity.child_of(mm_entity);
			entities[i] = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, entity);
		}
		return entities;

	}else{
		if(instance_count > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
		ERR_PRINT("Instance count exceeds maximum limit of uint32_t.");
		return entities;
		}

		ecs_bulk_desc_t bulk_desc = {0, nullptr, static_cast<int32_t>(instance_count), {
			world->component<MultiMeshInstanceComponent>(),
			world->component<Transform3DComponent>(),
			world->component<VisibilityComponent>(),
			world->component<DirtyTransform>(),
			ecs_pair(flecs::ChildOf, mm_entity),

		}, data.data()};
		print_line(itos(mm_entity.id()));

		auto flecs_entities = ecs_bulk_init(world->c_ptr(), &bulk_desc);

		entities.resize(instance_count);
		for (uint32_t i = 0; i < instance_count; i++) {
			const flecs::entity entity = world->get_alive(flecs_entities[i]);
			//entity.child_of(mm_entity);
			entities[i] = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, entity);
		}
		return entities;
	}

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
			.set<RenderInstanceComponent>(render_instance_component)
			.add<DirtyTransform>()
			.set_name(name.ascii().get_data());
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
										.add<DirtyTransform>()
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
			.add<DirtyTransform>()

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
			.add<DirtyTransform>()
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
										.add<DirtyTransform>()
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

void RenderUtility3D::_bind_methods(){
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_particles", "world_id", "transform", "particles_id", "particle_count", "scenario_id", "name"),
			&RenderUtility3D::create_particles);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_particles_with_object", "world_id", "gpu_particles_3d"),
			&RenderUtility3D::create_particles_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_mesh_instance", "world_id", "transform", "scenario_id", "name"),
			&RenderUtility3D::create_mesh_instance);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_mesh_instance_with_id", "world_id", "mesh_id", "transform", "name", "scenario_id"),
			&RenderUtility3D::create_mesh_instance_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_mesh_instance_with_object", "world_id", "mesh_instance_3d"),
			&RenderUtility3D::create_mesh_instance_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_multi_mesh", "world_id", "transform", "size", "mesh_id", "material_ids", "scenario_id", "name", "use_colors", "use_custom_data", "use_indirect"),
			&RenderUtility3D::create_multi_mesh, DEFVAL(false), DEFVAL(false), DEFVAL(false));
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_multi_mesh_with_object", "world_id", "multi_mesh_instance"),
			&RenderUtility3D::create_multi_mesh_with_object);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_multi_mesh_instance", "world_id", "transform", "index", "multi_mesh_id", "name"),
			&RenderUtility3D::create_multi_mesh_instance);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_multi_mesh_instances", "world_id", "transforms", "multi_mesh_entity_id"),
			&RenderUtility3D::create_multi_mesh_instances);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_camera_with_id", "world_id", "camera_id", "transform", "name"),
			&RenderUtility3D::create_camera_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_camera", "world_id", "transform", "name"),
			&RenderUtility3D::create_camera);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_camera_with_object", "world_id", "camera_3d"),
			&RenderUtility3D::create_camera_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_directional_light_with_id", "world_id", "light_id", "transform", "name"),
			&RenderUtility3D::create_directional_light_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_directional_light", "world_id", "transform", "name"),
			&RenderUtility3D::create_directional_light);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_directional_light_with_object", "world_id", "directional_light"),
			&RenderUtility3D::create_directional_light_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_omni_light_with_id", "world_id", "light_id", "transform", "scenario_id", "name"),
			&RenderUtility3D::create_omni_light_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_omni_light", "world_id", "transform", "scenario_id", "name"),
			&RenderUtility3D::create_omni_light);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_omni_light_with_object", "world_id", "omni_light"),
			&RenderUtility3D::create_omni_light_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_spot_light_with_id", "world_id", "light_id", "transform", "name"),
			&RenderUtility3D::create_spot_light_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_spot_light", "world_id", "transform", "name"),
			&RenderUtility3D::create_spot_light);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_spot_light_with_object", "world_id", "spot_light"),
			&RenderUtility3D::create_spot_light_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_reflection_probe", "world_id", "probe_id", "transform", "name"),
			&RenderUtility3D::create_reflection_probe);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_reflection_probe_with_object", "world_id", "reflection_probe"),
			&RenderUtility3D::create_reflection_probe_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_voxel_gi_with_id", "world_id", "voxel_gi_id", "transform", "name"),
			&RenderUtility3D::create_voxel_gi_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_voxel_gi", "world_id", "transform", "name"),
			&RenderUtility3D::create_voxel_gi);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_voxel_gi_with_object", "world_id", "voxel_gi"),
			&RenderUtility3D::create_voxel_gi_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_environment", "world_id", "environment_id", "name"),
			&RenderUtility3D::create_environment);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_environment_with_object", "world_id", "world_environment"),
			&RenderUtility3D::create_environment_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_skeleton", "world_id", "skeleton_id", "name"),
			&RenderUtility3D::create_skeleton);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_skeleton_with_object", "world_id", "skeleton_3d"),
			&RenderUtility3D::create_skeleton_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_scenario_with_id", "world_id", "scenario_id", "name"),
			&RenderUtility3D::create_scenario_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_scenario", "world_id", "name"),
			&RenderUtility3D::create_scenario);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_viewport_with_id", "world_id", "viewport_id", "name"),
			&RenderUtility3D::create_viewport_with_id);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_viewport_with_object", "world_id", "viewport"),
			&RenderUtility3D::create_viewport_with_object);

	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_compositor", "world_id", "compositor_id", "name"),
			&RenderUtility3D::create_compositor);
	ClassDB::bind_static_method(get_class_static(), D_METHOD("create_compositor_with_object", "world_id", "compositor"),
			&RenderUtility3D::create_compositor_with_object);
}
