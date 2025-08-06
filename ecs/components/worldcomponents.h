//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDCOMPONENTS_H
#define WORLDCOMPONENTS_H
#include "single_component_module.h"
#include "core/templates/rid.h"
#include "core/object/ref_counted.h"
#include "core/os/memory.h"
#include "servers/rendering_server.h"
#include "servers/physics_server_2d.h"
#include "servers/physics_server_3d.h"
#include "servers/navigation_server_2d.h"
#include "servers/navigation_server_3d.h"
#include "component_proxy.h"
#include "../flecs_types/flecs_component.h"
#include "core/string/ustring.h"


struct World2DComponent {
	RID canvas_id;
	RID navigation_map_id;
	RID space_id;
	bool is_valid() const { return canvas_id.is_valid() && navigation_map_id.is_valid() && space_id.is_valid(); }
	bool is_null() const { return canvas_id.is_null() && navigation_map_id.is_null() && space_id.is_null(); }
	World2DComponent() = default;
	virtual ~World2DComponent() {
		RenderingServer::get_singleton()->free(canvas_id);
		NavigationServer2D::get_singleton()->free(navigation_map_id);
		PhysicsServer2D::get_singleton()->free(space_id);
	}
};

class World2DComponentRef : public FlecsComponent<World2DComponent> {
	#define WORLD_2D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, canvas_id,World2DComponent)\
	DEFINE_PROPERTY(RID, navigation_map_id,World2DComponent)\
	DEFINE_PROPERTY(RID, space_id,World2DComponent)\

	#define WORLD_2D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, canvas_id, World2DComponentRef)\
	BIND_PROPERTY(RID, navigation_map_id, World2DComponentRef)\
	BIND_PROPERTY(RID, space_id, World2DComponentRef)\

	DEFINE_COMPONENT_PROXY(World2DComponent,
	WORLD_2D_COMPONENT_PROPERTIES,
	WORLD_2D_COMPONENT_BINDINGS);
};

using World2DComponentModule = SingleComponentModule<World2DComponent>;

struct World3DComponent {
	RID camera_attributes_id;
	RID environment_id;
	RID fallback_environment_id;
	RID navigation_map_id;
	RID scenario_id;
	RID space_id;
	bool is_valid() const {
		return camera_attributes_id.is_valid() &&
		environment_id.is_valid() &&
		fallback_environment_id.is_valid() &&
		navigation_map_id.is_valid() &&
		scenario_id.is_valid() &&
		space_id.is_valid();
	}
	bool is_null() const {
		return camera_attributes_id.is_null() &&
		environment_id.is_null() &&
		fallback_environment_id.is_null() &&
		navigation_map_id.is_null() &&
		scenario_id.is_null() &&
		space_id.is_null();
	}
	World3DComponent()= default;
	~World3DComponent() {
		RenderingServer::get_singleton()->free(camera_attributes_id);
		RenderingServer::get_singleton()->free(environment_id);
		RenderingServer::get_singleton()->free(fallback_environment_id);
		NavigationServer3D::get_singleton()->free(navigation_map_id);
		RenderingServer::get_singleton()->free(scenario_id);
		PhysicsServer3D::get_singleton()->free(space_id);
	}
};

class World3DComponentRef : public FlecsComponent<World3DComponent> {
	#define WORLD_3D_COMPONENT_PROPERTIES\
	DEFINE_PROPERTY(RID, camera_attributes_id,World3DComponent)\
	DEFINE_PROPERTY(RID, environment_id,World3DComponent)\
	DEFINE_PROPERTY(RID, fallback_environment_id,World3DComponent)\
	DEFINE_PROPERTY(RID, navigation_map_id,World3DComponent)\
	DEFINE_PROPERTY(RID, scenario_id,World3DComponent)\
	DEFINE_PROPERTY(RID, space_id,World3DComponent)\


	#define WORLD_3D_COMPONENT_BINDINGS\
	BIND_PROPERTY(RID, camera_attributes_id, World3DComponentRef)\
	BIND_PROPERTY(RID, environment_id, World3DComponentRef)\
	BIND_PROPERTY(RID, fallback_environment_id, World3DComponentRef)\
	BIND_PROPERTY(RID, navigation_map_id, World3DComponentRef)\
	BIND_PROPERTY(RID, scenario_id, World3DComponentRef)\
	BIND_PROPERTY(RID, space_id, World3DComponentRef)\

	DEFINE_COMPONENT_PROXY(World3DComponent,
	WORLD_3D_COMPONENT_PROPERTIES,
	WORLD_3D_COMPONENT_BINDINGS);
};

using World3DComponentModule = SingleComponentModule<World3DComponent>;
#endif //WORLDCOMPONENTS_H
