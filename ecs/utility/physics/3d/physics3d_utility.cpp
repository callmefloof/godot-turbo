#include "physics3d_utility.h"
#include "ecs/flecs_types/flecs_server.h"

RID Physics3DUtility::create_area(const RID &world_id, const String &name, const RID &space_id) {
    const RID area_id = PhysicsServer3D::get_singleton()->area_create();
    PhysicsServer3D::get_singleton()->area_set_space(area_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Area3DComponent comp;
    comp.area_id = area_id;
    flecs::entity e = world->entity();
    e.set<Area3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_body(const RID &world_id, const String &name, const RID &space_id) {
    const RID body_id = PhysicsServer3D::get_singleton()->body_create();
    PhysicsServer3D::get_singleton()->body_set_space(body_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Body3DComponent comp;
    comp.body_id = body_id;
    flecs::entity e = world->entity();
    e.set<Body3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_joint(const RID &world_id, const String &name, const RID &space_id) {
    const RID joint_id = PhysicsServer3D::get_singleton()->joint_create();
    //PhysicsServer3D::get_singleton()->joint(joint_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Joint3DComponent comp;
    comp.joint_id = joint_id;
    flecs::entity e = world->entity();
    e.set<Joint3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_soft_body(const RID &world_id, const String &name, const RID &space_id) {
    const RID soft_body_id = PhysicsServer3D::get_singleton()->soft_body_create();
    PhysicsServer3D::get_singleton()->soft_body_set_space(soft_body_id, space_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    SoftBody3DComponent comp;
    comp.soft_body_id = soft_body_id;
    flecs::entity e = world->entity();
    e.set<SoftBody3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

/// Create an Area3D entity from a Godot Area3D object
RID Physics3DUtility::create_area_with_object(const RID &world_id, Area3D *area_3d) {
    if (area_3d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID area_id = area_3d->get_rid();
    if (!area_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = area_3d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(area_3d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Area3DComponent comp;
    comp.area_id = area_id;
    flecs::entity e = world->entity();
    e.set<Area3DComponent>(comp)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(area_3d->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

    /// Create a RigidBody3D entity from a Godot RigidBody3D object
RID Physics3DUtility::create_rigid_body_with_object(const RID &world_id, RigidBody3D *body_3d) {
    if (body_3d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID body_id = body_3d->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = body_3d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(body_3d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Body3DComponent comp;
    comp.body_id = body_id;
    flecs::entity e = world->entity();
    e.set<Body3DComponent>(comp)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(body_3d->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_physics_body_with_object(const RID &world_id, PhysicsBody3D *physics_body) {
    if (physics_body == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID body_id = physics_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = physics_body->get_instance_id();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Body3DComponent comp;
    comp.body_id = body_id;
    flecs::entity e = world->entity();
    e.set<Body3DComponent>(comp)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(physics_body->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_joint_with_object(const RID &world_id, Joint3D *joint_3d) {
    if (joint_3d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID joint_id = joint_3d->get_rid();
    if (!joint_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = joint_3d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(joint_3d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    Joint3DComponent comp;
    comp.joint_id = joint_id;
    flecs::entity e = world->entity();
    e.set<Joint3DComponent>(comp)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(joint_3d->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Physics3DUtility::create_soft_body_with_object(const RID &world_id, SoftBody3D *soft_body_3d) {
    if (soft_body_3d == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID soft_body_id = soft_body_3d->get_physics_rid();
    if (!soft_body_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = soft_body_3d->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(soft_body_3d, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    if (!world) {
        ERR_FAIL_V(RID());
    }
    SoftBody3DComponent comp;
    comp.soft_body_id = soft_body_id;
    flecs::entity e = world->entity();
    e.set<SoftBody3DComponent>(comp)
     .set<ObjectInstanceComponent>(object_instance_component)
     .set_name(String(soft_body_3d->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}



void Physics3DUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), "create_area_with_object",
            &Physics3DUtility::create_area_with_object, "world", "area_3d");
    ClassDB::bind_static_method(get_class_static(), "create_rigid_body_with_object",
            &Physics3DUtility::create_rigid_body_with_object, "world", "rigid_body_3d");
    ClassDB::bind_static_method(get_class_static(), "create_physics_body_with_object",
            &Physics3DUtility::create_physics_body_with_object, "world", "physics_body_3d");
    ClassDB::bind_static_method(get_class_static(), "create_joint_with_object", 
            &Physics3DUtility::create_joint_with_object, "world", "joint_3d");
    ClassDB::bind_static_method(get_class_static(), "create_soft_body_with_object",
            &Physics3DUtility::create_soft_body_with_object, "world", "soft_body_3d");
    ClassDB::bind_static_method(get_class_static(), "create_area",
            &Physics3DUtility::create_area, "world", "name", "space_id");
    ClassDB::bind_static_method(get_class_static(), "create_body",
            &Physics3DUtility::create_body, "world", "name", "space_id");
}