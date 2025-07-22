//
// Created by Floof on 21-7-2025.
//
#include "flecs_script_system.h"
#include "../../../../core/variant/variant.h"
#include "../../../../core/variant/array.h"
#include "../components/queryable_component.h"
#include "flecs_entity.h"
#include "flecs_world.h"

flecs::query<> FlecsScriptSystem::get_query(const flecs::world &world, const Vector<String> &component_names) {
	const char *name = String(component_names.get(0)).ascii().get_data();
	flecs::query_builder<> query = world.query_builder<>().term_at<QueryableComponent>().with(name);
	for (auto it = component_names.begin(); component_names.end() != it; it.operator++()) {
		name = String(*it).ascii().get_data();
		query.with(name);
	}
	query.cache_kind(flecs::QueryCacheAll);
	return query.build();
}

void FlecsScriptSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("run"), &FlecsScriptSystem::run);
	ClassDB::bind_method(D_METHOD("set_callback", "p_callback"), &FlecsScriptSystem::set_callback);
	ClassDB::bind_method(D_METHOD("get_callback"), &FlecsScriptSystem::get_callback);


}
void FlecsScriptSystem::run() const {
	// Get the query based on required_components
	if (world_ref == nullptr) {
		ERR_PRINT("world is null");
		return;
	}
	const flecs::query<> query = get_query(world_ref->get_world(), required_components);
	query.each([&](flecs::entity e) {
		const Ref<FlecsEntity> wrapped = FlecsWorld::wrap_entity(e);
		Array vargs;
		vargs.append(wrapped); // No need for manual Variant
		const Variant ret = callback.callv(vargs);
	});
}
void FlecsScriptSystem::set_world(flecs::world *p_world) {
	world_ref = p_world;
}
flecs::world *FlecsScriptSystem::get_world() const {
	return world_ref;
}
void FlecsScriptSystem::set_required_components(const Vector<String> &p_required_components) {
	required_components = p_required_components;
}
Vector<String> FlecsScriptSystem::get_required_components() const {
	return required_components;
}
void FlecsScriptSystem::set_callback(const Callable &p_callback) {
	callback = p_callback;
}
Callable FlecsScriptSystem::get_callback() const {
	return callback;
}
Vector<String> FlecsScriptSystem::get_required_components() {
	return required_components;
}