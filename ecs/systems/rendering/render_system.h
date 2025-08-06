#pragma once
#include "../flecs_system.h"
#include "servers/display_server.h"


class RenderSystem : public FlecsSystem{
    GDCLASS(RenderSystem, FlecsSystem);
    protected:
        flecs::entity main_camera;
        float far_dist = 9999;
        Vector2i window_size = DisplayServer::get_singleton()->window_get_size();
    public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;
    static void _bind_methods();
    flecs::entity get_main_camera() const { return main_camera; }
	void set_main_camera(const flecs::entity &p_main_camera) { main_camera = p_main_camera; }
    float get_far_dist() const { return far_dist; }
	void set_far_dist(const float p_far_dist) { far_dist = p_far_dist; }
    Vector2i get_window_size() const;
    void set_window_size(const Vector2i &window); 
};
