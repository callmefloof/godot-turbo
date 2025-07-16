//
// Created by Floof on 15-7-2025.
//

#ifndef FLECS_COMPONENT_BASE_H
#define FLECS_COMPONENT_BASE_H
#include "../../../../core/io/resource.h"

class FlecsEntity;
class FlecsComponentBase : public Resource {
	GDCLASS(FlecsComponentBase, Resource);
protected:
	flecs::world *world = nullptr;
	flecs::entity *owner = nullptr;
	flecs::entity *component = nullptr;
public:
	FlecsComponentBase() = default;
	virtual ~FlecsComponentBase() = default;
	static void _bind_methods();
	virtual StringName get_type_name() const { return get_class_name(); }
	virtual flecs::entity* get_component() const { return component; };

};

inline void FlecsComponentBase::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_type_name"), &FlecsComponentBase::get_type_name);
}

#endif //FLECS_COMPONENT_BASE_H

