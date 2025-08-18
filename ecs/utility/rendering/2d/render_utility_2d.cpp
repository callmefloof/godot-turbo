#include "render_utility_2d.h"
#include "core/math/transform_2d.h"
#include "core/templates/rid.h"
#include "core/variant/typed_array.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/object_instance_component.h"
#include "ecs/flecs_types/flecs_server.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/rendering/rendering_components.h"
#include "flecs.h"

RID RenderUtility2D::create_mesh_instance_with_id(const RID &world_id, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
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
    const AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_id);

    flecs::entity e = world->entity();
    MeshComponent mc;
    mc.mesh_id = mesh_id;
    mc.material_ids = material_ids;
    mc.custom_aabb = custom_aabb;
    Transform2DComponent tc;
    tc.transform = transform;
    CanvasItemComponent cic;
    // CanvasItemComponent only stores an item name. Store the provided name (or type) there.
    cic.item_name = name;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<MeshComponent>(mc)
     .set<Transform2DComponent>(tc)
     .set<CanvasItemComponent>(cic)
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    RS::get_singleton()->canvas_item_add_mesh(canvas_item, mesh_id);
    RS::get_singleton()->canvas_item_set_parent(canvas_item, canvas_id);
    RS::get_singleton()->canvas_item_set_material(canvas_item, material_ids[0]);
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_mesh_instance(const RID &world_id, const Transform2D &transform, const String &name){
    Vector<RID> material_ids;
    const RID mesh_id = RS::get_singleton()->mesh_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), RID());
    }
    return create_mesh_instance_with_id(world_id, mesh_id, transform, name, world->get<World2DComponent>().canvas_id);
}

