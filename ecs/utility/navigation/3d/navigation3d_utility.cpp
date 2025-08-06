#include "navigation3d_utility.h"
#include "core/object/class_db.h"

flecs::entity Navigation3DUtility::_create_nav_agent(const flecs::world &world, const RID &agent, const String &name)  {
    return world.entity().set<NavAgent3DComponent>({ agent }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_agent(const flecs::world &world, const String &name)  {
    const RID nav_agent_id = NavigationServer3D::get_singleton()->agent_create();
    return world.entity().set<NavAgent3DComponent>({ nav_agent_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_link(const flecs::world &world, const String &name)  {
    const RID nav_link_id = NavigationServer3D::get_singleton()->link_create();
    return world.entity().set<NavLink3DComponent>({ nav_link_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_link(const flecs::world &world, const RID &link, const String &name) {
    return world.entity().set<NavLink3DComponent>({ link }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_obstacle(const flecs::world &world, const RID &obstacle, const String &name) {
    return world.entity().set<NavObstacle3DComponent>({ obstacle }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_obstacle(const flecs::world &world, const String &name)  {
    const RID nav_obstacle_id = NavigationServer3D::get_singleton()->obstacle_create();
    return world.entity().set<NavObstacle3DComponent>({ nav_obstacle_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_region(const flecs::world &world, const RID &region, const String &name) {
    return world.entity().set<NavRegion3DComponent>({ region }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_region(const flecs::world &world, const String &name)  {
    const RID nav_region_id = NavigationServer3D::get_singleton()->region_create();
    return world.entity().set<NavRegion3DComponent>({ nav_region_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_source_geometry_parser(const flecs::world &world, const RID &source_geometry_parser, const String &name)  {
    return world.entity().set<SourceGeometryParser3DComponent>({ source_geometry_parser }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_source_geometry_parser(const flecs::world &world, const String &name)  {
    const RID source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
    return world.entity().set<SourceGeometryParser3DComponent>({ source_geometry_parser_id }).set_name(name.ascii().get_data());
}

flecs::entity Navigation3DUtility::_create_nav_agent(const flecs::world &world, NavigationAgent3D *nav_agent) {
    if (nav_agent == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID agent_id = nav_agent->get_rid();
    if (!agent_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<NavAgent3DComponent>({ agent_id })
                            .set_name(String(nav_agent->get_name()).ascii().get_data());
    ObjectIDStorage::add(nav_agent, agent_id);
    return entity;
}

flecs::entity Navigation3DUtility::_create_nav_link(const flecs::world &world, NavigationLink3D *nav_link) {
    if (nav_link == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID link_id = nav_link->get_rid();
    if (!link_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    auto entity = world.entity()
                            .set<NavLink3DComponent>({ link_id })
                            .set_name(String(nav_link->get_name()).ascii().get_data());

    ObjectIDStorage::add(dynamic_cast<Object*>(nav_link), link_id);
    return entity;
}

flecs::entity Navigation3DUtility::_create_nav_obstacle(const flecs::world &world, NavigationObstacle3D *nav_obstacle) {
    if (nav_obstacle == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID obstacle_id = nav_obstacle->get_rid();
    if (!obstacle_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<NavObstacle3DComponent>({ obstacle_id })
                            .set_name(String(nav_obstacle->get_name()).ascii().get_data());
    ObjectIDStorage::add(dynamic_cast<Object*>(nav_obstacle), obstacle_id);
    return entity;
}

flecs::entity Navigation3DUtility::_create_nav_region(const flecs::world &world, NavigationRegion3D *nav_region) {
    if (nav_region == nullptr) {
        ERR_FAIL_V(flecs::entity());
    }
    const RID region_id = nav_region->get_rid();
    if (!region_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<NavRegion3DComponent>({ region_id })
                            .set_name(String(nav_region->get_name()).ascii().get_data());

    ObjectIDStorage::add(dynamic_cast<Object*>(nav_region), region_id);
    return entity;
}

flecs::entity Navigation3DUtility::_create_source_geometry_parser(const flecs::world &world, const Callable &callable, const String &name)  {
    const RID source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
    if (!source_geometry_parser_id.is_valid()) {
        ERR_FAIL_V(flecs::entity());
    }
    const flecs::entity entity = world.entity()
                            .set<SourceGeometryParser3DComponent>({ source_geometry_parser_id })
                            .set_name(name.ascii().get_data());
    NavigationServer3D::get_singleton()->source_geometry_parser_set_callback(
            source_geometry_parser_id,
            callable);
    return entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_agent_with_object(FlecsWorld *world, NavigationAgent3D *nav_agent) {
    if (nav_agent == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const Ref<FlecsEntity> entity = world->add_entity(_create_nav_agent(world->get_world(), nav_agent));
    NavAgent3DComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_link_with_object(FlecsWorld *world, NavigationLink3D *nav_link) {
    if (nav_link == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const Ref<FlecsEntity> entity = world->add_entity(_create_nav_link(world->get_world(), nav_link));
    NavLink3DComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_obstacle_with_object(FlecsWorld *world, NavigationObstacle3D *nav_obstacle) {
    if (nav_obstacle == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const Ref<FlecsEntity> entity = world->add_entity(_create_nav_obstacle(world->get_world(), nav_obstacle));
    NavObstacle3DComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_region_with_object(FlecsWorld *world, NavigationRegion3D *nav_region) {
    if (nav_region == nullptr) {
        ERR_FAIL_V(Ref<FlecsEntity>());
    }
    const Ref<FlecsEntity> entity = world->add_entity(_create_nav_region(world->get_world(), nav_region));
    NavRegion3DComponentRef::create_component(entity);
    return entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_source_geometry_parser(FlecsWorld *world, const Callable &callable, const String &name) {
    const flecs::entity entity = _create_source_geometry_parser(world->get_world(), callable, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    SourceGeometryParser3DComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_agent(FlecsWorld *world, const RID &agent, const String &name) {
    const flecs::entity entity = _create_nav_agent(world->get_world(), agent, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavAgent3DComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_link(FlecsWorld *world, const RID &link, const String &name) {
    const flecs::entity entity = _create_nav_link(world->get_world(), link, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavLink3DComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_obstacle(FlecsWorld *world, const RID &obstacle, const String &name) {
    const flecs::entity entity = _create_nav_obstacle(world->get_world(), obstacle, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavObstacle3DComponentRef::create_component(ref_entity);
    return ref_entity;
}

Ref<FlecsEntity> Navigation3DUtility::create_nav_region(FlecsWorld *world, const RID &region, const String &name) {
    const flecs::entity entity = _create_nav_region(world->get_world(), region, name);
    Ref<FlecsEntity> ref_entity = world->add_entity(entity);
    NavRegion3DComponentRef::create_component(ref_entity);
    return ref_entity;
}

void Navigation3DUtility::_bind_methods() {
   ClassDB::bind_static_method(get_class_static(), "create_nav_agent",
            &Navigation3DUtility::create_nav_agent, "world", "agent", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_agent_with_object",
            &Navigation3DUtility::create_nav_agent_with_object, "world", "nav_agent");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link",
            &Navigation3DUtility::create_nav_link, "world", "link", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link_with_object",  
            &Navigation3DUtility::create_nav_link_with_object, "world", "nav_link");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle",
            &Navigation3DUtility::create_nav_obstacle, "world", "obstacle", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle_with_object",
            &Navigation3DUtility::create_nav_obstacle_with_object, "world", "nav_obstacle");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region",
            &Navigation3DUtility::create_nav_region, "world", "region", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region_with_object",
            &Navigation3DUtility::create_nav_region_with_object, "world", "nav_region");
    ClassDB::bind_static_method(get_class_static(), "create_source_geometry_parser",
            &Navigation3DUtility::create_source_geometry_parser, "world", "callable", "name");

    }