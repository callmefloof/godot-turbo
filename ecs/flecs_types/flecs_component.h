//
// Created by Floof on 14-7-2025.
//


#pragma once
#include "../../../../core/error/error_macros.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/typedefs.h"
#include "../../thirdparty/nameof/include/nameof.hpp"
#include "flecs_component.h"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "../../../../core/variant/variant_utility.h"
class FlecsEntity;

template <typename T>
class FlecsComponent : public FlecsComponentBase {
	GDCLASS(FlecsComponent, FlecsComponentBase);
public:
	T* get_data();
	flecs::entity *get_owner() const;
	static uint64_t get_type_hash_static();
	void set_owner(flecs::entity *p_owner);
	void commit_to_entity(const Ref<FlecsEntity> &p_entity) const override;
	virtual void set_data(T* p_data);
	flecs::entity* get_component() const override;
	StringName get_type_name() const override;
	void apply_to_entity(flecs::entity& e) const override;
	void clear_component() override;
	//PackedByteArray byte_serialize() const;
	//void byte_deserialize(const PackedByteArray &p_ba);
	Ref<FlecsComponentBase> clone() const override;
	static void _bind_methods();

};


template <typename T>
T* FlecsComponent<T>::get_data(){
	return get_typed_data<T>();
}

template <typename T>
flecs::entity *FlecsComponent<T>::get_owner() const {
	return owner;
}
template <typename T>
uint64_t FlecsComponent<T>::get_type_hash_static() {
	static int type_marker;
	return reinterpret_cast<uint64_t>(&type_marker);
}

template <typename T>
void FlecsComponent<T>::set_owner(flecs::entity *p_owner) {
	if (owner) {
		ERR_PRINT("p_owner is null and/or owner is null");
		return;
	}
	if (p_owner == nullptr) {
		ERR_PRINT("p_owner is null and/or owner is null");
		return;
	}
	const char* c_type_name = String(get_type_name()).ascii().get_data();
	if (const flecs::entity comp = p_owner->world().lookup(c_type_name)) {
		if (p_owner->has(comp)) {
			owner = p_owner;
		}
	}
	ERR_PRINT("p_owner does not have component");

}

template <typename T>
void FlecsComponent<T>::commit_to_entity(const Ref<FlecsEntity>& p_entity) const {
	if (!p_entity.is_valid() || p_entity.is_null()) {
		ERR_PRINT("Entity is not valid");
	}
	const flecs::entity *e = p_entity->get_entity();
	if (data) {
		e->set<T>(*get_typed_data<T>());
	}
}
template <typename T>
void FlecsComponent<T>::set_data(T *p_data) {
	if (p_data != nullptr) {
		data = p_data;
		this->data = p_data;
		this->component_type_hash = get_type_hash_static();
		return;
	}
	ERR_PRINT("Data is null");
}

template <typename T>
flecs::entity *FlecsComponent<T>::get_component() const {
	return component;
}
template <typename T>
void FlecsComponent<T>::_bind_methods() {
	ClassDB::bind_method(D_METHOD("commit_to_entity"), &FlecsComponent::commit_to_entity);
	//ClassDB::bind_method(D_METHOD("byte_serialize"), &FlecsComponent::byte_serialize);
	//ClassDB::bind_method(D_METHOD("byte_deserialize", "p_ba"), &FlecsComponent::byte_deserialize);
	ClassDB::bind_method(D_METHOD("clone"), &FlecsComponent::clone);
}

template <typename T>
StringName FlecsComponent<T>::get_type_name() const {
	return get_class()+String("<" + String(NAMEOF_TYPE(T).data()) + ">");
}

template <typename T>
void FlecsComponent<T>::apply_to_entity(flecs::entity &e) const {
	if (data) {
		e.set<T>(*get_typed_data<T>());
		return;
	}
	ERR_PRINT("data is null");
}

template <typename T>
void FlecsComponent<T>::clear_component() {
	if (data) {
		*static_cast<T *>(data) = T{};
	}
}
// these cannot work due to the usage of RID's in components. I could refactor every RID into an uint64_t representation
// but that would be tedious
// template <typename T>
// PackedByteArray FlecsComponent<T>::byte_serialize() const {
// 	const Object* obj = this;
// 	const Variant variant = obj;
// 	return VariantUtilityFunctions::var_to_bytes_with_objects(variant);
// }
// template <typename T>
// void FlecsComponent<T>::byte_deserialize(const PackedByteArray &p_ba) {
// 	const Variant variant = VariantUtilityFunctions::bytes_to_var_with_objects(p_ba);
// 	if (!variant.is_ref_counted()) {
// 		ERR_PRINT("Failed to deserialize flecs component: variant not refcounted.");
// 		return;
// 	}
// 	if (variant.is_null()) {
// 		ERR_PRINT("Failed to deserialize flecs component: variant null.");
// 		return;
// 	}
// 	Object* obj = variant;
// 	if (obj == nullptr) {
// 		ERR_PRINT("Failed to deserialize flecs component: object is null.");
// 		return;
// 	}
// 	Ref<FlecsComponent<T>> flecs_comp = Object::cast_to<FlecsComponent<T>>(obj);
// 	if (!flecs_comp.is_valid()) {
// 		ERR_PRINT("Failed to deserialize flecs component: component object is invalid.");
// 		return;
// 	}
// 	if (flecs_comp.is_null()) {
// 		ERR_PRINT("Failed to deserialize flecs component: component object is null.");
// 		return;
// 	}
// 	set_data(flecs_comp->get_data());
// 	return;
// }


template <typename T>
Ref<FlecsComponentBase> FlecsComponent<T>::clone() const {
	Ref<FlecsComponent<T>> new_ref;
	new_ref.instantiate();
	if (data) {
		const auto typed = static_cast<T *>(data);
		/* Step 2: allocate memory and copy*/
		T *copied = memnew(T);
		*copied = *typed; /* copy the struct contents*/
		new_ref->set_data(copied);
	}
	return new_ref;
}



