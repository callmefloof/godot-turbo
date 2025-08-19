//
// Created by Floof on 21-7-2025.
//
#include "flecs_script_system.h"
#include "core/templates/rid.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "flecs.h"
#include "ecs/flecs_types/flecs_server.h"

flecs::query<> FlecsScriptSystem::get_query(const Vector<String> &component_names) {
	flecs::query_builder<> q = world->query_builder<>().cache_kind(flecs::QueryCacheAuto).with(component_names.get(0).ascii().get_data());

	for (int i = 0; i < component_names.size(); i++) {
		String cname = component_names[i];
		flecs::id comp_id = world->lookup(cname.ascii().get_data());

		if (!flecs::entity(*world, comp_id).is_valid()) {
			print_line("Invalid component name: %s", cname);
			continue;
		}

		q.term().id(comp_id);  // simple presence test
	}

	q.cache_kind(flecs::QueryCacheAll);
	return q.build();
}

void FlecsScriptSystem::init(const RID &game_world_id, const PackedStringArray &req_comps, const Callable& p_callable) {
	if(query.c_ptr()){
		query.destruct();
	}
	set_world(game_world_id);
	set_required_components(req_comps);
	set_callback(p_callable);
	query = get_query(req_comps);

}

void FlecsScriptSystem::reset(const RID &game_world_id, const PackedStringArray &req_comps, const Callable& p_callable){
	init(game_world_id, req_comps, p_callable);
}
void FlecsScriptSystem::run() const {
	// Get the query based on required_components
	query.each([=](flecs::entity e) {
		RID wrapped = FlecsServer::get_singleton()->_get_or_create_rid_for_entity(world_id,e);
		Array vargs;
		vargs.append(wrapped); // No need for manual Variant
		if (!callback.is_valid()) {
			WARN_PRINT("Callable is not valid!");
			return;
		}
		const Variant ret = callback.callv(vargs);
	});
}

void FlecsScriptSystem::set_required_components(const PackedStringArray &p_required_components) {
	required_components = p_required_components;
}
PackedStringArray FlecsScriptSystem::get_required_components() const {
	return required_components;
}
void FlecsScriptSystem::set_callback(const Callable &p_callback) {
	callback = p_callback;
}
Callable FlecsScriptSystem::get_callback() const {
	return callback;
}
PackedStringArray FlecsScriptSystem::get_required_components() {
	return required_components;
}

flecs::world* FlecsScriptSystem::_get_world() const {
	return world;
}
void FlecsScriptSystem::_set_world(flecs::world *p_world) {
	world = p_world;
}

RID FlecsScriptSystem::get_world() {
	if (!world || !world_id.is_valid()) {
		ERR_PRINT("FlecsScriptSystem::get_world: world is not set");
		return RID();
	}
	return world_id;
}

void FlecsScriptSystem::set_world(const RID &world_id) {
		this->world_id = world_id;
		world = FlecsServer::get_singleton()->_get_world(world_id);
		if (!world) {
			ERR_PRINT("FlecsScriptSystem::set_world: world_id is not a valid world");
		}
	}