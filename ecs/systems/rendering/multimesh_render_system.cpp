//
// Created by Floof on 28-7-2025.
//
#include "../../../../../core/math/aabb.h"
#include "../../../../../core/math/basis.h"
#include "../../../../../core/math/vector3.h"
#include "../../../../../core/math/plane.h"

#include "../../../../../core/math/vector2i.h"
#include "../../../../../scene/resources/3d/world_3d.h"
#include "../../../../../core/os/os.h"
#include "../../../../../servers/rendering_server.h"
#include "../../components/rendering/rendering_components.h"
#include "../../components/transform_3d_component.h"
#include "../commands/command.h"
#include "mulitmesh_render_system.h"
#include "occlusion/tile.h"

void FlecsMultiMeshRenderSystem::create_rendering(CommandQueue& command_queue) const {
	if (!main_camera.has<CameraComponent>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: CameraComponent not found");
		return;
	}
	if (!world.has<World3D>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: World3D not found");
		return;
	}
	const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
	if (cam_camera_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_camera_ref not found");
		return;
	}
	const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
	if (cam_transform_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_transform_ref not found");
		return;
	}
	world.system<const MultiMeshComponent>()
			.name("MultiMeshRenderSystem")
			.multi_threaded()
			.each([&](flecs::entity multi_mesh, const MultiMeshComponent &mmc) {
				auto q = world.query_builder<const MultiMeshInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
								 .with(flecs::ChildOf, multi_mesh)
								 .cache_kind(flecs::QueryCacheDefault)
								 .build();

				q.each([&](flecs::entity child, const MultiMeshInstanceComponent &mmi_comp, const Transform3DComponent &transform_comp, const VisibilityComponent &visibility_comp) {
					if (visibility_comp.visible) {
						command_queue.enqueue([=]() {
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp.index, transform_comp.transform);
						});
					} else {
						//Set it far away from render distance
						Transform3D far_transform;
						const Vector3 far_pos = cam_transform_ref->transform.get_origin() + Vector3(far_dist, far_dist, far_dist);
						far_transform.set_origin(far_pos);
						command_queue.enqueue([=]() {
							RS::get_singleton()->multimesh_instance_set_transform(mmc.multi_mesh_id, mmi_comp.index, far_transform);
						});
					}
				});
			});

}


 void FlecsMultiMeshRenderSystem::create_frustum_culling(CommandQueue &command_queue) const {
	if (!main_camera.has<CameraComponent>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: CameraComponent not found");
		return;
	}
	if (!world.has<World3D>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: World3D not found");
		return;
	}
	const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
	if (cam_camera_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_camera_ref not found");
		return;
	}
	const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
	if (cam_transform_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_transform_ref not found");
		return;
	}
	world.system<const MultiMeshComponent, const MeshComponent>()
			.name("MultiMeshRenderSystem")
			.multi_threaded()
			.each([&](flecs::entity multi_mesh, const MultiMeshComponent &mmc, const MeshComponent &mesh_comp) {
				auto q = world.query_builder<const MultiMeshInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
								 .with(flecs::ChildOf, multi_mesh)
								 .cache_kind(flecs::QueryCacheDefault)
								 .build();

				q.each([&](flecs::entity child, const MultiMeshInstanceComponent &mmi_comp, const Transform3DComponent &transform, const VisibilityComponent &visibility_comp) {
					if (!visibility_comp.visible) {
						return;
					}
					command_queue.enqueue([&]() {
						AABB local_aabb = RS::get_singleton()->mesh_get_custom_aabb(mesh_comp.mesh_id);
						AABB world_aabb = local_aabb;
						world_aabb.set_position(local_aabb.position + transform.transform.get_origin());
						world_aabb.set_size(local_aabb.size * transform.transform.get_basis().get_scale());
						world_aabb.set_size(transform.transform.basis.get_scale());
						Plane frustum_planes[6];
						for (int i = 0; i < 6; i++) {
							frustum_planes[i] = cam_camera_ref->frustum[i];
						}
						for (int i = 0; i < 6; ++i) {
							if (!world_aabb.intersects_plane(frustum_planes[i])) {
								child.add<FrustumCulled>();
							}
							else {
								child.remove<FrustumCulled>();
							}
						}
					});
				});
			});
}
void FlecsMultiMeshRenderSystem::create_occlusion_culling(CommandQueue &command_queue) {
	if (!main_camera.has<CameraComponent>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: CameraComponent not found");
		return;
	}
	if (!world.has<World3D>()) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: World3D not found");
		return;
	}
	const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
	if (cam_camera_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_camera_ref not found");
		return;
	}
	const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
	if (cam_transform_ref == nullptr) {
		ERR_PRINT("MultiMeshRenderSystem::initialize: cam_transform_ref not found");
		return;
	}
	world.system<Occluder>("Occluder::Update")
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

	world.system<Occludee>("Occludee::Update")
	.multi_threaded()
	.without<FrustumCulled>()
	.with<Transform3DComponent>()
	.with<VisibilityComponent>()
	.each([](flecs::entity entity, Occludee& occludee){
		const auto& transform = entity.parent().get<Transform3DComponent>();
		occludee.worldAABB.position = occludee.aabb.position + transform.transform.get_origin();
		occludee.worldAABB.size = transform.transform.get_basis().get_scale()*occludee.aabb.size;
	});

	world.system<const Occluder>("Occluder::Binning")
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

	world.system("Occluder::Rasterize").run([=](flecs::iter& it) {
		tile_occlusion_manager.rasterize_all_bins_parallel(OS::get_singleton()->get_processor_count());
	});

	world.system<Occludee>("Occludee::Cull")
	.multi_threaded()
	.without<FrustumCulled>()
	.each([=](flecs::entity entity, Occludee& occludee){
		if (!tile_occlusion_manager.is_visible(ScreenAABB::aabb_to_screen_aabb(occludee.worldAABB,window_size, cam_camera_ref->projection, cam_transform_ref->transform,cam_camera_ref->camera_offset))) {
			entity.add<Occluded>();
		}else {
			entity.remove<Occluded>();
		}
	});


}


void FlecsMultiMeshRenderSystem::create(CommandQueue& command_queue) const {
	create_frustum_culling( command_queue);
	create_rendering( command_queue);
}

void FlecsMultiMeshRenderSystem::_bind_methods() {
 	ClassDB::bind_static_method(get_class_static(),"get_singleton",&FlecsMultiMeshRenderSystem::get_singleton);
}

FlecsMultiMeshRenderSystem *FlecsMultiMeshRenderSystem::get_singleton() {
 	if (!Engine::get_singleton()->has_singleton("FlecsMultiMeshRenderSystem")) {
 		Engine::get_singleton()->add_singleton(Engine::Singleton("FlecsMultiMeshRenderSystem",memnew(FlecsMultiMeshRenderSystem), "MultiMeshRenderSystem"));
 	}
 	static FlecsMultiMeshRenderSystem* singleton = cast_to<FlecsMultiMeshRenderSystem>(Engine::get_singleton()->get_singleton_object("FlecsMultiMeshRenderSystem"));
 	return singleton;
}
Vector2i FlecsMultiMeshRenderSystem::get_window_size() const {
	return window_size;
}
void FlecsMultiMeshRenderSystem::set_window_size(const Vector2i &p_window_size) {
	this->window_size = p_window_size;
}