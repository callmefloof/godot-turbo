//
// Created by Floof on 21-7-2025.
//
#include "flecs_script_system.h"
#include "core/templates/rid.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "flecs.h"
#include "ecs/flecs_types/flecs_server.h"

flecs::query<> FlecsScriptSystem::get_query(const PackedStringArray &component_names) {
	if (component_names.size() == 0) {
		// Create a query that matches all entities
		return world->query_builder<>().cache_kind(flecs::QueryCacheAuto).with(flecs::Wildcard).build();
	}

	// Start the builder and add the first component (will be added again in the loop but that's fine)
	flecs::query_builder<> q = world->query_builder<>().cache_kind(flecs::QueryCacheAuto);

	for (int i = 0; i < component_names.size(); i++) {
		String cname = component_names.get(i);
		flecs::entity e = world->component(cname.ascii().get_data());

		if (!e.is_valid()) {
			print_line(String("Invalid component name: ") + cname);
			continue;
		}

		q.term().id(e.id());  // simple presence test
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
		// Array vargs;
		// vargs.append(wrapped); // No need for manual Variant
		if (!callback.is_valid()) {
			WARN_PRINT("Callable is not valid!");
			return;
		}

		callback.call_deferred (wrapped);
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

void FlecsScriptSystem::set_world(const RID &world_rid) {
		this->world_id = world_rid;
		world = FlecsServer::get_singleton()->_get_world(world_rid);
		if (!world) {
			ERR_PRINT("FlecsScriptSystem::set_world: world_id is not a valid world");
		}
}