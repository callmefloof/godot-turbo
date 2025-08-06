#include "occlusion_system.h"
#include "ecs/components/visibility_component.h"
#include "tile_occlusion_manager.h"
#include "ecs/components/worldcomponents.h"
#include "core/os/os.h"
#include "servers/rendering_server.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/transform_3d_component.h"
#include "../../commands/command.h"

void OcclusionSystem::create_occlusion_culling(CommandQueue &command_queue, PipelineManager &pipeline_manager) {
	if (!main_camera.has<CameraComponent>()) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: CameraComponent not found");
		return;
	}
	if (!world.has<World3DComponent>()) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: World3D not found");
		return;
	}
	const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
	if (cam_camera_ref == nullptr) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: cam_camera_ref not found");
		return;
	}
	const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
	if (cam_transform_ref == nullptr) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: cam_transform_ref not found");
		return;
	}
	flecs::system occlusion_update_tris = world.system<Occluder>("OcclusionSystem::Occluder::UpdateTris")
	.multi_threaded()
	.without<FrustumCulled>()
	.each([&](flecs::entity entity, Occluder& occ ) {
		tile_occlusion_manager.reset(window_size.width, window_size.height);
		if ((!entity.parent().has<Transform3DComponent>() && !entity.parent().has<VisibilityComponent>()) || entity.parent().has<FrustumCulled>()) {
			return;
		}
		if (!entity.get<VisibilityComponent>().visible) {
			return;
		}
		const auto& transform = entity.parent().get<Transform3DComponent>();

		PackedVector3Array world_vertices = PackedVector3Array();
		world_vertices.resize(occ.vertices.size());
		for (int i = 0; i < world_vertices.size(); i++) {
				world_vertices.set(i,world_vertices[i] + transform.transform.get_origin());
		}
		occ.screen_triangles.clear();
		occ.screen_triangles = ScreenTriangle::convert_to_screen_triangles(world_vertices, occ.indices, cam_transform_ref->transform,cam_camera_ref->projection, cam_camera_ref->camera_offset);
	});
	pipeline_manager.add_to_pipeline(occlusion_update_tris, flecs::OnUpdate, "MultiMeshRenderSystem::FrustumCulling");

	flecs::system occlusion_update_aabbs = world.system<Occludee>("OcclusionSystem::Occludee::UpdateAABBs")
	.multi_threaded()
	.without<FrustumCulled>()
	.with<Transform3DComponent>()
	.with<VisibilityComponent>()
	.each([](flecs::entity entity, Occludee& occludee){
		const auto& transform = entity.parent().get<Transform3DComponent>();
		occludee.worldAABB.position = occludee.aabb.position + transform.transform.get_origin();
		occludee.worldAABB.size = transform.transform.get_basis().get_scale()*occludee.aabb.size;
	});
	pipeline_manager.add_to_pipeline(occlusion_update_aabbs, flecs::OnUpdate, "OcclusionSystem::Occluder::UpdateTris");


	flecs::system binning = world.system<const Occluder>("OcclusionSystem::Occluder::Binning")
	.multi_threaded()
	.without<FrustumCulled>()
	.each([&](flecs::entity entity, const Occluder& occ) {
		if ((!entity.parent().has<Transform3DComponent>() && !entity.parent().has<VisibilityComponent>()) || entity.parent().has<FrustumCulled>()) {
			return;
		}
		if (!entity.get<VisibilityComponent>().visible) {
			return;
		}
		tile_occlusion_manager.bin_triangles(occ.screen_triangles);
	});

	pipeline_manager.add_to_pipeline(binning, flecs::OnUpdate, "OcclusionSystem::Occluder::UpdateAABBs");


	flecs::system rasterize = world.system("OcclusionSystem::Occluder::Rasterize").run([=](flecs::iter& it) {
		tile_occlusion_manager.rasterize_all_bins_parallel(OS::get_singleton()->get_processor_count());
	});

	pipeline_manager.add_to_pipeline(rasterize, flecs::OnUpdate, "OcclusionSystem::Occluder::Binning");

	flecs::system occlusion_cull = world.system<Occludee>("OcclusionSystem::Occludee::Cull")
	.multi_threaded()
	.without<FrustumCulled>()
	.each([=](flecs::entity entity, Occludee& occludee){
		if (!tile_occlusion_manager.is_visible(ScreenAABB::aabb_to_screen_aabb(occludee.worldAABB,window_size, cam_camera_ref->projection, cam_transform_ref->transform,cam_camera_ref->camera_offset))) {
			entity.add<Occluded>();
		}else {
			entity.remove<Occluded>();
		}
	});
	pipeline_manager.add_to_pipeline(occlusion_cull, flecs::OnUpdate, "OcclusionSystem::Occluder::Rasterize");


}