#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../components/scene_node_component.h"
#include "../components/transform_2d_component.h"
#include "../components/transform_3d_component.h"
#include "../../../../core/math/transform_2d.h"
#include "../../../../core/math/transform_3d.h"
#include "../../../../core/string/ustring.h"
#include "../../../../core/templates/vector.h"
#include "../../../../core/variant/variant.h"
#include "navigation/2d/navigation2d_utility.h"
#include "navigation/3d/navigation3d_utility.h"
#include "physics/2d/physics2d_utility.h"
#include "physics/3d/physics3d_utility.h"
#include "rendering/render_utility_2d.h"
#include "rendering/render_utility_3d.h"
#include "resource_object_utility.h"
#include "../../../../scene/3d/node_3d.h"
#include "../../../../scene/main/scene_tree.h"
#include "../../../../scene/main/window.h"
#include "../../../../scene/2d/mesh_instance_2d.h"
#include "../../../../scene/3d/navigation_agent_3d.h"
#include "../../../../scene/3d/navigation_link_3d.h"
#include "../../../../scene/3d/navigation_obstacle_3d.h"
#include "../../../../scene/3d/navigation_region_3d.h"
#include "../../../../scene/2d/navigation_agent_2d.h"
#include "../../../../scene/2d/navigation_link_2d.h"
#include "../../../../scene/2d/navigation_obstacle_2d.h"
#include "../../../../scene/2d/navigation_region_2d.h"
#include "../../../../scene/2d/physics/joints/joint_2d.h"
#include "../../../../scene/2d/physics/area_2d.h"
#include "../../../../scene/2d/physics/physics_body_2d.h"
#include "../../../../scene/2d/physics/rigid_body_2d.h"
#include "../../../../scene/2d/physics/static_body_2d.h"
#include "../../../../scene/3d/physics/joints/joint_3d.h"
#include "../../../../scene/3d/physics/rigid_body_3d.h"
#include "../../../../scene/3d/physics/static_body_3d.h"
#include "../../../../scene/3d/physics/area_3d.h"
#include "../../../../scene/3d/soft_body_3d.h"
#include "../../../../scene/3d/physics/physics_body_3d.h"
#include "../../../../scene/2d/multimesh_instance_2d.h"
#include "../../../../scene/3d/multimesh_instance_3d.h"
#include "../../../../scene/2d/skeleton_2d.h"
#include "../../../../scene/2d/light_2d.h"
#include "../../../../scene/2d/light_occluder_2d.h"
#include "../../../../scene/2d/gpu_particles_2d.h"
#include "../../../../scene/2d/node_2d.h"
#include "../../../../core/variant/binder_common.h"
#include "../../../../core/object/script_instance.h"

#include <cassert>

class SceneObjectUtility {
private:
	SceneObjectUtility() = default; // Prevent instantiation
	SceneObjectUtility(const SceneObjectUtility &) = delete; // Prevent copy
	SceneObjectUtility &operator=(const SceneObjectUtility &) = delete; // Prevent assignment
	SceneObjectUtility(SceneObjectUtility &&) = delete; // Prevent move
	SceneObjectUtility &operator=(SceneObjectUtility &&) = delete; // Prevent move assignment
public:
	static Vector<flecs::entity> create_entities_from_scene(const flecs::world& world, const SceneTree* tree )
	{
		if (tree == nullptr) {
			ERR_FAIL_V(Vector<flecs::entity>());
		}
		Vector<flecs::entity> entities;
		auto children = tree->get_root()->get_children();
		for (Variant &variant : children) {
			const Node *node = Object::cast_to<Node>(variant);
			if (node == nullptr) {
				continue;
			}
			entities.append_array(create_entities(world, node, entities));
		}
		return entities;
	}

	static Vector<flecs::entity> create_entities(const flecs::world &world, const Node *base_node, Vector<flecs::entity> &entities,
			int current_depth = 0, const int max_depth = 10000) {
		current_depth++;
		if (base_node == nullptr) {
			ERR_FAIL_COND_V(base_node == nullptr,entities);
		}
		if (current_depth > max_depth) {
			ERR_FAIL_COND_V(current_depth > max_depth,entities);
		}
		//get the children of Node
		auto children = base_node->get_children();
		for (Variant &variant : children ) {
			// for each node check to see if it is an object
			Node *child_node = Object::cast_to<Node>(variant);
			if (child_node == nullptr) {
				continue;
			}
			//gather resulting entities from entity creation
			Vector<flecs::entity> child_entity_result = create_entity(world, child_node);

			//check to see if the node has children
			TypedArray<Node> node_children = child_node->get_children();
			if (node_children.size() > 0) {
				//recursively check each child
				Vector<flecs::entity> child_entities = create_entities(world, child_node, entities, current_depth, max_depth);
				//repeat until wie hit our search limit
				entities.append_array(entities);
			}
		}
		//return resulting entities
		return entities;
	}

