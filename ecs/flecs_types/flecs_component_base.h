//
// Created by Floof on 15-7-2025.
//

#ifndef FLECS_COMPONENT_BASE_H
#define FLECS_COMPONENT_BASE_H
#include "../../../../core/error/error_macros.h"
#include "../../../../core/object/class_db.h"
#include "../../../../core/object/object.h"
#include "../../../../core/string/string_name.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "flecs_entity.h"

class FlecsComponentBase : public FlecsEntity {
	GDCLASS(FlecsComponentBase, FlecsEntity);
protected:
	flecs::world *world = nullptr;
	flecs::entity *owner = nullptr;
	void* data = nullptr;
	uint64_t component_type_hash = 0;
	template<typename T>
	static uint64_t type_hash() { return reinterpret_cast<uint64_t>(&typeid(T)); }

public:
	FlecsComponentBase() = default;
	~FlecsComponentBase() override = default;
	static void _bind_methods();
	virtual StringName get_type_name() const;
	virtual flecs::entity* get_component() const;
	virtual void clear_component() = 0;
	virtual void apply_to_entity(flecs::entity& e) const = 0;
	virtual void commit_to_entity(const Ref<FlecsEntity>& p_entity) const = 0;
	template<typename T>
	T* get_typed_data() const {
		if (component_type_hash != type_hash<T>()) {
			ERR_PRINT("Type mismatch");
			return nullptr;
		}
		return static_cast<T*>(data);
	}
	virtual bool is_dynamic() const { return false; }
	virtual void* get_data_ptr() const { return data; }
	virtual Ref<FlecsComponentBase> clone() const = 0;

};


#endif //FLECS_COMPONENT_BASE_H

