//
// Created by Floof on 12-7-2025.
//

#ifndef WORLDCOMPONENTS_H
#define WORLDCOMPONENTS_H
#include "single_component_module.h"
#include "../../../../core/templates/rid.h"
#include "../../../../servers/rendering_server.h"
#include "../../../../servers/physics_server_2d.h"
#include "../../../../servers/physics_server_3d.h"
#include "../../../../servers/navigation_server_2d.h"
#include "../../../../servers/navigation_server_3d.h"

struct World2DComponent : ScriptVisibleComponent {
	RID canvas_id;
	RID navigation_map_id;
	RID space_id;
	bool is_valid() const { return canvas_id.is_valid() && navigation_map_id.is_valid() && space_id.is_valid(); }
	bool is_null() const { return canvas_id.is_null() && navigation_map_id.is_null() && space_id.is_null(); }
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "canvas_id", canvas_id, Variant::RID);
		SET_SCRIPT_COMPONENT_VALUE(dict, "navigation_map_id", navigation_map_id, Variant::RID);
		SET_SCRIPT_COMPONENT_VALUE(dict, "space_id", space_id, Variant::RID);

	}
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["canvas_id"] = canvas_id;
		dict["navigation_map_id"] = navigation_map_id;
		dict["space_id"] = space_id;
		return dict;
	}
	virtual ~World2DComponent() {
		RenderingServer::get_singleton()->free(canvas_id);
		NavigationServer2D::get_singleton()->free(navigation_map_id);
		PhysicsServer2D::get_singleton()->free(space_id);
	}
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
	~World3DComponent() {
		RenderingServer::get_singleton()->free(camera_attributes_id);
		RenderingServer::get_singleton()->free(environment_id);
		RenderingServer::get_singleton()->free(fallback_environment_id);
		NavigationServer3D::get_singleton()->free(navigation_map_id);
		RenderingServer::get_singleton()->free(scenario_id);
		PhysicsServer3D::get_singleton()->free(space_id);
	}
};
using World3DComponentModule = SingleComponentModule<World3DComponent>;


#endif //WORLDCOMPONENTS_H
