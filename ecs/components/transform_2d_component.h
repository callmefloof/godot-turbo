#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "single_component_module.h"
#include <cassert>

struct Transform2DComponent {
	Transform2D transform;
};

#define TRANSFORM_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(Transform2D, transform)\

#define TRANSFORM_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(Transform2D, transform, Transform2DComponentRef)\

DEFINE_COMPONENT_PROXY(Transform2DComponentRef, Transform2DComponent,
TRANSFORM_3D_COMPONENT_PROPERTIES,
TRANSFORM_3D_COMPONENT_BINDINGS);

using Transform2DComponentModule = SingleComponentModule<Transform2DComponent>;

