#pragma once

#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "scene/main/node.h"
#include "scene/2d/node_2d.h"
#include "scene/3d/node_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/2d/physics/rigid_body_2d.h"
#include "scene/main/window.h"
#include "core/variant/variant.h"


struct NodeContainer {
	ObjectID id;                  // Rendering handle
	Node* node;   // Keeps the resource alive
	String class_name;        // For debugging / reflection

	bool operator==(const NodeContainer &other) const {
		return id == other.id && class_name == other.class_name;
	}
};

class NodeStorage {
private:
	Vector<NodeContainer> object_pool;
	Window* scene_node = nullptr;
public:
	NodeStorage() = default;
	~NodeStorage() {
		release_all();
	}
	 void make_inert(Node *p_node) {
		if (!p_node) return;

		p_node->set_process(false);
		p_node->set_physics_process(false);
		p_node->set_process_input(false);
		p_node->set_process_unhandled_input(false);
		p_node->set_process_unhandled_key_input(false);

		if (Node2D *n2d = Object::cast_to<Node2D>(p_node)) {
			n2d->set_visible(false);
		} else if (Node3D *n3d = Object::cast_to<Node3D>(p_node)) {
			n3d->set_visible(false);
		}

		if (RigidBody3D *rb = Object::cast_to<RigidBody3D>(p_node)) {
			rb->set_sleeping(true);
			rb->set_freeze_mode(RigidBody3D::FREEZE_MODE_KINEMATIC);
			rb->set_freeze_enabled(true);
			rb->set_collision_layer(0);
			rb->set_collision_mask(0);
		}
		if (RigidBody2D *rb = Object::cast_to<RigidBody2D>(p_node)) {
			rb->set_sleeping(true);
			rb->set_freeze_mode(RigidBody2D::FREEZE_MODE_KINEMATIC);
			rb->set_freeze_enabled(true);
			rb->set_collision_layer(0);
			rb->set_collision_mask(0);
		}
	}

	// Add Node + ObjectID to storage
	template <typename T = Node>
	 bool add(T* p_node, const ObjectID &p_id) {
		if (!p_node || !p_id.is_valid()) {
			return false;
		}

		NodeContainer container;
		container.id = p_id;
		container.class_name = p_node->get_class();
		container.node = p_node;

		make_inert(p_node);

		if (!scene_node) {
			scene_node = p_node->get_tree()->get_root();
		}

		Node *storage_parent = scene_node->get_node_or_null(NodePath(String("/root/NodeStorage")));
		if (!storage_parent) {
			storage_parent = memnew(Node);
			storage_parent->set_name("NodeStorage");
			scene_node->call_deferred("add_child", storage_parent);
		}

		if (p_node->get_parent()) {
			p_node->get_parent()->call_deferred("remove_child", p_node);
		}
		storage_parent->call_deferred("add_child", p_node);
		object_pool.append(container);
		return true;
	}

	// Remove resource by RID
	 bool release(const ObjectID &p_id) {
		for (auto it = object_pool.begin(); it != object_pool.end(); ++it) {
			if (it->id != p_id) {
				continue;
			}
            if (it->node) {
				it->node->queue_free();
			}

			object_pool.erase(*it);
			return true;
		}
		return false;
	}

	// Remove all resources
	 void release_all() {
		for (NodeContainer &container : object_pool) {
			if (container.node) {
                container.node->queue_free();
            }
            container.node = nullptr; // Clear the pointer to avoid dangling references
		}
		object_pool.clear();
	}

	// Check if RID exists
	 bool has(const ObjectID &p_id) {
		for (const NodeContainer &container : object_pool) {
			if (container.id == p_id) {
                return true;
            }
            continue;
		}
		return false;
	}

	// Get container by ID
	 NodeContainer *try_get(const ObjectID &p_id) {
		for (auto it = object_pool.begin(); it != object_pool.end(); ++it) {
			if (it->id == p_id) {
				return &*it;
			}
		}
		return nullptr;
	}
};
