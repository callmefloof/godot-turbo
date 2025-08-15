#include "render_utility_2d.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/object_instance_component.h"

flecs::entity RenderUtility2D::_create_mesh_instance(const flecs::world *world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id)  {
    Vector<RID> material_ids;
    const int surface_count = RS::get_singleton()->mesh_get_surface_count(mesh_id);
    for (int i = 0; i < surface_count; ++i) {
        if (const RID material_id = RS::get_singleton()->mesh_surface_get_material(mesh_id, i); material_id.is_valid()) {
            material_ids.push_back(material_id);
        } else {
            material_ids.push_back(RID()); // Use an empty RID if no material is set
        }
    }
    const RID canvas_item = RS::get_singleton()->canvas_item_create();


    const flecs::entity entity = world->entity()
                            .set<MeshComponent>({ mesh_id, material_ids })
                            .set<Transform2DComponent>({ transform }) // Default transform
                            .set<CanvasItemComponent>({ canvas_item, "MeshInstance2D" })
                            .set<VisibilityComponent>({ true }) // Default visibility
                            .set_name(name.ascii().get_data());
    RS::get_singleton()->canvas_item_add_mesh(canvas_item, mesh_id);
    RS::get_singleton()->canvas_item_set_parent(canvas_item, canvas_id);
    RS::get_singleton()->canvas_item_set_material(canvas_item, material_ids[0]);
    return entity;
}

flecs::entity RenderUtility2D::_create_mesh_instance(const flecs::world *world, const Transform2D &transform, const String &name){
    Vector<RID> material_ids;
    const RID mesh_id = RS::get_singleton()->mesh_create();
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), flecs::entity());
    }
    return _create_mesh_instance(world, mesh_id, transform, name, world->get<World2DComponent>().canvas_id);
}

