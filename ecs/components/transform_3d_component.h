#pragma once
#include "../../../../core/math/transform_3d.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "component_proxy.h"
#include "single_component_module.h"

#include <cassert>

struct Transform3DComponent {
	Transform3D transform;
};

#define TRANSFORM_3D_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(Transform3D, transform,Transform3DComponent)\

#define TRANSFORM_3D_COMPONENT_BINDINGS\
BIND_PROPERTY(Transform3D, transform, Transform3DComponentRef)\

class Transform3DComponentRef : public FlecsComponent<Transform3DComponent> {
GDCLASS(Transform3DComponentRef, FlecsComponent<Transform3DComponent>)
public : Transform3D get_transform() const {
		auto typed = get_typed_data<Transform3DComponent>();
		if (typed) {
			return typed->transform;
		}
		return default_value<Transform3D>();
	}
	void set_transform(Transform3D value) const {
		auto typed = get_typed_data<Transform3DComponent>();
		if (typed) {
			typed->transform = value;
		}
	}
	static Ref<Transform3DComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<Transform3DComponentRef> class_ref = Ref<Transform3DComponentRef>(memnew(Transform3DComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<Transform3DComponent>({});
		class_ref->set_data(&entity->get_mut<Transform3DComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<Transform3DComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<Transform3DComponent>();
			Transform3DComponent *copied = memnew(Transform3DComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(Transform3DComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "Transform3DComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "transform",
					&Transform3DComponentRef::get_transform);
			ClassDB::bind_method("set_"
								 "transform",
					&Transform3DComponentRef::set_transform);
			::ClassDB::add_property(Transform3DComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<Transform3D>::value, "transform"), _scs_create("set_"
																																							"transform"),
					_scs_create("get_"
								"transform"));
		} while (0);
		ClassDB::bind_static_method(Transform3DComponentRef::get_class_static(), "create_component", &Transform3DComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &Transform3DComponentRef::get_type_name);
	}
};
;

using Transform3DComponentModule = SingleComponentModule<Transform3DComponent>;
