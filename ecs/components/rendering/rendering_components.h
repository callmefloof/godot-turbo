#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../component_module_base.h"
#include "../../../../core/templates/rid.h"
#include "../../../../core/os/memory.h"
#include "../../../../core/templates/vector.h"
#include "../../../../modules/godot_turbo/ecs/components/component_proxy.h"
#include "../../../../servers/rendering_server.h"
#include "../../../../core/variant/typed_array.h"
#include "../flecs_types/flecs_entity.h"

struct MeshComponent {
	RID mesh_id;
	Vector<RID> material_ids;
	~MeshComponent() {
		for (const RID &mat_id : material_ids) {
			if (mat_id.is_valid()) {
				RenderingServer::get_singleton()->free(mat_id);
			}
		}
		if (mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(mesh_id);
		}
	}
};

#define MESH_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, mesh_id, MeshComponent)\
DEFINE_PROPERTY_ARRAY(RID, material_ids, MeshComponent)\


#define MESH_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, mesh_id, MeshComponentRef)\
BIND_VECTOR_PROPERTY(RID, material_ids, MeshComponentRef)\

class MeshComponentRef : public FlecsComponent<MeshComponent> {
GDCLASS(MeshComponentRef, FlecsComponent<MeshComponent>)
public : RID get_mesh_id() const {
		auto typed = get_typed_data<MeshComponent>();
		if (typed) {
			return typed->mesh_id;
		}
		return default_value<RID>();
	}
	void set_mesh_id(RID value) const {
		auto typed = get_typed_data<MeshComponent>();
		if (typed) {
			typed->mesh_id = value;
		}
	}
	TypedArray<RID> get_material_ids() const {
		TypedArray<RID> arr;
		auto typed = get_typed_data<MeshComponent>();
		if (typed) {
			for (int i = 0; i < typed->material_ids.size(); i++) {
				arr.push_back(typed->material_ids[i]);
			}
		}
		return arr;
	}
	void set_material_ids(const TypedArray<RID> &value) const {
		auto typed = get_typed_data<MeshComponent>();
		if (!typed) {
			return;
		}
		typed->material_ids.clear();
		for (int i = 0; i < value.size(); i++) {
			typed->material_ids.push_back(value.get(i));
		}
	}
	static Ref<MeshComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<MeshComponentRef> class_ref = Ref<MeshComponentRef>(memnew(MeshComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<MeshComponent>({});
		class_ref->set_data(&entity->get_mut<MeshComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<MeshComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<MeshComponent>();
			MeshComponent *copied = memnew(MeshComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(MeshComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "MeshComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "mesh_id",
					&MeshComponentRef::get_mesh_id);
			ClassDB::bind_method("set_"
								 "mesh_id",
					&MeshComponentRef::set_mesh_id);
			::ClassDB::add_property(MeshComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "mesh_id"), _scs_create("set_"
																																		   "mesh_id"),
					_scs_create("get_"
								"mesh_id"));
		} while (0) ClassDB::bind_method("get_"
										 "material_ids",
				&MeshComponentRef::get_material_ids);
		ClassDB::bind_method("set_"
							 "material_ids",
				&MeshComponentRef::set_material_ids);
		::ClassDB::add_property(MeshComponentRef::get_class_static(), PropertyInfo(Variant::ARRAY, "material_ids"), _scs_create("set_"
																																"material_ids"),
				_scs_create("get_"
							"material_ids"));
		ClassDB::bind_static_method(MeshComponentRef::get_class_static(), "create_component", &MeshComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &MeshComponentRef::get_type_name);
	}
};
;

struct MultiMeshComponent {
	RID multi_mesh_id;
	uint32_t instance_count = 0U;
	~MultiMeshComponent() {
		// Ensure that the RID is released when the component is destroyed
		if (multi_mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(multi_mesh_id);
		}
	}
};

#define MULTI_MESH_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, multi_mesh_id, MultiMeshComponent)\
DEFINE_PROPERTY(uint32_t, instance_count,MultiMeshComponent)\


#define MULTI_MESH_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, multi_mesh_id, MultiMeshComponentRef)\
BIND_PROPERTY(uint32_t, instance_count, MultiMeshComponentRef)\


