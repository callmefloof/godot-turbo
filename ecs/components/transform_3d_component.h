#pragma once
#include "core/math/transform_3d.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "component_proxy.h"
#include "single_component_module.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/flecs_types/flecs_world.h"

#include <cassert>

struct Transform3DComponent {
	Transform3D transform;
};

class Transform3DComponentRef : public FlecsComponent<Transform3DComponent> {
	#define TRANSFORM_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(Transform3D, transform,Transform3DComponent)\

	#define TRANSFORM_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(Transform3D, transform, Transform3DComponentRef)\

	DEFINE_COMPONENT_PROXY(Transform3DComponent,TRANSFORM_3D_COMPONENT_PROPERTIES,TRANSFORM_3D_COMPONENT_BINDINGS);
};

using Transform3DComponentModule = SingleComponentModule<Transform3DComponent>;
