//
// Created by Floof on 21-7-2025.
//

#ifndef FLECS_SCRIPT_SYSTEM_H
#define FLECS_SCRIPT_SYSTEM_H
#include "../../../../core/io/resource.h"
#include "../../../../core/object/object.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/templates/vector.h"
#include "../../../../core/typedefs.h"
#include "../../../../core/variant/callable.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "systems/commands/command.h"

class FlecsEntity;
class Variant;
struct QueryableComponent;

class FlecsScriptSystem : public Resource{
	GDCLASS(FlecsScriptSystem, Resource)
private:
	Callable callback;
	Vector<String> required_components;
	static flecs::query<> get_query(const flecs::world &world, const Vector<String> &component_names);
	flecs::world world_ref;


protected:
	static void _bind_methods();

public:
	void run() const;
	void set_world(const flecs::world &p_world);
	flecs::world &get_world();
	void set_required_components(const Vector<String> &p_required_components);
	Vector<String> get_required_components() const;
	void set_callback(const Callable& p_callback);
	Callable get_callback() const;
	Vector<String> get_required_components();
};
#endif //FLECS_SCRIPT_SYSTEM_H
