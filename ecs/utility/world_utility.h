//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDUTILITY_H
#define WORLDUTILITY_H
#include "core/object/ref_counted.h"
#include "core/templates/rid.h"
#include "scene/resources/3d/world_3d.h"
#include "scene/resources/world_2d.h"
#include "servers/navigation_server_2d.h"
#include "servers/navigation_server_3d.h"
#include "servers/physics_server_2d.h"
#include "servers/physics_server_3d.h"
#include "servers/rendering_server.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "components/world_components.h"
#include "scene/resources/camera_attributes.h"
#include "ecs/flecs_types/flecs_server.h"


class World2DUtility : public Object {
	GDCLASS(World2DUtility, Object)
public:
	World2DUtility() = default;
	~World2DUtility() = default;
	// This class is a utility for creating world entities in the ECS world.
	static void _create_world_2d(const flecs::world *world) {
		if (world->has<World2DComponent>()) {
			return;
		}
		const auto canvas = RS::get_singleton()->canvas_create();
		const auto map =  NavigationServer2D::get_singleton()->map_create();
		const auto space = 	PhysicsServer2D::get_singleton()->space_create();

	World2DComponent w2c;
	w2c.canvas_id = canvas;
	w2c.navigation_map_id = map;
	w2c.space_id = space;
	world->set<World2DComponent>(w2c);

	}

	static void _create_world_2d(const flecs::world *world, const Ref<World2D> &world_2d) {
		if (world->has<World2DComponent>()) {
			World2DComponent& mut_ref = world->get_mut<World2DComponent>();
			mut_ref.navigation_map_id = world_2d->get_navigation_map();
			mut_ref.canvas_id = world_2d->get_canvas();
			mut_ref.space_id = world_2d->get_space();
			return;
		}
		if (!world_2d.is_valid() || world_2d.is_null()) {
			_create_world_2d(world);
			return;
		}
	World2DComponent w2c;
	w2c.navigation_map_id = world_2d->get_navigation_map();
	w2c.canvas_id = world_2d->get_canvas();
	w2c.space_id = world_2d->get_space();
	world->set<World2DComponent>(w2c);
	}

	static void create_world_2d(const RID& world_id, const Ref<World2D> &world_2d) {
		if (!world_id.is_valid()) {
			ERR_FAIL_COND(!world_id.is_valid());
		}
		flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);

		if (world->has<World2DComponent>()) {
			World2DComponent& mut_ref = world->get_mut<World2DComponent>();
			mut_ref.navigation_map_id = world_2d->get_navigation_map();
			mut_ref.canvas_id = world_2d->get_canvas();
			mut_ref.space_id = world_2d->get_space();
			world->modified<World2DComponent>();
			// No need to create a new entity, just update the existing one.
			return;
		}
		if (!world_2d.is_valid() || world_2d.is_null()) {
			_create_world_2d(world);
			return;
		}
	}

	static void _bind_methods(){
		ClassDB::bind_static_method(get_class_static(), "create_world_2d",
				&World2DUtility::create_world_2d, "world", "world_2d");
	}
};

class World3DUtility : public Object {
	GDCLASS(World3DUtility, Object)
public:
	World3DUtility() = default;
	~World3DUtility() = default;
	// This class is a utility for creating world entities in the ECS world.
	static void _create_world_3d(const flecs::world *world) {
		if (world->has<World3DComponent>()) {
			return;
		}
	World3DComponent w3c;
	w3c.camera_attributes_id = RS::get_singleton()->camera_attributes_create();
	w3c.environment_id = RS::get_singleton()->environment_create();
	w3c.fallback_environment_id = RS::get_singleton()->environment_create();
	w3c.navigation_map_id = NavigationServer3D::get_singleton()->map_create();
	w3c.scenario_id = RS::get_singleton()->scenario_create();
	w3c.space_id = PhysicsServer3D::get_singleton()->space_create();
	world->set<World3DComponent>(w3c);
	}

	static void _create_world_3d(const flecs::world *world, const Ref<World3D> &world_3d) {
		if (world->has<World3DComponent>()) {
			World3DComponent& mut_ref = world->get_mut<World3DComponent>();
			mut_ref.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
			mut_ref.environment_id = world_3d->get_environment()->get_rid();
			mut_ref.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
			mut_ref.navigation_map_id = world_3d->get_navigation_map();
			mut_ref.scenario_id = world_3d->get_scenario();
			mut_ref.space_id = world_3d->get_space();
			return;
		}
		if (!world_3d.is_valid() || world_3d.is_null()) {
			_create_world_3d(world);
			return;
		}
	World3DComponent w3c;
	w3c.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
	w3c.environment_id = world_3d->get_environment()->get_rid();
	w3c.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
	w3c.navigation_map_id = world_3d->get_navigation_map();
	w3c.scenario_id = world_3d->get_scenario();
	w3c.space_id = world_3d->get_space();
	world->set<World3DComponent>(w3c);
	}

	static void create_world_3d(const RID& world_id, const Ref<World3D> &world_3d) {
		if (!world_id.is_valid()) {
			ERR_FAIL_COND(!world_id.is_valid());
		}
		flecs::world *world = FlecsServer::get_singleton()->_get_world(world_id);

		if (world->has<World3DComponent>()) {
			World3DComponent& mut_ref = world->get_mut<World3DComponent>();
			mut_ref.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
			mut_ref.environment_id = world_3d->get_environment()->get_rid();
			mut_ref.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
			mut_ref.navigation_map_id = world_3d->get_navigation_map();
			mut_ref.scenario_id = world_3d->get_scenario();
			mut_ref.space_id = world_3d->get_space();
			world->modified<World3DComponent>();
			// No need to create a new entity, just update the existing one.
			return;
		}
		if (!world_3d.is_valid() || world_3d.is_null()) {
			_create_world_3d(world);
			return;
		}
	}

	static void _bind_methods(){
		ClassDB::bind_static_method(get_class_static(), "create_world_3d",
				&World3DUtility::create_world_3d, "world", "world_3d");
	}
};

#endif //WORLDUTILITY_H
