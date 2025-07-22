//
// Created by Floof on 20-7-2025.
//

#pragma once
#include "../../../../core/os/memory.h"
#include "../flecs_types/flecs_component.h"
#include "../flecs_types/flecs_entity.h"
#include "../flecs_types/flecs_component_base.h"
#include "component_proxy.h"
#include "single_component_module.h"
#include "../../../../core/templates/rid.h"
#include "../../../../core/string/ustring.h"


struct QueryableComponent {
	int _dummy = 0;
};

class QueryableComponentRef : public FlecsComponent<QueryableComponent> {

	#define QUERYABLE_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(int, _dummy,QueryableComponent)

	#define QUERYABLE_COMPONENT_BINDINGS\
	BIND_PROPERTY(int, _dummy, QueryableComponentRef)

	DEFINE_COMPONENT_PROXY(QueryableComponentRef, QueryableComponent, QUERYABLE_COMPONENT_PROPERTIES, QUERYABLE_COMPONENT_BINDINGS);
};


using QueryableComponentModule = SingleComponentModule<QueryableComponent>;



