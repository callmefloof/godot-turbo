#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../components/scene_node_component.h"
#include "../components/transform_2d_component.h"
#include "../components/transform_3d_component.h"
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "navigation/2d/navigation2d_utility.h"
#include "navigation/3d/navigation3d_utility.h"
#include "physics/2d/physics2d_utility.h"
#include "physics/3d/physics3d_utility.h"
#include "rendering/render_utility_2d.h"
#include "rendering/render_utility_3d.h"
#include "resource_object_utility.h"
#include "scene/3d/node_3d.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include <scene/2d/mesh_instance_2d.h>
#include <cassert>

class SceneObjectUtility {
private:
	SceneObjectUtility() = default; // Prevent instantiation
	SceneObjectUtility(const SceneObjectUtility &) = delete; // Prevent copy
	SceneObjectUtility &operator=(const SceneObjectUtility &) = delete; // Prevent assignment
	SceneObjectUtility(SceneObjectUtility &&) = delete; // Prevent move
	SceneObjectUtility &operator=(SceneObjectUtility &&) = delete; // Prevent move assignment
public:
	static Vector<flecs::entity> CreateEntitiesFromScene(const flecs::world& world, const SceneTree* tree )
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
			entities.append_array(CreateEntities(world, node, entities));
		}
	}

	static Vector<flecs::entity> CreateEntities(const flecs::world &world, const Node *base_node, Vector<flecs::entity> &entities,
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
			Vector<flecs::entity> child_entity_result = CreateEntity(world, child_node);

			//check to see if the node has children
			if (TypedArray<Node> node_children = child_node->get_children(); node_children.size() > 0) {
				//recursively check each child
				Vector<flecs::entity> child_entities = CreateEntities(world, child_node, entities, current_depth, max_depth);
				//repeat until wie hit our search limit
				entities.append_array(entities);
			}
		}
		//return resulting entities
		return entities;
	}

	static Vector<flecs::entity> CreateEntity(const flecs::world &world, Node* node) {
		Vector<flecs::entity> result;
		if (node == nullptr) {
			ERR_FAIL_COND_V(node == nullptr,result);
		}

		// 3D navigation
		if (NavigationAgent3D *navigation_agent_3d = Object::cast_to<NavigationAgent3D>(node); navigation_agent_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::CreateNavAgent(world, navigation_agent_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationLink3D *navigation_link_3d = Object::cast_to<NavigationLink3D>(node); navigation_link_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::CreateNavLink(world, navigation_link_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationObstacle3D *navigation_obstacle_3d = Object::cast_to<NavigationObstacle3D>(node); navigation_obstacle_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::CreateNavObstacle(world, navigation_obstacle_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationRegion3D *navigation_region_3d = Object::cast_to<NavigationRegion3D>(node); navigation_region_3d != nullptr) {
			const flecs::entity entity = Navigation3DUtility::CreateNavRegion(world, navigation_region_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}

		// 2D navigation
		if (NavigationAgent2D *navigation_agent_2d = Object::cast_to<NavigationAgent2D>(node); navigation_agent_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::CreateNav2DAgent(world, navigation_agent_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationLink2D *navigation_link_2d = Object::cast_to<NavigationLink2D>(node); navigation_link_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::CreateNav2DLink(world, navigation_link_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationObstacle2D *navigation_obstacle_2d = Object::cast_to<NavigationObstacle2D>(node); navigation_obstacle_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::CreateNav2DObstacle(world, navigation_obstacle_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (NavigationRegion2D *navigation_region_2d = Object::cast_to<NavigationRegion2D>(node); navigation_region_2d != nullptr) {
			const flecs::entity entity = Navigation2DUtility::CreateNav2DRegion(world, navigation_region_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}

		//3D Physics
		if (Area3D* area_3d = Object::cast_to<Area3D>(node); area_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::CreateArea(world, area_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (RigidBody3D* rigid_body_3d = Object::cast_to<RigidBody3D>(node); rigid_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::CreateRigidBody(world, rigid_body_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (PhysicsBody3D *physics_body_3d = Object::cast_to<PhysicsBody3D>(node); physics_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::CreatePhysicsBody(world, physics_body_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Joint3D* joint_3d = Object::cast_to<Joint3D>(node); joint_3d) {
			const flecs::entity entity = Physics3DUtility::CreateJoint(world, joint_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (SoftBody3D* soft_body_3d = Object::cast_to<SoftBody3D>(node); soft_body_3d != nullptr) {
			const flecs::entity entity = Physics3DUtility::CreateSoftBody(world, soft_body_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}

		//2D Physics
		if (Area2D* area_2d = Object::cast_to<Area2D>(node); area_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::CreateArea(world, area_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (RigidBody2D* rigid_body_2d = Object::cast_to<RigidBody2D>(node); rigid_body_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::CreateRigidBody(world, rigid_body_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (PhysicsBody2D *physics_body_2d = Object::cast_to<PhysicsBody2D>(node); physics_body_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::CreatePhysicsBody(world, physics_body_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Joint2D *joint_2d = Object::cast_to<Joint2D>(node); joint_2d != nullptr) {
			const flecs::entity entity = Physics2DUtility::CreateJoint(world, joint_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}

		//3D nodes
		if (MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(node); mesh_instance != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateMeshInstance(world, mesh_instance);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (MultiMeshInstance3D *multi_mesh_instance_3d = Object::cast_to<MultiMeshInstance3D>(node); multi_mesh_instance_3d != nullptr) {
			const flecs::entity multi_mesh_instance_3d_entity = RenderUtility3D::CreateMultiMesh(world, multi_mesh_instance_3d);
			const uint32_t instance_count =  multi_mesh_instance_3d_entity.get<MultiMeshComponent>().instance_count;
			Vector<Transform3D> transforms;
			transforms.resize(instance_count);
			Vector<flecs::entity> multi_mesh_instances;
			multi_mesh_instances.append(multi_mesh_instance_3d_entity);
			multi_mesh_instances.append_array(RenderUtility3D::CreateMultiMeshInstances(world,transforms,multi_mesh_instance_3d_entity));
			GetScript(world, node, multi_mesh_instance_3d_entity, multi_mesh_instances);
			result.append_array(multi_mesh_instances);

		}
		if (GPUParticles3D *particles_3d = Object::cast_to<GPUParticles3D>(node); particles_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateParticles(world, particles_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (ReflectionProbe* reflection_probe = Object::cast_to<ReflectionProbe>(node); reflection_probe != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateReflectionProbe(world, reflection_probe);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Skeleton3D* skeleton_3d = Object::cast_to<Skeleton3D>(node); skeleton_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateSkeleton(world, skeleton_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (WorldEnvironment* world_environment = Object::cast_to<WorldEnvironment>(node); world_environment != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateEnvironment(world, world_environment);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Camera3D* camera_3d = Object::cast_to<Camera3D>(node); camera_3d != nullptr) {
			const flecs::entity entity =  RenderUtility3D::CreateCamera(world, camera_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (DirectionalLight3D* directional_light_3d = Object::cast_to<DirectionalLight3D>(node); directional_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateDirectionalLight(world, directional_light_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (OmniLight3D* omni_light_3d = Object::cast_to<OmniLight3D>(node); omni_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateOmniLight(world, omni_light_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (SpotLight3D* spot_light_3d = Object::cast_to<SpotLight3D>(node); spot_light_3d != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateSpotLight(world, spot_light_3d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Viewport* viewport = Object::cast_to<Viewport>(node); viewport != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateViewport(world, viewport);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (VoxelGI* voxel_gi = Object::cast_to<VoxelGI>(node); voxel_gi != nullptr) {
			const flecs::entity entity = RenderUtility3D::CreateVoxelGI(world, voxel_gi);
			GetScript(world, node, entity, result);
			result.append(entity);
		}

		// 2D nodes
		if (MeshInstance2D *mesh_instance_2d = Object::cast_to<MeshInstance2D>(node); mesh_instance_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateMeshInstance(world, mesh_instance_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (MultiMeshInstance2D *mmi = Object::cast_to<MultiMeshInstance2D>(node); mmi != nullptr) {const flecs::entity mmi_entity = RenderUtility2D::CreateMultiMesh(world, mmi);
			const flecs::entity multi_mesh_instance_2d_entity = RenderUtility2D::CreateMultiMesh(world, mmi);
			const uint32_t instance_count = mmi_entity.get<MultiMeshComponent>().instance_count;
			Vector<Transform2D> transforms;
			transforms.resize(instance_count);
			Vector<flecs::entity> multi_mesh_instances;
			GetScript(world, node, multi_mesh_instance_2d_entity, multi_mesh_instances);
			multi_mesh_instances.append_array(RenderUtility2D::CreateMultiMeshInstances(world,transforms,multi_mesh_instance_2d_entity));
			result.append(multi_mesh_instance_2d_entity);
		}
		if (Camera2D *camera_2d = Object::cast_to<Camera2D>(node); camera_2d != nullptr) {
			const flecs::entity node_camera = RenderUtility2D::CreateCamera2D(world, camera_2d);
			GetScript(world, node, node_camera, result);
			result.append(node_camera);
		}
		if (DirectionalLight2D* directional_light_2d = Object::cast_to<DirectionalLight2D>(node); directional_light_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateDirectionalLight(world, directional_light_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (PointLight2D* point_light_2d = Object::cast_to<PointLight2D>(node); point_light_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreatePointLight(world, point_light_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (Skeleton2D* skeleton_2d = Object::cast_to<Skeleton2D>(node); skeleton_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateSkeleton(world, skeleton_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (LightOccluder2D * light_occluder = Object::cast_to<LightOccluder2D>(node); light_occluder != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateLightOccluder(world, light_occluder);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		if (GPUParticles2D* gpu_particles_2d = Object::cast_to<GPUParticles2D>(node);  gpu_particles_2d != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateGPUParticles2D(world, gpu_particles_2d);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		//handle this as last as this is a generic visual 2d node cast
		if (CanvasItem* canvas_item = Object::cast_to<CanvasItem>(node); canvas_item != nullptr) {
			const flecs::entity entity = RenderUtility2D::CreateCanvasItem(world, canvas_item);
			GetScript(world, node, entity, result);
			result.append(entity);
		}
		const String name = node->get_name();
		// Handle other node types as needed
		const Node2D *node_2d = Object::cast_to<Node2D>(node);
		const Node3D *node_3d = Object::cast_to<Node3D>(node);
		if (node_2d != nullptr) {
			const Transform2D transform = node_2d->get_transform();
			flecs::entity node_3d_entity = world.entity().set<Transform2DComponent>({ transform });
			GetScript(world, node, node_3d_entity, result);
			result.append(node_3d_entity);

		} else if (node_3d != nullptr) {
			const Transform3D transform = node_3d->get_transform();
			const flecs::entity node_3d_entity = world.entity().set<Transform3DComponent>({ transform });
			GetScript(world, node, node_3d_entity, result);
			result.append(node_3d_entity);
		}
		else {
			const flecs::entity entity = world.entity().set<SceneNodeComponent<Node>>({
				node->get_instance_id(), node->get_class(), NodeRef<Node>(node->get_instance_id())
				})
				.set_name(name.ascii().get_data());
			GetScript(world, node, entity, result);
			result.append(entity);

		}


		return result;

	}
	static inline flecs::entity GetScript(const flecs::world &world, const Node* node, const flecs::entity node_entity, Vector<flecs::entity> &entities) {
		const Variant variant = node->get_script();
		if (const Ref<Script> script = variant; script.is_valid()) {
			const flecs::entity child_resource_entity = ResourceObjectUtility::CreateResourceEntity(world, script);
			child_resource_entity.add(flecs::ChildOf, node_entity);
			entities.append(child_resource_entity);
		}
	}
};

