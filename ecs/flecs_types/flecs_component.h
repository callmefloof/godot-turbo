//
// Created by Floof on 14-7-2025.
//

#pragma once
#include "../../../../core/object/object.h"
#include "../../../../core/typedefs.h"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "../../thirdparty/nameof/include/nameof.hpp"

template <typename T>
class FlecsComponent : public FlecsComponentBase {
	GDCLASS(FlecsComponent, FlecsComponentBase);
protected:
	T* data = nullptr;
public:
	T* get_data();
	flecs::entity *get_owner() const;
	void set_owner(flecs::entity *p_owner);
	void commit_to_entity(const Ref<FlecsEntity>& entity);
	void set_data(T* d);
	flecs::entity* get_component() const override;
	virtual StringName get_type_name() const override;
	static void _bind_methods();
};

template <typename T>
T* FlecsComponent<T>::get_data(){
	return data;
}

template <typename T>
flecs::entity* FlecsComponent<T>::get_owner() const {
	return owner;
}

template<typename T>
void FlecsComponent<T>::set_owner(flecs::entity *p_owner) {owner = p_owner;}

template <typename T>
void FlecsComponent<T>::commit_to_entity(const Ref<FlecsEntity>& entity) {
	if (!entity.is_valid() || entity.is_null()) {
		ERR_PRINT("Entity is not valid");
	}
	const flecs::entity e = entity->get_entity();
	if (data) {
		e.set<T>(*data);
	}
}
template <typename T>
void FlecsComponent<T>::set_data(T *d) {
	data = d;
}
template <typename T>
flecs::entity *FlecsComponent<T>::get_component() const {
	return owner->get<T>();
}
template <typename T>
void FlecsComponent<T>::_bind_methods() {
}

template <typename T>
StringName FlecsComponent<T>::get_type_name() const { return String(get_class_name()) + "<" + NAMEOF_TYPE(T) + ">"; }

