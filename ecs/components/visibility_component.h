//
// Created by Floof on 28-7-2025.
//

#ifndef VISIBILITY_COMPONENT_H
#define VISIBILITY_COMPONENT_H
#include "core/object/ref_counted.h"
#include "../flecs_types/flecs_component.h"
#include "single_component_module.h"
#include "component_proxy.h"
#include "core/config/engine.h"
#include "core/os/memory.h"

#include <cassert>

struct VisibilityComponent {
	bool visible = true;
};

class VisibilityComponentRef : public FlecsComponent<VisibilityComponent> {
	#define VISIBILITY_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(bool,visible,VisibilityComponent)

	#define VISIBILITY_COMPONENT_BINDINGS\
	BIND_PROPERTY(bool,visible,VisibilityComponentRef)

	DEFINE_COMPONENT_PROXY(VisibilityComponent, VISIBILITY_COMPONENT_PROPERTIES, VISIBILITY_COMPONENT_BINDINGS);

};

using VisibilityComponentModule = SingleComponentModule<VisibilityComponent>;

#endif //VISIBILITY_COMPONENT_H
