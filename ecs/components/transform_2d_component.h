#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "single_component_module.h"
#include <cassert>

struct Transform2DComponent : ScriptVisibleComponent {
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "transform",transform, Variant::TRANSFORM2D);
	}
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["transform"] = transform;
		return dict;
	}
	Transform2D transform;
};
using Transform2DComponentModule = SingleComponentModule<Transform2DComponent>;

