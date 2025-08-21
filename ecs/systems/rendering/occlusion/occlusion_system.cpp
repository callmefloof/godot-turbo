#include "occlusion_system.h"
#include "ecs/components/visibility_component.h"
#include "tile_occlusion_manager.h"
#include "ecs/components/worldcomponents.h"
#include "core/os/os.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/transform_3d_component.h"
#include "../../commands/command.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/dirty_transform.h"


void OcclusionSystem::create_occlusion_culling(Ref<CommandHandler>& command_handler_ref, PipelineManager &pipeline_manager_ref) {
	if(!world){
		ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
		return;
	}
	if (!main_camera.has<CameraComponent>()) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: CameraComponent not found");
		return;
	}
	if (!world->has<World3DComponent>()) {
		ERR_PRINT("OcclusionSystem::create_occlusion_culling: World3D not found");
		return;
	}
	pipeline_manager = &pipeline_manager_ref;
	command_handler = command_handler_ref;
	
	flecs::system occlusion_update_tris = world->system<Occluder>()
	.multi_threaded()
	.without<FrustumCulled>()
	.detect_changes()
	.interval(0.016)
	.each([&](flecs::entity entity, Occluder& occ ) {
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
		if(!entity.parent().is_valid()){
			//ERR_PRINT_ONCE("OcclusionSystem::create_occlusion_culling: entity parent is not valid");
			return;
		}
		if ((!entity.parent().has<Transform3DComponent>() && !entity.parent().has<VisibilityComponent>()) || entity.parent().has<FrustumCulled>()) {
			return;
		}
		if (!entity.get<VisibilityComponent>().visible) {
			return;
		}
		tile_occlusion_manager.reset(get_window_size().x, get_window_size().y);
		const auto& transform = entity.parent().get<Transform3DComponent>();

		PackedVector3Array world_vertices = PackedVector3Array();
		world_vertices.resize(occ.vertices.size());
		for (int i = 0; i < world_vertices.size(); i++) {
				world_vertices.set(i,world_vertices[i] + transform.transform.get_origin());
		}
		occ.screen_triangles.clear();
		occ.screen_triangles = ScreenTriangle::convert_to_screen_triangles(world_vertices, occ.indices, cam_transform_ref->transform,cam_camera_ref->projection, cam_camera_ref->camera_offset);
	});
	
	occlusion_update_tris.set_name("OcclusionSystem/Occluder: UpdateTris");
	flecs::entity_t phase = pipeline_manager->create_custom_phase("OcclusionSystem/Occluder: UpdateTris", "MultiMeshRenderSystem: FrustumCulling");
	pipeline_manager->add_to_pipeline(occlusion_update_tris, phase);

	flecs::system occlusion_update_aabbs = world->system<Occludee>()
	.multi_threaded()
	.without<FrustumCulled>()
	.with<Transform3DComponent>()
	.with<VisibilityComponent>()
	.interval(0.016)
	.detect_changes()
	.each([](flecs::entity entity, Occludee& occludee){
		const auto& transform = entity.parent().get<Transform3DComponent>();
		occludee.worldAABB.position = occludee.aabb.position + transform.transform.get_origin();
		occludee.worldAABB.size = transform.transform.get_basis().get_scale()*occludee.aabb.size;
	});
	occlusion_update_aabbs.set_name("OcclusionSystem/Occludee: UpdateAABBs");
	flecs::entity_t occlusion_update_aabbs_phase = pipeline_manager_ref.create_custom_phase("OcclusionSystem/Occludee: UpdateAABBs", "OcclusionSystem/Occluder: UpdateTris");
	pipeline_manager->add_to_pipeline(occlusion_update_aabbs, occlusion_update_aabbs_phase);


	flecs::system binning = world->system<const Occluder>()
	.multi_threaded()
	.without<FrustumCulled>()
	.detect_changes()
	.interval(0.016)
	.each([&](flecs::entity entity, const Occluder& occ) {
		if(!entity.parent().is_valid()){
			//ERR_PRINT_ONCE("OcclusionSystem::create_occlusion_culling: entity parent is not valid");
			return;
		}
		if ((!entity.parent().has<Transform3DComponent>() && !entity.parent().has<VisibilityComponent>()) || entity.parent().has<FrustumCulled>()) {
			return;
		}
		if (!entity.get<VisibilityComponent>().visible) {
			return;
		}
		tile_occlusion_manager.bin_triangles(occ.screen_triangles);
	});
	binning.set_name("OcclusionSystem/Occluder: Binning");
	flecs::entity_t binning_phase = pipeline_manager->create_custom_phase("OcclusionSystem/Occluder: Binning", "OcclusionSystem/Occluder: UpdateTris");
	pipeline_manager->add_to_pipeline(binning, binning_phase);


	flecs::system rasterize = world->system().run([=](flecs::iter& it) {
		tile_occlusion_manager.rasterize_all_bins_parallel(OS::get_singleton()->get_processor_count());
	});
	rasterize.set_name("OcclusionSystem/Occluder: Rasterize");

	flecs::entity_t rasterize_phase = pipeline_manager->create_custom_phase("OcclusionSystem/Occluder: Rasterize", "OcclusionSystem/Occluder: Binning");
	pipeline_manager->add_to_pipeline(rasterize, rasterize_phase);

	flecs::system occlusion_cull = world->system<Occludee>()
	.multi_threaded()
	.without<FrustumCulled>()
	.with<DirtyTransform>()
	.detect_changes()
	.interval(0.016)
	.each([=](flecs::entity entity, Occludee& occludee){
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
		if (!tile_occlusion_manager.is_visible(ScreenAABB::aabb_to_screen_aabb(occludee.worldAABB,get_window_size(), cam_camera_ref->projection, cam_transform_ref->transform,cam_camera_ref->camera_offset))) {
			entity.add<Occluded>();
		}else {
			entity.remove<Occluded>();
		}
	});
	occlusion_cull.set_name("OcclusionSystem/Occludee: OcclusionCull");
	flecs::entity_t occlusion_cull_phase = pipeline_manager->create_custom_phase("OcclusionSystem/Occludee: OcclusionCull", "OcclusionSystem/Occluder: Rasterize");
	pipeline_manager->add_to_pipeline(occlusion_cull, occlusion_cull_phase);


}