RID RenderUtility2D::create_mesh_instance_with_object(const RID &world_id, MeshInstance2D *mesh_instance_2d){
    Vector<RID> material_ids;
    const Ref<Mesh>& mesh = mesh_instance_2d->get_mesh();
    const RID& canvas_item = mesh_instance_2d->get_canvas_item();
    for (int i = 0; i < mesh->get_surface_count(); ++i) {
        if (Ref<Material> material = mesh->surface_get_material(i); material.is_valid()) {
            FlecsServer::get_singleton()->add_to_ref_storage(material, world_id);
            material_ids.push_back(material->get_rid());
        } else {
            material_ids.push_back(RID()); // Use an empty RID if no material is set
        }
    }
    if (const Node2D *parent = Object::cast_to<Node2D>(mesh_instance_2d->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
    }

    FlecsServer::get_singleton()->add_to_ref_storage(mesh, world_id);
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = mesh_instance_2d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(mesh_instance_2d, world_id);
    AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh->get_rid());
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    MeshComponent mc;
    mc.mesh_id = mesh->get_rid();
    mc.material_ids = material_ids;
    mc.custom_aabb = custom_aabb;
    CanvasItemComponent cic;
    // Store the instance class name in item_name for reflection
    cic.item_name = mesh_instance_2d->get_class_name();
    Transform2DComponent tc;
    tc.transform = mesh_instance_2d->get_transform();
    VisibilityComponent vc;
    vc.visible = true;

    e.set<MeshComponent>(mc)
     .set<CanvasItemComponent>(cic)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<VisibilityComponent>(vc)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(mesh_instance_2d->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_multi_mesh(const RID &world_id, const Transform2D &transform, const uint32_t size, const Ref<Mesh> mesh, const String &name, const RID &texture_id, const bool use_colors, const bool use_custom_data, const bool use_indirect)  {
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
    AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh->get_rid());
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    MultiMeshComponent mmc;
    mmc.multi_mesh_id = multi_mesh_id;
    mmc.instance_count = size;
    MeshComponent mc;
    mc.mesh_id = mesh->get_rid();
    mc.material_ids = material_ids;
    mc.custom_aabb = custom_aabb;
    CanvasItemComponent cic;
    cic.item_name = String("MultiMesh2D");
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<MultiMeshComponent>(mmc)
     .set<MeshComponent>(mc)
     .set<CanvasItemComponent>(cic)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

TypedArray<RID> RenderUtility2D::create_multi_mesh_with_object(const RID &world_id, MultiMeshInstance2D *multi_mesh_instance)  {
    const Ref<MultiMesh> &multi_mesh_ref = multi_mesh_instance->get_multimesh();
    if (multi_mesh_ref.is_null()) {
        ERR_FAIL_V(TypedArray<RID>());
    }
    if (!multi_mesh_ref.is_valid()) {
        ERR_FAIL_V(TypedArray<RID>());
    }
    FlecsServer::get_singleton()->add_to_ref_storage(multi_mesh_ref, world_id);
    const Ref<MultiMesh> &multi_mesh_instance_ref = multi_mesh_instance->get_multimesh();
    if (multi_mesh_instance_ref.is_null() || !multi_mesh_instance_ref.is_valid()){
        ERR_FAIL_V(TypedArray<RID>());
    }
    const RID& multi_mesh_id = multi_mesh_instance->get_multimesh()->get_rid();
    if (!multi_mesh_id.is_valid() || multi_mesh_id.is_null()) {
        ERR_FAIL_V(TypedArray<RID>());
    }
    FlecsServer::get_singleton()->add_to_ref_storage(multi_mesh_instance_ref, world_id);
    const Ref<Mesh>& mesh = multi_mesh_instance->get_multimesh()->get_mesh();
    if (mesh.is_null() || !mesh.is_valid()) {
        ERR_FAIL_V(TypedArray<RID>());
    }
    FlecsServer::get_singleton()->add_to_ref_storage(mesh, world_id);
    const RID& canvas_item = multi_mesh_instance->get_canvas_item();
    if (!canvas_item.is_valid() || canvas_item.is_null()) {
        ERR_FAIL_V(TypedArray<RID>());
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
    FlecsServer::get_singleton()->add_to_node_storage(multi_mesh_instance, world_id);
    const AABB custom_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh->get_rid());
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(TypedArray<RID>());
    }

    flecs::entity e = world->entity();
    MultiMeshComponent mmc;
    mmc.multi_mesh_id = multi_mesh_id;
    mmc.instance_count = static_cast<uint32_t>(multi_mesh_ref->get_instance_count());
    MeshComponent mc;
    mc.mesh_id = mesh->get_rid();
    mc.material_ids = material_ids;
    mc.custom_aabb = custom_aabb;
    CanvasItemComponent cic;
    cic.item_name = String("MultiMesh2D");
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<MultiMeshComponent>(mmc)
     .set<MeshComponent>(mc)
     .set<CanvasItemComponent>(cic)
     .set<Transform2DComponent>(tc)
     .set<VisibilityComponent>(vc)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(name.ascii().get_data());

    if (const Node2D *parent = Object::cast_to<Node2D>(multi_mesh_instance->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item, parent->get_canvas_item());
    }
    TypedArray<RID> entities;
    TypedArray<Transform2D> transforms;
    transforms.resize(multi_mesh_ref->get_instance_count());
    for (uint32_t i = 0; i < multi_mesh_ref->get_instance_count(); ++i) {
        transforms[i] = multi_mesh_ref->get_instance_transform(i);
    }
    entities.append(FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e));
    entities.append_array(create_multi_mesh_instances(world_id, transforms, multi_mesh_id));
    return entities;
}

RID RenderUtility2D::create_multi_mesh_instance(const RID &world_id, const Transform2D &transform, const uint32_t index, const String &name)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    MultiMeshInstanceComponent mmic;
    mmic.index = index;
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<MultiMeshInstanceComponent>(mmic)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

