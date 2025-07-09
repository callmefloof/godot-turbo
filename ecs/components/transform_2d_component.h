#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "single_component_module.h"
#include <cassert>

struct Transform2DComponent {
	Transform2D transform;
};
using Transform2DComponentModule = SingleComponentModule<Transform2DComponent>;

