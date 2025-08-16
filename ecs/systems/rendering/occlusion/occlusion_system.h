#pragma once
#include "../../commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "../render_system.h"
#include "tile_occlusion_manager.h"
#include "../../pipeline_manager.h"

class OcclusionSystem : public RenderSystem {
    GDCLASS(OcclusionSystem, RenderSystem); 
protected:
	TileOcclusionManager tile_occlusion_manager;
public: 
    void create_occlusion_culling(Ref<CommandHandler>& command_handler, PipelineManager &pipeline_manager);
};