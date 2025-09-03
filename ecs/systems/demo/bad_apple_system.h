#pragma once
#include "ecs/systems/commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"


class BadAppleSystem : public Object {
    GDCLASS(BadAppleSystem, Object);
    flecs::entity mm_entity;
    RID gd_mm_entity;
    RID world_id;
    flecs::world *world = nullptr;
    VideoStreamPlayer *video_player = nullptr;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;

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

};