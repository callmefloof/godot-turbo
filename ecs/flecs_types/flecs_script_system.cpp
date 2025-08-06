//
// Created by Floof on 21-7-2025.
//
#include "flecs_script_system.h"
#include "core/variant/variant.h"
#include "core/variant/array.h"
#include "flecs_entity.h"
#include "flecs_world.h"

flecs::query<> FlecsScriptSystem::get_query(const Vector<String> &component_names) {
	flecs::query_builder<> q = world_ref->get_world().query_builder<>().cache_kind(flecs::QueryCacheAuto).with(component_names.get(0).ascii().get_data());

	for (int i = 0; i < component_names.size(); i++) {
		String cname = component_names[i];
		flecs::id comp_id = world_ref->get_world().lookup(cname.ascii().get_data());

		if (!flecs::entity(world_ref->get_world(), comp_id).is_valid()) {
			print_line("Invalid component name: %s", cname);
			continue;
		}

		q.term().id(comp_id);  // simple presence test
	}

	q.cache_kind(flecs::QueryCacheAll);
	return q.build();

}

void FlecsScriptSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("run"), &FlecsScriptSystem::run);
	ClassDB::bind_method(D_METHOD("set_callback", "p_callback"), &FlecsScriptSystem::set_callback);
	ClassDB::bind_method(D_METHOD("get_callback"), &FlecsScriptSystem::get_callback);


}
void FlecsScriptSystem::init(const Ref<FlecsWorld> &p_world, const Vector<String> &p_required_components, const Callable& p_callable) {
	if(query.c_ptr()){
		query.destruct();
	}
	set_world(p_world.ptr());
	set_required_components(p_required_components);
	set_callback(p_callable);
	query = get_query(p_required_components);

}

void FlecsScriptSystem::reset(const Ref<FlecsWorld> &p_world, const Vector<String> &p_required_components, const Callable& p_callable){
	init(p_world,p_required_components,p_callable);

}
void FlecsScriptSystem::run() const {
	// Get the query based on required_components

	query.each([=](flecs::entity e) {
		Ref<FlecsEntity> wrapped = world_ref->add_entity(e);
		Array vargs;
		vargs.append(wrapped); // No need for manual Variant
		if (!callback.is_valid()) {
			WARN_PRINT("Callable is not valid!");
			return;
		}
		const Variant ret = callback.callv(vargs);
	});
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