#pragma once
#include "flecs_component_base.h"

#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include "core/variant/variant_utility.h"
#include "thirdparty/nameof/include/nameof.hpp"
#include "flecs_component_base.h"
#include "flecs_entity.h"
#include "type_id_generator.h"

template<typename T>
class FlecsTag : public FlecsComponentBase {
    public:
	FlecsTag() = default;
	~FlecsTag() override = default;
	Ref<FlecsEntity> get_owner() const;
	flecs::entity get_internal_owner() const override;
	void set_flecs_owner(flecs::entity p_owner);
	void set_owner(const Ref<FlecsEntity> &p_owner);
	flecs::id get_internal_component() const override;
	StringName get_type_name() const override;
	static int type_id() {
		static int id = TypeIDGenerator::get_type_id<T>();
		return id;
	}
	int get_type_id() const override;
    void clear_component() override;
	Ref<FlecsComponentBase> clone() const override;

};

template <typename T>
flecs::entity FlecsTag<T>::get_internal_owner() const {
	return owner;
}
template <typename T>
Ref<FlecsEntity> FlecsTag<T>::get_owner() const {
	return gd_owner;
}
template <typename T>
void FlecsTag<T>::set_owner(const Ref<FlecsEntity> &p_owner) {
#ifdef DEBUG_ENABLED
	if (!p_owner.is_valid()) {
		ERR_PRINT("FlecsComponent::set_owner called with invalid owner Ref");
	}
#endif
	gd_owner = p_owner.is_valid() ? p_owner : nullptr;

}

template <typename T>
void FlecsTag<T>::set_flecs_owner(const flecs::entity p_owner) {
	const char* c_type_name = String(get_type_name()).ascii().get_data();
	if (const flecs::entity comp = p_owner.world().lookup(c_type_name)) {
		if (p_owner.has(comp)) {
			owner = p_owner;
		}
	}
	ERR_PRINT("p_owner does not have component");

}

template <typename T>
flecs::id FlecsTag<T>::get_internal_component() const {
	return component;
}

template <typename T>
int FlecsTag<T>::get_type_id() const {
	return type_id();
}

template <typename T>
StringName FlecsTag<T>::get_type_name() const {
	return get_class()+String("<" + String(NAMEOF_TYPE(T).data()) + ">");
}

template <typename T>
void FlecsTag<T>::clear_component() {
	WARN_PRINT("Tag component cannot be cleared.");
    return;
}

template <typename T>
Ref<FlecsComponentBase> FlecsTag<T>::clone() const {
	Ref<FlecsTag<T>> new_ref;
	new_ref.instantiate();
	return new_ref;
}


