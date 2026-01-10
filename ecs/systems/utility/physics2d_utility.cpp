#include "physics2d_utility.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

RID Physics2DUtility::create_area(const RID &world_id, const String &name, const RID &space_id) {
    const RID area_id = PhysicsServer2D::get_singleton()->area_create();
    PhysicsServer2D::get_singleton()->area_set_space(area_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Area2DComponent comp;
    comp.area_id = area_id;
    flecs::entity e = world->entity().set<Area2DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_body(const RID &world_id, const String &name, const RID &space_id) {
    const RID body_id = PhysicsServer2D::get_singleton()->body_create();
    PhysicsServer2D::get_singleton()->body_set_space(body_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Body2DComponent comp;
    comp.body_id = body_id;
    flecs::entity e = world->entity().set<Body2DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_joint(const RID &world_id, const String &name, const RID &space_id) {
    const RID joint_id = PhysicsServer2D::get_singleton()->joint_create();
    //no way to find to space?
    //PhysicsServer2D::get_singleton()->joint(joint_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Joint2DComponent comp;
    comp.joint_id = joint_id;
    flecs::entity e = world->entity().set<Joint2DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_area_with_object(const RID &world_id, Area2D *area_2d) {
    if (area_2d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID area_id = area_2d->get_rid();
    if (!area_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = area_2d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(area_2d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Area2DComponent comp;
    comp.area_id = area_id;
    const flecs::entity e = world->entity()
                            .set<Area2DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(area_2d->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_rigid_body_with_object(const RID &world_id, RigidBody2D *rigid_body) {
    if (rigid_body == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID body_id = rigid_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = rigid_body->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(rigid_body, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Body2DComponent comp;
    comp.body_id = body_id;
    const flecs::entity e = world->entity()
                            .set<Body2DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(rigid_body->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_physics_body_with_object(const RID &world_id, PhysicsBody2D *physics_body) {
    if (physics_body == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID body_id = physics_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = physics_body->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(physics_body, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Body2DComponent comp;
    comp.body_id = body_id;
    const flecs::entity e = world->entity()
                            .set<Body2DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(physics_body->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics2DUtility::create_joint_with_object(const RID &world_id, Joint2D *joint_2d) {
    if (joint_2d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID joint_id = joint_2d->get_rid();
    if (!joint_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = joint_2d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(joint_2d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    Joint2DComponent comp;
    comp.joint_id = joint_id;
    const flecs::entity e = world->entity()
                            .set<Joint2DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(joint_2d->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}


void Physics2DUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_area_with_object", "world_id", "area_2d"),
            &Physics2DUtility::create_area_with_object);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_rigid_body_with_object", "world_id", "rigid_body"),
            &Physics2DUtility::create_rigid_body_with_object);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_physics_body_with_object", "world_id", "physics_body"),
            &Physics2DUtility::create_physics_body_with_object);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_joint_with_object", "world_id", "joint_2d"),
            &Physics2DUtility::create_joint_with_object);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_area", "world_id", "name", "space_id"),
            &Physics2DUtility::create_area);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_body", "world_id", "name", "space_id"),
            &Physics2DUtility::create_body);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_joint", "world_id", "name", "space_id"),
            &Physics2DUtility::create_joint);
}
