#include "physics2d_utility.h"

flecs::entity Physics2DUtility::_create_area(const flecs::world &world, const String &name, const RID &space_id) {
    const RID area_id = PhysicsServer2D::get_singleton()->area_create();
    PhysicsServer2D::get_singleton()->area_set_space(area_id, space_id);
    return world.entity().set<Area2DComponent>({ area_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics2DUtility::_create_body(const flecs::world &world, const String &name, const RID &space_id) {
    const RID body_id = PhysicsServer2D::get_singleton()->body_create();
    PhysicsServer2D::get_singleton()->body_set_space(body_id, space_id);
    return world.entity().set<Body2DComponent>({ body_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics2DUtility::_create_joint(const flecs::world &world, const String &name, const RID &space_id) {
    const RID joint_id = PhysicsServer2D::get_singleton()->joint_create();
    //no way to find to space?
    //PhysicsServer2D::get_singleton()->joint(joint_id, space_id);
    return world.entity().set<Joint2DComponent>({ joint_id }).set_name(name.ascii().get_data());
}

flecs::entity Physics2DUtility::_create_area(const flecs::world &world, Area2D *area_2d) {
    if (area_2d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID area_id = area_2d->get_rid();
    if (!area_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<Area2DComponent>({ area_id })
                            .set_name(String(area_2d->get_name()).ascii().get_data());
    ObjectIDStorage::add(dynamic_cast<Object*>(area_2d), area_id);
    return entity;
}

flecs::entity Physics2DUtility::_create_rigid_body(const flecs::world &world, RigidBody2D *rigid_body) {
    if (rigid_body == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID body_id = rigid_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<Body2DComponent>({ body_id })
                            .set_name(String(rigid_body->get_name()).ascii().get_data());
    ObjectIDStorage::add(dynamic_cast<Object*>(rigid_body), body_id);
    return entity;
}

flecs::entity Physics2DUtility::_create_physics_body(const flecs::world &world, PhysicsBody2D *physics_body) {
    if (physics_body == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID body_id = physics_body->get_rid();
    if (!body_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<Body2DComponent>({ body_id })
                            .set_name(String(physics_body->get_name()).ascii().get_data());
    ObjectIDStorage::add(dynamic_cast<Object*>(physics_body), body_id);
    return entity;
}

flecs::entity Physics2DUtility::_create_joint(const flecs::world &world, Joint2D *joint_2d) {
    if (joint_2d == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID joint_id = joint_2d->get_rid();
    if (!joint_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<Joint2DComponent>({ joint_id })
                            .set_name(String(joint_2d->get_name()).ascii().get_data());
    ObjectIDStorage::add(dynamic_cast<Object*>(joint_2d), joint_id);
    return entity;
}

Ref<FlecsEntity> Physics2DUtility::create_area_with_object(FlecsWorld *world, Area2D *area_2d) {
	if (area_2d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_area(world->get_world(), area_2d);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Area2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_rigid_body_with_object(FlecsWorld *world, RigidBody2D *rigid_body) {
	if (rigid_body == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_rigid_body(world->get_world(), rigid_body);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Body2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_physics_body_with_object(FlecsWorld *world, PhysicsBody2D *physics_body) {
	if (physics_body == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_physics_body(world->get_world(), physics_body);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Body2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_joint_with_object(FlecsWorld *world, Joint2D *joint_2d) {
	if (joint_2d == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_joint(world->get_world(), joint_2d);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Joint2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_area(FlecsWorld *world, const String &name, const RID &space_id) {
    flecs::entity e = _create_area(world->get_world(), name, space_id);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Area2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_body(FlecsWorld *world, const String &name, const RID &space_id) {
    flecs::entity e = _create_body(world->get_world(), name, space_id);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Body2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Physics2DUtility::create_joint(FlecsWorld *world, const String &name, const RID &space_id) {
    flecs::entity e = _create_joint(world->get_world(), name, space_id);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    Joint2DComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

void Physics2DUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), "create_area_with_object",
        &Physics2DUtility::create_area_with_object, "world", "area_2d");
    ClassDB::bind_static_method(get_class_static(), "create_rigid_body_with_object",
        &Physics2DUtility::create_rigid_body_with_object, "world", "rigid_body");
    ClassDB::bind_static_method(get_class_static(), "create_physics_body_with_object",
        &Physics2DUtility::create_physics_body_with_object, "world", "physics_body");
    ClassDB::bind_static_method(get_class_static(), "create_joint_with_object",
        &Physics2DUtility::create_joint_with_object, "world", "joint_2d");
    ClassDB::bind_static_method(get_class_static(), "create_area",
        &Physics2DUtility::create_area, "world", "name", "space_id");
    ClassDB::bind_static_method(get_class_static(), "create_body",
        &Physics2DUtility::create_body, "world", "name", "space_id");
    ClassDB::bind_static_method(get_class_static(), "create_joint",
        &Physics2DUtility::create_joint, "world", "name", "space_id");
}
