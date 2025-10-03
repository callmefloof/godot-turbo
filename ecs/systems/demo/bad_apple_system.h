#pragma once
#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/variant/variant.h"
#include "ecs/systems/commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"
#include "ecs/components/rendering/rendering_components.h"
#include <cassert>


enum BASMode {
    REGULAR = 0,
    INVERTED = 1,
    RANDOM = 2
};

class BadAppleSystem : public Object {
    GDCLASS(BadAppleSystem, Object);
    flecs::entity mm_entity;
    RID gd_mm_entity;
    RID world_id;
    flecs::world *world = nullptr;
    VideoStreamPlayer *video_player = nullptr;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;
    BASMode mode = BASMode::REGULAR;
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
    int get_mode() const { return static_cast<int>(mode); }
    void set_mode(int m) { ERR_FAIL_COND_MSG(m < 0 || m >= 3, "Invalid mode"); mode = static_cast<BASMode>(m); }
    static void _bind_methods();

};