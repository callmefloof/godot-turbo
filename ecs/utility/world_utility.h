//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDUTILITY_H
#define WORLDUTILITY_H
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/templates/rid.h"
#include "../../../../scene/resources/3d/world_3d.h"
#include "../../../../scene/resources/world_2d.h"
#include "../../../../servers/navigation_server_2d.h"
#include "../../../../servers/physics_server_2d.h"
#include "../../../../servers/physics_server_3d.h"
#include "../../../../servers/rendering_server.h"
#include "../../thirdparty/flecs/distr/flecs.h"
#include "../components/component_module_base.h"
#include "../components/single_component_module.h"
#include "../components/worldcomponents.h"
#include "scene/resources/camera_attributes.h"

class World2DUtility {
	static inline void CreateWorld2D(const flecs::world &world) {
		if (world.has<World2DComponent>()) {
			return;
		}
		const auto canvas = RS::get_singleton()->canvas_create();
		const auto map =  NavigationServer2D::get_singleton()->map_create();
		const auto space = 	PhysicsServer2D::get_singleton()->space_create();

		world.set<World2DComponent>({  });
		world.get_mut<World2DComponent>().canvas_id = canvas;
		world.get_mut<World2DComponent>().navigation_map_id = map;
		world.get_mut<World2DComponent>().space_id = space;

	}

	static inline void CreateWorld2D(const flecs::world &world, const Ref<World2D> &world_2d) {
		if (world.has<World2DComponent>()) {
			World2DComponent& mut_ref = world.get_mut<World2DComponent>();
			mut_ref.navigation_map_id = world_2d->get_navigation_map();
			mut_ref.canvas_id = world_2d->get_canvas();
			mut_ref.space_id = world_2d->get_space();
			return;
		}
		if (!world_2d.is_valid() || world_2d.is_null()) {
			CreateWorld2D(world);
			return;
		}
		world.set<World2DComponent>({

		});
		world.get_mut<World2DComponent>().navigation_map_id = world_2d->get_navigation_map();
		world.get_mut<World2DComponent>().canvas_id = world_2d->get_canvas();
		world.get_mut<World2DComponent>().space_id = world_2d->get_space();
	}
};

class World3DUtility {
	static inline void CreateWorld3D(const flecs::world &world) {
		if (world.has<World3DComponent>()) {
			return;
		}
		world.set<World3DComponent>({
			RS::get_singleton()->camera_attributes_create(),
			RS::get_singleton()->environment_create(),
			RS::get_singleton()->environment_create(),
			NavigationServer3D::get_singleton()->map_create(),
			RS::get_singleton()->scenario_create(),
			PhysicsServer3D::get_singleton()->space_create()
		});
	}

	static inline void CreateWorld3D(const flecs::world &world, const Ref<World3D> &world_3d) {
		if (world.has<World3DComponent>()) {
			World3DComponent& mut_ref = world.get_mut<World3DComponent>();
			mut_ref.camera_attributes_id = world_3d->get_camera_attributes()->get_rid();
			mut_ref.environment_id = world_3d->get_environment()->get_rid();
			mut_ref.fallback_environment_id = world_3d->get_fallback_environment()->get_rid();
			mut_ref.navigation_map_id = world_3d->get_navigation_map();
			mut_ref.scenario_id = world_3d->get_scenario();
			mut_ref.space_id = world_3d->get_space();
			return;
		}
		if (!world_3d.is_valid() || world_3d.is_null()) {
			CreateWorld3D(world);
			return;
		}
		world.set<World3DComponent>({
			world_3d->get_camera_attributes()->get_rid(),
			world_3d->get_environment()->get_rid(),
			world_3d->get_fallback_environment()->get_rid(),
			world_3d->get_navigation_map(),
			world_3d->get_scenario(),
			world_3d->get_space()
		});
	}
};

#endif //WORLDUTILITY_H
