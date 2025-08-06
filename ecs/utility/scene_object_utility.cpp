#include "scene_object_utility.h"

#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/scene_node_component.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/transform_3d_component.h"
#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "navigation/2d/navigation2d_utility.h"
#include "navigation/3d/navigation3d_utility.h"
#include "physics/2d/physics2d_utility.h"
#include "physics/3d/physics3d_utility.h"
#include "rendering/2d/render_utility_2d.h"
#include "rendering/3d/render_utility_3d.h"
#include "resource_object_utility.h"
#include "scene/3d/node_3d.h"
#include "scene/main/scene_tree.h"
#include "scene/main/window.h"
#include "scene/2d/mesh_instance_2d.h"
#include "scene/3d/navigation_agent_3d.h"
#include "scene/3d/navigation_link_3d.h"
#include "scene/3d/navigation_obstacle_3d.h"
#include "scene/3d/navigation_region_3d.h"
#include "scene/2d/navigation_agent_2d.h"
#include "scene/2d/navigation_link_2d.h"
#include "scene/2d/navigation_obstacle_2d.h"
#include "scene/2d/navigation_region_2d.h"
#include "scene/2d/physics/joints/joint_2d.h"
#include "scene/2d/physics/area_2d.h"
#include "scene/2d/physics/physics_body_2d.h"
#include "scene/2d/physics/rigid_body_2d.h"
#include "scene/2d/physics/static_body_2d.h"
#include "scene/3d/physics/joints/joint_3d.h"
#include "scene/3d/physics/rigid_body_3d.h"
#include "scene/3d/physics/static_body_3d.h"
#include "scene/3d/physics/area_3d.h"
#include "scene/3d/soft_body_3d.h"
#include "scene/3d/physics/physics_body_3d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/3d/multimesh_instance_3d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/2d/light_2d.h"
#include "scene/2d/light_occluder_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/node_2d.h"
#include "core/variant/binder_common.h"
#include "core/object/script_instance.h"
#include "scene/3d/gpu_particles_3d.h"
#include "scene/3d/reflection_probe.h"
#include "scene/3d/skeleton_3d.h"
#include "scene/3d/world_environment.h"
#include "scene/3d/light_3d.h"
#include "scene/3d/voxel_gi.h"
#include "scene/3d/occluder_instance_3d.h"

Array SceneObjectUtility::create_entities_from_scene(FlecsWorld *world, const SceneTree *tree){
    if (tree == nullptr) {
        ERR_FAIL_V(TypedArray<FlecsEntity>());
    }
    TypedArray<FlecsEntity> entities;
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

Array SceneObjectUtility::create_entities(FlecsWorld *world, const Node *base_node, Array &entities, int current_depth, const int max_depth)  {
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
    TypedArray<FlecsEntity> child_entity_result = create_entity(world, child_node);

    //check to see if the node has children
    TypedArray<Node> node_children = child_node->get_children();
    if (node_children.size() > 0) {
        //recursively check each child
        TypedArray<FlecsEntity> child_entities = create_entities(world, child_node, entities, current_depth, max_depth);
        //repeat until wie hit our search limit
        entities.append_array(entities);
    }
}
//return resulting entities
return entities;
}

