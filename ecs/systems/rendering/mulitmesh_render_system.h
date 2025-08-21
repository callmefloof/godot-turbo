#pragma once
#include "render_system.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "../pipeline_manager.h"
#include "ecs/systems/commands/command.h"

struct FrameCounter {
	int frame = 0;
};

class MultiMeshRenderSystem : public RenderSystem {
protected:
	flecs::entity pipeline;
public:
	MultiMeshRenderSystem() = default;
	MultiMeshRenderSystem(flecs::world *p_world) {
		world = p_world;
		pipeline = world->get_pipeline();
	}
	~MultiMeshRenderSystem() override = default;
	void create_rendering(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager);
	void create_frustum_culling(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager);
};
