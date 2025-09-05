#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/rendering/rendering_components.h"
#include "servers/rendering_server.h"
#include "render_system.h"
#include "../commands/command.h"
#include "../pipeline_manager.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/dirty_transform.h"

class MeshRenderSystem : public RenderSystem
{


public:
	MeshRenderSystem() = default;
	~MeshRenderSystem() override = default;
	float far_dist = 9999;
	void create_mesh_render_system(Ref<CommandHandler>& command_handler_ref, PipelineManager& pipeline_manager_ref) {
		flecs::world *w = resolve_world();
		if (!w) {
			ERR_PRINT("MeshRenderSystem::create_rendering: world is null");
			return;
		}
		if (!w->c_ptr()) {
			ERR_PRINT("World is not initialized!");
			return;
		}
		if(command_handler.is_null()){
			command_handler = command_handler_ref;
		}
		if(command_handler.is_null()){
			ERR_PRINT("MultiMeshRenderSystem::create_rendering: command_handler is null");
			return;
		}
		if(!pipeline_manager){
			pipeline_manager = &pipeline_manager_ref;
		}
		if(!pipeline_manager){
			ERR_PRINT("MultiMeshRenderSystem::create_rendering: pipeline_manager is null");
			return;
		}

		// Debug: Print before creating the system
		print_line("Creating MeshRenderSystem...");

		flecs::system mesh_render_system = w->system<const RenderInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
				.detect_changes()
				.with<DirtyTransform>()
				.multi_threaded()
							.each([&](flecs::entity entity, const RenderInstanceComponent& render_instance_comp, const Transform3DComponent& transform_3d_component, const VisibilityComponent& visibility_comp) {
						if (visibility_comp.visible) {
							if(!entity.has<Occluded>()){
								if(!render_instance_comp.instance_id.is_valid()) {
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is 0, this should not happen");
									return;
								}
								if (render_instance_comp.instance_id.is_null()) {
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is null, this should not happen");
									return;
								}
								if(!render_instance_comp.instance_id.is_valid()){
									ERR_PRINT_ONCE("MeshRenderSystem::create_rendering: render_instance_id is not valid, this should not happen");
									return;
								}
								command_handler->enqueue_command([=]() {
									//RS::get_singleton()->instance_set_transform(render_instance_comp.instance_id, transform_3d_component.transform);
								});
								return;
								
							}
						}
						Transform3D transform_far;
						const Vector3 far_pos = Vector3(far_dist,far_dist,far_dist);
						transform_far.set_origin(far_pos);
						//command_queue.enqueue([=]() {
							//RS::get_singleton()->instance_set_transform(render_instance_comp.instance_id, transform_far);
						//});
						return;
					
					
				});
		mesh_render_system.set_name("MeshRenderSystem: Render");

		// Debug: Print after creating the system
		print_line("MeshRenderSystem created with ID: " + itos((uint64_t)mesh_render_system.id()));

		// Debug: Check if dependency exists
		print_line("Checking dependency: OcclusionSystem/Occludee: OcclusionCull");
		flecs::entity_t phase = pipeline_manager->create_custom_phase("MeshRenderSystem: Render", "OcclusionSystem/Occludee: OcclusionCull");
		pipeline_manager->add_to_pipeline(mesh_render_system,phase);

		// Debug: Confirm system added to pipeline
		print_line("MeshRenderSystem added to pipeline with dependency: OcclusionSystem/Occludee: OcclusionCull");
	}
};


