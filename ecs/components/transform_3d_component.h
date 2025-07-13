#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "single_component_module.h"
#include "core/math/transform_3d.h"
#include <cassert>
struct Transform3DComponent : ScriptVisibleComponent {
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "transform",transform, Variant::TRANSFORM3D);
	}
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["transform"] = transform;
		return dict;
	}
	Transform3D transform;
};
using Transform3DComponentModule = SingleComponentModule<Transform3DComponent>;
