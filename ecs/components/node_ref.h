//
// Created by Floof on 15-7-2025.
//
#ifndef NODE_REF_H
#define NODE_REF_H
#include "scene/main/node.h"
#include "core/object/ref_counted.h"
#include "core/object/object_id.h"
#include "core/object/object.h"
#include "core/typedefs.h"
#include "core/variant/dictionary.h"

#include "core/string/ustring.h"
#include "thirdparty/flecs/distr/flecs.h"

class NodeRef : public RefCounted {
	GDCLASS(NodeRef, RefCounted)
private:
	Node *node; // Pointer to the node, can be nullptr if the node is not found
public:
	ObjectID id; // Unique identifier for the node
	NodeRef() :
			node(nullptr) {}
	explicit NodeRef(const ObjectID& id) {
		this->id = id;
		if (id.is_valid()) {
			node = Object::cast_to<Node>(ObjectDB::get_instance(id));
		} else {
			node = nullptr;
		}
	}
	Node *get_node() {
		if (!id.is_valid())
		{
			return nullptr;
		}
		if (node == nullptr) {
			node = Object::cast_to<Node>(ObjectDB::get_instance(id));
		}
		return node;
	}
};

#endif //NODE_REF_H
