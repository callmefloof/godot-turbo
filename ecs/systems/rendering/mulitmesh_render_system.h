#pragma once

#include "../../../../../core/math/transform_3d.h"
#include "../../../../../core/templates/rid.h"
#include "../../../thirdparty/flecs/distr/flecs.h"

#include "../../../../../core/object/object.h"
#include "../../../../../core/string/ustring.h"
#include "../../components/visibility_component.h"
#include "occlusion/tile_occlusion_manager.h"

#include <vector>

class FlecsMultiMeshRenderSystem : public Object {
GDCLASS(FlecsMultiMeshRenderSystem, Object);
private:



protected:
	TileOcclusionManager tile_occlusion_manager = TileOcclusionManager();
	flecs::world world;
	flecs::entity main_camera;
	Vector2i window_size = DisplayServer::get_singleton()->window_get_size();
	float far_dist = 9999;
	void create_rendering(CommandQueue& command_queue) const;
	void create_frustum_culling(CommandQueue& command_queue) const;
	void create_occlusion_culling(CommandQueue& command_queue);
public:
	FlecsMultiMeshRenderSystem() = default;
	void create(CommandQueue &command_queue) const;
	~FlecsMultiMeshRenderSystem() override = default;
	flecs::world get_world() const { return world; }
	void set_world(const flecs::world &p_world) { world = p_world; }
	flecs::entity get_main_camera() const { return main_camera; }
	void set_main_camera(const flecs::entity &p_main_camera) { main_camera = p_main_camera; }
	float get_far_dist() const { return far_dist; }
	void set_far_dist(const float p_far_dist) { far_dist = p_far_dist; }
	static void _bind_methods();
	static FlecsMultiMeshRenderSystem* get_singleton();
	Vector2i get_window_size() const;
	void set_window_size(const Vector2i &window_size);
};
