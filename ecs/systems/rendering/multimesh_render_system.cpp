//
// Created by Floof on 28-7-2025.
//
#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "ecs/components/worldcomponents.h"
#include "core/os/os.h"
#include "servers/rendering_server.h"
#include "ecs/components/transform_3d_component.h"
#include "../commands/command.h"
#include "mulitmesh_render_system.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/visibility_component.h"

void MultiMeshRenderSystem::create_rendering(CommandQueue& command_queue, PipelineManager& pipeline_manager) const {
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

	flecs::system multi_mesh_render_system = world->system<const MultiMeshComponent>()
			
			.multi_threaded()
					.cache_kind(flecs::QueryCacheAuto)
			.with<VisibilityComponent>()
			.without<FrustumCulled>()
			.without<Occluded>()
			.each([&](flecs::entity multi_mesh, const MultiMeshComponent &mmc) {
				multi_mesh.children([&](flecs::entity child) {
					const auto mmi_comp = child.try_get<MultiMeshInstanceComponent>();
					const auto transform_comp = child.try_get<Transform3DComponent>();
					const auto visibility_comp = child.try_get<VisibilityComponent>();
					if (mmi_comp == nullptr || transform_comp == nullptr || visibility_comp == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem: Missing required components on child entity.");
						return;
					}
					
					if (visibility_comp->visible) {
						command_queue.enqueue([=](){
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp->index, transform_comp->transform);
						});
					} else {
						//Set it far away from render distance
						Transform3D far_transform;
						const Vector3 far_pos = cam_transform_ref->transform.get_origin() + Vector3(far_dist, far_dist, far_dist);
						far_transform.set_origin(far_pos);
						command_queue.enqueue([=]() {
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp->index, far_transform);
						});

					}
			});
		});
	multi_mesh_render_system.set_name("MultiMeshRenderSystem: Render");
	flecs::entity_t phase = pipeline_manager.create_custom_phase("MultiMeshRenderSystem: Render", "OcclusionSystem/Occludee: OcclusionCull");
	pipeline_manager.add_to_pipeline(multi_mesh_render_system, phase);

}


 void MultiMeshRenderSystem::create_frustum_culling(CommandQueue &command_queue, PipelineManager& pipeline_manager) const {
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


	flecs::system frustum_culling_system = world->system<const MultiMeshComponent, const MeshComponent>("MultiMeshRenderSystem: FrustumCulling")
			
			.multi_threaded()
					.each([&](flecs::entity entity, const MultiMeshComponent &mmc, const MeshComponent &mesh_comp) {
				entity.children([&](flecs::entity child) {
					const auto mmi_comp = child.try_get<MultiMeshInstanceComponent>();
					const auto transform_comp = child.try_get<Transform3DComponent>();
					const auto visibility_comp = child.try_get<VisibilityComponent>();
					if (mmi_comp == nullptr || transform_comp == nullptr || visibility_comp == nullptr) {
						ERR_PRINT("MultiMeshRenderSystem: Missing required components on child entity.");
						return;
					}
					if (!visibility_comp->visible) {
						return;
					}
					command_queue.enqueue([=]() {
						AABB local_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_comp.mesh_id);
						AABB world_aabb = local_aabb;
						world_aabb.set_position(local_aabb.position + transform_comp->transform.get_origin());
						world_aabb.set_size(local_aabb.size * transform_comp->transform.get_basis().get_scale());
						world_aabb.set_size(transform_comp->transform.basis.get_scale());
						
						Plane frustum_planes[6];
						for (int i = 0; i < 6; i++) {
							frustum_planes[i] = cam_camera_ref->frustum[i];
						}
						
						for (int i = 0; i < 6; ++i) {
							if (!world_aabb.intersects_plane(frustum_planes[i])) {
								child.add<FrustumCulled>();
							} else {
								child.remove<FrustumCulled>();
							}
						}
					});
				});
			});
	frustum_culling_system.set_name("MultiMeshRenderSystem: FrustumCulling");
	pipeline_manager.add_to_pipeline(frustum_culling_system, flecs::OnUpdate);
}

void MultiMeshRenderSystem::_bind_methods() {
 	//ClassDB::bind_static_method(get_class_static(),"get_singleton",&MultiMeshRenderSystem::get_singleton);
}

// MultiMeshRenderSystem *MultiMeshRenderSystem::get_singleton() {
//  	if (!Engine::get_singleton()->has_singleton("MultiMeshRenderSystem")) {
//  		Engine::get_singleton()->add_singleton(Engine::Singleton("MultiMeshRenderSystem",memnew(MultiMeshRenderSystem), "MultiMeshRenderSystem"));
//  	}
//  	static MultiMeshRenderSystem* singleton = cast_to<MultiMeshRenderSystem>(Engine::get_singleton()->get_singleton_object("MultiMeshRenderSystem"));
//  	return singleton;
// }