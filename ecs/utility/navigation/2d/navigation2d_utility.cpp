#include "navigation2d_utility.h"
#include "core/object/class_db.h"
#include "ecs/components/navigation/2d/2d_navigation_components.h"
#include "ecs/components/object_instance_component.h"
#include "servers/navigation_server_2d.h"
#include "ecs/flecs_types/flecs_server.h"

RID Navigation2DUtility::create_nav_agent_with_id(const RID &world_id, const RID &agent, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavAgent2DComponent navigation_2d_component;
    navigation_2d_component.agent_id = agent;
    flecs::entity e = world->entity().set<NavAgent2DComponent>(navigation_2d_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_agent(const RID &world_id, const String &name) {
    const RID nav_agent_id = NavigationServer2D::get_singleton()->agent_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavAgent2DComponent navigation_2d_component;
    navigation_2d_component.agent_id = nav_agent_id;
    flecs::entity e = world->entity().set<NavAgent2DComponent>(navigation_2d_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_link(const RID &world_id, const String &name) {
    const RID nav_link_id = NavigationServer2D::get_singleton()->link_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavLink2DComponent link_component;
    link_component.link_id = nav_link_id;
    flecs::entity e = world->entity().set<NavLink2DComponent>(link_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_link_with_id(const RID &world_id, const RID &link, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavLink2DComponent link_component;
    link_component.link_id = link;
    flecs::entity e = world->entity().set<NavLink2DComponent>(link_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_obstacle_with_id(const RID &world_id, const RID &obstacle, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavObstacle2DComponent obstacle_component;
    obstacle_component.obstacle_id = obstacle;
    flecs::entity e = world->entity().set<NavObstacle2DComponent>(obstacle_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_obstacle(const RID &world_id, const String &name) {
    const RID nav_obstacle_id = NavigationServer2D::get_singleton()->obstacle_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavObstacle2DComponent obstacle_component;
    obstacle_component.obstacle_id = nav_obstacle_id;
    flecs::entity e = world->entity().set<NavObstacle2DComponent>(obstacle_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_region_with_id(const RID &world_id, const RID &region, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavRegion2DComponent region_component;
    region_component.region_id = region;
    flecs::entity e = world->entity().set<NavRegion2DComponent>(region_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_region(const RID &world_id, const String &name) {
    const RID nav_region_id = NavigationServer2D::get_singleton()->region_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    NavRegion2DComponent region_component;
    region_component.region_id = nav_region_id;
    flecs::entity e = world->entity().set<NavRegion2DComponent>(region_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}


RID Navigation2DUtility::create_sg_parser_with_id(const RID &world_id, const RID &source_geometry_parser, const String &name) {
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser2DComponent parser_component;
    parser_component.source_geometry_parser_id = source_geometry_parser;
    flecs::entity e = world->entity().set<SourceGeometryParser2DComponent>(parser_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_source_geometry_parser(const RID &world_id, const String &name) {
    const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser2DComponent parser_component;
    parser_component.source_geometry_parser_id = source_geometry_parser_id;
    flecs::entity e = world->entity().set<SourceGeometryParser2DComponent>(parser_component).set_name(name.ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_agent_with_object(const RID &world_id, NavigationAgent2D *nav_agent) {
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
    NavAgent2DComponent agent_component;
    agent_component.agent_id = agent_id;
    flecs::entity e = world->entity()
                            .set<NavAgent2DComponent>(agent_component)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_agent->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_link_with_object(const RID &world_id, NavigationLink2D *nav_link) {
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
    NavLink2DComponent link_component;
    link_component.link_id = link_id;
    flecs::entity e = world->entity()
                            .set<NavLink2DComponent>(link_component)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_link->get_name()).ascii().get_data());

    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_obstacle_with_object(const RID &world_id, NavigationObstacle2D *nav_obstacle) {
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
    NavObstacle2DComponent obstacle_component;
    obstacle_component.obstacle_id = obstacle_id;
    flecs::entity e = world->entity()
                            .set<NavObstacle2DComponent>(obstacle_component)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_obstacle->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_nav_region_with_object(const RID &world_id, NavigationRegion2D *nav_region) {
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
    NavRegion2DComponent region_component;
    region_component.region_id = region_id;
    flecs::entity e = world->entity()
                            .set<NavRegion2DComponent>(region_component)
                            .set<ObjectInstanceComponent>(object_instance_component)
                            .set_name(String(nav_region->get_name()).ascii().get_data());
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}

RID Navigation2DUtility::create_sg_parser_with_callable(const RID &world_id, const Callable &callable, const String &name) {
    const RID source_geometry_parser_id = NavigationServer2D::get_singleton()->source_geometry_parser_create();
    if (!source_geometry_parser_id.is_valid()) {
        ERR_FAIL_V(RID());
    }
    flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);
    SourceGeometryParser2DComponent parser_component;
    parser_component.source_geometry_parser_id = source_geometry_parser_id;
    flecs::entity e = world->entity()
                            .set<SourceGeometryParser2DComponent>(parser_component)
                            .set_name(name.ascii().get_data());
    NavigationServer2D::get_singleton()->source_geometry_parser_set_callback(
            source_geometry_parser_id,
            callable);
    return FlecsServer::get_singleton()->_create_rid_for_entity(world_id, e);
}


void Navigation2DUtility::_bind_methods() {
    

    ClassDB::bind_static_method(get_class_static(), "create_nav_agent",
            &Navigation2DUtility::create_nav_agent, "world", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_agent_with_id",
            &Navigation2DUtility::create_nav_agent_with_id, "world", "agent_id", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_agent_with_object",
            &Navigation2DUtility::create_nav_agent_with_object, "world", "nav_agent");

    ClassDB::bind_static_method(get_class_static(), "create_nav_link",
            &Navigation2DUtility::create_nav_link, "world", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link_with_id",
            &Navigation2DUtility::create_nav_link_with_id, "world", "link_id", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_link_with_object",
            &Navigation2DUtility::create_nav_link_with_object, "world", "nav_link");

    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle",
            &Navigation2DUtility::create_nav_obstacle, "world", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle_with_id",
            &Navigation2DUtility::create_nav_obstacle_with_id, "world", "obstacle_id", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_obstacle_with_object",
            &Navigation2DUtility::create_nav_obstacle_with_object, "world", "nav_obstacle");

    ClassDB::bind_static_method(get_class_static(), "create_nav_region",
            &Navigation2DUtility::create_nav_region, "world", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region_with_id",
            &Navigation2DUtility::create_nav_region_with_id, "world", "region_id", "name");
    ClassDB::bind_static_method(get_class_static(), "create_nav_region_with_object",
            &Navigation2DUtility::create_nav_region_with_object, "world", "nav_region");

    ClassDB::bind_static_method(get_class_static(), "create_source_geometry_parser",
            &Navigation2DUtility::create_source_geometry_parser, "world", "name");
    ClassDB::bind_static_method(get_class_static(), "create_sg_parser_with_callable",
        &Navigation2DUtility::create_sg_parser_with_callable, "world", "callable", "name");
    ClassDB::bind_static_method(get_class_static(), "create_sg_parser_with_id",
        &Navigation2DUtility::create_sg_parser_with_id, "world", "name");

}