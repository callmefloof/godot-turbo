#pragma once
#include "render_system.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "../pipeline_manager.h"

struct FrameCounter {
	int frame = 0;
};

class MultiMeshRenderSystem : public RenderSystem {
GDCLASS(MultiMeshRenderSystem, RenderSystem);
private:

protected:
	flecs::entity pipeline;
public:
	MultiMeshRenderSystem() = default;
	~MultiMeshRenderSystem() override = default;
	void create_rendering(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager) const;
	void create_frustum_culling(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager) const;
	static void _bind_methods();
	static MultiMeshRenderSystem* get_singleton();
};
