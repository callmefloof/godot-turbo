#include "render_system.h"


void RenderSystem::_bind_methods(){

}

Vector2i RenderSystem::get_window_size() const {
	return window_size;
}

void RenderSystem::set_window_size(const Vector2i &p_window_size) {
	this->window_size = p_window_size;
}