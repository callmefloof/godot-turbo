#include "flecs_component_base.h"

template<typename T>
class FlecsSingletonComponent : public FlecsComponentBase {
public:
	FlecsSingletonComponent() = default;
	~FlecsSingletonComponent() override = default;
	T &get_data() const;
	flecs::entity get_internal_owner() const override;
	virtual void set_data(T &p_data);
	flecs::id get_internal_component() const override;
	void set_internal_component(const flecs::id& component);
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
T& FlecsSingletonComponent<T>::get_data() const{
	T* data = try_get_world_typed_data<T>();
	if(!data){
		if(!Engine::get_singleton()->is_editor_hint()){
			WARN_PRINT("Data is null. Are you trying to instantiate a tag type as a component?");
			WARN_PRINT("Returing base type T");
		}
		static T empty = T{};
		return empty;
	}
	return *data;
}

template <typename T>
flecs::entity FlecsSingletonComponent<T>::get_internal_owner() const {
    WARN_PRINT("FlecsSingletonComponent<T>'s owner is the flecs world, do not use this method");
	return owner;
}

template <typename T>
void FlecsSingletonComponent<T>::set_data(T &p_data) {
	world->set<T>(p_data);
	world->modified<T>();
}

template <typename T>
flecs::id FlecsSingletonComponent<T>::get_internal_component() const {
	return component;
}

template <typename T>
inline void FlecsSingletonComponent<T>::set_internal_component(const flecs::id &p_component) {
	component = p_component;
}

template <typename T>
int FlecsSingletonComponent<T>::get_type_id() const {
	return type_id();
}

template <typename T>
StringName FlecsSingletonComponent<T>::get_type_name() const {
	return get_class()+String("<" + String(NAMEOF_TYPE(T).data()) + ">");
}

template <typename T>
void FlecsSingletonComponent<T>::clear_component() {
	world->set<T>({});
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
Ref<FlecsComponentBase> FlecsSingletonComponent<T>::clone() const {
	Ref<FlecsSingletonComponent<T>> new_ref;
	new_ref.instantiate();
	new_ref->set_data(get_data());
	new_ref->set_internal_owner(get_internal_owner());
	new_ref->set_internal_component(get_internal_component());
	new_ref->set_internal_world(get_internal_world());

	return new_ref;
}