#pragma once
#include "../../commands/command.h"
#include "../render_system.h"
#include "tile_occlusion_manager.h"
#include "../../pipeline_manager.h"

class OcclusionSystem : public RenderSystem {

protected:
	TileOcclusionManager tile_occlusion_manager;
public: 
    void create_occlusion_culling(Ref<CommandHandler>& command_handler, PipelineManager &pipeline_manager);
};