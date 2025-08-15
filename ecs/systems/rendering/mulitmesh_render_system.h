#pragma once
#include "render_system.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "../pipeline_manager.h"

class MultiMeshRenderSystem : public RenderSystem {
GDCLASS(MultiMeshRenderSystem, RenderSystem);
private:

protected:
	flecs::entity pipeline;
public:
	MultiMeshRenderSystem() = default;
	~MultiMeshRenderSystem() override = default;
	void create_rendering(CommandQueue& command_queue, PipelineManager& pipeline_manager) const;
	void create_frustum_culling(CommandQueue& command_queue, PipelineManager& pipeline_manager) const;
	static void _bind_methods();
	static MultiMeshRenderSystem* get_singleton();
};
