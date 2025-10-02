#pragma once
#include "core/variant/variant.h"
#include "ecs/systems/commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"
#include "ecs/components/rendering/rendering_components.h"


class BadAppleSystem : public Object {
    GDCLASS(BadAppleSystem, Object);
    flecs::entity mm_entity;
    RID gd_mm_entity;
    RID world_id;
    flecs::world *world = nullptr;
    VideoStreamPlayer *video_player = nullptr;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;

    struct ImageData {
        PackedByteArray data;
        int width = 0;
        int height = 0;
        Image::Format format = Image::FORMAT_MAX;
    } image_data;
    MultiMeshComponent component;

    public:
    

    BadAppleSystem() = default;
    ~BadAppleSystem() = default;
    void start();
    void set_mm_entity(const RID& mm_entity);
    void set_video_player(VideoStreamPlayer *p_video_player);
    VideoStreamPlayer *get_video_player() const;
    RID get_mm_entity() const;
    void set_world_id(const RID& p_world_id);
    RID get_world_id() const;
    Color _get_color_at_ofs(const Image::Format format, const uint8_t *ptr, uint32_t ofs) const;
    Color get_pixel(const ImageData& image_data, const int x, const int y) const;

    static void _bind_methods();

};