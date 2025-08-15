//
// Created by Floof on 15-7-2025.
//

#ifndef FLECS_COMPONENT_BASE_H
#define FLECS_COMPONENT_BASE_H
#include "core/error/error_macros.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/string/string_name.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/io/resource.h"
#include "core/object/ref_counted.h"
#include "core/typedefs.h"
#include <cassert>
#include <type_traits>

class FlecsEntity;

class FlecsComponentBase : public Resource {
	GDCLASS(FlecsComponentBase, Resource);
protected:
	flecs::world* world = nullptr;
	flecs::entity owner;
	Ref<FlecsEntity> gd_owner;
	flecs::id component;

public:
	FlecsComponentBase() = default;
	~FlecsComponentBase() override = default;
	static void _bind_methods();
	virtual StringName get_type_name() const = 0;
	virtual flecs::id get_internal_component() const;
	virtual flecs::entity get_internal_owner() const;
	virtual void set_component(const flecs::entity& p_component);
	virtual flecs::world* get_internal_world() const;
	virtual void set_internal_world(flecs::world* p_world);
	virtual void set_internal_owner(const flecs::entity& p_owner);
	virtual void clear_component() = 0;
	template<typename T>
	T* try_get_world_typed_data() const {
		if(std::is_empty<T>::value){
			ERR_PRINT("Type is empty, returning nullptr");
			return nullptr;
		}
		const flecs::entity expected_component = world->component<T>();

	#ifdef DEBUG_ENABLED
	#ifdef DEBUG_COMPONENT_ID
		print_line(vformat("Expected ID: %d | Actual ID: %d",
			(uint64_t)expected_component.raw_id(),
			(uint64_t)component.raw_id()));
		print_line(expected_component == component ? "ID Match" : "ID Mismatch");
	#endif
	#endif

		CRASH_COND_MSG(expected_component != component,
			"get_typed_data<T>() called with wrong component type!");

		return world->try_get_mut<T>();
	}
	template<typename T>
	T* try_get_typed_data() const {
		if(std::is_empty<T>::value){
			ERR_PRINT("Type is empty, returning nullptr");
			return nullptr;
		}
		const flecs::entity expected_component = world->component<T>();

#ifdef DEBUG_ENABLED
#ifdef DEBUG_COMPONENT_ID
		print_line(vformat("Expected ID: %d | Actual ID: %d",
			(uint64_t)expected_component.raw_id(),
			(uint64_t)component.raw_id()));
		print_line(expected_component == component ? "ID Match" : "ID Mismatch");
#endif
#endif

		CRASH_COND_MSG(expected_component != component,
			"get_typed_data<T>() called with wrong component type!");

		return owner.try_get_mut<T>();
	}
	virtual int get_type_id() const = 0;
	virtual bool is_dynamic() const { return false; }
	virtual Ref<FlecsComponentBase> clone() const = 0;

};


#endif //FLECS_COMPONENT_BASE_H

