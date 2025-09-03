//
// Created by Floof on 28-7-2025.
//
#include "core/math/projection.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/os/memory.h"
#include "core/templates/vector.h"
#include "core/variant/variant.h"
#include "flecs.h"
#include "mulitmesh_render_system.h"

#include "core/math/aabb.h"
#include "core/math/plane.h"
#include "ecs/components/worldcomponents.h"
#include "scene/resources/shader.h"
#include "servers/rendering/rendering_device.h"
#include "servers/rendering/rendering_device_commons.h"
#include "servers/rendering_server.h"
#include "ecs/components/transform_3d_component.h"
#include "../commands/command.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/dirty_transform.h"
#include "servers/rendering/rendering_device_binds.h"
#include <cstdint>
#include <cstring>

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

	flecs::system multi_mesh_render_system = world->system<const MultiMeshComponent>()
			.multi_threaded()
			.cache_kind(flecs::QueryCacheAuto)
			.with<VisibilityComponent>()
			.with<DirtyTransform>()
			.detect_changes()
			.each([&](flecs::entity mm_instance, const MultiMeshComponent &mm_comp) {

					if(!world){
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: world is null");
						return;
					}
					if (!main_camera.has<CameraComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: CameraComponent not found");
						return;
					}
					if (!world->has<World3DComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: World3D not found");
						return;
					}
					const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
					if (cam_camera_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: cam_camera_ref not found");
						return;
					}
					const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
					if (cam_transform_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: cam_transform_ref not found");
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
					Vector<float> buffer;
					if(mm_comp.transform_format == RS::MULTIMESH_TRANSFORM_2D){
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: 2D Transform not supported");
						return;
					}
					const int base_size = 12;
					int padding = 0;
					if(mm_comp.has_color) {
						padding += 4;
					}
					if(mm_comp.has_data) {
						padding += 4;
					}
					const int buffer_elem_size = base_size + padding;
					buffer.resize(mm_comp.instance_count * buffer_elem_size);
					if(!multimesh_data_map.has(mm_instance)){
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: multimesh_data_map is null");
						return;
					}
					for(const auto& instance : multimesh_data_map[mm_instance].instances) {
						//For Transform3D the float-order is: (basis.x.x, basis.y.x, basis.z.x, origin.x, basis.x.y, basis.y.y, basis.z.y, origin.y, basis.x.z, basis.y.z, basis.z.z, origin.z).
						buffer.write[instance.index * buffer_elem_size + 0] = instance.transform[0].x;
						buffer.write[instance.index * buffer_elem_size + 1] = instance.transform[1].y;
						buffer.write[instance.index * buffer_elem_size + 2] = instance.transform[2].z;
						buffer.write[instance.index * buffer_elem_size + 3] = instance.transform[3].x;
						buffer.write[instance.index * buffer_elem_size + 4] = instance.transform[4].y;
						buffer.write[instance.index * buffer_elem_size + 5] = instance.transform[5].y;
						buffer.write[instance.index * buffer_elem_size + 6] = instance.transform[6].y;
						buffer.write[instance.index * buffer_elem_size + 7] = instance.transform[7].y;
						buffer.write[instance.index * buffer_elem_size + 8] = instance.transform[8].z;
						buffer.write[instance.index * buffer_elem_size + 9] = instance.transform[9].z;
						buffer.write[instance.index * buffer_elem_size + 10] = instance.transform[10].z;
						buffer.write[instance.index * buffer_elem_size + 11] = instance.transform[11].z;
						if(mm_comp.has_color) {
							buffer.write[instance.index * buffer_elem_size + 12] = instance.color.r;
							buffer.write[instance.index * buffer_elem_size + 13] = instance.color.g;
							buffer.write[instance.index * buffer_elem_size + 14] = instance.color.b;
							buffer.write[instance.index * buffer_elem_size + 15] = instance.color.a;
						}
						if(mm_comp.has_data) {
							buffer.write[instance.index * buffer_elem_size + 16] = instance.data.x;
							buffer.write[instance.index * buffer_elem_size + 17] = instance.data.y;
							buffer.write[instance.index * buffer_elem_size + 18] = instance.data.z;
							buffer.write[instance.index * buffer_elem_size + 19] = instance.data.w;
						}
					}

					RS::get_singleton()->multimesh_set_buffer(mm_comp.multi_mesh_id, buffer);
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

	Plane camera_planes[6];
	Transform3D cam_xform;

	RenderingDevice *rendering_device = RS::get_singleton()->get_rendering_device();

	flecs::system frustum_culling_system = world->system<const MultiMeshComponent>()
			.multi_threaded()
			.cached()
			.each([&](flecs::entity mm_entity, const MultiMeshComponent &mmi_comp) {
				command_handler->enqueue_command([&](){
					if(!world){
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: world is null");
						return;
					}
					if (!main_camera.has<CameraComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: CameraComponent not found");
						return;
					}
					if (!world->has<World3DComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: World3D not found");
						return;
					}
					const auto cam_camera_ref = main_camera.try_get<CameraComponent>();
					if (cam_camera_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: cam_camera_ref not found");
						return;
					}
					const auto cam_transform_ref = main_camera.try_get<Transform3DComponent>();
					if (cam_transform_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: cam_transform_ref not found");
						return;
					}
					bool recomp = false;

					if(!multimesh_data_map.has(mm_entity)){
						MultiMeshData mmd;
						mmd.num_instances = mmi_comp.instance_count;
						multimesh_data_map.insert(mm_entity, mmd);
						recomp = true;
					}
					if(multimesh_data_map[mm_entity].num_instances != mmi_comp.instance_count){
						multimesh_data_map[mm_entity].num_instances = mmi_comp.instance_count;
						recomp = true;
					}
					if(recomp){
						String error;
						Vector<uint8_t> spirv_data = rendering_device->shader_compile_spirv_from_source(RenderingDeviceCommons::SHADER_STAGE_COMPUTE, shader_code.replace("!NUM_INSTANCES!", itos(multimesh_data_map[mm_entity].num_instances)),RenderingDevice::SHADER_LANGUAGE_GLSL, &error, true);
						if (spirv_data.is_empty()) {
							ERR_PRINT("Failed to compile shader: " + error);
							return;
						}
						multimesh_data_map[mm_entity].frustum_cull_shader->set_stage_bytecode(RD::ShaderStage::SHADER_STAGE_COMPUTE, spirv_data);
					}
					if(!multimesh_data_map[mm_entity].query.c_ptr()){
						auto query_builder = world->query_builder<const MultiMeshInstanceComponent, const Transform3DComponent, const VisibilityComponent, FrustumCulled>();
						query_builder.with<DirtyTransform>();
						query_builder.up().with<MultiMeshComponent>();
						query_builder.cache_kind(flecs::QueryCacheAll);
						
						multimesh_data_map[mm_entity].query = query_builder.build();

					}

						

					MultiMeshData &data = multimesh_data_map[mm_entity];
					if(data.instances.size() != data.num_instances){
						data.instances.resize(data.num_instances);
					}
					data.query.each([&](flecs::entity e, const MultiMeshInstanceComponent &mmi, const Transform3DComponent &xform, const VisibilityComponent &vis, FrustumCulled &fc) {
						if (vis.visible) {
							MultiMeshInstanceData instance_data;
							instance_data.index = mmi.index;
							PackedVector4Array transform_array;
							const Basis b = xform.transform.basis;
							const Vector3 origin = xform.transform.get_origin();
							transform_array.push_back(Vector4(b.rows[0].x, b.rows[0].y, b.rows[0].z, 0));
							transform_array.push_back(Vector4(b.rows[1].x, b.rows[1].y, b.rows[1].z, 0));
							transform_array.push_back(Vector4(b.rows[2].x, b.rows[2].y, b.rows[2].z, 0));
							transform_array.push_back(Vector4(origin.x, origin.y, origin.z, 1));
							instance_data.transform = transform_array;
							PackedVector3Array aabb;

							if(mmi.custom_aabb != AABB()){
								aabb.push_back(mmi.custom_aabb.position);
								aabb.push_back(mmi.custom_aabb.size);
								instance_data.aabb = aabb;
							} else {
								aabb.push_back(Vector3(-0.5, -0.5, -0.5));
								aabb.push_back(Vector3(1, 1, 1));
							}
							if(e.has<MultiMeshInstanceDataComponent>()){
								const auto mmi_data = e.get<MultiMeshInstanceDataComponent>();
								instance_data.color = mmi_data.color;
								instance_data.data = mmi_data.data;
							} 

							multimesh_data_map[mm_entity].instances.write[mmi.index] = instance_data;

						} else {
							// If not visible, push a far away AABB
							MultiMeshInstanceData instance_data;
							instance_data.index = mmi.index;
							PackedVector4Array transform_array;
							const Basis b = xform.transform.basis;
							const Vector3 origin = Vector3(100000, 100000, 100000);
							transform_array.push_back(Vector4(b.rows[0].x, b.rows[0].y, b.rows[0].z, 0));
							transform_array.push_back(Vector4(b.rows[1].x, b.rows[1].y, b.rows[1].z, 0));
							transform_array.push_back(Vector4(b.rows[2].x, b.rows[2].y, b.rows[2].z, 0));
							transform_array.push_back(Vector4(origin.x, origin.y, origin.z, 1));
							instance_data.transform = transform_array;
							PackedVector3Array aabb;
							aabb.push_back(Vector3(-0.5, -0.5, -0.5));
							aabb.push_back(Vector3(1, 1, 1));
							instance_data.aabb = aabb;
							multimesh_data_map[mm_entity].instances.write[mmi.index] = instance_data;
						}
					});
					for (int i = 0; i < 6; ++i) {
						camera_planes[i] = cam_camera_ref->frustum[i];
					}

					PackedByteArray multimesh_data_in;
					PackedByteArray aabbs;
					PackedByteArray transforms;
					PackedByteArray culled_objects;
					Vector<uint32_t> culled_objects_vec;
					culled_objects_vec.resize_zeroed(data.num_instances);
					const int instance_count = mmi_comp.instance_count;

					for(int i = 0; i < instance_count; i++) {
						const MultiMeshInstanceData &instance = multimesh_data_map[mm_entity].instances[i];
						aabbs.append_array(instance.aabb.to_byte_array());
					}
					for(int i = 0; i < instance_count; i++) {
						const MultiMeshInstanceData &instance = multimesh_data_map[mm_entity].instances[i];
						transforms.append_array(instance.transform.to_byte_array());
					}
					multimesh_data_in.append_array(aabbs);
					multimesh_data_in.append_array(transforms);
					multimesh_data_in.append_array(culled_objects);
					multimesh_data_in.append_array(culled_objects_vec.to_byte_array());
					const RID multimesh_data_buffer = rendering_device->storage_buffer_create(multimesh_data_in.size(), multimesh_data_in);
					PackedByteArray camera_frustum_data;
					PackedVector4Array camera_frustum;
					for(int i = 0; i < 6; ++i) {
						const Vector3& normal = camera_planes[i].normal;
						camera_frustum.push_back(Vector4(normal.x, normal.y, normal.z, camera_planes[i].d));
					}
					camera_frustum_data.append_array(camera_frustum.to_byte_array());
					const RID camera_frustum_buffer = rendering_device->storage_buffer_create(camera_frustum_data.size(), camera_frustum_data);

					// Construct driver-level uniforms directly (RD::Uniform) and use the
					// public RenderingDevice::uniform_set_create overload that accepts
					// a collection of RD::Uniform. This avoids accessing engine-internal
					// RDUniform wrapper internals and matches the engine-instantiated
					// template variants (Vector<RD::Uniform>).
					Vector<RD::Uniform> driver_uniforms;
					driver_uniforms.resize(2);
					RD::Uniform *uniform_ptrw = driver_uniforms.ptrw();
					uniform_ptrw[0] = RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 0, multimesh_data_buffer);
					uniform_ptrw[1] = RD::Uniform(RD::UNIFORM_TYPE_UNIFORM_BUFFER, 1, camera_frustum_buffer);

					const RID uniform_set = rendering_device->uniform_set_create(driver_uniforms, multimesh_data_map[mm_entity].frustum_cull_shader->get_rid(), 1);
					const RID pipeline = rendering_device->compute_pipeline_create(multimesh_data_map[mm_entity].frustum_cull_shader->get_rid());
					const RenderingDevice::ComputeListID compute_list = rendering_device->compute_list_begin();
					rendering_device->compute_list_bind_compute_pipeline(compute_list, pipeline);
					rendering_device->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
					rendering_device->compute_list_bind_uniform_set(compute_list, uniform_set, 1);
					rendering_device->compute_list_dispatch(compute_list, 64, 1, 1);
					rendering_device->compute_list_end();

					rendering_device->submit();
					rendering_device->sync();
					PackedByteArray read_back = rendering_device->buffer_get_data(multimesh_data_buffer);
					// 4 bytes == float32
					// 12 bytes == Vector3
					// 4 bytes == uint32
					// 16 bytes == Vector4
					// occlusion results are at (12 bytes * ((instance_count + 1) *2)) + (16 bytes * ((instance_count + 1) *2)) 
					// so offset = (12 * instance_count * 2) + (16 * instance_count * 2) = 56 * instance_count
					// size of occlusion results = (4 bytes * (instance_count + 1)))
					const size_t offset = ((sizeof(Vector3) *2) *data.num_instances) + (sizeof(Vector4) *2 * data.num_instances);
					const size_t size = (sizeof(uint32_t) * (data.num_instances + 1));
					PackedByteArray occlusion_bin_data = read_back.slice(offset, offset + size);
					Vector<uint32_t> new_culled_objects;
					new_culled_objects.resize(data.num_instances);
					memcpy(new_culled_objects.ptrw(), occlusion_bin_data.ptr(), size);

					multimesh_data_map[mm_entity].query.each([&](flecs::entity e, const MultiMeshInstanceComponent &mmi, const Transform3DComponent &xform, const VisibilityComponent &vis, FrustumCulled &fc) {
						fc.is_culled = new_culled_objects[mmi.index] == 1;
					});
					// End of frustum culling
					rendering_device->free(multimesh_data_buffer);
					rendering_device->free(camera_frustum_buffer);
					// Free any other resources as needed
				});
			});


	frustum_culling_system.set_name("MultiMeshRenderSystem: FrustumCulling");
	//flecs::entity_t frustum_culling_phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem: FrustumCulling", "MultiMeshRenderSystem: UpdateCameraFrustum");
	pipeline_manager->add_to_pipeline(frustum_culling_system, flecs::OnUpdate);
}