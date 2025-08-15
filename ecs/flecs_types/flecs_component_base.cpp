//
// Created by Floof on 16-7-2025.
//
#include "flecs_component_base.h"

#include "../../thirdparty/flecs/distr/flecs.h"
#include "flecs_world.h"


void FlecsComponentBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type_name"),&FlecsComponentBase::get_type_name);
	ClassDB::bind_method(D_METHOD("clear_component"),&FlecsComponentBase::clear_component);

}

flecs::id FlecsComponentBase::get_internal_component() const {
	return component;
}
flecs::entity FlecsComponentBase::get_internal_owner() const {
	return owner;
}
void FlecsComponentBase::set_component(const flecs::entity& p_component) {
	print_line("Setting component ID: " + itos((uint64_t)p_component.raw_id()));
	component = p_component;
}
void FlecsComponentBase::set_internal_world( flecs::world* p_world) {
	world = p_world;
}
void FlecsComponentBase::set_internal_owner(const flecs::entity& p_owner) {
	owner = p_owner;
}
flecs::world* FlecsComponentBase::get_internal_world() const {
	return world;
}


