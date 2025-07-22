//
// Created by Floof on 16-7-2025.
//
#include "flecs_component_base.h"

#include "../../thirdparty/flecs/distr/flecs.h"
#include "flecs_world.h"

void FlecsComponentBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type_name"),&FlecsComponentBase::get_type_name);
	ClassDB::bind_method(D_METHOD("clear_component"),&FlecsComponentBase::clear_component);
	ClassDB::bind_method(D_METHOD("commit_to_entity", "p_entity"),&FlecsComponentBase::commit_to_entity);

}

StringName FlecsComponentBase::get_type_name() const { return get_class(); }
flecs::entity *FlecsComponentBase::get_component() const {
	return owner;
}


