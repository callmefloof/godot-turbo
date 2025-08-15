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
		if(!world){
			ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
			return;
		}
		if (!world->c_ptr()) {
			ERR_PRINT("World is not initialized!");
			return;
		}

		// Debug: Print before creating the system
		print_line("Creating MeshRenderSystem...");

		flecs::system mesh_render_system = world->system<const RenderInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
				.multi_threaded()
							.each([&](flecs::entity entity, const RenderInstanceComponent& render_instance_comp, const Transform3DComponent& transform_3d_component, const VisibilityComponent& visibility_comp) {
						if (visibility_comp.visible) {
							if(!entity.has<Occluded>()){
								if(!render_instance_comp.render_instance_id.is_valid()) {
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is 0, this should not happen");
									return;
								}
								if (render_instance_comp.render_instance_id.is_null()) {
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is null, this should not happen");
									return;
								}
								if(!render_instance_comp.render_instance_id.is_valid()){
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is not valid, this should not happen");
									return;
								}
								command_queue.enqueue([=]() {
									RS::get_singleton()->instance_set_transform(render_instance_comp.render_instance_id, transform_3d_component.transform);
								});
								return;
								
							}
						}
						Transform3D transform_far;
						const Vector3 far_pos = Vector3(far_dist,far_dist,far_dist);
						transform_far.set_origin(far_pos);
						command_queue.enqueue([=]() {
							RS::get_singleton()->instance_set_transform(render_instance_comp.render_instance_id, transform_far);
						});
						return;
					
					
				});
		mesh_render_system.set_name("MeshRenderSystem: Render");

		// Debug: Print after creating the system
		print_line("MeshRenderSystem created with ID: " + itos((uint64_t)mesh_render_system.id()));

		// Debug: Check if dependency exists
		print_line("Checking dependency: OcclusionSystem/Occludee: OcclusionCull");
		flecs::entity_t phase = pipeline_manager.create_custom_phase("MeshRenderSystem: Render", "OcclusionSystem/Occludee: OcclusionCull");
		pipeline_manager.add_to_pipeline(mesh_render_system,phase);

		// Debug: Confirm system added to pipeline
		print_line("MeshRenderSystem added to pipeline with dependency: OcclusionSystem/Occludee: OcclusionCull");
	}
};


