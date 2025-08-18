#pragma once
#include "flecs.h"
#include "core/math/vector2i.h"

class RenderSystem {

    protected:
        flecs::entity main_camera;
        float far_dist = 9999;
        flecs::world* world = nullptr;

    public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;
    flecs::entity get_main_camera() const { return main_camera; }
	void set_main_camera(const flecs::entity &p_main_camera) { main_camera = p_main_camera; }
    float get_far_dist() const { return far_dist; }
	void set_far_dist(const float p_far_dist) { far_dist = p_far_dist; }
    Vector2i get_window_size() const;
    void set_window_size(const Vector2i &window); 
    flecs::world* get_world() const;
	void set_world(flecs::world *p_world);
};