TypedArray<RID> RenderUtility2D::create_multi_mesh_instances(
    const RID &world_id,
        const TypedArray<Transform2D>& transform,
        const RID &multi_mesh) {
    TypedArray<RID> instances;
    flecs::entity multi_mesh_entity = FlecsServer::get_singleton()->_get_entity(multi_mesh, world_id);
    const auto &[multi_mesh_id, instance_count] = multi_mesh_entity.get<MultiMeshComponent>();
    instances.resize(instance_count);
    for (uint32_t i = 0; i < instance_count; ++i) {
        instances[i] = create_multi_mesh_instance(world_id, transform[i], i, multi_mesh_entity.name() + " - Instance: #" + String::num_int64(i));
    }
    return instances;
}

RID RenderUtility2D::create_camera_with_id(const RID &world_id, const RID &camera_id, const Transform2D &transform, const String &name)  {

    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    CameraComponent cc;
    cc.camera_id = camera_id;
    Transform2DComponent tc;
    tc.transform = transform;

    e.set<CameraComponent>(cc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_camera_with_object(const RID &world_id, Camera2D *camera_2d) {
    if (camera_2d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID camera_id = RS::get_singleton()->camera_create();
    if (!camera_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = camera_2d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(camera_2d, world_id);

    flecs::entity e = world->entity();
    Transform2DComponent tc;
    tc.transform = camera_2d->get_transform();
    CameraComponent cc;
    cc.camera_id = camera_id;

    e.set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<CameraComponent>(cc)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(camera_2d->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform2D &transform, const String &name) {
    if (!light_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(),RID());
    }
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_DIRECTIONAL);

    flecs::entity e = world->entity();
    DirectionalLight2DComponent dlc;
    dlc.light_id = light_id;
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<DirectionalLight2DComponent>(dlc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_directional_light(const RID &world_id, const Transform2D &transform, const String &name)  {
    const RID directional_light_id = RS::get_singleton()->canvas_light_create();
    return create_directional_light_with_id(world_id, directional_light_id, transform, name);
}

RID RenderUtility2D::create_directional_light_with_object(const RID &world_id, DirectionalLight2D *directional_light)  {
    if (directional_light == nullptr) {
        ERR_FAIL_V(RID());
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
    FlecsServer::get_singleton()->add_to_node_storage(directional_light, world_id);

    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    DirectionalLight2DComponent dlc;
    dlc.light_id = light_id;
    Transform2DComponent tc;
    tc.transform = directional_light->get_transform();
    VisibilityComponent vc;
    vc.visible = true;

    e.set<DirectionalLight2DComponent>(dlc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<ObjectInstanceComponent>(object_instance_component)
     .set<VisibilityComponent>(vc)
     .set_name(String(directional_light->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_point_light(const RID &world_id, const Transform2D &transform, const String &name)  {
    const RID light_id = RS::get_singleton()->canvas_light_create();
    if (!light_id.is_valid()) {
        ERR_FAIL_COND_V(!light_id.is_valid(),RID());
    }
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(),RID());
    }
    RS::get_singleton()->canvas_light_attach_to_canvas(light_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_light_set_mode(light_id, RS::CanvasLightMode::CANVAS_LIGHT_MODE_POINT);
    flecs::entity e = world->entity();
    PointLightComponent plc2;
    plc2.light_id = light_id;
    Transform2DComponent tc2;
    tc2.transform = transform;
    VisibilityComponent vc2;
    vc2.visible = true;

    e.set<PointLightComponent>(plc2)
     .set<Transform2DComponent>(tc2).add<DirtyTransform>()
     .set<VisibilityComponent>(vc2)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_point_light_with_object(const RID &world_id, PointLight2D *point_light)  {
    if (point_light == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID light_id = RS::get_singleton()->canvas_light_create();
    if (!light_id.is_valid()) {
        ERR_FAIL_V(RID());
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
    FlecsServer::get_singleton()->add_to_node_storage(point_light, world_id);

    PointLightComponent plc;
    plc.light_id = light_id;

    Transform2DComponent tc;
    tc.transform = point_light->get_transform();

    VisibilityComponent vc;
    vc.visible = true;

    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    e.set<PointLightComponent>(plc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<ObjectInstanceComponent>(object_instance_component)
     .set<VisibilityComponent>(vc)
     .set_name(String(point_light->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_canvas_item_with_object(const RID &world_id, CanvasItem *canvas_item)  {
    if (canvas_item == nullptr) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = canvas_item->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(canvas_item, world_id);

    CanvasItemComponent cic;
    // CanvasItemComponent stores an item name in the component definition.
    cic.item_name = canvas_item->get_name();

    Transform2DComponent tc;
    tc.transform = canvas_item->get_transform();

    VisibilityComponent vc;
    vc.visible = canvas_item->is_visible();

    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    e.set<CanvasItemComponent>(cic)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<ObjectInstanceComponent>(object_instance_component)
     .set<VisibilityComponent>(vc)
     .set_name(String(canvas_item->get_name()).ascii().get_data());

    if (const Node2D *parent = Object::cast_to<Node2D>(canvas_item->get_parent()); parent != nullptr) {
        RS::get_singleton()->canvas_item_set_parent(canvas_item->get_canvas_item(), parent->get_canvas_item());
    }
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_canvas_item_with_id(const RID &world_id, const RID &canvas_item_id, const Transform2D &transform, const String &name, const String &class_name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    flecs::entity e = world->entity();
    CanvasItemComponent cic2;
    // CanvasItemComponent stores an item name; use provided class_name as the item_name for consistency
    cic2.item_name = class_name;
    Transform2DComponent tc2;
    tc2.transform = transform;
    VisibilityComponent vc2;
    vc2.visible = true;

    e.set<CanvasItemComponent>(cic2)
     .set<Transform2DComponent>(tc2).add<DirtyTransform>()
     .set<VisibilityComponent>(vc2)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}
RID RenderUtility2D::create_skeleton_with_id(const RID &world_id, const RID &skeleton_id, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    flecs::entity e = world->entity();
    SkeletonComponent sc;
    sc.skeleton_id = skeleton_id;
    Transform2DComponent tc;
    tc.transform = Transform2D();
    VisibilityComponent vc;
    vc.visible = true;

    e.set<SkeletonComponent>(sc)
     .set<Transform2DComponent>(tc)
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_skeleton_with_object(const RID &world_id, Skeleton2D *skeleton_2d)  {
    const RID skeleton_id = RS::get_singleton()->skeleton_create();
    if (skeleton_2d == nullptr) {
        ERR_FAIL_V(RID());
    }
    if (!skeleton_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    RS::get_singleton()->skeleton_allocate_data(skeleton_id, skeleton_2d->get_bone_count(), false);
    for (int i = 0; i < skeleton_2d->get_bone_count(); ++i) {
        RS::get_singleton()->skeleton_bone_set_transform_2d(skeleton_id, i, skeleton_2d->get_bone(i)->get_transform());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = skeleton_2d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(skeleton_2d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    SkeletonComponent sc;
    sc.skeleton_id = skeleton_id;
    Transform2DComponent tc;
    tc.transform = skeleton_2d->get_transform();

    VisibilityComponent skel_vc;
    skel_vc.visible = skeleton_2d->is_visible();

    e.set<SkeletonComponent>(sc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<ObjectInstanceComponent>(object_instance_component)
     .set<VisibilityComponent>(skel_vc)
     .set_name(String(skeleton_2d->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_light_occluder_with_object(const RID &world_id, LightOccluder2D *light_occluder) 	{
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
    FlecsServer::get_singleton()->add_to_node_storage(light_occluder, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }

    flecs::entity e = world->entity();
    LightOccluderComponent loc;
    loc.light_occluder_id = light_occluder_id;
    Transform2DComponent tc;
    tc.transform = light_occluder->get_transform();
    VisibilityComponent vc;
    vc.visible = light_occluder->is_visible();

    e.set<LightOccluderComponent>(loc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<ObjectInstanceComponent>(object_instance_component)
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_light_occluder_with_id(const RID &world_id, const RID &light_occluder_id, const Transform2D &transform, const RID &canvas_id, const String &name)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    flecs::entity e = world->entity();
    LightOccluderComponent loc;
    loc.light_occluder_id = light_occluder_id;
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<LightOccluderComponent>(loc)
     .set<Transform2DComponent>(tc).add<DirtyTransform>()
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());
    RS::get_singleton()->canvas_light_occluder_attach_to_canvas(light_occluder_id, canvas_id);
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_light_occluder(const RID &world_id, const Transform2D &transform, const String &name) {
    const auto light_occluder_id = RS::get_singleton()->canvas_light_occluder_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), RID());
    }
    return create_light_occluder_with_id(world_id, light_occluder_id, transform, world->get<World2DComponent>().canvas_id, name);
}

RID RenderUtility2D::create_gpu_particles_with_id(const RID &world_id, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform, const String &name)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    flecs::entity e = world->entity();
    ParticlesComponent pc;
    pc.particles_id = particles_id;
    Transform2DComponent tc;
    tc.transform = transform;
    VisibilityComponent vc;
    vc.visible = true;

    e.set<ParticlesComponent>(pc)
     .set<Transform2DComponent>(tc)
     .set<VisibilityComponent>(vc)
     .set_name(name.ascii().get_data());

    if (!world->has<World2DComponent>()) {
        ERR_FAIL_COND_V(!world->has<World2DComponent>(), RID());
    }

    RS::get_singleton()->canvas_item_set_parent(canvas_item_id, world->get<World2DComponent>().canvas_id);
    RS::get_singleton()->canvas_item_add_particles(canvas_item_id, particles_id, texture_id);
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID RenderUtility2D::create_gpu_particles_with_object(const RID &world_id, GPUParticles2D *gpu_particles, uint32_t count, const uint32_t max_depth)  {
    count++;
    if (gpu_particles == nullptr) {
        ERR_FAIL_COND_V(gpu_particles == nullptr, RID());
    }
    if (count > max_depth) {
        ERR_FAIL_COND_V(count <= max_depth, RID());
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
    if (texture.is_valid() && !texture.is_null()) {
        texture_id = texture->get_rid();
    }
    RS::get_singleton()->particles_set_amount_ratio(new_particles_id, gpu_particles->get_amount_ratio());

    RS::get_singleton()->particles_set_seed(new_particles_id, gpu_particles->get_seed());
    RID new_gpu_particle_entity_rid = create_gpu_particles_with_id(world_id, canvas_item_id, new_particles_id, texture_id, gpu_particles->get_transform(), String(gpu_particles->get_name()));

    //copied from GPUParticles2D::_attach_sub_emitter
    const NodePath sub_emitter_path = gpu_particles->get_sub_emitter();
    if (Node *n = gpu_particles->get_node_or_null(sub_emitter_path)) {
            if (GPUParticles2D *sen = Object::cast_to<GPUParticles2D>(n); sen && sen != gpu_particles) {
            RID particle_child_rid = create_gpu_particles_with_object(world_id, sen, count);
            flecs::entity particle_child_entity = FlecsServer::get_singleton()->_get_entity(particle_child_rid, world_id);
            flecs::entity new_gpu_particle_entity = FlecsServer::get_singleton()->_get_entity(new_gpu_particle_entity_rid, world_id);

            RS::get_singleton()->particles_set_subemitter(new_particles_id, particle_child_entity.get<ParticlesComponent>().particles_id);
            // Optional: link parent/child in flecs if desired
            particle_child_entity.child(new_gpu_particle_entity);
        }
    }
    // end copy

    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = gpu_particles->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(gpu_particles, world_id);

    flecs::entity new_gpu_particle_entity = FlecsServer::get_singleton()->_get_entity(new_gpu_particle_entity_rid, world_id);
    new_gpu_particle_entity.set<ObjectInstanceComponent>(object_instance_component);

    return new_gpu_particle_entity_rid;
}