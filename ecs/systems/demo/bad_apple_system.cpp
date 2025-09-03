#include "bad_apple_system.h"
#include "flecs_server.h"
#include "scene/resources/texture.h"
#include "ecs/components/rendering/rendering_components.h"
#include "servers/rendering_server.h"
#include "core/math/vector2i.h"
#include "systems/pipeline_manager.h"
#include <optional>


void BadAppleSystem::start() {
    if(!world) {
        ERR_PRINT_ONCE("World is not set for BadAppleSystem.");
        return;
    }
    if (!video_player) {
        ERR_PRINT_ONCE("Video player is not set for BadAppleSystem.");
        return;
    }
    if (!mm_entity.is_valid()) {
        ERR_PRINT_ONCE("MM entity is not set for BadAppleSystem.");
        return;
    }
    if(!mm_entity.has<MultiMeshComponent>()) {
        ERR_PRINT_ONCE("MM entity does not have MultiMeshComponent for BadAppleSystem.");
        return;
    }
    if(!command_handler.is_valid()) {
        ERR_PRINT_ONCE("CommandHandler is not set for BadAppleSystem.");
        return;
    }
    if(!pipeline_manager) {
        ERR_PRINT_ONCE("PipelineManager is not set for BadAppleSystem.");
        return;
    }
    auto bad_apple_system = world->system<const MultiMeshInstanceComponent>()
        .with(flecs::ChildOf, mm_entity)
        .multi_threaded()
        .each([&](flecs::entity e, const MultiMeshInstanceComponent &mm_instance) {
            if (!video_player) {
                ERR_PRINT_ONCE("Video player is not set for BadAppleSystem.");
            }
            if(!video_player->has_autoplay() && !video_player->is_playing()) {
                return;
            }
            Ref<Texture2D> texture = video_player->get_video_texture();
            if (!texture.is_valid()) {
                ERR_PRINT_ONCE("Video player texture is not valid.");
                return;
            }
            if (texture->get_width() == 0 || texture->get_height() == 0) {
                ERR_PRINT_ONCE("Video player texture has invalid dimensions.");
                return;
            }
            Ref<Image> image = texture->get_image();
            if (!image.is_valid()) {
                ERR_PRINT_ONCE("Video player image is not valid.");
                return;
            }
            if (image->get_width() == 0 || image->get_height() == 0) {
                ERR_PRINT_ONCE("Video player image has invalid dimensions.");
                return;
            }

            const int width = image->get_width();
            const int height = image->get_height();
            const int x = mm_instance.index % width;
            const int y = height - 1 - (mm_instance.index / width);
            Color color = image->get_pixel(x,y);
            const MultiMeshComponent* mm_comp = mm_entity.try_get<MultiMeshComponent>();
            if(!mm_comp) {
                ERR_PRINT_ONCE("MultiMeshComponent is not set for BadAppleSystem.");
                return;
            }
            if(color.r == color.g && color.g == color.b && Math::is_equal_approx(color.b, 0)) {
                command_handler->enqueue_command([=]() {
                    RS::get_singleton()->multimesh_instance_set_color(mm_comp->multi_mesh_id,mm_instance.index, Color(1, 1, 1, 1));
                });
                return;
            }else{
                command_handler->enqueue_command([=]() {
                    RS::get_singleton()->multimesh_instance_set_color(mm_comp->multi_mesh_id,mm_instance.index, Color(0, 0, 0, 1));
                });
                return;
            }
        });
}

RID BadAppleSystem::get_mm_entity() const {
    return gd_mm_entity;
}

void BadAppleSystem::set_mm_entity(const RID& mm_entity) {
    gd_mm_entity = mm_entity;
}

void BadAppleSystem::set_video_player(VideoStreamPlayer *p_video_player) {
    video_player = p_video_player;
}

VideoStreamPlayer *BadAppleSystem::get_video_player() const {
	return video_player;
}

void BadAppleSystem::set_world_id(const RID& p_world_id) {
    world_id = p_world_id;
    world = FlecsServer::get_singleton()->_get_world(world_id);
    std::optional<PipelineManager> pipeline_opt = FlecsServer::get_singleton()->_get_pipeline_manager(world_id);
    pipeline_manager = pipeline_opt ? &(*pipeline_opt) : nullptr;
    command_handler = FlecsServer::get_singleton()->get_render_system_command_handler(world_id);
}

RID BadAppleSystem::get_world_id() const {
    return world_id;
}