	static Vector<flecs::entity> create_entity(const flecs::world &world, Node* node) {
		Vector<flecs::entity> result;
		if (node == nullptr) {
			ERR_FAIL_COND_V(node == nullptr,result);
		}

		// 3D navigation
		NavigationAgent3D *navigation_agent_3d = Object::cast_to<NavigationAgent3D>(node);
		if (navigation_agent_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::create_nav_agent(world, navigation_agent_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationLink3D *navigation_link_3d = Object::cast_to<NavigationLink3D>(node);
		if ( navigation_link_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::create_nav_link(world, navigation_link_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationObstacle3D *navigation_obstacle_3d = Object::cast_to<NavigationObstacle3D>(node);
		if ( navigation_obstacle_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::create_nav_obstacle(world, navigation_obstacle_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationRegion3D *navigation_region_3d = Object::cast_to<NavigationRegion3D>(node);
		if (navigation_region_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::create_nav_region(world, navigation_region_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationAgent2D *navigation_agent_2d = Object::cast_to<NavigationAgent2D>(node);
		// 2D navigation
		if ( navigation_agent_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::create_nav_2d_agent(world, navigation_agent_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationLink2D *navigation_link_2d = Object::cast_to<NavigationLink2D>(node);
		if ( navigation_link_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::create_nav_2d_link(world, navigation_link_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationObstacle2D *navigation_obstacle_2d = Object::cast_to<NavigationObstacle2D>(node);
		if ( navigation_obstacle_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::create_nav_2d_obstacle(world, navigation_obstacle_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		NavigationRegion2D *navigation_region_2d = Object::cast_to<NavigationRegion2D>(node);
		if ( navigation_region_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::create_nav_2d_region(world, navigation_region_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}

		//3D Physics
		Area3D* area_3d = Object::cast_to<Area3D>(node);
		if ( area_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::create_area(world, area_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		RigidBody3D* rigid_body_3d = Object::cast_to<RigidBody3D>(node);
		if ( rigid_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::create_rigid_body(world, rigid_body_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		PhysicsBody3D *physics_body_3d = Object::cast_to<PhysicsBody3D>(node);
		if ( physics_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::create_physics_body(world, physics_body_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Joint3D* joint_3d = Object::cast_to<Joint3D>(node);
		if (joint_3d) {
			const flecs::entity entity = Physics3DUtility::create_joint(world, joint_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		SoftBody3D* soft_body_3d = Object::cast_to<SoftBody3D>(node);
		if ( soft_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::create_soft_body(world, soft_body_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}

		//2D Physics
		Area2D* area_2d = Object::cast_to<Area2D>(node);
		if ( area_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::create_area(world, area_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		RigidBody2D* rigid_body_2d = Object::cast_to<RigidBody2D>(node);
		if ( rigid_body_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::create_rigid_body(world, rigid_body_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		PhysicsBody2D *physics_body_2d = Object::cast_to<PhysicsBody2D>(node);
		if ( physics_body_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::create_physics_body(world, physics_body_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Joint2D *joint_2d = Object::cast_to<Joint2D>(node);
		if ( joint_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::create_joint(world, joint_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}

		//3D nodes
		MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(node);
		if ( mesh_instance != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_mesh_instance(world, mesh_instance);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		MultiMeshInstance3D *multi_mesh_instance_3d = Object::cast_to<MultiMeshInstance3D>(node);
		if ( multi_mesh_instance_3d != nullptr) {
			const flecs::entity multi_mesh_instance_3d_entity = RenderUtility3D::create_multi_mesh(world, multi_mesh_instance_3d);
			const uint32_t instance_count =  multi_mesh_instance_3d_entity.get<MultiMeshComponent>().instance_count;
			Vector<Transform3D> transforms;
			transforms.resize(instance_count);
			Vector<flecs::entity> multi_mesh_instances;
			multi_mesh_instances.append(multi_mesh_instance_3d_entity);
			multi_mesh_instances.append_array(RenderUtility3D::create_multi_mesh_instances(world,transforms,multi_mesh_instance_3d_entity));
			get_script(world, node, multi_mesh_instance_3d_entity, multi_mesh_instances);
			result.append_array(multi_mesh_instances);

			return result;
		}
		GPUParticles3D *particles_3d = Object::cast_to<GPUParticles3D>(node);
		if ( particles_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_particles(world, particles_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		ReflectionProbe* reflection_probe = Object::cast_to<ReflectionProbe>(node);
		if ( reflection_probe != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_reflection_probe(world, reflection_probe);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Skeleton3D* skeleton_3d = Object::cast_to<Skeleton3D>(node);
		if ( skeleton_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_skeleton(world, skeleton_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		WorldEnvironment* world_environment = Object::cast_to<WorldEnvironment>(node);
		if ( world_environment != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_environment(world, world_environment);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Camera3D* camera_3d = Object::cast_to<Camera3D>(node);
		if ( camera_3d != nullptr) {
			const flecs::entity entity =  RenderUtility3D::create_camera(world, camera_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		DirectionalLight3D* directional_light_3d = Object::cast_to<DirectionalLight3D>(node);
		if ( directional_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_directional_light(world, directional_light_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		OmniLight3D* omni_light_3d = Object::cast_to<OmniLight3D>(node);
		if ( omni_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_omni_light(world, omni_light_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		SpotLight3D* spot_light_3d = Object::cast_to<SpotLight3D>(node);
		if ( spot_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_spot_light(world, spot_light_3d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Viewport* viewport = Object::cast_to<Viewport>(node);
		if ( viewport != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_viewport(world, viewport);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		VoxelGI* voxel_gi = Object::cast_to<VoxelGI>(node);
		if ( voxel_gi != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_voxel_gi(world, voxel_gi);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}

		OccluderInstance3D* occluder_component = Object::cast_to<OccluderInstance3D>(node);
		if (occluder_component != nullptr) {
			const flecs::entity entity = RenderUtility3D::create_occluder(world, occluder_component);
			result.append(entity);
			return result;
		}

		// 2D nodes
		MeshInstance2D *mesh_instance_2d = Object::cast_to<MeshInstance2D>(node);
		if ( mesh_instance_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_mesh_instance(world, mesh_instance_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		MultiMeshInstance2D *mmi = Object::cast_to<MultiMeshInstance2D>(node);
		if ( mmi != nullptr) {const flecs::entity mmi_entity = RenderUtility2D::create_multi_mesh(world, mmi);
			const flecs::entity multi_mesh_instance_2d_entity = RenderUtility2D::create_multi_mesh(world, mmi);
			const uint32_t instance_count = mmi_entity.get<MultiMeshComponent>().instance_count;
			Vector<Transform2D> transforms;
			transforms.resize(instance_count);
			Vector<flecs::entity> multi_mesh_instances;
			get_script(world, node, multi_mesh_instance_2d_entity, multi_mesh_instances);
			multi_mesh_instances.append_array(RenderUtility2D::create_multi_mesh_instances(world,transforms,multi_mesh_instance_2d_entity));
			result.append(multi_mesh_instance_2d_entity);
			return result;
		}
		Camera2D *camera_2d = Object::cast_to<Camera2D>(node);
		if ( camera_2d != nullptr) {
			const flecs::entity node_camera = RenderUtility2D::create_camera_2d(world, camera_2d);
			get_script(world, node, node_camera, result);
			result.append(node_camera);
			return result;
		}
		DirectionalLight2D* directional_light_2d = Object::cast_to<DirectionalLight2D>(node);
		if ( directional_light_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_directional_light(world, directional_light_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		PointLight2D* point_light_2d = Object::cast_to<PointLight2D>(node);
		if ( point_light_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_point_light(world, point_light_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		Skeleton2D* skeleton_2d = Object::cast_to<Skeleton2D>(node);
		if ( skeleton_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_skeleton(world, skeleton_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		LightOccluder2D * light_occluder = Object::cast_to<LightOccluder2D>(node);
		if ( light_occluder != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_light_occluder(world, light_occluder);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		GPUParticles2D* gpu_particles_2d = Object::cast_to<GPUParticles2D>(node);
		if (  gpu_particles_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_gpu_particles_2d(world, gpu_particles_2d);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		//handle this as last as this is a generic visual 2d node cast
		CanvasItem* canvas_item = Object::cast_to<CanvasItem>(node);
		if ( canvas_item != nullptr) {
			const flecs::entity entity = RenderUtility2D::create_canvas_item(world, canvas_item);
			get_script(world, node, entity, result);
			result.append(entity);
			return result;
		}
		const String name = node->get_name();
		const flecs::entity entity = world.entity().set<SceneNodeComponent<Node>>({
			node->get_instance_id(), node->get_class(), NodeRef<Node>(node->get_instance_id())
			})
			.set_name(name.ascii().get_data());
		get_script(world, node, entity, result);
		result.append(entity);
		return result;
	}
	
	static inline flecs::entity get_script(const flecs::world &world, const Node* node, const flecs::entity node_entity, Vector<flecs::entity> &entities) {
		const Variant variant = node->get_script();
		const Ref<Script> script = Ref<Script>(VariantCaster<Script*>::cast(variant));
		if ( script.is_valid()) {
			const flecs::entity child_resource_entity = ResourceObjectUtility::CreateResourceEntity(world, script);
			child_resource_entity.add(flecs::ChildOf, node_entity);
			entities.append(child_resource_entity);
			return child_resource_entity;
		}
		ERR_PRINT("No script found. returning empty flecs component");
		return flecs::entity();
	}
};