Array SceneObjectUtility::create_entity(FlecsWorld *world, Node *node)  {
    TypedArray<FlecsEntity> result;
    if (node == nullptr) {
        ERR_FAIL_COND_V(node == nullptr,result);
    }

    // 3D navigation
    NavigationAgent3D *navigation_agent_3d = Object::cast_to<NavigationAgent3D>(node);
    if (navigation_agent_3d != nullptr) {
        const FlecsEntity* entity = Navigation3DUtility::create_nav_agent_with_object(world, navigation_agent_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationLink3D *navigation_link_3d = Object::cast_to<NavigationLink3D>(node);
    if ( navigation_link_3d != nullptr) {
        const FlecsEntity* entity = Navigation3DUtility::create_nav_link_with_object(world, navigation_link_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationObstacle3D *navigation_obstacle_3d = Object::cast_to<NavigationObstacle3D>(node);
    if ( navigation_obstacle_3d != nullptr) {
        const FlecsEntity* entity = Navigation3DUtility::create_nav_obstacle_with_object(world, navigation_obstacle_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationRegion3D *navigation_region_3d = Object::cast_to<NavigationRegion3D>(node);
    if (navigation_region_3d != nullptr) {
        const FlecsEntity* entity = Navigation3DUtility::create_nav_region_with_object(world, navigation_region_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationAgent2D *navigation_agent_2d = Object::cast_to<NavigationAgent2D>(node);
    // 2D navigation
    if ( navigation_agent_2d != nullptr) {
        const FlecsEntity* entity = Navigation2DUtility::create_nav_agent_with_object(world, navigation_agent_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationLink2D *navigation_link_2d = Object::cast_to<NavigationLink2D>(node);
    if ( navigation_link_2d != nullptr) {
        const FlecsEntity* entity = Navigation2DUtility::create_nav_link_with_object(world, navigation_link_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationObstacle2D *navigation_obstacle_2d = Object::cast_to<NavigationObstacle2D>(node);
    if ( navigation_obstacle_2d != nullptr) {
        const FlecsEntity* entity = Navigation2DUtility::create_nav_obstacle_with_object(world, navigation_obstacle_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    NavigationRegion2D *navigation_region_2d = Object::cast_to<NavigationRegion2D>(node);
    if ( navigation_region_2d != nullptr) {
        const FlecsEntity* entity = Navigation2DUtility::create_nav_region_with_object(world, navigation_region_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }

    //3D Physics
    Area3D* area_3d = Object::cast_to<Area3D>(node);
    if ( area_3d != nullptr) {
        const FlecsEntity* entity = Physics3DUtility::create_area_with_object(world, area_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    RigidBody3D* rigid_body_3d = Object::cast_to<RigidBody3D>(node);
    if ( rigid_body_3d != nullptr) {
        const FlecsEntity* entity = Physics3DUtility::create_rigid_body_with_object(world, rigid_body_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    PhysicsBody3D *physics_body_3d = Object::cast_to<PhysicsBody3D>(node);
    if ( physics_body_3d != nullptr) {
        const FlecsEntity* entity = Physics3DUtility::create_physics_body_with_object(world, physics_body_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Joint3D* joint_3d = Object::cast_to<Joint3D>(node);
    if (joint_3d) {
        const FlecsEntity* entity = Physics3DUtility::create_joint_with_object(world, joint_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    SoftBody3D* soft_body_3d = Object::cast_to<SoftBody3D>(node);
    if ( soft_body_3d != nullptr) {
        const FlecsEntity* entity = Physics3DUtility::create_soft_body_with_object(world, soft_body_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }

    //2D Physics
    Area2D* area_2d = Object::cast_to<Area2D>(node);
    if ( area_2d != nullptr) {
        const FlecsEntity* entity = Physics2DUtility::create_area_with_object(world, area_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    RigidBody2D* rigid_body_2d = Object::cast_to<RigidBody2D>(node);
    if ( rigid_body_2d != nullptr) {
        const FlecsEntity* entity = Physics2DUtility::create_rigid_body_with_object(world, rigid_body_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    PhysicsBody2D *physics_body_2d = Object::cast_to<PhysicsBody2D>(node);
    if ( physics_body_2d != nullptr) {
        const FlecsEntity* entity = Physics2DUtility::create_physics_body_with_object(world, physics_body_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Joint2D *joint_2d = Object::cast_to<Joint2D>(node);
    if ( joint_2d != nullptr) {
        const FlecsEntity* entity = Physics2DUtility::create_joint_with_object(world, joint_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }

    //3D nodes
    MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(node);
    if ( mesh_instance != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_mesh_instance_with_object(world, mesh_instance).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    MultiMeshInstance3D *multi_mesh_instance_3d = Object::cast_to<MultiMeshInstance3D>(node);
    if ( multi_mesh_instance_3d != nullptr) {
        TypedArray<FlecsEntity> multi_mesh_instance_3d_entities = RenderUtility3D::create_multi_mesh_with_object(world, multi_mesh_instance_3d);
        FlecsEntity* multi_mesh_instance_3d_entity = static_cast<Ref<FlecsEntity>>(multi_mesh_instance_3d_entities[0]).ptr();
        const uint32_t instance_count =  multi_mesh_instance_3d_entity->get_entity().get<MultiMeshComponent>().instance_count;
        TypedArray<FlecsEntity> multi_mesh_instances;
        for(uint32_t i = 0; i < instance_count; i++) {
            if(i == 0){
                //skip the first entity as it is the MultiMeshInstance2D itself
                continue;
            }
            multi_mesh_instances.append(multi_mesh_instance_3d_entities[i]);
        }
        get_script(world, node, multi_mesh_instance_3d_entity, multi_mesh_instances);
        result.append_array(multi_mesh_instance_3d_entities);
        return result;
    }
    GPUParticles3D *particles_3d = Object::cast_to<GPUParticles3D>(node);
    if ( particles_3d != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_particles_with_object(world, particles_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    ReflectionProbe* reflection_probe = Object::cast_to<ReflectionProbe>(node);
    if ( reflection_probe != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_reflection_probe_with_object(world, reflection_probe).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Skeleton3D* skeleton_3d = Object::cast_to<Skeleton3D>(node);
    if ( skeleton_3d != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_skeleton_with_object(world, skeleton_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    WorldEnvironment* world_environment = Object::cast_to<WorldEnvironment>(node);
    if ( world_environment != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_environment_with_object(world, world_environment).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Camera3D* camera_3d = Object::cast_to<Camera3D>(node);
    if ( camera_3d != nullptr) {
        const FlecsEntity* entity =  RenderUtility3D::create_camera_with_object(world, camera_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    DirectionalLight3D* directional_light_3d = Object::cast_to<DirectionalLight3D>(node);
    if ( directional_light_3d != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_directional_light_with_object(world, directional_light_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    OmniLight3D* omni_light_3d = Object::cast_to<OmniLight3D>(node);
    if ( omni_light_3d != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_omni_light_with_object(world, omni_light_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    SpotLight3D* spot_light_3d = Object::cast_to<SpotLight3D>(node);
    if ( spot_light_3d != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_spot_light_with_object(world, spot_light_3d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Viewport* viewport = Object::cast_to<Viewport>(node);
    if ( viewport != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_viewport_with_object(world, viewport).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    VoxelGI* voxel_gi = Object::cast_to<VoxelGI>(node);
    if ( voxel_gi != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_voxel_gi_with_object(world, voxel_gi).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }

    OccluderInstance3D* occluder_component = Object::cast_to<OccluderInstance3D>(node);
    if (occluder_component != nullptr) {
        const FlecsEntity* entity = RenderUtility3D::create_occluder_with_object(world, occluder_component).ptr();
        result.append(entity);
        return result;
    }

    // 2D nodes
    MeshInstance2D *mesh_instance_2d = Object::cast_to<MeshInstance2D>(node);
    if ( mesh_instance_2d != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_mesh_instance_with_object(world, mesh_instance_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    MultiMeshInstance2D *mmi = Object::cast_to<MultiMeshInstance2D>(node);
    if ( mmi != nullptr) {
        TypedArray<FlecsEntity> multi_mesh_instance_2d_entities = RenderUtility2D::create_multi_mesh_with_object(world, mmi);
        Variant variant =  multi_mesh_instance_2d_entities[0];
        Object* obj = variant;
        FlecsEntity* multi_mesh_instance_2d_entity = Object::cast_to<FlecsEntity>(obj);
        const flecs::entity mmi_entity = multi_mesh_instance_2d_entity->get_entity();
        const uint32_t instance_count = mmi_entity.get<MultiMeshComponent>().instance_count;
        Vector<Transform2D> transforms;
        transforms.resize(instance_count);
        uint32_t count = instance_count;
        TypedArray<FlecsEntity> multi_mesh_instances;
        for(uint32_t i = 0; i < count; i++) {
            if(i == 0){
                //skip the first entity as it is the MultiMeshInstance2D itself
                continue;
            }
            multi_mesh_instances.append(multi_mesh_instance_2d_entities[i]);
        }
        get_script(world, node, multi_mesh_instance_2d_entity, multi_mesh_instances);
        result.append_array(multi_mesh_instance_2d_entities);
        return result;
    }
    Camera2D *camera_2d = Object::cast_to<Camera2D>(node);
    if ( camera_2d != nullptr) {
        const FlecsEntity* node_camera = RenderUtility2D::create_camera_2d_with_object(world, camera_2d).ptr();
        get_script(world, node, node_camera, result);
        result.append(node_camera);
        return result;
    }
    DirectionalLight2D* directional_light_2d = Object::cast_to<DirectionalLight2D>(node);
    if ( directional_light_2d != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_directional_light_with_object(world, directional_light_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    PointLight2D* point_light_2d = Object::cast_to<PointLight2D>(node);
    if ( point_light_2d != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_point_light_with_object(world, point_light_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    Skeleton2D* skeleton_2d = Object::cast_to<Skeleton2D>(node);
    if ( skeleton_2d != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_skeleton_with_object(world, skeleton_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    LightOccluder2D * light_occluder = Object::cast_to<LightOccluder2D>(node);
    if ( light_occluder != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_light_occluder_with_object(world, light_occluder).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    GPUParticles2D* gpu_particles_2d = Object::cast_to<GPUParticles2D>(node);
    if (  gpu_particles_2d != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_gpu_particles_2d_with_object(world, gpu_particles_2d).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    //handle this as last as this is a generic visual 2d node cast
    CanvasItem* canvas_item = Object::cast_to<CanvasItem>(node);
    if ( canvas_item != nullptr) {
        const FlecsEntity* entity = RenderUtility2D::create_canvas_item_with_object(world, canvas_item).ptr();
        get_script(world, node, entity, result);
        result.append(entity);
        return result;
    }
    const String name = node->get_name();
    FlecsEntity* entity = world->create_entity().ptr();
    entity->set_name(name);
    entity->set_entity_name(name);
    SceneNodeComponent scene_node_component = {
        node->get_instance_id(), node->get_class(), memnew(NodeRef(node->get_instance_id()))
    };
    entity->get_entity().set<SceneNodeComponent>(scene_node_component);
    SceneNodeComponentRef::create_component(entity);

    get_script(world, node, entity, result);
    result.append(entity);
    return result;
}

Ref<FlecsEntity> SceneObjectUtility::get_script(FlecsWorld *world, const Node *node, const FlecsEntity*node_entity, Array &entities) {
    const Variant variant = node->get_script();
    const Ref<Script> script = Ref<Script>(VariantCaster<Script*>::cast(variant));
    if ( script.is_valid()) {
        const Ref<FlecsEntity> child_resource_entity = ResourceObjectUtility::create_resource_entity(world, script);
        child_resource_entity->get_entity().add(flecs::ChildOf, node_entity->get_entity());
        entities.append(child_resource_entity);
        return child_resource_entity;
    }
    ERR_PRINT("No script found. returning empty flecs component");
    return Ref<FlecsEntity>();
}

void SceneObjectUtility::_bind_methods() {
    // ClassDB::bind_static_method(get_class_static(), "create_entities_from_scene",
    //     &SceneObjectUtility::create_entities_from_scene, "world", "tree");
    // ClassDB::bind_static_method(get_class_static(), "create_entities",
    //     &SceneObjectUtility::create_entities, "world", "base_node", "entities", "current_depth", "max_depth");
    // ClassDB::bind_static_method(get_class_static(), "create_entity",
    //     &SceneObjectUtility::create_entity, "world", "node");
}