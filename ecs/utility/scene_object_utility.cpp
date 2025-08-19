#include "scene_object_utility.h"

#include "core/templates/rid.h"
#include "flecs_server.h"
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

TypedArray<RID> SceneObjectUtility::create_entities_from_scene(const RID &world_id, SceneTree *tree){
    if (tree == nullptr) {
        ERR_FAIL_V(TypedArray<RID>());
    }
    TypedArray<RID> entities;
    auto children = tree->get_root()->get_children();
    for (Variant &variant : children) {
        const Node *node = Object::cast_to<Node>(variant);
        if (node == nullptr) {
            continue;
        }
        entities.append_array(create_entities(world_id, node, entities));
    }
    return entities;
}

TypedArray<RID> SceneObjectUtility::create_entities(const RID &world_id, const Node *base_node, const TypedArray<RID> &entities, int current_depth, const int max_depth)  {
current_depth++;
if (base_node == nullptr) {
    ERR_FAIL_COND_V(base_node == nullptr,entities);
}
if (current_depth > max_depth) {
    ERR_FAIL_COND_V(current_depth > max_depth,entities);
}
TypedArray<RID> result_entities = TypedArray<RID>(entities);
//get the children of Node
auto children = base_node->get_children();
for (Variant &variant : children ) {
    // for each node check to see if it is an object
    Node *child_node = Object::cast_to<Node>(variant);
    if (child_node == nullptr) {
        continue;
    }
    //gather resulting entities from entity creation
    TypedArray<RID> child_entity_result = create_entity(world_id, child_node);

    //check to see if the node has children
    TypedArray<Node> node_children = child_node->get_children();
    if (node_children.size() > 0) {
        //recursively check each child
        //repeat until we hit our search limit
        result_entities.append_array(create_entities(world_id, child_node, result_entities, current_depth, max_depth));
    }
}

//return resulting entities
return result_entities;
}

