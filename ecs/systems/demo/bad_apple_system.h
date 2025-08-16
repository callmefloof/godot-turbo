#include "ecs/systems/flecs_system.h"
#include "ecs/systems/commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"
#include "core/math/vector2i.h"

class FlecsEntity;

class BadAppleSystem : public FlecsSystem {
    GDCLASS(BadAppleSystem, FlecsSystem)
    flecs::entity mm_entity;
    FlecsEntity *gd_mm_entity = nullptr;
    VideoStreamPlayer *video_player = nullptr;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;

    public:
    

    BadAppleSystem() = default;
    ~BadAppleSystem() = default;
    void start();
    void set_mm_entity(FlecsEntity * p_mm_entity);
    void set_video_player(VideoStreamPlayer *p_video_player);
    VideoStreamPlayer *get_video_player() const;
    FlecsEntity* get_mm_entity() const;
    static void _bind_methods();

};