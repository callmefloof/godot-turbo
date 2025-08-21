//
// Created by Floof on 28-7-2025.
//
#include "mulitmesh_render_system.h"

#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "ecs/components/worldcomponents.h"
#include "servers/rendering_server.h"
#include "ecs/components/transform_3d_component.h"
#include "../commands/command.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/dirty_transform.h"

void MultiMeshRenderSystem::create_rendering(Ref<CommandHandler>& command_handler_ref, PipelineManager& pipeline_manager_ref) {


	world->component<FrameCounter>();
	world->set<FrameCounter>({});
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

	flecs::system multi_mesh_render_system = world->system<const MultiMeshInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
			.multi_threaded()
			.cache_kind(flecs::QueryCacheAuto)
			.with<VisibilityComponent>()
			.without<FrustumCulled>()
			.without<Occluded>()
			.with<DirtyTransform>()
			.detect_changes()
			.each([&](flecs::entity mm_instance, const MultiMeshInstanceComponent &mmi_comp, const Transform3DComponent &transform_comp, const VisibilityComponent &visibility_comp) {
					
					if(!world){
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
						return;
					}
					if (!main_camera.has<CameraComponent>()) {
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: CameraComponent not found");
						return;
					}
					if (!world->has<World3DComponent>()) {
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: World3D not found");
						return;
					}
					const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
					if (cam_camera_ref == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: cam_camera_ref not found");
						return;
					}
					const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
					if (cam_transform_ref == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: cam_transform_ref not found");
						return;
					}
					auto& fc = world->get_mut<FrameCounter>().frame;
					
					const int batch_size = 50000;
					int idx = mm_instance.id() % batch_size;
					if(idx != (fc % batch_size)) {
						// Skip this instance if the index does not match the current frame
						return;
					}
					fc++;
					if (visibility_comp.visible) {
						command_handler->enqueue_command([=](){
						//Set the transform of the multimesh instance
						const MultiMeshComponent& mmc = mm_instance.parent().get<MultiMeshComponent>();
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp.index, transform_comp.transform);
							mm_instance.remove<DirtyTransform>();
						});
					} else {
						//Set it far away from render distance
						Transform3D far_transform;
						const Vector3 far_pos = cam_transform_ref->transform.get_origin() + Vector3(far_dist, far_dist, far_dist);
						far_transform.set_origin(far_pos);
						const MultiMeshComponent& mmc = mm_instance.parent().get<MultiMeshComponent>();
						command_handler->enqueue_command([=]() {
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp.index, far_transform);
							mm_instance.remove<DirtyTransform>();
						});

					}
		});
	multi_mesh_render_system.set_name("MultiMeshRenderSystem: Render");
	flecs::entity_t phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem: Render", "OcclusionSystem/Occludee: OcclusionCull");
	pipeline_manager->add_to_pipeline(multi_mesh_render_system, phase);

}


 void MultiMeshRenderSystem::create_frustum_culling(Ref<CommandHandler>& command_handler_ref, PipelineManager& pipeline_manager_ref) {
	if(!world){
		ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
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


	flecs::system frustum_culling_system = world->system<const MultiMeshInstanceComponent, const Transform3DComponent,const VisibilityComponent>("MultiMeshRenderSystem: FrustumCulling")
			
			.multi_threaded()
			.detect_changes()
			.interval(0.016)
			.each([&](flecs::entity mm_instance, const MultiMeshInstanceComponent &mmi_comp, const Transform3DComponent &transform_comp, const VisibilityComponent &visibility_comp) {
					if(!world){
						ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
						return;
					}
					if (!main_camera.has<CameraComponent>()) {
						ERR_PRINT("MultiMeshRenderSystem::create_frustum_culling: CameraComponent not found");
						return;
					}
					if (!world->has<World3DComponent>()) {
						ERR_PRINT("MultiMeshRenderSystem::create_frustum_culling: World3D not found");
						return;
					}
					const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
					if (cam_camera_ref == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem::create_frustum_culling: cam_camera_ref not found");
						return;
					}
					const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
					if (cam_transform_ref == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem::create_frustum_culling: cam_transform_ref not found");
						return;
					}

					if (!visibility_comp.visible) {
						return;
					}
					command_handler->enqueue_command([=]() {
						AABB local_aabb = mm_instance.parent().get<MeshComponent>().custom_aabb;
						AABB world_aabb = local_aabb;
						world_aabb.set_position(local_aabb.position + transform_comp.transform.get_origin());
						world_aabb.set_size(local_aabb.size * transform_comp.transform.get_basis().get_scale());
						world_aabb.set_size(transform_comp.transform.basis.get_scale());

						Plane frustum_planes[6];
						for (int i = 0; i < 6; i++) {
							frustum_planes[i] = cam_camera_ref->frustum[i];
						}
						
						for (int i = 0; i < 6; ++i) {
							if (!world_aabb.intersects_plane(frustum_planes[i])) {
								mm_instance.add<FrustumCulled>();
							} else {
								mm_instance.remove<FrustumCulled>();
							}
						}
					});
				});
	frustum_culling_system.set_name("MultiMeshRenderSystem: FrustumCulling");
	pipeline_manager->add_to_pipeline(frustum_culling_system, flecs::OnUpdate);
}