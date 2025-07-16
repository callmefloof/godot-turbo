#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "single_component_module.h"
#include "core/math/transform_3d.h"
#include <cassert>
#include "component_proxy.h"

struct Transform3DComponent {
	Transform3D transform;
};

#define TRANSFORM_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(Transform3D, transform)\

#define TRANSFORM_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(Transform3D, transform, Transform3DComponentRef)\

DEFINE_COMPONENT_PROXY(Transform3DComponentRef, Transform3DComponent,
TRANSFORM_3D_COMPONENT_PROPERTIES,
TRANSFORM_3D_COMPONENT_BINDINGS);

using Transform3DComponentModule = SingleComponentModule<Transform3DComponent>;
