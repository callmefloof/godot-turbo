#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "single_component_module.h"
#include "core/math/transform_3d.h"
#include <cassert>
struct Transform3DComponent {
	Transform3D transform;
};
using Transform3DComponentModule = SingleComponentModule<Transform3DComponent>;
