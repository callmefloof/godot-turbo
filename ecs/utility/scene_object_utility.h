#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "servers/rendering_server.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "../components/scene_node_component.h"
#include "../components/transform_2d_component.h"
#include "../components/transform_3d_component.h"
#include "../components/rendering/rendering_components.h"
#include "navigation/2d/navigation2d_utility.h"
#include "navigation/3d/navigation3d_utility.h"
#include "physics/2d/physics2d_utility.h"
#include "physics/3d/physics3d_utility.h"
#include "rendering/render_utility_2d.h"
#include "core/variant/variant.h"
#include "resource_object_utility.h"
#include "core/templates/vector.h"
#include "core/templates/rid_owner.h"
#include "scene/main/viewport.h"
#include <cassert>
#include "scene/3d/node_3d.h"
#include <scene/2d/mesh_instance_2d.h>
#include "rendering/render_utility_3d.h"



namespace godot_turbo::utility {

class SceneObjectUtility {
private:
	SceneObjectUtility() = default; // Prevent instantiation
	SceneObjectUtility(const SceneObjectUtility &) = delete; // Prevent copy
	SceneObjectUtility &operator=(const SceneObjectUtility &) = delete; // Prevent assignment
	SceneObjectUtility(SceneObjectUtility &&) = delete; // Prevent move
	SceneObjectUtility &operator=(SceneObjectUtility &&) = delete; // Prevent move assignment
public:
	static inline Vector<flecs::entity> CreateEntitiesFromScene(flecs::world& world, SceneTree* tree )
	{
		if (tree == nullptr) {
			ERR_FAIL_V(Vector<flecs::entity>());
		}
		Vector<flecs::entity> entities;
		auto children = tree->get_root()->get_children();
		for (Variant &variant : children) {
			Node *node = Object::cast_to<Node>(variant);
			if (node == nullptr) {
				continue;
			}


			auto entity = CreateEntity(world, node);
			auto node_children = node->get_children();
			if (!entity.is_valid()) {
				ERR_FAIL_V(entities);
			}
			if (node_children.size() > 0)
			{
				for (Variant &child_variant : node_children) {
					Node *child_node = Object::cast_to<Node>(child_variant);
					if (child_node == nullptr) {
						continue;
					}
					auto child_entity = CreateEntity(world, child_node);
					if (!child_entity.is_valid()) {
						ERR_FAIL_V(entities);
					}
					entity.child(child_entity);
				}
			}
			

			
			entities.push_back(entity);
		}
	}

	static inline Vector<flecs::entity> CreateEntities(flecs::world &world, Node *base_node, Vector<flecs::entity> &entities,
			int current_depth, const int max_depth = 10000) {
		if (base_node == nullptr) {
			ERR_FAIL_V(entities);
		}
		current_depth++;
		if (current_depth > max_depth) {
			ERR_FAIL_V(entities);
		}
		auto children = base_node->get_children();
		for (Variant &variant : children ) {
			Node *node = Object::cast_to<Node>(variant);
			if (node == nullptr) {
				continue;
			}
			auto entity = CreateEntity(world, node);
			if (!entity.is_valid()) {
				ERR_FAIL_V(entities);
			}
			auto node_children = node->get_children();
			if (node_children.size() > 0) {
				auto child_entities = CreateEntities(world, node, entities, current_depth, max_depth);
				for (flecs::entity &child_entity : child_entities) {
					entity.child(child_entity);
				}
			}
			entities.push_back(entity);
			
		}
		return entities;
	}
	//template"typename T = Node"
	static inline flecs::entity CreateEntity(flecs::world &world, Node* node)
	{

		String name = node->get_name();

		MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(node);
		if (mesh_instance != nullptr) {
			return RenderUtility3D::CreateMeshInstance3D(world, mesh_instance);
		}
		MeshInstance2D *mesh_instance_2d = Object::cast_to<MeshInstance2D>(node);
		if (mesh_instance_2d != nullptr) {
			return RenderUtility2D::CreateMeshInstance(world, mesh_instance_2d);
		}

		NavigationAgent2D *navigation_agent_2d = Object::cast_to<NavigationAgent2D>(node);
		if (navigation_agent_2d != nullptr) {
			return Navigation2DUtility::CreateNav2DAgent(world, navigation_agent_2d);
		}

		NavigationRegion2D *navigation_region_2d = Object::cast_to<NavigationRegion2D>(node);
		if (navigation_region_2d != nullptr) {
			return Navigation2DUtility::CreateNav2DRegion(world, navigation_region_2d);
		}
		NavigationRegion3D *navigation_region_3d = Object::cast_to<NavigationRegion3D>(node);
		if (navigation_region_3d != nullptr) {
			return Navigation3DUtility::CreateNav3DRegion(world, navigation_region_3d);
		}
		PhysicsBody2D *physics_body_2d = Object::cast_to<PhysicsBody2D>(node);
		if (physics_body_2d != nullptr) {
			return Physics2DUtility::CreatePhysicsBody(world, physics_body_2d);
		}
		PhysicsBody3D *physics_body_3d = Object::cast_to<PhysicsBody3D>(node);
		if (physics_body_3d != nullptr) {
			return Physics3DUtility::CreatePhysicsBody(world, physics_body_3d);
		}
		// Handle other node types as needed
		Node2D *node_2d = Object::cast_to<Node2D>(node);
		Node3D *node_3d = Object::cast_to<Node3D>(node);
		if (node_2d != nullptr) {

			Transform2D transform = node_2d->get_transform();
			return world.entity().set<Transform2DComponent>({ transform }).set_name(name.ascii().get_data());
		} else if (node_3d != nullptr) {
			Transform3D transform = node_3d->get_transform();
			return world.entity().set<Transform3DComponent>({ transform }).set_name(name.ascii().get_data());
		}

		//// WORK ON THIS
		if (node == nullptr) {
			ERR_FAIL_V(flecs::entity());
		}


		const auto variant = node->get_script();
		Ref<Script> script = variant;
		flecs::entity entity = world.entity().set<SceneNodeComponent<Node>>({
			node->get_instance_id(), node->get_class(), NodeRef<Node>(node->get_instance_id())
			})
			.set_name(name.ascii().get_data());
		if (script.is_valid()) {
			////entity.child(ResourceObjectUtility::CreateResourceEntity<Script>(world, script)); <----- FIX THIS
		} 
			

		return entity;

	}

};
}
