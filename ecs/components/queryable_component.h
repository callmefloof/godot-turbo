//
// Created by Floof on 20-7-2025.
//

#pragma once
#include "../../../../core/os/memory.h"
#include "../flecs_types/flecs_component.h"
#include "../flecs_types/flecs_entity.h"
#include "../flecs_types/flecs_component_base.h"
#include "component_proxy.h"
#include "single_component_module.h"

struct QueryableComponent {};

using QueryableComponentModule = SingleComponentModule<QueryableComponent>;

#define QUERYABLE_COMPONENT_PROPERTIES\

#define QUERYABLE_COMPONENT_BINDINGS\

class QueryableComponentRef : public FlecsComponent<QueryableComponent> {
GDCLASS(QueryableComponentRef, FlecsComponent<QueryableComponent>)
public : static Ref<QueryableComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<QueryableComponentRef> class_ref = Ref<QueryableComponentRef>(memnew(QueryableComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<QueryableComponent>({});
		class_ref->set_data(&entity->get_mut<QueryableComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<QueryableComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<QueryableComponent>();
			QueryableComponent *copied = memnew(QueryableComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(QueryableComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "QueryableComponent"; }
	static void _bind_methods() {
		;
		ClassDB::bind_static_method(QueryableComponentRef::get_class_static(), "create_component", &QueryableComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &QueryableComponentRef::get_type_name);
	}
};
;
