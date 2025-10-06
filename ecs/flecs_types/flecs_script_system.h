//
// Created by Floof on 21-7-2025.
//

#ifndef FLECS_SCRIPT_SYSTEM_H
#define FLECS_SCRIPT_SYSTEM_H
#include "core/templates/rid.h"
#include "core/typedefs.h"
#include "core/variant/callable.h"
#include "core/variant/variant.h"
#include "thirdparty/flecs/distr/flecs.h"

class FlecsScriptSystem {
	Callable callback;
	PackedStringArray required_components;
	flecs::query<> get_query(const PackedStringArray &component_names);
	RID world_id;
	flecs::world *world = nullptr;
	flecs::query<> query;

public:
	void init(const RID &world_id, const PackedStringArray &req_comps, const Callable& p_callable);
	void reset(const RID &world_id, const PackedStringArray &req_comps, const Callable& p_callable);
	void run() const;
	void set_required_components(const PackedStringArray &req_comps);
	PackedStringArray get_required_components() const;
	void set_callback(const Callable& p_callback);
	Callable get_callback() const;
	PackedStringArray get_required_components();
	flecs::world* _get_world() const;
	void _set_world(flecs::world *p_world);
	RID get_world();
	void set_world(const RID &world_id);
};
#endif //FLECS_SCRIPT_SYSTEM_H
