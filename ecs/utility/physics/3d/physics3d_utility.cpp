#include "physics3d_utility.h"

flecs::entity Physics3DUtility::_create_area(const flecs::world *world, const String &name, const RID &space_id) {
    const RID area_id = PhysicsServer3D::get_singleton()->area_create();
    PhysicsServer3D::get_singleton()->area_set_space(area_id, space_id);
    return world->entity().set<Area3DComponent>({ area_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics3DUtility::_create_body_3d(const flecs::world *world, const String &name, const RID &space_id) {
    const RID body_id = PhysicsServer3D::get_singleton()->body_create();
    PhysicsServer3D::get_singleton()->body_set_space(body_id, space_id);
    return world->entity().set<Body3DComponent>({ body_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics3DUtility::_create_joint(const flecs::world *world, const String &name, const RID &space_id) {
    const RID joint_id = PhysicsServer3D::get_singleton()->joint_create();
    //PhysicsServer3D::get_singleton()->joint(joint_id, space_id);
    return world->entity().set<Joint3DComponent>({ joint_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics3DUtility::_create_soft_body(const flecs::world *world, const String &name, const RID &space_id) {
    const RID soft_body_id = PhysicsServer3D::get_singleton()->soft_body_create();
    PhysicsServer3D::get_singleton()->soft_body_set_space(soft_body_id, space_id);
    return world->entity().set<SoftBody3DComponent>({ soft_body_id }).set_name(name.ascii().get_data());
}

/// Create an Area3D entity from a Godot Area3D object
flecs::entity Physics3DUtility::_create_area(const flecs::world *world, Area3D *area_3d) {
    if (area_3d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID area_id = area_3d->get_rid();
    if (!area_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.instance_id = area_3d->get_instance_id();
    NodeStorage::add(area_3d, area_3d->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<Area3DComponent>({ area_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(area_3d->get_name()).ascii().get_data());
    
    return entity;
}

    /// Create a RigidBody3D entity from a Godot RigidBody3D object
flecs::entity Physics3DUtility::_create_rigid_body(const flecs::world *world, RigidBody3D *body_3d) {
    if (body_3d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID body_id = body_3d->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.instance_id = body_3d->get_instance_id();
    NodeStorage::add(body_3d, body_3d->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<Body3DComponent>({ body_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(body_3d->get_name()).ascii().get_data());
    
    return entity;
}

flecs::entity Physics3DUtility::_create_physics_body(const flecs::world *world, PhysicsBody3D *physics_body) {
    if (physics_body == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID body_id = physics_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.instance_id = physics_body->get_instance_id();
    const flecs::entity entity = world->entity()
                            .set<Body3DComponent>({ body_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(physics_body->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Physics3DUtility::_create_joint(const flecs::world *world, Joint3D *joint_3d) {
    if (joint_3d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID joint_id = joint_3d->get_rid();
    if (!joint_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.instance_id = joint_3d->get_instance_id();
    NodeStorage::add(joint_3d, joint_3d->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<Joint3DComponent>({ joint_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(joint_3d->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Physics3DUtility::_create_soft_body(const flecs::world *world, SoftBody3D *soft_body_3d) {
    if (soft_body_3d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID soft_body_id = soft_body_3d->get_physics_rid();
    if (!soft_body_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.instance_id = soft_body_3d->get_instance_id();
    NodeStorage::add(soft_body_3d, soft_body_3d->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<SoftBody3DComponent>({ soft_body_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(soft_body_3d->get_name()).ascii().get_data());
    return entity;
}

Ref<FlecsEntity> Physics3DUtility::create_area_with_object(FlecsWorld *world, Area3D *area_3d) {
    if (area_3d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const flecs::entity e = _create_area(world->get_world_ref(), area_3d);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Area3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_rigid_body_with_object(FlecsWorld *world, RigidBody3D *rigid_body_3d) {
    if (rigid_body_3d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const flecs::entity e = _create_rigid_body(world->get_world_ref(), rigid_body_3d);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Body3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_physics_body_with_object(FlecsWorld *world, PhysicsBody3D *physics_body_3d) {
    if (physics_body_3d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const flecs::entity e = _create_physics_body(world->get_world_ref(), physics_body_3d);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Body3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_joint_with_object(FlecsWorld *world, Joint3D *joint_3d) {
    if (joint_3d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const flecs::entity e = _create_joint(world->get_world_ref(), joint_3d);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Joint3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_soft_body_with_object(FlecsWorld *world, SoftBody3D *soft_body_3d) {
    if (soft_body_3d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const flecs::entity e = _create_soft_body(world->get_world_ref(), soft_body_3d);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    SoftBody3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_area(FlecsWorld *world, const String &name, const RID &space_id) {
    const flecs::entity e = _create_area(world->get_world_ref(), name, space_id);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Area3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_body(FlecsWorld *world, const String &name, const RID &space_id) {
    const flecs::entity e = _create_body_3d(world->get_world_ref(), name, space_id);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Body3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_joint(FlecsWorld *world, const String &name, const RID &space_id) {
    const flecs::entity e = _create_joint(world->get_world_ref(), name, space_id);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    Joint3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Physics3DUtility::create_soft_body(FlecsWorld *world, const String &name, const RID &space_id) {
    const flecs::entity e = _create_soft_body(world->get_world_ref(), name, space_id);
    Ref<FlecsEntity> ref_entity = world->add_entity(e);
    SoftBody3DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
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