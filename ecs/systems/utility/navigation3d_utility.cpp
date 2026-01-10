#include "navigation3d_utility.h"
#include "core/object/class_db.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "servers/navigation_server_3d.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

RID Navigation3DUtility::create_nav_agent_with_id(const RID &world_id, const RID &agent, const String &name)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavAgent3DComponent comp;
    comp.agent_id = agent;
    flecs::entity e = world->entity().set<NavAgent3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_agent(const RID &world_id, const String &name)  {
    const RID nav_agent_id = NavigationServer3D::get_singleton()->agent_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavAgent3DComponent comp;
    comp.agent_id = nav_agent_id;
    flecs::entity e = world->entity().set<NavAgent3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_link(const RID &world_id, const String &name)  {
    const RID nav_link_id = NavigationServer3D::get_singleton()->link_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavLink3DComponent comp;
    comp.link_id = nav_link_id;
    flecs::entity e = world->entity().set<NavLink3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_link_with_id(const RID &world_id, const RID &link, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavLink3DComponent comp;
    comp.link_id = link;
    flecs::entity e = world->entity().set<NavLink3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_obstacle_with_id(const RID &world_id, const RID &obstacle, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavObstacle3DComponent comp;
    comp.obstacle_id = obstacle;
    flecs::entity e = world->entity().set<NavObstacle3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_obstacle(const RID &world_id, const String &name)  {
    const RID nav_obstacle_id = NavigationServer3D::get_singleton()->obstacle_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavObstacle3DComponent comp;
    comp.obstacle_id = nav_obstacle_id;
    flecs::entity e = world->entity().set<NavObstacle3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_region_with_id(const RID &world_id, const RID &region, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavRegion3DComponent comp;
    comp.region_id = region;
    flecs::entity e = world->entity().set<NavRegion3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_region(const RID &world_id, const String &name)  {
    const RID nav_region_id = NavigationServer3D::get_singleton()->region_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavRegion3DComponent comp;
    comp.region_id = nav_region_id;
    flecs::entity e = world->entity().set<NavRegion3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_sgp_with_id(const RID &world_id, const RID &source_geometry_parser, const String &name)  {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser3DComponent comp;
    comp.source_geometry_parser_id = source_geometry_parser;
    flecs::entity e = world->entity().set<SourceGeometryParser3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_source_geometry_parser(const RID &world_id, const String &name)  {
    const RID source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser3DComponent comp;
    comp.source_geometry_parser_id = source_geometry_parser_id;
    flecs::entity e = world->entity().set<SourceGeometryParser3DComponent>(comp).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_agent_with_object(const RID &world_id, NavigationAgent3D *nav_agent) {
    if (nav_agent == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID agent_id = nav_agent->get_rid();
    if (!agent_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_agent->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(nav_agent, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavAgent3DComponent comp;
    comp.agent_id = agent_id;
    flecs::entity e = world->entity()
                            .set<NavAgent3DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_agent->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_link_with_object(const RID &world_id, NavigationLink3D *nav_link) {
    if (nav_link == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID link_id = nav_link->get_rid();
    if (!link_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_link->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(nav_link, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavLink3DComponent comp;
    comp.link_id = link_id;
    flecs::entity e = world->entity()
                            .set<NavLink3DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_link->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_obstacle_with_object(const RID &world_id, NavigationObstacle3D *nav_obstacle) {
    if (nav_obstacle == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID obstacle_id = nav_obstacle->get_rid();
    if (!obstacle_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_obstacle->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(nav_obstacle, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavObstacle3DComponent comp;
    comp.obstacle_id = obstacle_id;
    flecs::entity e = world->entity()
                            .set<NavObstacle3DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_obstacle->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_nav_region_with_object(const RID &world_id, NavigationRegion3D *nav_region) {
    if (nav_region == nullptr) {
        ERR_FAIL_V(RID());
    }
    const RID region_id = nav_region->get_rid();
    if (!region_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    ObjectInstanceComponent object_instance_component;
    object_instance_component.object_instance_id = nav_region->get_instance_id();
    FlecsServer::get_singleton()->add_to_node_storage(nav_region, world_id);
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavRegion3DComponent comp;
    comp.region_id = region_id;
    flecs::entity e = world->entity()
                            .set<NavRegion3DComponent>(comp)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_region->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation3DUtility::create_sgp_with_callable(const RID &world_id, const Callable &callable, const String &name)  {
    const RID source_geometry_parser_id = NavigationServer3D::get_singleton()->source_geometry_parser_create();
    if (!source_geometry_parser_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser3DComponent comp;
    comp.source_geometry_parser_id = source_geometry_parser_id;
    flecs::entity e = world->entity().set<SourceGeometryParser3DComponent>(comp).set_name(name.ascii().get_data());
    NavigationServer3D::get_singleton()->source_geometry_parser_set_callback(
            source_geometry_parser_id,
            callable);
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

void Navigation3DUtility::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_agent", "world_id", "name"),
            &Navigation3DUtility::create_nav_agent);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_agent_with_id", "world_id", "agent_id", "name"),
            &Navigation3DUtility::create_nav_agent_with_id);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_agent_with_object", "world_id", "nav_agent"),
            &Navigation3DUtility::create_nav_agent_with_object);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_link", "world_id", "name"),
            &Navigation3DUtility::create_nav_link);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_link_with_id", "world_id", "link_id", "name"),
            &Navigation3DUtility::create_nav_link_with_id);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_link_with_object", "world_id", "nav_link"),
            &Navigation3DUtility::create_nav_link_with_object);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_obstacle", "world_id", "name"),
            &Navigation3DUtility::create_nav_obstacle);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_obstacle_with_id", "world_id", "obstacle_id", "name"),
            &Navigation3DUtility::create_nav_obstacle_with_id);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_obstacle_with_object", "world_id", "nav_obstacle"),
            &Navigation3DUtility::create_nav_obstacle_with_object);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_region", "world_id", "name"),
            &Navigation3DUtility::create_nav_region);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_region_with_id", "world_id", "region_id", "name"),
            &Navigation3DUtility::create_nav_region_with_id);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_nav_region_with_object", "world_id", "nav_region"),
            &Navigation3DUtility::create_nav_region_with_object);

    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_sgp_with_id", "world_id", "source_geometry_parser_id", "name"),
            &Navigation3DUtility::create_sgp_with_id);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_sgp_with_callable", "world_id", "callable", "name"),
            &Navigation3DUtility::create_sgp_with_callable);
}