flecs::entity RenderUtility2D::_create_mesh_instance(const flecs::world *world, MeshInstance2D *mesh_instance_2d){
    Vector<RID> material_ids;
    const Ref<Mesh>& mesh = mesh_instance_2d->get_mesh();
    const RID& canvas_item = mesh_instance_2d->get_canvas_item();
    for (int i = 0; i < mesh->get_surface_count(); ++i) {
        if (Ref<Material> material = mesh->surface_get_material(i); material.is_valid()) {
            RefStorage::add(material, material->get_rid());
            material_ids.push_back(material->get_rid());
        } else {
            material_ids.push_back(RID()); // Use an empty RID if no material is set
        }
    }
    if (const Node2D *parent = Object::cast_to<Node2D>(mesh_instance_2d->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
    }

    RefStorage::add(mesh, mesh_instance_2d->get_canvas_item());
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = mesh_instance_2d->get_instance_id();
    NodeStorage::add(mesh_instance_2d, mesh_instance_2d->get_instance_id());
    return world->entity()
            .set<MeshComponent>({ mesh->get_rid(), material_ids })
            .set<CanvasItemComponent>({ canvas_item, mesh_instance_2d->get_class_name() })
            .set<Transform2DComponent>({ mesh_instance_2d->get_transform() })
            .set<VisibilityComponent>({ true })
            .set<ObjectInstanceComponent>(object_instance_component)
            .set_name(String(mesh_instance_2d->get_name()).ascii().get_data());
}

flecs::entity RenderUtility2D::_create_multi_mesh(const flecs::world *world, const Transform2D &transform, const uint32_t size, const Ref<Mesh> mesh, const String &name, const RID &texture_id, const bool use_colors, const bool use_custom_data, const bool use_indirect)  {
    const RID& multi_mesh_id = RS::get_singleton()->multimesh_create();
    const RID& canvas_item = RS::get_singleton()->canvas_item_create();
    RS::get_singleton()->multimesh_allocate_data(multi_mesh_id, size, RS::MULTIMESH_TRANSFORM_2D, use_colors, use_custom_data, use_indirect);
    RS::get_singleton()->canvas_item_add_multimesh(canvas_item, multi_mesh_id, texture_id);
    RS::get_singleton()->multimesh_set_mesh(multi_mesh_id, mesh->get_rid());
    Vector<RID> material_ids;
    material_ids.resize(mesh->get_surface_count());
    for(int i = 0; i < mesh->get_surface_count(); i++){
        material_ids.write[i] = mesh->surface_get_material(i)->get_rid();
    }
    const flecs::entity entity = world->entity()
                            .set<MultiMeshComponent>({ multi_mesh_id, size })
                            .set<MeshComponent>({ mesh->get_rid(), material_ids})
                            .set<CanvasItemComponent>({ canvas_item })
                            .set<Transform2DComponent>({ transform })
                            .set<VisibilityComponent>({ true })
                            .set_name(name.ascii().get_data());

    return entity;
}

flecs::entity RenderUtility2D::_create_multi_mesh(const flecs::world *world, MultiMeshInstance2D *multi_mesh_instance)  {
    const Ref<MultiMesh> &multi_mesh_ref = multi_mesh_instance->get_multimesh();
    if (multi_mesh_ref.is_null()) {
        ERR_FAIL_V(flecs::entity());
    }
    if (!multi_mesh_ref.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    RefStorage::add(multi_mesh_ref, multi_mesh_ref->get_rid());
    const Ref<MultiMesh> &multi_mesh_instance_ref = multi_mesh_instance->get_multimesh();
    if (multi_mesh_instance_ref.is_null() || !multi_mesh_instance_ref.is_valid()){
        ERR_FAIL_V(flecs::entity());
    }
    const RID& multi_mesh_id = multi_mesh_instance->get_multimesh()->get_rid();
    if (!multi_mesh_id.is_valid() || multi_mesh_id.is_null()) {
        ERR_FAIL_V(flecs::entity());
    }
    RefStorage::add(multi_mesh_instance_ref, multi_mesh_id);
    const Ref<Mesh>& mesh = multi_mesh_instance->get_multimesh()->get_mesh();
    if (mesh.is_null() || !mesh.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    RefStorage::add(mesh, mesh->get_rid());
    const RID& canvas_item = multi_mesh_instance->get_canvas_item();
    if (!canvas_item.is_valid() || canvas_item.is_null()) {
        ERR_FAIL_V(flecs::entity());
    }
    const Transform2D& transform = multi_mesh_instance->get_transform();
    const String& name = multi_mesh_instance->get_name();
    Vector<RID> material_ids;
    material_ids.resize(mesh->get_surface_count());
    for(int i = 0; i < mesh->get_surface_count(); i++){
        material_ids.write[i] = mesh->surface_get_material(i)->get_rid();
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = multi_mesh_instance->get_instance_id();
    NodeStorage::add(multi_mesh_instance, multi_mesh_instance->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<MultiMeshComponent>({ multi_mesh_id, static_cast<uint32_t>(multi_mesh_ref->get_instance_count()) })
                            .set<MeshComponent>({ mesh->get_rid(), material_ids })
                            .set<CanvasItemComponent>({ canvas_item, "MultiMesh2D" })
                            .set<Transform2DComponent>({ transform })
                            .set<VisibilityComponent>({ true })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(name.ascii().get_data());

    if (const Node2D *parent = Object::cast_to<Node2D>(multi_mesh_instance->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
    }
    
    return entity;
}

flecs::entity RenderUtility2D::_create_multi_mesh_instance(const flecs::world *world, const Transform2D &transform, const uint32_t index, const String &name)  {
    return world->entity()
            .set<MultiMeshInstanceComponent>({ index })
            .set<Transform2DComponent>({ transform })
            .set<VisibilityComponent>({ true })
            .set_name(name.ascii().get_data());
}

Vector<flecs::entity> RenderUtility2D::_create_multi_mesh_instances(
        const flecs::world *world,
        const Vector<Transform2D>& transform,
        const flecs::entity &multi_mesh) {
    Vector<flecs::entity> instances;

    const auto &[multi_mesh_id, instance_count] = multi_mesh.get<MultiMeshComponent>();
    for (uint32_t i = 0; i < instance_count; ++i) {
        instances.append(_create_multi_mesh_instance(world, transform[i], i, multi_mesh.name() + " - Instance: #" + String::num_int64(i)));
    }
    return instances;
}

flecs::entity RenderUtility2D::_create_camera_2d(const flecs::world *world, const RID &camera_id, const Transform2D &transform, const String &name)  {
    return world->entity()
        .set<CameraComponent>({ camera_id })
        .set<Transform2DComponent>({ transform })
        .set_name(name.ascii().get_data());
}

flecs::entity RenderUtility2D::_create_camera_2d(const flecs::world *world, Camera2D *camera_2d) {
    if (camera_2d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID camera_id = RS::get_singleton()->camera_create();
    if (!camera_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = camera_2d->get_instance_id();
    NodeStorage::add(camera_2d, camera_2d->get_instance_id());
    const flecs::entity camera = world->entity()
                            .set<Transform2DComponent>({ camera_2d->get_transform() })
                            .set<CameraComponent>({ camera_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(camera_2d->get_name()).ascii().get_data());
    
    return camera;
}

flecs::entity RenderUtility2D::_create_directional_light(const flecs::world *world, const RID &light_id, const Transform2D &transform, const String &name) {
    if (!light_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(),flecs::entity());
    }
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);
    const flecs::entity directional_light = world->entity()
                                        .set<DirectionalLight2DComponent>({ light_id })
                                        .set<Transform2DComponent>({ transform })
                                        .set<VisibilityComponent>({ true }) // Default visibility
                                        .set_name(name.ascii().get_data());
    return directional_light;
}

flecs::entity RenderUtility2D::_create_directional_light(const flecs::world *world, const Transform2D &transform, const String &name)  {
    const RID directional_light_id = RS::get_singleton()->canvas_light_create();
    return _create_directional_light(world, directional_light_id, transform, name);
}

flecs::entity RenderUtility2D::_create_directional_light(const flecs::world *world, DirectionalLight2D *directional_light)  {
    if (directional_light == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID light_id = RS::get_singleton()->canvas_light_create();
    //best attempt to copy settings
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, directional_light->get_canvas());
    RS::get_singleton()->canvas_item_set_light_mask(light_id, directional_light->get_light_mask());
    RS::get_singleton()->canvas_light_set_color(light_id, directional_light->get_color());
    RS::get_singleton()->canvas_light_set_energy(light_id, directional_light->get_energy());
    RS::get_singleton()->canvas_light_set_enabled(light_id, directional_light->is_enabled());
    RS::get_singleton()->canvas_light_set_z_range(light_id, directional_light->get_z_range_min(), directional_light->get_z_range_max());
    RS::get_singleton()->canvas_light_set_layer_range(light_id, directional_light->get_layer_range_min(), directional_light->get_layer_range_max());
    RS::get_singleton()->canvas_light_set_item_cull_mask(light_id, directional_light->get_item_cull_mask());
    RS::get_singleton()->canvas_light_set_item_shadow_cull_mask(light_id, directional_light->get_item_shadow_cull_mask());
    RS::get_singleton()->canvas_light_set_directional_distance(light_id, directional_light->get_max_distance());
    RS::get_singleton()->canvas_light_set_blend_mode(light_id, static_cast<RS::CanvasLightBlendMode>(directional_light->get_blend_mode()));
    RS::get_singleton()->canvas_light_set_shadow_enabled(light_id, directional_light->is_shadow_enabled());
    RS::get_singleton()->canvas_light_set_shadow_filter(light_id, static_cast<RS::CanvasLightShadowFilter>(directional_light->get_shadow_filter()));
    RS::get_singleton()->canvas_light_set_shadow_color(light_id, directional_light->get_shadow_color());
    RS::get_singleton()->canvas_light_set_shadow_smooth(light_id, directional_light->get_shadow_smooth());
    RS::get_singleton()->canvas_light_set_transform(light_id, directional_light->get_transform());
    RS::get_singleton()->canvas_light_set_interpolated(light_id, directional_light->is_physics_interpolated());
    RS::get_singleton()->canvas_light_reset_physics_interpolation(light_id);
    RS::get_singleton()->canvas_light_transform_physics_interpolation(light_id, directional_light->get_transform());
    RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);

    if (const Node2D *parent = Object::cast_to<Node2D>(directional_light->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(light_id, parent->get_canvas_item());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = directional_light->get_instance_id();
    NodeStorage::add(directional_light, directional_light->get_instance_id());

    auto &entity = world->entity()
                            .set<DirectionalLight2DComponent>({ light_id })
                            .set<Transform2DComponent>({ directional_light->get_transform() })
                            .set_name(String(directional_light->get_name()).ascii().get_data())
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set<VisibilityComponent>({ true }); // Default visibility
 
    return entity;
}

flecs::entity RenderUtility2D::_create_point_light(const flecs::world *world, const Transform2D &transform, const String &name)  {
    const RID light_id = RS::get_singleton()->canvas_light_create();
    if (!light_id.is_valid()) {
        ERR_FAIL_COND_V(!light_id.is_valid(),flecs::entity());
    }
    return _create_point_light(world, light_id, transform, name);
}

flecs::entity RenderUtility2D::_create_point_light(const flecs::world *world, const RID &light_id, const Transform2D &transform, const String &name)  {
    if (!light_id.is_valid()) {
        ERR_FAIL_COND_V(!light_id.is_valid(),flecs::entity());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(),flecs::entity());

    }
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);
    const flecs::entity point_light = world->entity()
                                        .set<DirectionalLight2DComponent>({ light_id })
                                        .set<Transform2DComponent>({ transform })
                                        .set<VisibilityComponent>({ true }) // Default visibility
                                        .set_name(name.ascii().get_data());
    return point_light;
}

flecs::entity RenderUtility2D::_create_point_light(const flecs::world *world, PointLight2D *point_light)  {
    if (point_light == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID light_id = RS::get_singleton()->canvas_light_create();
    if (!light_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }

    //best attempt to copy settings
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, point_light->get_canvas());
    RS::get_singleton()->canvas_item_set_light_mask(light_id, point_light->get_light_mask());
    RS::get_singleton()->canvas_light_set_color(light_id, point_light->get_color());
    RS::get_singleton()->canvas_light_set_energy(light_id, point_light->get_energy());
    RS::get_singleton()->canvas_light_set_enabled(light_id, point_light->is_enabled());
    RS::get_singleton()->canvas_light_set_z_range(light_id, point_light->get_z_range_min(), point_light->get_z_range_max());
    RS::get_singleton()->canvas_light_set_layer_range(light_id, point_light->get_layer_range_min(), point_light->get_layer_range_max());
    RS::get_singleton()->canvas_light_set_item_cull_mask(light_id, point_light->get_item_cull_mask());
    RS::get_singleton()->canvas_light_set_item_shadow_cull_mask(light_id, point_light->get_item_shadow_cull_mask());
    RS::get_singleton()->canvas_light_set_blend_mode(light_id, static_cast<RenderingServer::CanvasLightBlendMode>(point_light->get_blend_mode()));
    RS::get_singleton()->canvas_light_set_shadow_enabled(light_id, point_light->is_shadow_enabled());
    RS::get_singleton()->canvas_light_set_shadow_filter(light_id, static_cast<RenderingServer::CanvasLightShadowFilter>(point_light->get_shadow_filter()));
    RS::get_singleton()->canvas_light_set_shadow_color(light_id, point_light->get_shadow_color());
    RS::get_singleton()->canvas_light_set_shadow_smooth(light_id, point_light->get_shadow_smooth());
    RS::get_singleton()->canvas_light_set_transform(light_id, point_light->get_transform());
    RS::get_singleton()->canvas_light_set_interpolated(light_id, point_light->is_physics_interpolated());
    RS::get_singleton()->canvas_light_reset_physics_interpolation(light_id);
    RS::get_singleton()->canvas_light_transform_physics_interpolation(light_id, point_light->get_transform());
    RS::get_singleton()->canvas_light_set_mode(light_id, RenderingServer::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);

    if (const Node2D *parent = Object::cast_to<Node2D>(point_light->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(light_id, parent->get_canvas_item());
    }

    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = point_light->get_instance_id();
    NodeStorage::add(point_light, point_light->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<PointLightComponent>({ light_id })
                            .set<Transform2DComponent>({ point_light->get_transform() })
                            .set_name(String(point_light->get_name()).ascii().get_data())
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set<VisibilityComponent>({ true }); // Default visibility

    return entity;
}

flecs::entity RenderUtility2D::_create_canvas_item(const flecs::world *world, CanvasItem *canvas_item)  {
    if (canvas_item == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = canvas_item->get_instance_id();
    NodeStorage::add(canvas_item, canvas_item->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<CanvasItemComponent>({ canvas_item->get_canvas_item(), canvas_item->get_class() })
                            .set<Transform2DComponent>({canvas_item->get_transform()})
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set<VisibilityComponent>({ canvas_item->is_visible() }) // Default visibility
                            .set_name(String(canvas_item->get_name()).ascii().get_data());

    if (const Node2D *parent = Object::cast_to<Node2D>(canvas_item->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item->get_canvas_item(), parent->get_canvas_item());
    }
    return entity;
}

flecs::entity RenderUtility2D::_create_canvas_item(const flecs::world *world, const RID &canvas_item_id, const Transform2D &transform, const String &name, const String &class_name) {
    return world->entity()
            .set<CanvasItemComponent>({ canvas_item_id, class_name })
            .set<Transform2DComponent>({ transform })
            .set<VisibilityComponent>({ true }) // Default visibility
            .set_name(name.ascii().get_data());
}
flecs::entity RenderUtility2D::_create_skeleton(const flecs::world *world, const RID &skeleton_id, const String &name) {
    return world->entity()
            .set<SkeletonComponent>({ skeleton_id })
            .set<Transform2DComponent>({ Transform2D() }) // Default transform
            .set<VisibilityComponent>({ true }) // Default visibility
            .set_name(name.ascii().get_data());
}

flecs::entity RenderUtility2D::_create_skeleton(const flecs::world *world, Skeleton2D *skeleton_2d)  {
    const RID skeleton_id = RS::get_singleton()->skeleton_create();
    if (skeleton_2d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    if (!skeleton_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_2d->get_bone_count(), false);
    for (int i = 0; i < skeleton_2d->get_bone_count(); ++i) {
        RS::get_singleton()->skeleton_bone_set_transform_2d(skeleton_id, i, skeleton_2d->get_bone(i)->get_transform());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = skeleton_2d->get_instance_id();
    NodeStorage::add(skeleton_2d, skeleton_2d->get_instance_id());
    return world->entity()
            .set<SkeletonComponent>({ skeleton_id })
            .set<Transform2DComponent>({ skeleton_2d->get_transform() })
            .set<ObjectInstanceComponent>(object_instance_component)
            .set<VisibilityComponent>({ skeleton_2d->is_visible() }) // Default;
            .set_name(String(skeleton_2d->get_name()).ascii().get_data());
}

flecs::entity RenderUtility2D::_create_light_occluder(const flecs::world *world, LightOccluder2D *light_occluder) 	{
    const String name = light_occluder->get_name();
    const RID light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
    RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, light_occluder->get_canvas());
    RS::get_singleton()->canvas_light_occluder_set_as_sdf_collision(light_occluder_id, light_occluder->is_set_as_sdf_collision());
    RS::get_singleton()->canvas_light_occluder_set_light_mask(light_occluder_id, light_occluder->get_occluder_light_mask());
    if (const auto polygon = light_occluder->get_occluder_polygon(); polygon.is_valid() && polygon != nullptr)
    {
        RS::get_singleton()->canvas_light_occluder_set_polygon(light_occluder_id, polygon->get_rid());
    }
    RS::get_singleton()->canvas_light_occluder_set_enabled(light_occluder_id, light_occluder->is_enabled());
    RS::get_singleton()->canvas_light_occluder_transform_physics_interpolation(light_occluder_id, light_occluder->get_transform());
    RS::get_singleton()->canvas_light_occluder_set_transform(light_occluder_id, light_occluder->get_transform());
    RS::get_singleton()->canvas_light_occluder_reset_physics_interpolation(light_occluder_id);
    RS::get_singleton()->canvas_light_occluder_set_interpolated(light_occluder_id, light_occluder->is_physics_interpolated());
    if (const Node2D *parent = Object::cast_to<Node2D>(light_occluder->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(light_occluder_id, parent->get_canvas_item());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = light_occluder->get_instance_id();
    NodeStorage::add(light_occluder, light_occluder->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<LightOccluderComponent>({light_occluder_id})
                            .set<Transform2DComponent>({ light_occluder->get_transform() })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set<VisibilityComponent>({ light_occluder->is_visible() }) // Default visibility
                            .set_name(name.ascii().get_data());

    return entity;
}

flecs::entity RenderUtility2D::_create_light_occluder(const flecs::world *world, const RID &light_occluder_id, const Transform2D &transform, const RID &canvas_id, const String &name)  {
    const auto entity = world->entity()
                            .set<LightOccluderComponent>({ light_occluder_id })
                            .set<Transform2DComponent>({ transform })
                            .set<VisibilityComponent>({ true }) // Default visibility
                            .set_name(name.ascii().get_data());
    RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, canvas_id);
    return entity;
}

flecs::entity RenderUtility2D::_create_light_occluder(const flecs::world *world, const Transform2D &transform, const String &name) {
    const auto light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
    if (!world->has<World2DComponent>()
        && world->get<World2DComponent>().is_valid()
        && !world->get<World2DComponent>().is_null()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), flecs::entity());
    }
    return _create_light_occluder(world, light_occluder_id, transform,world->get<World2DComponent>().canvas_id, name);
}

flecs::entity RenderUtility2D::_create_gpu_particles_2d(const flecs::world *world, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform, const String &name)  {
    const auto entity = world->entity()
    .set<ParticlesComponent>({ particles_id })
    .set<Transform2DComponent>({ transform })
    .set<VisibilityComponent>({ true }) // Default visibility
    .set_name(name.ascii().get_data());
    if (!world->has<World2DComponent>()
            && world->get<World2DComponent>().is_valid()
            && !world->get<World2DComponent>().is_null()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), flecs::entity());
            }

    RS::get_singleton()->canvas_item_set_parent(canvas_item_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_item_add_particles(canvas_item_id, particles_id, texture_id);
    return entity;
}

flecs::entity RenderUtility2D::_create_gpu_particles_2d(const flecs::world *world, GPUParticles2D *gpu_particles, uint32_t count, const uint32_t max_depth)  {
    count++;
    if (gpu_particles == nullptr) {
        ERR_FAIL_COND_V(gpu_particles == nullptr, flecs::entity());
    }
    if (count > max_depth) {
        ERR_FAIL_COND_V(count <= max_depth, flecs::entity());
    }
    const RID new_particles_id = RS::get_singleton()->particles_create();
    const RID canvas_item_id = RS::get_singleton()->canvas_item_create();

    //Setting parameters manually, copied from GPUParticles2D class
    RS::get_singleton()->particles_set_emitting(new_particles_id,gpu_particles->is_emitting());
    RS::get_singleton()->particles_set_amount(new_particles_id, gpu_particles->get_amount());
    RS::get_singleton()->particles_set_lifetime(new_particles_id, gpu_particles->get_lifetime());
    RS::get_singleton()->particles_set_one_shot(new_particles_id,gpu_particles->get_one_shot());
    RS::get_singleton()->particles_set_pre_process_time(new_particles_id,gpu_particles->get_pre_process_time());
    RS::get_singleton()->particles_set_explosiveness_ratio(new_particles_id,gpu_particles->get_explosiveness_ratio());
    RS::get_singleton()->particles_set_randomness_ratio(new_particles_id,gpu_particles->get_randomness_ratio());


    //taken from set_visibility_rect in GPUParticles2D
    const Rect2 visibility_rect = gpu_particles->get_visibility_rect();
    AABB aabb;
    aabb.position.x = visibility_rect.position.x;
    aabb.position.y = visibility_rect.position.y;
    aabb.size.x = visibility_rect.size.x;
    aabb.size.y = visibility_rect.size.y;
    RS::get_singleton()->particles_set_custom_aabb(new_particles_id, aabb);
    // end of set_visibility_rect


    RS::get_singleton()->particles_set_use_local_coordinates(new_particles_id,gpu_particles->get_use_local_coordinates());

    //check if we can set a process_material, if not, skip and fail silently
    if (const Ref<Material> process_material = gpu_particles->get_process_material(); process_material.is_valid() && !process_material.is_null()) {
        RS::get_singleton()->particles_set_process_material(new_particles_id,process_material->get_rid());
    }

    RS::get_singleton()->particles_set_speed_scale(new_particles_id,gpu_particles->get_speed_scale());
    RS::get_singleton()->particles_set_collision_base_size(new_particles_id,gpu_particles->get_collision_base_size());
    RS::get_singleton()->particles_set_trails(new_particles_id,gpu_particles->is_trail_enabled(), static_cast<float>(gpu_particles->get_trail_lifetime()));

    const RID mesh_id = RS::get_singleton()->mesh_create();
    const Ref<Texture2D> texture = gpu_particles->get_texture();
    // takes from GPUParticles2D::_notification
    if (gpu_particles->is_trail_enabled()){
        PackedVector2Array points;
        PackedVector2Array uvs;
        PackedInt32Array bone_indices;
        PackedFloat32Array bone_weights;
        PackedInt32Array indices;
        const int trail_sections = gpu_particles->get_trail_sections();
        const int trail_section_subdivisions = gpu_particles->get_trail_section_subdivisions();
        const Vector2 size = texture.is_valid()
                                && !texture.is_null()
                                ? texture->get_size()
                                : Vector2();

        const int total_segments = trail_sections * trail_section_subdivisions;
        real_t depth = size.height * trail_sections;

        for (int j = 0; j <= total_segments; j++) {
            real_t v = j;
            v /= total_segments;

            real_t y = depth * v;
            //y = (depth * 0.5) - y;

            int bone = j / trail_section_subdivisions;
            real_t blend = 1.0 - real_t(j % trail_section_subdivisions) / real_t(trail_section_subdivisions);

            real_t s = size.width;

            points.push_back(Vector2(-s * 0.5, 0));
            points.push_back(Vector2(+s * 0.5, 0));

            uvs.push_back(Vector2(0, v));
            uvs.push_back(Vector2(1, v));

            for (int i = 0; i < 2; i++) {
                bone_indices.push_back(bone);
                bone_indices.push_back(MIN(trail_sections, bone + 1));
                bone_indices.push_back(0);
                bone_indices.push_back(0);

                bone_weights.push_back(blend);
                bone_weights.push_back(1.0 - blend);
                bone_weights.push_back(0);
                bone_weights.push_back(0);
            }

            if (j > 0) {
                int base = j * 2 - 2;
                indices.push_back(base + 0);
                indices.push_back(base + 1);
                indices.push_back(base + 2);

                indices.push_back(base + 1);
                indices.push_back(base + 3);
                indices.push_back(base + 2);
            }
            }

            Array arr;
            arr.resize(RS::ARRAY_MAX);
            arr[RS::ARRAY_VERTEX] = points;
            arr[RS::ARRAY_TEX_UV] = uvs;
            arr[RS::ARRAY_BONES] = bone_indices;
            arr[RS::ARRAY_WEIGHTS] = bone_weights;
            arr[RS::ARRAY_INDEX] = indices;

            RS::get_singleton()->mesh_add_surface_from_arrays(mesh_id, RS::PRIMITIVE_TRIANGLES, arr, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);

            Vector<Transform3D> xforms;
            for (int i = 0; i <= trail_sections; i++) {
                Transform3D xform;
                /*
                xform.origin.y = depth / 2.0 - size.height * real_t(i);
                xform.origin.y = -xform.origin.y; //bind is an inverse transform, so negate y */
                xforms.push_back(xform);
            }

            RS::get_singleton()->particles_set_trail_bind_poses(new_particles_id, xforms);
    }
    //end segment

    RS::get_singleton()->particles_set_interp_to_end(new_particles_id,gpu_particles->get_interp_to_end());
    RS::get_singleton()->particles_set_fixed_fps(new_particles_id,gpu_particles->get_fixed_fps());
    RS::get_singleton()->particles_set_fractional_delta(new_particles_id,gpu_particles->get_fractional_delta());
    RS::get_singleton()->particles_set_interpolate(new_particles_id,gpu_particles->get_interpolate());
    RS::get_singleton()->particles_set_draw_order(new_particles_id, static_cast<RS::ParticlesDrawOrder>(gpu_particles->get_draw_order()));

    RID texture_id = RID();
    if (texture.is_valid() && texture.is_null()) {
        texture_id = texture->get_rid();
    }
    RS::get_singleton()->particles_set_amount_ratio(new_particles_id, gpu_particles->get_amount_ratio());

    //cannot be ported due to it being directly associated with the Node2D type
    //PackedStringArray get_configuration_warnings() const override;

    RS::get_singleton()->particles_set_seed(new_particles_id, gpu_particles->get_seed());
    flecs::entity new_gpu_particle_entity = _create_gpu_particles_2d(world, canvas_item_id, new_particles_id,texture_id ,gpu_particles->get_transform(), String(gpu_particles->get_name()));

    //copied from GPUParticles2D::_attach_sub_emitter
    const NodePath sub_emitter_path = gpu_particles->get_sub_emitter();
    if (Node *n = gpu_particles->get_node_or_null(sub_emitter_path)) {
        if (GPUParticles2D *sen = Object::cast_to<GPUParticles2D>(n); sen && sen != gpu_particles) {
            flecs::entity particle_child = _create_gpu_particles_2d(world, sen, count);

            RS::get_singleton()->particles_set_subemitter(new_particles_id, particle_child.get<ParticlesComponent>().particles_id);
            particle_child.child(new_gpu_particle_entity);
        }
    }
    // end copy

    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = gpu_particles->get_instance_id();
    NodeStorage::add(gpu_particles, gpu_particles->get_instance_id());
    new_gpu_particle_entity.set<ObjectInstanceComponent>(object_instance_component);

    return new_gpu_particle_entity;
}

Ref<FlecsEntity> RenderUtility2D::create_mesh_instance(const Ref<FlecsWorld> &world, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id) {
    if (!world.is_valid() || !world.is_null()) {
        ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = world->add_entity(_create_mesh_instance(world->get_world_ref(), mesh_id, transform, name, canvas_id));
    MeshComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_mesh_instance_with_object(const Ref<FlecsWorld> &world, MeshInstance2D *mesh_instance_2d) {
    if (!world.is_valid() || !world.is_null()) {
        ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = world->add_entity(_create_mesh_instance(world->get_world_ref(), mesh_instance_2d));
    MeshComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}
TypedArray<FlecsEntity> RenderUtility2D::create_multi_mesh(FlecsWorld *flecs_world, const Transform2D &transform, uint32_t size, const Ref<Mesh>& mesh, const RID &texture_id, const String &name, bool use_colors, bool use_custom_data, bool use_indirect) {
    TypedArray<FlecsEntity> entities;
	flecs::entity e = _create_multi_mesh(flecs_world->get_world_ref(), transform, size, mesh, name, texture_id, use_colors, use_custom_data, use_indirect);
	Ref<FlecsEntity> flecs_entity = flecs_world->add_entity(e);
	entities.push_back(flecs_entity);
	MultiMeshComponentRef::create_component(flecs_entity);
	MeshComponentRef::create_component(flecs_entity);
	Transform2DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
    VisibilityComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);
	e.children([&](flecs::entity child) {
		Ref<FlecsEntity> child_entity = flecs_world->add_entity(child);
		entities.push_back(child_entity);
		MultiMeshInstanceComponentRef::create_component(child_entity);
		RenderInstanceComponentRef::create_component(child_entity);
		Transform2DComponentRef::create_component(child_entity);
		VisibilityComponentRef::create_component(child_entity);
        ObjectInstanceComponentRef::create_component(child_entity);
	});

	return entities;
}

TypedArray<FlecsEntity> RenderUtility2D::create_multi_mesh_with_object(const Ref<FlecsWorld> &world, MultiMeshInstance2D *multi_mesh_instance) {
    if (!world.is_valid() || !world.is_null()) {
        ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), TypedArray<FlecsEntity>());
    }
    TypedArray<FlecsEntity> entities;
	flecs::entity e = _create_multi_mesh(world->get_world_ref(), multi_mesh_instance);
	Ref<FlecsEntity> flecs_entity = world->add_entity(e);
	entities.push_back(flecs_entity);
	MultiMeshComponentRef::create_component(flecs_entity);
	MeshComponentRef::create_component(flecs_entity);
	Transform2DComponentRef::create_component(flecs_entity);
	RenderInstanceComponentRef::create_component(flecs_entity);
    VisibilityComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);

	e.children([&](flecs::entity child) {
		Ref<FlecsEntity> child_entity = world->add_entity(child);
		entities.push_back(child_entity);
		MultiMeshInstanceComponentRef::create_component(child_entity);
		RenderInstanceComponentRef::create_component(child_entity);
		Transform2DComponentRef::create_component(child_entity);
		VisibilityComponentRef::create_component(child_entity);
		ObjectInstanceComponentRef::create_component(child_entity);
	});

	return entities;
}

Ref<FlecsEntity> RenderUtility2D::create_multi_mesh_instance(const Ref<FlecsWorld> &world, const Transform2D &transform, const uint32_t index, const String &name) {
    if (!world.is_valid() || !world.is_null()) {
        ERR_FAIL_COND_V(!world.is_valid() || !world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = world->add_entity(_create_multi_mesh_instance(world->get_world_ref(), transform, index, name));
    MultiMeshInstanceComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_camera_2d(const Ref<FlecsWorld> &flecs_world, const RID &camera_id, const Transform2D &transform, const String &name) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_camera_2d(flecs_world->get_world_ref(), camera_id, transform, name));
    CameraComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_camera_2d_with_object(const Ref<FlecsWorld> &flecs_world, Camera2D *camera_2d) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_camera_2d(flecs_world->get_world_ref(), camera_2d));
    CameraComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_directional_light(const Ref<FlecsWorld> &flecs_world, const RID &light_id, const Transform2D &transform, const String &name) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_directional_light(flecs_world->get_world_ref(), light_id, transform, name));
    DirectionalLight2DComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_directional_light_with_object(const Ref<FlecsWorld> &flecs_world, DirectionalLight2D *directional_light) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_directional_light(flecs_world->get_world_ref(), directional_light));
    DirectionalLight2DComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_point_light(const Ref<FlecsWorld> &flecs_world, const RID &light_id, const Transform2D &transform, const String &name) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_point_light(flecs_world->get_world_ref(), light_id, transform, name));
    PointLightComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_point_light_with_object(const Ref<FlecsWorld> &flecs_world, PointLight2D *point_light) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_point_light(flecs_world->get_world_ref(), point_light));
    PointLightComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);

    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_canvas_item_with_object(const Ref<FlecsWorld> &flecs_world, CanvasItem *canvas_item) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_canvas_item(flecs_world->get_world_ref(), canvas_item));
    CanvasItemComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);

    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_canvas_item(const Ref<FlecsWorld> &flecs_world, const RID &canvas_item_id, const Transform2D &transform, const String &name, const String &class_name) {
    if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_canvas_item(flecs_world->get_world_ref(), canvas_item_id, transform, name, class_name));
    CanvasItemComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_skeleton(const Ref<FlecsWorld> &flecs_world, const RID &skeleton_id, const String &name) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_skeleton(flecs_world->get_world_ref(), skeleton_id, name));
    SkeletonComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_skeleton_with_object(const Ref<FlecsWorld> &flecs_world, Skeleton2D *skeleton_2d) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_skeleton(flecs_world->get_world_ref(), skeleton_2d));
    SkeletonComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_light_occluder_with_object(const Ref<FlecsWorld> &flecs_world, LightOccluder2D *light_occluder) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_light_occluder(flecs_world->get_world_ref(), light_occluder));
    LightOccluderComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);

    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_light_occluder(const Ref<FlecsWorld> &flecs_world, const RID &light_occluder_id, const Transform2D &transform, const RID &canvas_id, const String &name) {
	if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_light_occluder(flecs_world->get_world_ref(), light_occluder_id, transform, canvas_id, name));
    LightOccluderComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_gpu_particles_2d(const Ref<FlecsWorld> &flecs_world, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform, const String &name) {
    if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_gpu_particles_2d(flecs_world->get_world_ref(), canvas_item_id, particles_id, texture_id, transform, name));
    ParticlesComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> RenderUtility2D::create_gpu_particles_2d_with_object(const Ref<FlecsWorld> &flecs_world, GPUParticles2D *gpu_particles, uint32_t count, const uint32_t max_depth) {
    if (!flecs_world.is_valid() || !flecs_world.is_null()) {
        ERR_FAIL_COND_V(!flecs_world.is_valid() || !flecs_world.is_null(), Ref<FlecsEntity>());
    }
    Ref<FlecsEntity> entity = flecs_world->add_entity(_create_gpu_particles_2d(flecs_world->get_world_ref(), gpu_particles, count, max_depth));
    ParticlesComponentRef::create_component(entity);
    Transform2DComponentRef::create_component(entity);
    RenderInstanceComponentRef::create_component(entity);
    VisibilityComponentRef::create_component(entity);
    ObjectInstanceComponentRef::create_component(entity);
    return entity;
}