TypedArray<RID> SceneObjectUtility::create_entity(const RID &world_id, Node *node)  {
    TypedArray<RID> result;
    if (node == nullptr) {
        ERR_FAIL_COND_V(node == nullptr,result);
    }

    // 3D navigation
    NavigationAgent3D *navigation_agent_3d = Object::cast_to<NavigationAgent3D>(node);
    if (navigation_agent_3d != nullptr) {
        const RID entity = Navigation3DUtility::create_nav_agent_with_object(world_id, navigation_agent_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationLink3D *navigation_link_3d = Object::cast_to<NavigationLink3D>(node);
    if ( navigation_link_3d != nullptr) {
        const RID entity = Navigation3DUtility::create_nav_link_with_object(world_id, navigation_link_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationObstacle3D *navigation_obstacle_3d = Object::cast_to<NavigationObstacle3D>(node);
    if ( navigation_obstacle_3d != nullptr) {
        const RID entity = Navigation3DUtility::create_nav_obstacle_with_object(world_id, navigation_obstacle_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationRegion3D *navigation_region_3d = Object::cast_to<NavigationRegion3D>(node);
    if (navigation_region_3d != nullptr) {
        const RID entity = Navigation3DUtility::create_nav_region_with_object(world_id, navigation_region_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationAgent2D *navigation_agent_2d = Object::cast_to<NavigationAgent2D>(node);
    // 2D navigation
    if ( navigation_agent_2d != nullptr) {
        const RID entity = Navigation2DUtility::create_nav_agent_with_object(world_id, navigation_agent_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationLink2D *navigation_link_2d = Object::cast_to<NavigationLink2D>(node);
    if ( navigation_link_2d != nullptr) {
        const RID entity = Navigation2DUtility::create_nav_link_with_object(world_id, navigation_link_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationObstacle2D *navigation_obstacle_2d = Object::cast_to<NavigationObstacle2D>(node);
    if ( navigation_obstacle_2d != nullptr) {
        const RID entity = Navigation2DUtility::create_nav_obstacle_with_object(world_id, navigation_obstacle_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    NavigationRegion2D *navigation_region_2d = Object::cast_to<NavigationRegion2D>(node);
    if ( navigation_region_2d != nullptr) {
        const RID entity = Navigation2DUtility::create_nav_region_with_object(world_id, navigation_region_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }

    //3D Physics
    Area3D* area_3d = Object::cast_to<Area3D>(node);
    if ( area_3d != nullptr) {
        const RID entity = Physics3DUtility::create_area_with_object(world_id, area_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    RigidBody3D* rigid_body_3d = Object::cast_to<RigidBody3D>(node);
    if ( rigid_body_3d != nullptr) {
        const RID entity = Physics3DUtility::create_rigid_body_with_object(world_id, rigid_body_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    PhysicsBody3D *physics_body_3d = Object::cast_to<PhysicsBody3D>(node);
    if ( physics_body_3d != nullptr) {
        const RID entity = Physics3DUtility::create_physics_body_with_object(world_id, physics_body_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Joint3D* joint_3d = Object::cast_to<Joint3D>(node);
    if (joint_3d) {
        const RID entity = Physics3DUtility::create_joint_with_object(world_id, joint_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    SoftBody3D* soft_body_3d = Object::cast_to<SoftBody3D>(node);
    if ( soft_body_3d != nullptr) {
        const RID entity = Physics3DUtility::create_soft_body_with_object(world_id, soft_body_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }

    //2D Physics
    Area2D* area_2d = Object::cast_to<Area2D>(node);
    if ( area_2d != nullptr) {
        const RID entity = Physics2DUtility::create_area_with_object(world_id, area_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    RigidBody2D* rigid_body_2d = Object::cast_to<RigidBody2D>(node);
    if ( rigid_body_2d != nullptr) {
        const RID entity = Physics2DUtility::create_rigid_body_with_object(world_id, rigid_body_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    PhysicsBody2D *physics_body_2d = Object::cast_to<PhysicsBody2D>(node);
    if ( physics_body_2d != nullptr) {
        const RID entity = Physics2DUtility::create_physics_body_with_object(world_id, physics_body_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Joint2D *joint_2d = Object::cast_to<Joint2D>(node);
    if ( joint_2d != nullptr) {
        const RID entity = Physics2DUtility::create_joint_with_object(world_id, joint_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }

    //3D nodes
    MeshInstance3D *mesh_instance = Object::cast_to<MeshInstance3D>(node);
    if ( mesh_instance != nullptr) {
        const RID entity = RenderUtility3D::create_mesh_instance_with_object(world_id, mesh_instance);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    MultiMeshInstance3D *multi_mesh_instance_3d = Object::cast_to<MultiMeshInstance3D>(node);
    if ( multi_mesh_instance_3d != nullptr) {
        TypedArray<RID> multi_mesh_instance_3d_entities = RenderUtility3D::create_multi_mesh_with_object(world_id, multi_mesh_instance_3d);
        if(multi_mesh_instance_3d_entities.size() == 0){
            ERR_FAIL_COND_V(multi_mesh_instance_3d_entities.size() == 0 ,result);
        RID multi_mesh_instance_3d_entity = multi_mesh_instance_3d_entities[0];
        const uint32_t instance_count = FlecsServer::get_singleton()->_get_entity(multi_mesh_instance_3d_entity,world_id).get<MultiMeshComponent>().instance_count;
        TypedArray<RID> multi_mesh_instances;
        for(uint32_t i = 0; i < instance_count; i++) {
            if(i == 0){
                //skip the first entity as it is the MultiMeshInstance2D itself
                continue;
            }
            multi_mesh_instances.append(multi_mesh_instance_3d_entities[i]);
        }
        result.append_array(multi_mesh_instance_3d_entities);
        result.append(get_node_script(world_id, node, multi_mesh_instance_3d_entity));
        return result;
    }
    GPUParticles3D *particles_3d = Object::cast_to<GPUParticles3D>(node);
    if ( particles_3d != nullptr) {
        const RID entity = RenderUtility3D::create_particles_with_object(world_id, particles_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    ReflectionProbe* reflection_probe = Object::cast_to<ReflectionProbe>(node);
    if ( reflection_probe != nullptr) {
        const RID entity = RenderUtility3D::create_reflection_probe_with_object(world_id, reflection_probe);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Skeleton3D* skeleton_3d = Object::cast_to<Skeleton3D>(node);
    if ( skeleton_3d != nullptr) {
        const RID entity = RenderUtility3D::create_skeleton_with_object(world_id, skeleton_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    WorldEnvironment* world_id_environment = Object::cast_to<WorldEnvironment>(node);
    if ( world_id_environment != nullptr) {
        const RID entity = RenderUtility3D::create_environment_with_object(world_id, world_id_environment);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Camera3D* camera_3d = Object::cast_to<Camera3D>(node);
    if ( camera_3d != nullptr) {
        const RID entity =  RenderUtility3D::create_camera_with_object(world_id, camera_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    DirectionalLight3D* directional_light_3d = Object::cast_to<DirectionalLight3D>(node);
    if ( directional_light_3d != nullptr) {
        const RID entity = RenderUtility3D::create_directional_light_with_object(world_id, directional_light_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    OmniLight3D* omni_light_3d = Object::cast_to<OmniLight3D>(node);
    if ( omni_light_3d != nullptr) {
        const RID entity = RenderUtility3D::create_omni_light_with_object(world_id, omni_light_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    SpotLight3D* spot_light_3d = Object::cast_to<SpotLight3D>(node);
    if ( spot_light_3d != nullptr) {
        const RID entity = RenderUtility3D::create_spot_light_with_object(world_id, spot_light_3d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Viewport* viewport = Object::cast_to<Viewport>(node);
    if ( viewport != nullptr) {
        const RID entity = RenderUtility3D::create_viewport_with_object(world_id, viewport);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    VoxelGI* voxel_gi = Object::cast_to<VoxelGI>(node);
    if ( voxel_gi != nullptr) {
        const RID entity = RenderUtility3D::create_voxel_gi_with_object(world_id, voxel_gi);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }

    OccluderInstance3D* occluder_component = Object::cast_to<OccluderInstance3D>(node);
    if (occluder_component != nullptr) {
        const RID entity = RenderUtility3D::create_occluder_with_object(world_id, occluder_component);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }

    // 2D nodes
    MeshInstance2D *mesh_instance_2d = Object::cast_to<MeshInstance2D>(node);
    if ( mesh_instance_2d != nullptr) {
        const RID entity = RenderUtility2D::create_mesh_instance_with_object(world_id, mesh_instance_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    MultiMeshInstance2D *mmi = Object::cast_to<MultiMeshInstance2D>(node);
    if ( mmi != nullptr) {
        TypedArray<RID> multi_mesh_instance_2d_entities = RenderUtility2D::create_multi_mesh_with_object(world_id, mmi);
        RID multi_mesh_instance_2d_entity = multi_mesh_instance_2d_entities[0];
        const flecs::entity mmi_entity = FlecsServer::get_singleton()->_get_entity(multi_mesh_instance_2d_entity, world_id);
        const uint32_t instance_count = mmi_entity.get<MultiMeshComponent>().instance_count;
        Vector<Transform2D> transforms;
        transforms.resize(instance_count);
        uint32_t count = instance_count;
        TypedArray<RID> multi_mesh_instances;
        for(uint32_t i = 0; i < count; i++) {
            if(i == 0){
                //skip the first entity as it is the MultiMeshInstance2D itself
                continue;
            }
            multi_mesh_instances.append(multi_mesh_instance_2d_entities[i]);
        }
        
        get_node_script(world_id, node, multi_mesh_instance_2d_entity);
        result.append_array(multi_mesh_instance_2d_entities);
        result.append(get_node_script(world_id, node, multi_mesh_instance_2d_entity));
        return result;
    }
    Camera2D *camera_2d = Object::cast_to<Camera2D>(node);
    if ( camera_2d != nullptr) {
        const RID node_camera = RenderUtility2D::create_camera_with_object(world_id, camera_2d);
        result.append(node_camera);
        result.append(get_node_script(world_id, node, node_camera));
        return result;
    }
    DirectionalLight2D* directional_light_2d = Object::cast_to<DirectionalLight2D>(node);
    if ( directional_light_2d != nullptr) {
        const RID entity = RenderUtility2D::create_directional_light_with_object(world_id, directional_light_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    PointLight2D* point_light_2d = Object::cast_to<PointLight2D>(node);
    if ( point_light_2d != nullptr) {
        const RID entity = RenderUtility2D::create_point_light_with_object(world_id, point_light_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    Skeleton2D* skeleton_2d = Object::cast_to<Skeleton2D>(node);
    if ( skeleton_2d != nullptr) {
        const RID entity = RenderUtility2D::create_skeleton_with_object(world_id, skeleton_2d);
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    LightOccluder2D * light_occluder = Object::cast_to<LightOccluder2D>(node);
    if ( light_occluder != nullptr) {
        const RID entity = RenderUtility2D::create_light_occluder_with_object(world_id, light_occluder);
        
        result.append(entity);
        result.append(get_node_script(world_id, node, entity));
        return result;
    }
    GPUParticles2D* gpu_particles_2d = Object::cast_to<GPUParticles2D>(node);
    if (  gpu_particles_2d != nullptr) {
    const RID entity = RenderUtility2D::create_gpu_particles_with_object(world_id, gpu_particles_2d);
        get_node_script(world_id, node, entity);
        result.append(entity);
        return result;
    }
    //handle this as last as this is a generic visual 2d node cast
    CanvasItem* canvas_item = Object::cast_to<CanvasItem>(node);
    if ( canvas_item != nullptr) {
        const RID entity = RenderUtility2D::create_canvas_item_with_object(world_id, canvas_item);
        get_node_script(world_id, node, entity );
        result.append(entity);
        return result;
    }
    flecs::world *flecs_world = FlecsServer::get_singleton()->_get_world(world_id);
    String name = node->get_name();
    name = name + "_" + itos(Math::rand());
    flecs::entity e = flecs_world->entity();
    e.set_name(name.ascii().get_data());
    SceneNodeComponent scene_node_component;
    scene_node_component.node_id = node->get_instance_id();
    scene_node_component.class_name = node->get_class();
    e.set<SceneNodeComponent>(scene_node_component);
    const RID entity = FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
    // Note: SceneNodeComponentRef::create_component expects a Ref<FlecsEntity>, handled by caller if needed

    RID script_entity = get_node_script(world_id, node, entity);
    if(script_entity.is_valid()){
        result.append(script_entity);
    }
    result.append(entity);
    
    }
    return result;
}

RID SceneObjectUtility::get_node_script(const RID &world_id, const Node *node, const RID& entity_id) {
    const Variant variant = node->get_script();
    const Ref<Script> node_script = Ref<Script>(VariantCaster<Script*>::cast(variant));
    if ( node_script.is_valid()) {
        const RID child_resource_entity = ResourceObjectUtility::create_resource_entity(world_id, node_script);
        flecs::entity child_resource_flecs_entity = FlecsServer::get_singleton()->_get_entity(child_resource_entity, world_id);
        flecs::entity flecs_entity = FlecsServer::get_singleton()->_get_entity(entity_id, world_id);
        child_resource_flecs_entity.add(flecs::ChildOf, flecs_entity);
        return child_resource_entity;
    }
    WARN_PRINT("No script found. returning empty flecs component");
    return RID();
}

void SceneObjectUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), "get_singleton", &SceneObjectUtility::get_singleton);
    ClassDB::bind_method(D_METHOD("create_entities_from_scene", "world_id", "tree"), &SceneObjectUtility::create_entities_from_scene);
    ClassDB::bind_method(D_METHOD("create_entities", "world_id", "base_node", "entities", "current_depth", "max_depth"), &SceneObjectUtility::create_entities, DEFVAL(0), DEFVAL(10000));
    ClassDB::bind_method(D_METHOD("create_entity", "world_id", "node"), &SceneObjectUtility::create_entity);
    ClassDB::bind_method(D_METHOD("get_node_script", "world_id", "node", "node_entity"), &SceneObjectUtility::get_node_script);
    
}

SceneObjectUtility* SceneObjectUtility::get_singleton() {
    if (instance == nullptr) {
        instance = memnew(SceneObjectUtility);
    }
    return instance;
}