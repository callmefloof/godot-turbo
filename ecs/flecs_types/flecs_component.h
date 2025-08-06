//
// Created by Floof on 14-7-2025.
//


#pragma once
#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "core/variant/variant_utility.h"
#include "thirdparty/nameof/include/nameof.hpp"
#include "flecs_component.h"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "type_id_generator.h"
class FlecsEntity;

template <typename T>
class FlecsComponent : public FlecsComponentBase {
public:
	FlecsComponent() = default;
	~FlecsComponent() override = default;
	T &get_data() const;
	Ref<FlecsEntity> get_owner() const;
	flecs::entity get_internal_owner() const override;
	void set_flecs_owner(flecs::entity p_owner);
	void set_owner(const Ref<FlecsEntity> &p_owner);
	virtual void set_data(T &p_data);
	flecs::id get_internal_component() const override;
	StringName get_type_name() const override;
	void clear_component() override;
	//PackedByteArray byte_serialize() const;
	//void byte_deserialize(const PackedByteArray &p_ba);
	Ref<FlecsComponentBase> clone() const override;
	static int type_id() {
		static int id = TypeIDGenerator::get_type_id<T>();
		return id;
	}
	int get_type_id() const override;

};

template <typename T>
T& FlecsComponent<T>::get_data() const{
	return get_typed_data<T>();
}

template <typename T>
flecs::entity FlecsComponent<T>::get_internal_owner() const {
	return owner;
}
template <typename T>
Ref<FlecsEntity> FlecsComponent<T>::get_owner() const {
	return gd_owner;
}
template <typename T>
void FlecsComponent<T>::set_owner(const Ref<FlecsEntity> &p_owner) {
#ifdef DEBUG_ENABLED
	if (!p_owner.is_valid()) {
		ERR_PRINT("FlecsComponent::set_owner called with invalid owner Ref");
	}
#endif
	gd_owner = p_owner.is_valid() ? p_owner : nullptr;

}

template <typename T>
void FlecsComponent<T>::set_flecs_owner(const flecs::entity p_owner) {
	const char* c_type_name = String(get_type_name()).ascii().get_data();
	if (const flecs::entity comp = p_owner.world().lookup(c_type_name)) {
		if (p_owner.has(comp)) {
			owner = p_owner;
		}
	}
	ERR_PRINT("p_owner does not have component");

}

template <typename T>
void FlecsComponent<T>::set_data(T &p_data) {
	owner.set<T>(p_data);
	owner.modified<T>();
}

template <typename T>
flecs::id FlecsComponent<T>::get_internal_component() const {
	return component;
}

template <typename T>
int FlecsComponent<T>::get_type_id() const {
	return type_id();
}

template <typename T>
StringName FlecsComponent<T>::get_type_name() const {
	return get_class()+String("<" + String(NAMEOF_TYPE(T).data()) + ">");
}

template <typename T>
void FlecsComponent<T>::clear_component() {
	owner.set<T>({});
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
	new_ref->set_data(get_data());
	return new_ref;
}