 class MultiMeshComponentRef : public FlecsComponent<MultiMeshComponent> {
GDCLASS(MultiMeshComponentRef, FlecsComponent<MultiMeshComponent>)
public : RID get_multi_mesh_id() const {
		auto typed = get_typed_data<MultiMeshComponent>();
		if (typed) {
			return typed->multi_mesh_id;
		}
		return default_value<RID>();
	}
	void set_multi_mesh_id(RID value) const {
		auto typed = get_typed_data<MultiMeshComponent>();
		if (typed) {
			typed->multi_mesh_id = value;
		}
	}
	uint32_t get_instance_count() const {
		auto typed = get_typed_data<MultiMeshComponent>();
		if (typed) {
			return typed->instance_count;
		}
		return default_value<uint32_t>();
	}
	void set_instance_count(uint32_t value) const {
		auto typed = get_typed_data<MultiMeshComponent>();
		if (typed) {
			typed->instance_count = value;
		}
	}
	static Ref<MultiMeshComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<MultiMeshComponentRef> class_ref = Ref<MultiMeshComponentRef>(memnew(MultiMeshComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<MultiMeshComponent>({});
		class_ref->set_data(&entity->get_mut<MultiMeshComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<MultiMeshComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<MultiMeshComponent>();
			MultiMeshComponent *copied = memnew(MultiMeshComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(MultiMeshComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "MultiMeshComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "multi_mesh_id",
					&MultiMeshComponentRef::get_multi_mesh_id);
			ClassDB::bind_method("set_"
								 "multi_mesh_id",
					&MultiMeshComponentRef::set_multi_mesh_id);
			::ClassDB::add_property(MultiMeshComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "multi_mesh_id"), _scs_create("set_"
																																					  "multi_mesh_id"),
					_scs_create("get_"
								"multi_mesh_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "instance_count",
					&MultiMeshComponentRef::get_instance_count);
			ClassDB::bind_method("set_"
								 "instance_count",
					&MultiMeshComponentRef::set_instance_count);
			::ClassDB::add_property(MultiMeshComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<uint32_t>::value, "instance_count"), _scs_create("set_"
																																							"instance_count"),
					_scs_create("get_"
								"instance_count"));
		}
		while (0)
			;
		ClassDB::bind_static_method(MultiMeshComponentRef::get_class_static(), "create_component", &MultiMeshComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &MultiMeshComponentRef::get_type_name);
	}
};
;


struct MultiMeshInstanceComponent {
	uint32_t index;
};

#define MULTI_MESH_INSTANCE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(uint32_t, index,MultiMeshInstanceComponent )\


#define MULTI_MESH_INSTANCE_COMPONENT_BINDINGS\
BIND_PROPERTY(uint32_t, index, MultiMeshInstanceComponentRef)\


class MultiMeshInstanceComponentRef : public FlecsComponent<MultiMeshInstanceComponent> {
GDCLASS(MultiMeshInstanceComponentRef, FlecsComponent<MultiMeshInstanceComponent>)
public : uint32_t get_index() const {
		auto typed = get_typed_data<MultiMeshInstanceComponent>();
		if (typed) {
			return typed->index;
		}
		return default_value<uint32_t>();
	}
	void set_index(uint32_t value) const {
		auto typed = get_typed_data<MultiMeshInstanceComponent>();
		if (typed) {
			typed->index = value;
		}
	}
	static Ref<MultiMeshInstanceComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<MultiMeshInstanceComponentRef> class_ref = Ref<MultiMeshInstanceComponentRef>(memnew(MultiMeshInstanceComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<MultiMeshInstanceComponent>({});
		class_ref->set_data(&entity->get_mut<MultiMeshInstanceComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<MultiMeshInstanceComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<MultiMeshInstanceComponent>();
			MultiMeshInstanceComponent *copied = memnew(MultiMeshInstanceComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(MultiMeshInstanceComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "MultiMeshInstanceComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "index",
					&MultiMeshInstanceComponentRef::get_index);
			ClassDB::bind_method("set_"
								 "index",
					&MultiMeshInstanceComponentRef::set_index);
			::ClassDB::add_property(MultiMeshInstanceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<uint32_t>::value, "index"), _scs_create("set_"
																																						   "index"),
					_scs_create("get_"
								"index"));
		} while (0);
		ClassDB::bind_static_method(MultiMeshInstanceComponentRef::get_class_static(), "create_component", &MultiMeshInstanceComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &MultiMeshInstanceComponentRef::get_type_name);
	}
};
;

struct ParticlesComponent {
	RID particles_id;
	~ParticlesComponent() {
		if (particles_id.is_valid()) {
			RenderingServer::get_singleton()->free(particles_id);
		}
	}
};

#define PARTICLES_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, particles_id, ParticlesComponent)\


#define PARTICLES_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, particles_id, ParticlesComponentRef)\

#define PARTICLES_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, particles_id))\

#define PARTICLES_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(particles_id)\

class ParticlesComponentRef : public FlecsComponent<ParticlesComponent> {
GDCLASS(ParticlesComponentRef, FlecsComponent<ParticlesComponent>)
public : RID get_particles_id() const {
		auto typed = get_typed_data<ParticlesComponent>();
		if (typed) {
			return typed->particles_id;
		}
		return default_value<RID>();
	}
	void set_particles_id(RID value) const {
		auto typed = get_typed_data<ParticlesComponent>();
		if (typed) {
			typed->particles_id = value;
		}
	}
	static Ref<ParticlesComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<ParticlesComponentRef> class_ref = Ref<ParticlesComponentRef>(memnew(ParticlesComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<ParticlesComponent>({});
		class_ref->set_data(&entity->get_mut<ParticlesComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<ParticlesComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<ParticlesComponent>();
			ParticlesComponent *copied = memnew(ParticlesComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(ParticlesComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "ParticlesComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "particles_id",
					&ParticlesComponentRef::get_particles_id);
			ClassDB::bind_method("set_"
								 "particles_id",
					&ParticlesComponentRef::set_particles_id);
			::ClassDB::add_property(ParticlesComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "particles_id"), _scs_create("set_"
																																					 "particles_id"),
					_scs_create("get_"
								"particles_id"));
		} while (0);
		ClassDB::bind_static_method(ParticlesComponentRef::get_class_static(), "create_component", &ParticlesComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &ParticlesComponentRef::get_type_name);
	}
};
;

struct ReflectionProbeComponent {
	RID probe_id;
	~ReflectionProbeComponent() {
		if (probe_id.is_valid()) {
			RenderingServer::get_singleton()->free(probe_id);
		}
	}
};

#define REFLECTION_PROBE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, probe_id, ReflectionProbeComponent)\


#define REFLECTION_PROBE_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, probe_id, ReflectionProbeComponentRef)\

#define REFLECTION_PROBE_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, probe_id))\

#define REFLECTION_PROBE_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(probe_id)\

class ReflectionProbeComponentRef : public FlecsComponent<ReflectionProbeComponent> {
GDCLASS(ReflectionProbeComponentRef, FlecsComponent<ReflectionProbeComponent>)
public : RID get_probe_id() const {
		auto typed = get_typed_data<ReflectionProbeComponent>();
		if (typed) {
			return typed->probe_id;
		}
		return default_value<RID>();
	}
	void set_probe_id(RID value) const {
		auto typed = get_typed_data<ReflectionProbeComponent>();
		if (typed) {
			typed->probe_id = value;
		}
	}
	static Ref<ReflectionProbeComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<ReflectionProbeComponentRef> class_ref = Ref<ReflectionProbeComponentRef>(memnew(ReflectionProbeComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<ReflectionProbeComponent>({});
		class_ref->set_data(&entity->get_mut<ReflectionProbeComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<ReflectionProbeComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<ReflectionProbeComponent>();
			ReflectionProbeComponent *copied = memnew(ReflectionProbeComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(ReflectionProbeComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "ReflectionProbeComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "probe_id",
					&ReflectionProbeComponentRef::get_probe_id);
			ClassDB::bind_method("set_"
								 "probe_id",
					&ReflectionProbeComponentRef::set_probe_id);
			::ClassDB::add_property(ReflectionProbeComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "probe_id"), _scs_create("set_"
																																					   "probe_id"),
					_scs_create("get_"
								"probe_id"));
		} while (0);
		ClassDB::bind_static_method(ReflectionProbeComponentRef::get_class_static(), "create_component", &ReflectionProbeComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &ReflectionProbeComponentRef::get_type_name);
	}
};
;

struct SkeletonComponent {
	RID skeleton_id;
	~SkeletonComponent() {
		if (skeleton_id.is_valid()) {
			RenderingServer::get_singleton()->free(skeleton_id);
		}
	}
};

#define SKELETON_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, skeleton_id,SkeletonComponent)\


#define SKELETON_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, skeleton_id, SkeletonComponentRef)\

#define SKELETON_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, skeleton_id))\

#define SKELETON_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(skeleton_id)\

class SkeletonComponentRef : public FlecsComponent<SkeletonComponent> {
GDCLASS(SkeletonComponentRef, FlecsComponent<SkeletonComponent>)
public : RID get_skeleton_id() const {
		auto typed = get_typed_data<SkeletonComponent>();
		if (typed) {
			return typed->skeleton_id;
		}
		return default_value<RID>();
	}
	void set_skeleton_id(RID value) const {
		auto typed = get_typed_data<SkeletonComponent>();
		if (typed) {
			typed->skeleton_id = value;
		}
	}
	static Ref<SkeletonComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SkeletonComponentRef> class_ref = Ref<SkeletonComponentRef>(memnew(SkeletonComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SkeletonComponent>({});
		class_ref->set_data(&entity->get_mut<SkeletonComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SkeletonComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SkeletonComponent>();
			SkeletonComponent *copied = memnew(SkeletonComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SkeletonComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SkeletonComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "skeleton_id",
					&SkeletonComponentRef::get_skeleton_id);
			ClassDB::bind_method("set_"
								 "skeleton_id",
					&SkeletonComponentRef::set_skeleton_id);
			::ClassDB::add_property(SkeletonComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "skeleton_id"), _scs_create("set_"
																																				   "skeleton_id"),
					_scs_create("get_"
								"skeleton_id"));
		} while (0);
		ClassDB::bind_static_method(SkeletonComponentRef::get_class_static(), "create_component", &SkeletonComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SkeletonComponentRef::get_type_name);
	}
};
;

struct EnvironmentComponent {
	RID environment_id;
	~EnvironmentComponent() {
		if (environment_id.is_valid()) {
			RenderingServer::get_singleton()->free(environment_id);
		}
	}
};

#define ENVIRONMENT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, environment_id,EnvironmentComponent)\


#define ENVIRONMENT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, environment_id, EnvironmentComponentRef)\

#define ENVIRONMENT_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, environment_id))\

#define ENVIRONMENT_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(environment_id)\

class EnvironmentComponentRef : public FlecsComponent<EnvironmentComponent> {
GDCLASS(EnvironmentComponentRef, FlecsComponent<EnvironmentComponent>)
public : RID get_environment_id() const {
		auto typed = get_typed_data<EnvironmentComponent>();
		if (typed) {
			return typed->environment_id;
		}
		return default_value<RID>();
	}
	void set_environment_id(RID value) const {
		auto typed = get_typed_data<EnvironmentComponent>();
		if (typed) {
			typed->environment_id = value;
		}
	}
	static Ref<EnvironmentComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<EnvironmentComponentRef> class_ref = Ref<EnvironmentComponentRef>(memnew(EnvironmentComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<EnvironmentComponent>({});
		class_ref->set_data(&entity->get_mut<EnvironmentComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<EnvironmentComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<EnvironmentComponent>();
			EnvironmentComponent *copied = memnew(EnvironmentComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(EnvironmentComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "EnvironmentComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "environment_id",
					&EnvironmentComponentRef::get_environment_id);
			ClassDB::bind_method("set_"
								 "environment_id",
					&EnvironmentComponentRef::set_environment_id);
			::ClassDB::add_property(EnvironmentComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "environment_id"), _scs_create("set_"
																																						 "environment_id"),
					_scs_create("get_"
								"environment_id"));
		} while (0);
		ClassDB::bind_static_method(EnvironmentComponentRef::get_class_static(), "create_component", &EnvironmentComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &EnvironmentComponentRef::get_type_name);
	}
};
;

struct CameraComponent {
	RID camera_id;
	~CameraComponent() {
		if (camera_id.is_valid()) {
			RenderingServer::get_singleton()->free(camera_id);
		}
	}
};

#define CAMERA_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, camera_id,CameraComponent)\


#define CAMERA_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, camera_id, CameraComponentRef)\

#define CAMERA_COMPONENT_FIELDS_TYPES(APPLY_MACRO)\
APPLY_MACRO((RID, camera_id))\

#define CAMERA_COMPONENT_FIELDS_NAMES(APPLY_MACRO)\
APPLY_MACRO(camera_id)\

class CameraComponentRef : public FlecsComponent<CameraComponent> {
GDCLASS(CameraComponentRef, FlecsComponent<CameraComponent>)
public : RID get_camera_id() const {
		auto typed = get_typed_data<CameraComponent>();
		if (typed) {
			return typed->camera_id;
		}
		return default_value<RID>();
	}
	void set_camera_id(RID value) const {
		auto typed = get_typed_data<CameraComponent>();
		if (typed) {
			typed->camera_id = value;
		}
	}
	static Ref<CameraComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<CameraComponentRef> class_ref = Ref<CameraComponentRef>(memnew(CameraComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<CameraComponent>({});
		class_ref->set_data(&entity->get_mut<CameraComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<CameraComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<CameraComponent>();
			CameraComponent *copied = memnew(CameraComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(CameraComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "CameraComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "camera_id",
					&CameraComponentRef::get_camera_id);
			ClassDB::bind_method("set_"
								 "camera_id",
					&CameraComponentRef::set_camera_id);
			::ClassDB::add_property(CameraComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "camera_id"), _scs_create("set_"
																																			   "camera_id"),
					_scs_create("get_"
								"camera_id"));
		} while (0);
		ClassDB::bind_static_method(CameraComponentRef::get_class_static(), "create_component", &CameraComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &CameraComponentRef::get_type_name);
	}
};
;

struct CompositorComponent {
	RID compositor_id;
	~CompositorComponent() {
		if (compositor_id.is_valid()) {
			RenderingServer::get_singleton()->free(compositor_id);
		}
	}
};

#define COMPOSITOR_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, compositor_id,CompositorComponent)\


#define COMPOSITOR_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, compositor_id, CompositorComponentRef)\

class CompositorComponentRef : public FlecsComponent<CompositorComponent> {
GDCLASS(CompositorComponentRef, FlecsComponent<CompositorComponent>)
public : RID get_compositor_id() const {
		auto typed = get_typed_data<CompositorComponent>();
		if (typed) {
			return typed->compositor_id;
		}
		return default_value<RID>();
	}
	void set_compositor_id(RID value) const {
		auto typed = get_typed_data<CompositorComponent>();
		if (typed) {
			typed->compositor_id = value;
		}
	}
	static Ref<CompositorComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<CompositorComponentRef> class_ref = Ref<CompositorComponentRef>(memnew(CompositorComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<CompositorComponent>({});
		class_ref->set_data(&entity->get_mut<CompositorComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<CompositorComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<CompositorComponent>();
			CompositorComponent *copied = memnew(CompositorComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(CompositorComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "CompositorComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "compositor_id",
					&CompositorComponentRef::get_compositor_id);
			ClassDB::bind_method("set_"
								 "compositor_id",
					&CompositorComponentRef::set_compositor_id);
			::ClassDB::add_property(CompositorComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "compositor_id"), _scs_create("set_"
																																					   "compositor_id"),
					_scs_create("get_"
								"compositor_id"));
		} while (0);
		ClassDB::bind_static_method(CompositorComponentRef::get_class_static(), "create_component", &CompositorComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &CompositorComponentRef::get_type_name);
	}
};
;

struct DirectionalLight3DComponent {
	RID directional_light_id;
	~DirectionalLight3DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

#define DIRECTIONAL_LIGHT_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, directional_light_id,DirectionalLight3DComponent)\


#define DIRECTIONAL_LIGHT_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, directional_light_id, DirectionalLight3DComponentRef)\

class DirectionalLight3DComponentRef : public FlecsComponent<DirectionalLight3DComponent> {
GDCLASS(DirectionalLight3DComponentRef, FlecsComponent<DirectionalLight3DComponent>)
public : RID get_directional_light_id() const {
		auto typed = get_typed_data<DirectionalLight3DComponent>();
		if (typed) {
			return typed->directional_light_id;
		}
		return default_value<RID>();
	}
	void set_directional_light_id(RID value) const {
		auto typed = get_typed_data<DirectionalLight3DComponent>();
		if (typed) {
			typed->directional_light_id = value;
		}
	}
	static Ref<DirectionalLight3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<DirectionalLight3DComponentRef> class_ref = Ref<DirectionalLight3DComponentRef>(memnew(DirectionalLight3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<DirectionalLight3DComponent>({});
		class_ref->set_data(&entity->get_mut<DirectionalLight3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<DirectionalLight3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<DirectionalLight3DComponent>();
			DirectionalLight3DComponent *copied = memnew(DirectionalLight3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(DirectionalLight3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "DirectionalLight3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "directional_light_id",
					&DirectionalLight3DComponentRef::get_directional_light_id);
			ClassDB::bind_method("set_"
								 "directional_light_id",
					&DirectionalLight3DComponentRef::set_directional_light_id);
			::ClassDB::add_property(DirectionalLight3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "directional_light_id"), _scs_create("set_"
																																									  "directional_light_id"),
					_scs_create("get_"
								"directional_light_id"));
		} while (0);
		ClassDB::bind_static_method(DirectionalLight3DComponentRef::get_class_static(), "create_component", &DirectionalLight3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &DirectionalLight3DComponentRef::get_type_name);
	}
};
;

struct DirectionalLight2DComponent {
	RID directional_light_id;
	~DirectionalLight2DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

#define DIRECTIONAL_LIGHT_2D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, directional_light_id,DirectionalLight3DComponent)\


#define DIRECTIONAL_LIGHT_2D_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, directional_light_id, DirectionalLight2DComponentRef)\

class DirectionalLight2DComponentRef : public FlecsComponent<DirectionalLight3DComponent> {
GDCLASS(DirectionalLight2DComponentRef, FlecsComponent<DirectionalLight3DComponent>)
public : RID get_directional_light_id() const {
		auto typed = get_typed_data<DirectionalLight3DComponent>();
		if (typed) {
			return typed->directional_light_id;
		}
		return default_value<RID>();
	}
	void set_directional_light_id(RID value) const {
		auto typed = get_typed_data<DirectionalLight3DComponent>();
		if (typed) {
			typed->directional_light_id = value;
		}
	}
	static Ref<DirectionalLight2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<DirectionalLight2DComponentRef> class_ref = Ref<DirectionalLight2DComponentRef>(memnew(DirectionalLight2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<DirectionalLight3DComponent>({});
		class_ref->set_data(&entity->get_mut<DirectionalLight3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<DirectionalLight2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<DirectionalLight3DComponent>();
			DirectionalLight3DComponent *copied = memnew(DirectionalLight3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(DirectionalLight3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "DirectionalLight3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "directional_light_id",
					&DirectionalLight2DComponentRef::get_directional_light_id);
			ClassDB::bind_method("set_"
								 "directional_light_id",
					&DirectionalLight2DComponentRef::set_directional_light_id);
			::ClassDB::add_property(DirectionalLight2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "directional_light_id"), _scs_create("set_"
																																									  "directional_light_id"),
					_scs_create("get_"
								"directional_light_id"));
		} while (0);
		ClassDB::bind_static_method(DirectionalLight2DComponentRef::get_class_static(), "create_component", &DirectionalLight2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &DirectionalLight2DComponentRef::get_type_name);
	}
};
;

struct PointLightComponent {
	RID point_light_id;
	~PointLightComponent() {
		if (point_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(point_light_id);
		}
	}
};

#define POINT_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, point_light_id,PointLightComponent)\


#define POINT_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, point_light_id, PointLightComponentRef)\

class PointLightComponentRef : public FlecsComponent<PointLightComponent> {
GDCLASS(PointLightComponentRef, FlecsComponent<PointLightComponent>)
public : RID get_point_light_id() const {
		auto typed = get_typed_data<PointLightComponent>();
		if (typed) {
			return typed->point_light_id;
		}
		return default_value<RID>();
	}
	void set_point_light_id(RID value) const {
		auto typed = get_typed_data<PointLightComponent>();
		if (typed) {
			typed->point_light_id = value;
		}
	}
	static Ref<PointLightComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<PointLightComponentRef> class_ref = Ref<PointLightComponentRef>(memnew(PointLightComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<PointLightComponent>({});
		class_ref->set_data(&entity->get_mut<PointLightComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<PointLightComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<PointLightComponent>();
			PointLightComponent *copied = memnew(PointLightComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(PointLightComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "PointLightComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "point_light_id",
					&PointLightComponentRef::get_point_light_id);
			ClassDB::bind_method("set_"
								 "point_light_id",
					&PointLightComponentRef::set_point_light_id);
			::ClassDB::add_property(PointLightComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "point_light_id"), _scs_create("set_"
																																						"point_light_id"),
					_scs_create("get_"
								"point_light_id"));
		} while (0);
		ClassDB::bind_static_method(PointLightComponentRef::get_class_static(), "create_component", &PointLightComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &PointLightComponentRef::get_type_name);
	}
};
;

struct LightOccluderComponent {
	RID light_occluder_id;
	~LightOccluderComponent() {
		if (light_occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(light_occluder_id);
		}
	}
};

#define LIGHT_OCCLUDER_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, light_occluder_id,LightOccluderComponent)\


#define LIGHT_OCCLUDER_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, light_occluder_id, LightOccluderComponentRef)\

class LightOccluderComponentRef : public FlecsComponent<LightOccluderComponent> {
GDCLASS(LightOccluderComponentRef, FlecsComponent<LightOccluderComponent>)
public : RID get_light_occluder_id() const {
		auto typed = get_typed_data<LightOccluderComponent>();
		if (typed) {
			return typed->light_occluder_id;
		}
		return default_value<RID>();
	}
	void set_light_occluder_id(RID value) const {
		auto typed = get_typed_data<LightOccluderComponent>();
		if (typed) {
			typed->light_occluder_id = value;
		}
	}
	static Ref<LightOccluderComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<LightOccluderComponentRef> class_ref = Ref<LightOccluderComponentRef>(memnew(LightOccluderComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<LightOccluderComponent>({});
		class_ref->set_data(&entity->get_mut<LightOccluderComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<LightOccluderComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<LightOccluderComponent>();
			LightOccluderComponent *copied = memnew(LightOccluderComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(LightOccluderComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "LightOccluderComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "light_occluder_id",
					&LightOccluderComponentRef::get_light_occluder_id);
			ClassDB::bind_method("set_"
								 "light_occluder_id",
					&LightOccluderComponentRef::set_light_occluder_id);
			::ClassDB::add_property(LightOccluderComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "light_occluder_id"), _scs_create("set_"
																																							  "light_occluder_id"),
					_scs_create("get_"
								"light_occluder_id"));
		} while (0);
		ClassDB::bind_static_method(LightOccluderComponentRef::get_class_static(), "create_component", &LightOccluderComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &LightOccluderComponentRef::get_type_name);
	}
};
;


struct OmniLightComponent {
	RID omni_light_id;
	~OmniLightComponent() {
		if (omni_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(omni_light_id);
		}
	}
};

#define OMNI_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, omni_light_id,OmniLightComponent)\


#define OMNI_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, omni_light_id, OmniLightComponentRef)\

class OmniLightComponentRef : public FlecsComponent<OmniLightComponent> {
GDCLASS(OmniLightComponentRef, FlecsComponent<OmniLightComponent>)
public : RID get_omni_light_id() const {
		auto typed = get_typed_data<OmniLightComponent>();
		if (typed) {
			return typed->omni_light_id;
		}
		return default_value<RID>();
	}
	void set_omni_light_id(RID value) const {
		auto typed = get_typed_data<OmniLightComponent>();
		if (typed) {
			typed->omni_light_id = value;
		}
	}
	static Ref<OmniLightComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<OmniLightComponentRef> class_ref = Ref<OmniLightComponentRef>(memnew(OmniLightComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<OmniLightComponent>({});
		class_ref->set_data(&entity->get_mut<OmniLightComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<OmniLightComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<OmniLightComponent>();
			OmniLightComponent *copied = memnew(OmniLightComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(OmniLightComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "OmniLightComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "omni_light_id",
					&OmniLightComponentRef::get_omni_light_id);
			ClassDB::bind_method("set_"
								 "omni_light_id",
					&OmniLightComponentRef::set_omni_light_id);
			::ClassDB::add_property(OmniLightComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "omni_light_id"), _scs_create("set_"
																																					  "omni_light_id"),
					_scs_create("get_"
								"omni_light_id"));
		} while (0);
		ClassDB::bind_static_method(OmniLightComponentRef::get_class_static(), "create_component", &OmniLightComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &OmniLightComponentRef::get_type_name);
	}
};
;

struct SpotLightComponent {
	RID spot_light_id;
	~SpotLightComponent() {
		if (spot_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(spot_light_id);
		}
	}
};

#define SPOT_LIGHT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, spot_light_id,SpotLightComponent)\


#define SPOT_LIGHT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, spot_light_id, SpotLightComponentRef)\

class SpotLightComponentRef : public FlecsComponent<SpotLightComponent> {
GDCLASS(SpotLightComponentRef, FlecsComponent<SpotLightComponent>)
public : RID get_spot_light_id() const {
		auto typed = get_typed_data<SpotLightComponent>();
		if (typed) {
			return typed->spot_light_id;
		}
		return default_value<RID>();
	}
	void set_spot_light_id(RID value) const {
		auto typed = get_typed_data<SpotLightComponent>();
		if (typed) {
			typed->spot_light_id = value;
		}
	}
	static Ref<SpotLightComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SpotLightComponentRef> class_ref = Ref<SpotLightComponentRef>(memnew(SpotLightComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SpotLightComponent>({});
		class_ref->set_data(&entity->get_mut<SpotLightComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SpotLightComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SpotLightComponent>();
			SpotLightComponent *copied = memnew(SpotLightComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SpotLightComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SpotLightComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "spot_light_id",
					&SpotLightComponentRef::get_spot_light_id);
			ClassDB::bind_method("set_"
								 "spot_light_id",
					&SpotLightComponentRef::set_spot_light_id);
			::ClassDB::add_property(SpotLightComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "spot_light_id"), _scs_create("set_"
																																					  "spot_light_id"),
					_scs_create("get_"
								"spot_light_id"));
		} while (0);
		ClassDB::bind_static_method(SpotLightComponentRef::get_class_static(), "create_component", &SpotLightComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SpotLightComponentRef::get_type_name);
	}
};
;

struct ViewportComponent {
	RID viewport_id;
};

#define VIEWPORT_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, viewport_id,ViewportComponent)\


#define VIEWPORT_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, viewport_id, ViewportComponentRef)\

class ViewportComponentRef : public FlecsComponent<ViewportComponent> {
GDCLASS(ViewportComponentRef, FlecsComponent<ViewportComponent>)
public : RID get_viewport_id() const {
		auto typed = get_typed_data<ViewportComponent>();
		if (typed) {
			return typed->viewport_id;
		}
		return default_value<RID>();
	}
	void set_viewport_id(RID value) const {
		auto typed = get_typed_data<ViewportComponent>();
		if (typed) {
			typed->viewport_id = value;
		}
	}
	static Ref<ViewportComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<ViewportComponentRef> class_ref = Ref<ViewportComponentRef>(memnew(ViewportComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<ViewportComponent>({});
		class_ref->set_data(&entity->get_mut<ViewportComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<ViewportComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<ViewportComponent>();
			ViewportComponent *copied = memnew(ViewportComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(ViewportComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "ViewportComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "viewport_id",
					&ViewportComponentRef::get_viewport_id);
			ClassDB::bind_method("set_"
								 "viewport_id",
					&ViewportComponentRef::set_viewport_id);
			::ClassDB::add_property(ViewportComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "viewport_id"), _scs_create("set_"
																																				   "viewport_id"),
					_scs_create("get_"
								"viewport_id"));
		} while (0);
		ClassDB::bind_static_method(ViewportComponentRef::get_class_static(), "create_component", &ViewportComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &ViewportComponentRef::get_type_name);
	}
};
;

struct VoxelGIComponent {
	RID voxel_gi_id;
	~VoxelGIComponent() {
		if (voxel_gi_id.is_valid()) {
			RenderingServer::get_singleton()->free(voxel_gi_id);
		}
	}
};

#define VOXEL_GI_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, voxel_gi_id,VoxelGIComponent)\


#define VOXEL_GI_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, voxel_gi_id, VoxelGIComponentRef)\

class VoxelGIComponentRef : public FlecsComponent<VoxelGIComponent> {
GDCLASS(VoxelGIComponentRef, FlecsComponent<VoxelGIComponent>)
public : RID get_voxel_gi_id() const {
		auto typed = get_typed_data<VoxelGIComponent>();
		if (typed) {
			return typed->voxel_gi_id;
		}
		return default_value<RID>();
	}
	void set_voxel_gi_id(RID value) const {
		auto typed = get_typed_data<VoxelGIComponent>();
		if (typed) {
			typed->voxel_gi_id = value;
		}
	}
	static Ref<VoxelGIComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<VoxelGIComponentRef> class_ref = Ref<VoxelGIComponentRef>(memnew(VoxelGIComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<VoxelGIComponent>({});
		class_ref->set_data(&entity->get_mut<VoxelGIComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<VoxelGIComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<VoxelGIComponent>();
			VoxelGIComponent *copied = memnew(VoxelGIComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(VoxelGIComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "VoxelGIComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "voxel_gi_id",
					&VoxelGIComponentRef::get_voxel_gi_id);
			ClassDB::bind_method("set_"
								 "voxel_gi_id",
					&VoxelGIComponentRef::set_voxel_gi_id);
			::ClassDB::add_property(VoxelGIComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "voxel_gi_id"), _scs_create("set_"
																																				  "voxel_gi_id"),
					_scs_create("get_"
								"voxel_gi_id"));
		} while (0);
		ClassDB::bind_static_method(VoxelGIComponentRef::get_class_static(), "create_component", &VoxelGIComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &VoxelGIComponentRef::get_type_name);
	}
};
;

struct ScenarioComponent {
	RID id;
};

#define SCENARIO_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, id,ScenarioComponent)\


#define SCENARIO_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, id, ScenarioComponentRef)\

class ScenarioComponentRef : public FlecsComponent<ScenarioComponent> {
GDCLASS(ScenarioComponentRef, FlecsComponent<ScenarioComponent>)
public : RID get_id() const {
		auto typed = get_typed_data<ScenarioComponent>();
		if (typed) {
			return typed->id;
		}
		return default_value<RID>();
	}
	void set_id(RID value) const {
		auto typed = get_typed_data<ScenarioComponent>();
		if (typed) {
			typed->id = value;
		}
	}
	static Ref<ScenarioComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<ScenarioComponentRef> class_ref = Ref<ScenarioComponentRef>(memnew(ScenarioComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<ScenarioComponent>({});
		class_ref->set_data(&entity->get_mut<ScenarioComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<ScenarioComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<ScenarioComponent>();
			ScenarioComponent *copied = memnew(ScenarioComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(ScenarioComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "ScenarioComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "id",
					&ScenarioComponentRef::get_id);
			ClassDB::bind_method("set_"
								 "id",
					&ScenarioComponentRef::set_id);
			::ClassDB::add_property(ScenarioComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "id"), _scs_create("set_"
																																		  "id"),
					_scs_create("get_"
								"id"));
		} while (0);
		ClassDB::bind_static_method(ScenarioComponentRef::get_class_static(), "create_component", &ScenarioComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &ScenarioComponentRef::get_type_name);
	}
};
;

struct RenderInstanceComponent {
	RID instance_id;
	~RenderInstanceComponent() {
		if (instance_id.is_valid()) {
			RenderingServer::get_singleton()->free(instance_id);
		}
	}
};

#define RENDER_INSTANCE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, instance_id,RenderInstanceComponent)\


#define RENDER_INSTANCE_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, instance_id, RenderInstanceComponentRef)\

class RenderInstanceComponentRef : public FlecsComponent<RenderInstanceComponent> {
GDCLASS(RenderInstanceComponentRef, FlecsComponent<RenderInstanceComponent>)
public : RID get_instance_id() const {
		auto typed = get_typed_data<RenderInstanceComponent>();
		if (typed) {
			return typed->instance_id;
		}
		return default_value<RID>();
	}
	void set_instance_id(RID value) const {
		auto typed = get_typed_data<RenderInstanceComponent>();
		if (typed) {
			typed->instance_id = value;
		}
	}
	static Ref<RenderInstanceComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<RenderInstanceComponentRef> class_ref = Ref<RenderInstanceComponentRef>(memnew(RenderInstanceComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<RenderInstanceComponent>({});
		class_ref->set_data(&entity->get_mut<RenderInstanceComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<RenderInstanceComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<RenderInstanceComponent>();
			RenderInstanceComponent *copied = memnew(RenderInstanceComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(RenderInstanceComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "RenderInstanceComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "instance_id",
					&RenderInstanceComponentRef::get_instance_id);
			ClassDB::bind_method("set_"
								 "instance_id",
					&RenderInstanceComponentRef::set_instance_id);
			::ClassDB::add_property(RenderInstanceComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "instance_id"), _scs_create("set_"
																																						 "instance_id"),
					_scs_create("get_"
								"instance_id"));
		} while (0);
		ClassDB::bind_static_method(RenderInstanceComponentRef::get_class_static(), "create_component", &RenderInstanceComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &RenderInstanceComponentRef::get_type_name);
	}
};
;

struct CanvasItemComponent {
	RID canvas_item_id;
	StringName class_name;
	~CanvasItemComponent() {
		if (canvas_item_id.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_clear(canvas_item_id);
			RenderingServer::get_singleton()->free(canvas_item_id);
		}
	}
};

#define CANVAS_ITEM_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(RID, canvas_item_id,CanvasItemComponent)\
DEFINE_PROPERTY(StringName, class_name,CanvasItemComponent)\


#define CANVAS_ITEM_COMPONENT_BINDINGS\
BIND_PROPERTY(RID, canvas_item_id, CanvasItemComponentRef)\
BIND_PROPERTY(StringName, class_name, CanvasItemComponentRef)\

class CanvasItemComponentRef : public FlecsComponent<CanvasItemComponent> {
GDCLASS(CanvasItemComponentRef, FlecsComponent<CanvasItemComponent>)
public : RID get_canvas_item_id() const {
		auto typed = get_typed_data<CanvasItemComponent>();
		if (typed) {
			return typed->canvas_item_id;
		}
		return default_value<RID>();
	}
	void set_canvas_item_id(RID value) const {
		auto typed = get_typed_data<CanvasItemComponent>();
		if (typed) {
			typed->canvas_item_id = value;
		}
	}
	StringName get_class_name() const {
		auto typed = get_typed_data<CanvasItemComponent>();
		if (typed) {
			return typed->class_name;
		}
		return default_value<StringName>();
	}
	void set_class_name(StringName value) const {
		auto typed = get_typed_data<CanvasItemComponent>();
		if (typed) {
			typed->class_name = value;
		}
	}
	static Ref<CanvasItemComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<CanvasItemComponentRef> class_ref = Ref<CanvasItemComponentRef>(memnew(CanvasItemComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<CanvasItemComponent>({});
		class_ref->set_data(&entity->get_mut<CanvasItemComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<CanvasItemComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<CanvasItemComponent>();
			CanvasItemComponent *copied = memnew(CanvasItemComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(CanvasItemComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "CanvasItemComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "canvas_item_id",
					&CanvasItemComponentRef::get_canvas_item_id);
			ClassDB::bind_method("set_"
								 "canvas_item_id",
					&CanvasItemComponentRef::set_canvas_item_id);
			::ClassDB::add_property(CanvasItemComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<RID>::value, "canvas_item_id"), _scs_create("set_"
																																						"canvas_item_id"),
					_scs_create("get_"
								"canvas_item_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "class_name",
					&CanvasItemComponentRef::get_class_name);
			ClassDB::bind_method("set_"
								 "class_name",
					&CanvasItemComponentRef::set_class_name);
			::ClassDB::add_property(CanvasItemComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<StringName>::value, "class_name"), _scs_create("set_"
																																						   "class_name"),
					_scs_create("get_"
								"class_name"));
		}
		while (0)
			;
		ClassDB::bind_static_method(CanvasItemComponentRef::get_class_static(), "create_component", &CanvasItemComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &CanvasItemComponentRef::get_type_name);
	}
};
;

struct OccluderComponent {
	RID occluder_id;
	~OccluderComponent() {
		if (occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(occluder_id);
		}
	}
};

struct RenderingBaseComponents{
	flecs::component<MeshComponent> mesh;
	flecs::component<MultiMeshComponent> multi_mesh;
	flecs::component<MultiMeshInstanceComponent> mesh_instance;
	flecs::component<ParticlesComponent> particles;
	flecs::component<ReflectionProbeComponent> probe;
	flecs::component<SkeletonComponent> skeleton;
	flecs::component<EnvironmentComponent> environment;
	flecs::component<CameraComponent> camera;
	flecs::component<CompositorComponent> compositor;
	flecs::component<DirectionalLight3DComponent> directional_light;
	flecs::component<DirectionalLight2DComponent> directional_light_2d;
	flecs::component<PointLightComponent> point_light;
	flecs::component<OmniLightComponent> omni_light;
	flecs::component<SpotLightComponent> spot_light;
	flecs::component<ViewportComponent> viewport;
	flecs::component<ScenarioComponent> scenario;
	flecs::component<VoxelGIComponent> voxel_gi;
	flecs::component<RenderInstanceComponent> instance;
	flecs::component<CanvasItemComponent> canvas_item;
	flecs::component<OccluderComponent> occluder;

	explicit RenderingBaseComponents(const flecs::world &world) :
			mesh(world.component<MeshComponent>("MeshComponent")),
			multi_mesh(world.component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world.component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world.component<ParticlesComponent>("ParticlesComponent")),
			probe(world.component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world.component<SkeletonComponent>("SkeletonComponent")),
			environment(world.component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world.component<CameraComponent>("CameraComponent")),
			compositor(world.component<CompositorComponent>("CompositorComponent")),
			directional_light(world.component<DirectionalLight3DComponent>("DirectionalLightComponent")),
			directional_light_2d(world.component<DirectionalLight2DComponent>("DirectionalLight2DComponent")),
			point_light(world.component<PointLightComponent>("PointLightComponent")),
			omni_light(world.component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world.component<SpotLightComponent>("SpotLightComponent")),
			viewport(world.component<ViewportComponent>("ViewportComponent")),
			scenario(world.component<ScenarioComponent>("ScenarioComponent")),
			voxel_gi(world.component<VoxelGIComponent>("VoxelGIComponent")),
			instance(world.component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")),
			occluder(world.component<OccluderComponent>("OccluderComponent")) {}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
