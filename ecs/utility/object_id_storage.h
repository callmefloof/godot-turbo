#pragma once
#include "core/object/object_id.h"
#include "core/templates/oa_hash_map.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "scene/main/node.h"
#include "scene/main/scene_tree.h"

struct ObjectIDContainer {
	ObjectID id;
	RID instance;
	String class_name;
	ObjectIDContainer(const ObjectID &p_id, const String &p_class_name, const RID& p_instance) :
			id(p_id), class_name(p_class_name), instance(p_instance) {};
};

class ObjectIDStorage {

private:
	static inline Vector<ObjectIDContainer> object_id_pool;

public:

	static inline bool add(Object *p_obj, const RID &p_rid)
	{
		if (p_obj == nullptr) {
			return false;
		}
		Node* node = Object::cast_to<Node>(p_obj);
		if (node != nullptr && node->is_inside_tree()) {
			node->get_tree()->get_current_scene()->remove_child(node);
		}
		ObjectIDContainer container(p_obj->get_instance_id(),p_obj->get_class(),p_rid);
		return object_id_pool.append(container);
	}

	static inline bool release(const RID &p_instance) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->instance != p_instance) {
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
				node->get_tree()->get_current_scene()->remove_child(node);
				node->queue_free();
				return object_id_pool.erase(*it);
			}
		}
		return false;
	}

	static inline bool release(const ObjectID &p_id)
	{
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->id != p_id) {
				continue;
			}
			if (!p_id.is_valid()) {
				return object_id_pool.erase(*it);
			}
			auto obj = ObjectDB::get_instance(p_id);
			auto node = Object::cast_to<Node>(obj);
			if (node == nullptr) {
				memdelete(obj);
				return object_id_pool.erase(*it);
			}
			if (node->is_inside_tree()) {
				node->get_tree()->get_current_scene()->remove_child(node);
				node->queue_free();
				return object_id_pool.erase(*it);
			}
		}
		return false;
	}

	static inline bool has(const ObjectID &p_id) {
		for (const ObjectIDContainer &container : object_id_pool) {
			if (container.id == p_id) {
				return true;
			}
		}
		return false;
	}

	static inline bool has(const RID& p_instance) {
	for (const ObjectIDContainer &container : object_id_pool) {
		if (container.instance == p_instance) {
			return true;
		}
	}
	return false;
	}
	static inline ObjectIDContainer *get(const ObjectID &p_id) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->id == p_id) {
				return &(*it);
			}
		}
		return nullptr;
	}

	static inline ObjectIDContainer *get(const RID &p_instance) {
		for (Vector<ObjectIDContainer>::Iterator it = object_id_pool.begin(); it != object_id_pool.end(); ++it) {
			if (it->instance == p_instance) {
				return &(*it);
			}
		}
		return nullptr;
	}
};
