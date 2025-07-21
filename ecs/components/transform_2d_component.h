#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../../../../core/math/transform_2d.h"
#include "single_component_module.h"
#include "../flecs_types/flecs_entity.h"
#include <cassert>

struct Transform2DComponent {
	Transform2D transform;
};

#define TRANSFORM_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(Transform2D, transform,Transform2DComponent)\

#define TRANSFORM_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(Transform2D, transform, Transform2DComponentRef)\

class Transform2DComponentRef : public FlecsComponent<Transform2DComponent> {
GDCLASS(Transform2DComponentRef, FlecsComponent<Transform2DComponent>)
public : Transform2D get_transform() const {
		auto typed = get_typed_data<Transform2DComponent>();
		if (typed) {
			return typed->transform;
		}
		return default_value<Transform2D>();
	}
	void set_transform(Transform2D value) const {
		auto typed = get_typed_data<Transform2DComponent>();
		if (typed) {
			typed->transform = value;
		}
	}
	static Ref<Transform2DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Transform2DComponentRef> class_ref = Ref<Transform2DComponentRef>(memnew(Transform2DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Transform2DComponent>({});
		class_ref->set_data(&entity->get_mut<Transform2DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Transform2DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Transform2DComponent>();
			Transform2DComponent *copied = memnew(Transform2DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Transform2DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Transform2DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "transform",
					&Transform2DComponentRef::get_transform);
			ClassDB::bind_method("set_"
								 "transform",
					&Transform2DComponentRef::set_transform);
			::ClassDB::add_property(Transform2DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<Transform2D>::value, "transform"), _scs_create("set_"
																																							"transform"),
					_scs_create("get_"
								"transform"));
		} while (0);
		ClassDB::bind_static_method(Transform2DComponentRef::get_class_static(), "create_component", &Transform2DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Transform2DComponentRef::get_type_name);
	}
};
;

using Transform2DComponentModule = SingleComponentModule<Transform2DComponent>;

