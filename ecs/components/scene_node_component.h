#pragma once
#include "../flecs_types/flecs_component.h"
#include "component_proxy.h"
#include "../../../../core/object/object_id.h"
#include "../../../../core/os/memory.h"
#include "node_ref.h"
#include "../../../../scene/main/node.h"
#include "single_component_module.h"

struct SceneNodeComponent {
	ObjectID node_id; // Unique identifier for the node
	StringName class_name; // Class name of the node
	Ref<NodeRef> node_ref; // Reference to the node
};

#define SCENE_NODE_COMPONENT_PROPERTIES\
DEFINE_PROPERTY(ObjectID, node_id,SceneNodeComponent)\
DEFINE_PROPERTY(StringName, class_name,SceneNodeComponent)\
DEFINE_PROPERTY(Ref<NodeRef>, node_ref,SceneNodeComponent)\

#define SCENE_NODE_COMPONENT_BINDINGS\
BIND_PROPERTY(ObjectID, node_id, SceneNodeComponentRef)\
BIND_PROPERTY(StringName, class_name, SceneNodeComponentRef)\
BIND_PROPERTY(Ref<NodeRef>, node_ref, SceneNodeComponentRef)\

class SceneNodeComponentRef : public FlecsComponent<SceneNodeComponent> {
GDCLASS(SceneNodeComponentRef, FlecsComponent<SceneNodeComponent>)
public : ObjectID get_node_id() const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			return typed->node_id;
		}
		return default_value<ObjectID>();
	}
	void set_node_id(ObjectID value) const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			typed->node_id = value;
		}
	}
	StringName get_class_name() const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			return typed->class_name;
		}
		return default_value<StringName>();
	}
	void set_class_name(StringName value) const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			typed->class_name = value;
		}
	}
	Ref<NodeRef> get_node_ref() const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			return typed->node_ref;
		}
		return default_value<Ref<NodeRef>>();
	}
	void set_node_ref(Ref<NodeRef> value) const {
		auto typed = get_typed_data<SceneNodeComponent>();
		if (typed) {
			typed->node_ref = value;
		}
	}
	static Ref<SceneNodeComponentRef> create_component(const Ref<FlecsEntity> &owner) {
		Ref<SceneNodeComponentRef> class_ref = Ref<SceneNodeComponentRef>(memnew(SceneNodeComponentRef));
		const flecs::entity *entity = owner->get_entity();
		entity->set<SceneNodeComponent>({});
		class_ref->set_data(&entity->get_mut<SceneNodeComponent>());
		return class_ref;
	}
	static uint64_t get_type_hash_static() {
		static int type_marker;
		return reinterpret_cast<uint64_t>(&type_marker);
	}
	Ref<FlecsComponentBase> clone() const override {
		Ref<SceneNodeComponentRef> new_ref;
		new_ref.instantiate();
		if (data) {
			const auto typed = get_typed_data<SceneNodeComponent>();
			SceneNodeComponent *copied = memnew(SceneNodeComponent);
			*copied = *typed;
			new_ref->set_data(copied);
		}
		return new_ref;
	}
	void set_data(SceneNodeComponent *d) {
		this->data = d;
		this->component_type_hash = get_type_hash_static();
	}
	StringName get_type_name() const override { return "SceneNodeComponent"; }
	static void _bind_methods() {
		do {
			ClassDB::bind_method("get_"
								 "node_id",
					&SceneNodeComponentRef::get_node_id);
			ClassDB::bind_method("set_"
								 "node_id",
					&SceneNodeComponentRef::set_node_id);
			::ClassDB::add_property(SceneNodeComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<ObjectID>::value, "node_id"), _scs_create("set_"
																																					 "node_id"),
					_scs_create("get_"
								"node_id"));
		} while (0) do {
			ClassDB::bind_method("get_"
								 "class_name",
					&SceneNodeComponentRef::get_class_name);
			ClassDB::bind_method("set_"
								 "class_name",
					&SceneNodeComponentRef::set_class_name);
			::ClassDB::add_property(SceneNodeComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<StringName>::value, "class_name"), _scs_create("set_"
																																						  "class_name"),
					_scs_create("get_"
								"class_name"));
		}
		while (0) {
			do {
				ClassDB::bind_method("get_"
									 "node_ref",
						&SceneNodeComponentRef::get_node_ref);
				ClassDB::bind_method("set_"
									 "node_ref",
						&SceneNodeComponentRef::set_node_ref);
				::ClassDB::add_property(SceneNodeComponentRef::get_class_static(), PropertyInfo(VariantTypeMap<Ref<NodeRef>>::value, "node_ref"), _scs_create("set_"
																																							  "node_ref"),
						_scs_create("get_"
									"node_ref"));
			} while (0);
		}
		ClassDB::bind_static_method(SceneNodeComponentRef::get_class_static(), "create_component", &SceneNodeComponentRef::create_component, "owner");
		ClassDB::bind_method(D_METHOD("get_type_name"), &SceneNodeComponentRef::get_type_name);
	}
};
;

template<typename T = Node>
using SceneNodeComponentModule = SingleComponentModule<SceneNodeComponent>;
