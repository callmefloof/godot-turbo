#include "navigation2d_utility.h"

flecs::entity Navigation2DUtility::_create_nav_2d_agent(const flecs::world *world, const RID &agent, const String &name) {
    return world->entity().set<NavAgent2DComponent>({ agent }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_agent(const flecs::world *world, const String &name) {
    const RID nav_agent_id = NavigationServer2D::get_singleton()->agent_create();
    return world->entity().set<NavAgent2DComponent>({ nav_agent_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_link(const flecs::world *world, const String &name) {
    const RID nav_link_id = NavigationServer2D::get_singleton()->link_create();
    return world->entity().set<NavLink2DComponent>({ nav_link_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_link(const flecs::world *world, const RID &link, const String &name) {
    return world->entity().set<NavLink2DComponent>({ link }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_obstacle(const flecs::world *world, const RID &obstacle, const String &name) {
    return world->entity().set<NavObstacle2DComponent>({ obstacle }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_obstacle(const flecs::world *world, const String &name) {
    const RID nav_obstacle_id = NavigationServer2D::get_singleton()->obstacle_create();
    return world->entity().set<NavObstacle2DComponent>({ nav_obstacle_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_region(const flecs::world *world, const RID &region, const String &name) {
    return world->entity().set<NavRegion2DComponent>({ region }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_region(const flecs::world *world, const String &name) {
    const RID nav_region_id = NavigationServer2D::get_singleton()->region_create();
    return world->entity().set<NavRegion2DComponent>({ nav_region_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_source_geometry_parser_2d(const flecs::world *world, const RID &source_geometry_parser, const String &name) {
    return world->entity().set<SourceGeometryParser2DComponent>({ source_geometry_parser }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_source_geometry_parser_2d(const flecs::world *world, const String &name) {
    const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
    return world->entity().set<SourceGeometryParser2DComponent>({ source_geometry_parser_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation2DUtility::_create_nav_2d_agent(const flecs::world *world, NavigationAgent2D *nav_agent) {
    if (nav_agent == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID agent_id = nav_agent->get_rid();
    if (!agent_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_agent->get_instance_id();
    NodeStorage::add(nav_agent, nav_agent->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<NavAgent2DComponent>({ agent_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_agent->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Navigation2DUtility::_create_nav_2d_link(const flecs::world *world, NavigationLink2D *nav_link) {
    if (nav_link == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID link_id = nav_link->get_rid();
    if (!link_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_link->get_instance_id();
    NodeStorage::add(nav_link, nav_link->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<NavLink2DComponent>({ link_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_link->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Navigation2DUtility::_create_nav_2d_obstacle(const flecs::world *world, NavigationObstacle2D *nav_obstacle) {
    if (nav_obstacle == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID obstacle_id = nav_obstacle->get_rid();
    if (!obstacle_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_obstacle->get_instance_id();
    NodeStorage::add(nav_obstacle, nav_obstacle->get_instance_id());
    const flecs::entity entity = world->entity()
                            .set<NavObstacle2DComponent>({ obstacle_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_obstacle->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Navigation2DUtility::_create_nav_2d_region(const flecs::world *world, NavigationRegion2D *nav_region) {
    if (nav_region == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID region_id = nav_region->get_rid();
    if (!region_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_region->get_instance_id();
    NodeStorage::add(nav_region, nav_region->get_instance_id());

    const flecs::entity entity = world->entity()
                            .set<NavRegion2DComponent>({ region_id })
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_region->get_name()).ascii().get_data());
    return entity;
}

flecs::entity Navigation2DUtility::_create_source_geometry_parser_2d(const flecs::world *world, const Callable &callable, const String &name) {
    const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
    if (!source_geometry_parser_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world->entity()
                            .set<SourceGeometryParser2DComponent>({ source_geometry_parser_id })
                            .set_name(name.ascii().get_data());
    NavigationServer2D::get_singleton()->source_geometry_parser_set_callback(
            source_geometry_parser_id,
            callable);
    return entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_source_geometry_parser(FlecsWorld *world, const Callable &callable, const String &name) {
    const flecs::entity entity = _create_source_geometry_parser_2d(world->get_world_ref(), callable, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    SourceGeometryParser2DComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_agent(FlecsWorld *world, const RID &agent, const String &name) {
    const flecs::entity entity = _create_nav_2d_agent(world->get_world_ref(), agent, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavAgent2DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_link(FlecsWorld *world, const RID &link, const String &name) {
    const flecs::entity entity = _create_nav_2d_link(world->get_world_ref(), link, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavLink2DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_obstacle(FlecsWorld *world, const RID &obstacle, const String &name) {
    const flecs::entity entity = _create_nav_2d_obstacle(world->get_world_ref(), obstacle, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavObstacle2DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);

    return ref_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_region(FlecsWorld *world, const RID &region, const String &name) {
    const flecs::entity entity = _create_nav_2d_region(world->get_world_ref(), region, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavRegion2DComponentRef::create_component(ref_entity);
    ObjectInstanceComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_agent_with_object(FlecsWorld *world, NavigationAgent2D *nav_agent) {
    if (nav_agent == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_nav_2d_agent(world->get_world_ref(), nav_agent);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    NavAgent2DComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_link_with_object(FlecsWorld *world, NavigationLink2D *nav_link) {
    if (nav_link == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_nav_2d_link(world->get_world_ref(), nav_link);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    NavLink2DComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_obstacle_with_object(FlecsWorld *world, NavigationObstacle2D *nav_obstacle) {
    if (nav_obstacle == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_nav_2d_obstacle(world->get_world_ref(), nav_obstacle);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    NavObstacle2DComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

Ref<FlecsEntity> Navigation2DUtility::create_nav_region_with_object(FlecsWorld *world, NavigationRegion2D *nav_region) {
    if (nav_region == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    flecs::entity e = _create_nav_2d_region(world->get_world_ref(), nav_region);
    Ref<FlecsEntity> flecs_entity = world->add_entity(e);
    NavRegion2DComponentRef::create_component(flecs_entity);
    ObjectInstanceComponentRef::create_component(flecs_entity);
    return flecs_entity;
}

void Navigation2DUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), "create_nav_agent",
            &Navigation2DUtility::create_nav_agent, "world", "agent", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_agent_with_object",
            &Navigation2DUtility::create_nav_agent_with_object, "world", "nav_agent");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link",  
            &Navigation2DUtility::create_nav_link, "world", "link", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link_with_object",
            &Navigation2DUtility::create_nav_link_with_object, "world", "nav_link");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle",
            &Navigation2DUtility::create_nav_obstacle, "world", "obstacle", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle_with_object",
            &Navigation2DUtility::create_nav_obstacle_with_object, "world", "nav_obstacle");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region",
            &Navigation2DUtility::create_nav_region, "world", "region", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region_with_object",
            &Navigation2DUtility::create_nav_region_with_object, "world", "nav_region");
    ClassDB::bind_static_method(get_class_static(), "create_source_geometry_parser",
            &Navigation2DUtility::create_source_geometry_parser, "world", "callable", "name");
}