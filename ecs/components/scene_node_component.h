#pragma once
#include "core/object/object_id.h"
#include "core/string/ustring.h"
#include "single_component_module.h"
#include "scene/main/node.h"

template <typename T>
struct NodeRef
{
private:
	T *node; // Pointer to the node, can be nullptr if the node is not found
public:
	ObjectID id; // Unique identifier for the node
	NodeRef() :
			node(nullptr) {}
	explicit NodeRef(ObjectID id) {
		this->id = id;
		if (id.is_valid()) {
			node = Object::cast_to<T>(ObjectDB::get_instance(id));
		} else {
			node = nullptr;
		}
	}
	T *get_node() {
		if (!id.is_valid())
		{
			return nullptr;
		}
		if (node == nullptr) {
			node = Object::cast_to<T>(ObjectDB::get_instance(id));
		}
		return node;
	}
};


template<typename T = Node>
struct SceneNodeComponent : ScriptVisibleComponent {
	ObjectID node_id; // Unique identifier for the node
	String class_name; // Class name of the node
	NodeRef<T> node_ref; // Reference to the node
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["node_id"] = node_id;
		dict["class_name"] = class_name;
		dict["node_ref"] = node_ref;
		return dict;
	}
};

template<typename T = Node>
using SceneNodeComponentModule = SingleComponentModule<SceneNodeComponent<T>>;
