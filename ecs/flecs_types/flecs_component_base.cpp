//
// Created by Floof on 16-7-2025.
//
#include "flecs_component_base.h"


void FlecsComponentBase::_bind_methods() {

}

StringName FlecsComponentBase::get_type_name() const { return get_class(); }
flecs::entity *FlecsComponentBase::get_component() const {
	return entity;
}


