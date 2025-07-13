#pragma once
#include "core/object/object_id.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "scene/main/node.h"

struct ObjectIDContainer {
	ObjectID id;
	RID rid;
	String class_name;
	bool operator==(const ObjectIDContainer &other) const {
		return id == other.id && rid == other.rid && class_name == other.class_name;
	}
	
};

class ObjectIDStorage {

private:
	static Vector<ObjectIDContainer> object_id_pool;

public:

	static bool add(Object *p_obj, const RID &p_rid)
	{
		if (p_obj == nullptr) {
			return false;
		}
		Node* node = Object::cast_to<Node>(p_obj);
		if (node != nullptr && node->is_inside_tree()) {
			node->get_parent()->remove_child(node);
		}
		ObjectIDContainer container;

		container.rid = p_rid;
		container.class_name = p_obj->get_class();
		container.id = p_obj->get_instance_id();
		return object_id_pool.append(container);
	}

	static bool release(const RID &p_instance) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->rid != p_instance) {
				continue;
			}
			if (!p_instance.is_valid()) {
				return object_id_pool.erase(*it);
			}
			auto obj = ObjectDB::get_instance(it->id);
			auto node = Object::cast_to<Node>(obj);
			if (node == nullptr) {
				memdelete(obj);
				return object_id_pool.erase(*it);
			}
			if (node->is_inside_tree()) {
				node->get_parent()->remove_child(node);
				node->queue_free();
				return object_id_pool.erase(*it);
			}
		}
		return false;
	}

	static bool release(const ObjectID &p_id)
	{
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->id != p_id) {
				continue;
			}
			if (!p_id.is_valid()) {
				return object_id_pool.erase(*it);
			}
			const auto obj = ObjectDB::get_instance(p_id);
			auto node = Object::cast_to<Node>(obj);
			if (node == nullptr) {
				memdelete(obj);
				return object_id_pool.erase(*it);
			}
			if (node->is_inside_tree()) {
				node->get_parent()->remove_child(node);	
			}
			node->queue_free();
			return object_id_pool.erase(*it);
		}
		return false;
	}

	static bool has(const ObjectID &p_id) {
		for (const ObjectIDContainer &container : object_id_pool) {
			if (container.id == p_id) {
				return true;
			}
		}
		return false;
	}

	static bool has(const RID& p_instance) {
	for (const ObjectIDContainer &container : object_id_pool) {
		if (container.rid == p_instance) {
			return true;
		}
	}
	return false;
	}
	static ObjectIDContainer *get(const ObjectID &p_id) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->id == p_id) {
				return &*it;
			}
		}
		return nullptr;
	}

	static ObjectIDContainer *get(const RID &p_instance) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->rid == p_instance) {
				return &*it;
			}
		}
		return nullptr;
	}
};
