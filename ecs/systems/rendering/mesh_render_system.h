#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/rendering/rendering_components.h"
#include "servers/rendering_server.h"
#include "render_system.h"
#include "../commands/command.h"
#include "../pipeline_manager.h"
#include "ecs/components/visibility_component.h"

class MeshRenderSystem : public RenderSystem
{


public:
	MeshRenderSystem() = default;
	~MeshRenderSystem() override = default;
	float far_dist = 9999;
	void create_mesh_render_system(CommandQueue& command_queue, PipelineManager& pipeline_manager) const {
		flecs::system mesh_render_system = world.system<const RenderInstanceComponent, const Transform3DComponent, const VisibilityComponent>("MeshRenderSystem:Render")
				.each([=](flecs::entity entity, const RenderInstanceComponent& render_instance_comp, const Transform3DComponent& transform_3d_component, const VisibilityComponent& visibility_comp) {
					//command_queue.enqueue([=](){
						if (visibility_comp.visible) {
							if(!entity.has<Occluded>()){
								RS::get_singleton()->instance_set_transform(render_instance_comp.render_instance_id, transform_3d_component.transform);
								return;
							}
						}
						Transform3D transform_far;
						const Vector3 far_pos = Vector3(far_dist,far_dist,far_dist);
						transform_far.set_origin(far_pos);
						RS::get_singleton()->instance_set_transform(render_instance_comp.render_instance_id, transform_far);
						return;
					//});
					
				});
		pipeline_manager.add_to_pipeline(mesh_render_system, flecs::OnUpdate, "OcclusionSystem::Occludee::Cull");
	}
};


