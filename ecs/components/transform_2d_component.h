#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "single_component_module.h"
#include <cassert>

namespace godot_turbo::components {

struct Transform2DComponent {
	Transform2D transform;
};

using Transform2DComponentModule = SingleComponentModule<Transform2DComponent>;


} // namespace godot_turbo::components
