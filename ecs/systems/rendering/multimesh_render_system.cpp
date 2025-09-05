//
// Created by Floof on 28-7-2025.
//
#include "core/math/projection.h"
#include "core/math/vector3.h"
#include "core/math/vector4.h"
#include "core/os/memory.h"
#include "core/templates/rid.h"
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
#include "../../flecs_types/flecs_server.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <mutex>
#include <algorithm>
#include <type_traits>
#include <numeric>
#include <random>
#include <chrono>
#include <cstdlib>
#include <cstdio>
// Fast multiple-producer/multiple-consumer concurrent queue for lock-free enqueue/dequeue
#include "../../../thirdparty/concurrentqueue/concurrentqueue.h"

// Global mutex used to protect per-parent instance buffer writes in the
// PrepareBuffer system. Replace with a per-MultiMeshData mutex for finer
// granularity if needed.
static std::mutex g_multimesh_mutex;

// Pending instance update queued by worker threads in PrepareBuffer and
// applied in a single-threaded flush before the cull step. This avoids
// per-instance locking in the hot path while keeping writes serialized
// relative to the cull.
struct PendingInstanceUpdate {
	flecs::entity_t parent_entity;
	uint32_t index;
	float transform[16];
	float aabb[6];
	bool has_color;
	float color[4];
	bool has_data;
	float data_vals[4];
};

static moodycamel::ConcurrentQueue<PendingInstanceUpdate> g_pending_updates;

// Per-thread staging: each worker thread writes into its own vector to avoid
// global atomics/locks. The flush will merge these vectors single-threaded.
struct ThreadStaging {
	std::vector<PendingInstanceUpdate> items;
};

static std::mutex g_staging_registry_mutex;
static std::vector<ThreadStaging*> g_thread_stagings;

static ThreadStaging *get_thread_staging() {
	thread_local ThreadStaging *st = nullptr;
	if (st) { return st; }
	st = new ThreadStaging();
	st->items.reserve(1024);
	std::lock_guard<std::mutex> lg(g_staging_registry_mutex);
	g_thread_stagings.push_back(st);
	return st;
}

// Radix sort (LSD) specialized for PendingInstanceUpdate keyed by
// flecs::entity_t. This avoids the cost of std::sort for large batches of
// updates and produces stable, linear-time sorting based on integer keys.
// Radix sort indices by parent id to avoid copying large PendingInstanceUpdate
static void radix_sort_indices_by_parent(const std::vector<PendingInstanceUpdate> &arr, std::vector<uint32_t> &out_idx) {
	using key_t = std::make_unsigned_t<decltype(PendingInstanceUpdate::parent_entity)>;
	const size_t n = arr.size();
	out_idx.resize(n);
	if (n == 0) { return; }
	if (n <= 64) {
		// small arrays: use std::stable_sort on indices
		for (size_t i = 0; i < n; ++i) { out_idx[i] = static_cast<uint32_t>(i); }
		std::stable_sort(out_idx.begin(), out_idx.end(), [&](uint32_t a, uint32_t b) {
			return arr[a].parent_entity < arr[b].parent_entity;
		});
		return;
	}

	// Precompute unsigned keys once to avoid repeated memory reads/casts
	std::vector<key_t> keys(n);
	for (size_t i = 0; i < n; ++i) {
		keys[i] = static_cast<key_t>(arr[i].parent_entity);
	}

	// Use 32-bit index arrays for smaller, cache-friendly moves
	std::vector<uint32_t> idx(n);
	std::iota(idx.begin(), idx.end(), 0u);
	std::vector<uint32_t> tmp_idx(n);

	// 16-bit radix per-pass reduces number of passes for typical 32/64-bit keys
	const size_t key_bytes = sizeof(key_t);
	const size_t passes = (key_bytes + 1) / 2; // number of 16-bit passes
	const size_t RADIX = 1u << 16; // 65536
	std::vector<uint32_t> count(RADIX);

	for (size_t pass = 0; pass < passes; ++pass) {
		std::fill(count.begin(), count.end(), 0u);
		const size_t shift = pass * 16;

		// Count using precomputed keys
		for (size_t i = 0; i < n; ++i) {
			key_t k = keys[idx[i]];
			uint32_t bucket = static_cast<uint32_t>((k >> shift) & 0xFFFFu);
			++count[bucket];
		}

		// Prefix sum
		uint32_t sum = 0u;
		for (size_t i = 0; i < RADIX; ++i) {
			uint32_t c = count[i];
			count[i] = sum;
			sum += c;
		}

		// Scatter indices
		for (size_t i = 0; i < n; ++i) {
			key_t k = keys[idx[i]];
			uint32_t bucket = static_cast<uint32_t>((k >> shift) & 0xFFFFu);
			tmp_idx[count[bucket]++] = idx[i];
		}

		idx.swap(tmp_idx);
	}

	// Write back into caller's uint32_t vector
	for (size_t i = 0; i < n; ++i) { out_idx[i] = idx[i]; }
}


void MultiMeshRenderSystem::create_rendering(Ref<CommandHandler>& command_handler_ref, PipelineManager& pipeline_manager_ref) {


	flecs::world *w = resolve_world();
	if (!w) {
		ERR_PRINT("MultiMeshRenderSystem::create_rendering: world is null");
		return;
	}
	w->component<FrameCounter>();
	w->set<FrameCounter>({});
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

	// Capture a snapshot of the owning world's RID for use inside the
	// per-entity lambda so we don't capture the `flecs::world*` pointer.
	RID multi_mesh_world_rid = world_rid.load();

	// No rendering_device needed here; per-entity lambda resolves world and
	// uses the global multimesh map via an explicit pointer capture.

	// Create a pointer to the global multimesh map and capture it explicitly
	// to avoid implicit capturing of 'this'.
	auto *multimesh_map_ptr_local = &multimesh_data_map;

	flecs::system multi_mesh_render_system = w->system<const MultiMeshComponent>()
			.cache_kind(flecs::QueryCacheAuto)
			.with<VisibilityComponent>()
			.with<DirtyTransform>()
			.detect_changes()
			.each([multi_mesh_world_rid, main_camera_snapshot = main_camera, multimesh_map_ptr_local](flecs::entity mm_instance, const MultiMeshComponent &mm_comp) {

					// Resolve the world inside the lambda on the executing thread.
					flecs::world *world_snapshot = FlecsServer::get_singleton()->_get_world(multi_mesh_world_rid);
					if (!world_snapshot) {
						ERR_PRINT_ONCE(String("MultiMeshRenderSystem::create_rendering: _get_world returned null for world_id=") + String::num_uint64(multi_mesh_world_rid.get_id()));
						return;
					}

					// world validity already checked at setup time; re-check here.
					if (!main_camera_snapshot.has<CameraComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: CameraComponent not found");
						return;
					}
					if (!world_snapshot->has<World3DComponent>()) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: World3D not found");
						return;
					}
					const auto cam_camera_ref = main_camera_snapshot.try_get<CameraComponent>();
					if (cam_camera_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: cam_camera_ref not found");
						return;
					}
					const auto cam_transform_ref = main_camera_snapshot.try_get<Transform3DComponent>();
					if (cam_transform_ref == nullptr) {
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: cam_transform_ref not found");
						return;
					}
					auto& fc = world_snapshot->get_mut<FrameCounter>().frame;

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
					if(!multimesh_map_ptr_local->has(mm_instance)){
						ERR_PRINT_ONCE("MultiMeshRenderSystem::create_rendering: multimesh_data_map is null");
						return;
					}
					auto instances = (*multimesh_map_ptr_local)[mm_instance].instances;
					const uint32_t num_instances = (*multimesh_map_ptr_local)[mm_instance].num_instances;
					for(int i = 0; i < num_instances; i++) {
						//For Transform3D the float-order is: (basis.x.x, basis.y.x, basis.z.x, origin.x, basis.x.y, basis.y.y, basis.z.y, origin.y, basis.x.z, basis.y.z, basis.z.z, origin.z).
						buffer.write[i * buffer_elem_size + 0] = instances.transforms[i * buffer_elem_size + 0];
						buffer.write[i * buffer_elem_size + 1] = instances.transforms[i * buffer_elem_size + 1];
						buffer.write[i * buffer_elem_size + 2] = instances.transforms[i * buffer_elem_size + 2];
						buffer.write[i * buffer_elem_size + 3] = instances.transforms[i * buffer_elem_size + 3];
						buffer.write[i * buffer_elem_size + 4] = instances.transforms[i * buffer_elem_size + 4];
						buffer.write[i * buffer_elem_size + 5] = instances.transforms[i * buffer_elem_size + 5];
						buffer.write[i * buffer_elem_size + 6] = instances.transforms[i * buffer_elem_size + 6];
						buffer.write[i * buffer_elem_size + 7] = instances.transforms[i * buffer_elem_size + 7];
						buffer.write[i * buffer_elem_size + 8] = instances.transforms[i * buffer_elem_size + 8];
						buffer.write[i * buffer_elem_size + 9] = instances.transforms[i * buffer_elem_size + 9];
						buffer.write[i * buffer_elem_size + 10] = instances.transforms[i * buffer_elem_size + 10];
						buffer.write[i * buffer_elem_size + 11] = instances.transforms[i * buffer_elem_size + 11];
						if(mm_comp.has_color) {
							buffer.write[i * buffer_elem_size + 12] = instances.colors[i * buffer_elem_size + 0];
							buffer.write[i * buffer_elem_size + 13] = instances.colors[i * buffer_elem_size + 1];
							buffer.write[i * buffer_elem_size + 14] = instances.colors[i * buffer_elem_size + 2];
							buffer.write[i * buffer_elem_size + 15] = instances.colors[i * buffer_elem_size + 3];
						}
						if(mm_comp.has_data) {
							buffer.write[i * buffer_elem_size + 16] = instances.data[i * buffer_elem_size + 0];
							buffer.write[i * buffer_elem_size + 17] = instances.data[i * buffer_elem_size + 1];
							buffer.write[i * buffer_elem_size + 18] = instances.data[i * buffer_elem_size + 2];
							buffer.write[i * buffer_elem_size + 19] = instances.data[i * buffer_elem_size + 3];
						}
					}

					RS::get_singleton()->multimesh_set_buffer(mm_comp.multi_mesh_id, buffer);
		});
	multi_mesh_render_system.set_name("MultiMeshRenderSystem: Render");
	flecs::entity_t phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem: Render", "OcclusionSystem/Occludee: OcclusionCull");
	pipeline_manager->add_to_pipeline(multi_mesh_render_system, phase);

}


 void MultiMeshRenderSystem::create_frustum_culling(Ref<CommandHandler>& command_handler_ref, PipelineManager& pipeline_manager_ref) {

	// If environment variable GODOT_TURBO_MICROBENCH is set, run a small
	// micro-benchmark that measures enqueue/dequeue/sort cost in isolation.
	auto run_microbench = [&]() {
	const char *env = std::getenv("GODOT_TURBO_MICROBENCH");
	if (!env) { return; }

		const size_t N = 200000; // number of pending updates to simulate
		std::vector<PendingInstanceUpdate> items;
		items.reserve(N);
		std::mt19937_64 rng(123456);
		std::uniform_int_distribution<uint64_t> uid(1, 1ull << 40);

		for (size_t i = 0; i < N; ++i) {
			PendingInstanceUpdate p{};
			p.parent_entity = static_cast<flecs::entity_t>(uid(rng));
			p.index = static_cast<uint32_t>(i % 1000);
			items.push_back(p);
		}

		// Measure try_enqueue_bulk
		auto t0 = std::chrono::high_resolution_clock::now();
		bool ok = g_pending_updates.try_enqueue_bulk(items.data(), items.size());
		auto t1 = std::chrono::high_resolution_clock::now();
		double enqueue_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

		// Drain with try_dequeue_bulk
		std::vector<PendingInstanceUpdate> drained;
		drained.resize(N);
		auto t2 = std::chrono::high_resolution_clock::now();
		size_t got = g_pending_updates.try_dequeue_bulk(drained.data(), drained.size());
		auto t3 = std::chrono::high_resolution_clock::now();
		double dequeue_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();

		// Measure radix sort on the drained vector
		std::vector<uint32_t> idx;
		auto t4 = std::chrono::high_resolution_clock::now();
		radix_sort_indices_by_parent(drained, idx);
		auto t5 = std::chrono::high_resolution_clock::now();
		double sort_ms = std::chrono::duration<double, std::milli>(t5 - t4).count();

		// Print results to console (use stdout so it's visible in builds)
		std::printf("microbench: enqueue_ok=%d enqueue_ms=%.3f dequeue_got=%zu dequeue_ms=%.3f sort_ms=%.3f\n",
					ok ? 1 : 0, enqueue_ms, got, dequeue_ms, sort_ms);
	};

	run_microbench();

	// Resolve the world from the system's stored RID. This avoids relying on
	// an external world_id parameter and prevents races where callers pass
	// an out-of-date id. If the stored RID is not set or cannot be resolved,
	// bail early.
	flecs::world *w = resolve_world();
	if (!w) {
		// If the system hasn't been assigned a world, log the numeric RID if
		// available for easier debugging.
		RID stored = world_rid.load();
		if (stored.is_valid()) {
			ERR_PRINT(String("MultiMeshRenderSystem::create_frustum_culling: _get_world returned null for stored world_id=") + String::num_uint64(stored.get_id()));
		} else {
			ERR_PRINT("MultiMeshRenderSystem::create_frustum_culling: stored world_id is not set");
		}
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


	flecs::system frustum_culling_system_resize_buffer = w->system<const MultiMeshComponent>()
		.multi_threaded()
		.each([&](flecs::entity e, const MultiMeshComponent &mmi_comp) {
			const flecs::entity_t mm_entity_t = e.id();
			auto *multimesh_map_ptr = &multimesh_data_map;

			// Ensure the map entry exists. Use the global mutex only for map
			// insertion; once present we lock the per-MultiMeshData mutex for
			// resizing to reduce contention.
			{
				std::lock_guard<std::mutex> lg_map(g_multimesh_mutex);
				if(!(*multimesh_map_ptr).has(mm_entity_t)){
					(*multimesh_map_ptr).insert(mm_entity_t, MultiMeshData());
				}
			}
			MultiMeshData &data = (*multimesh_map_ptr)[mm_entity_t];
			const size_t instance_count = static_cast<size_t>(mmi_comp.instance_count);
			{
				std::lock_guard<std::mutex> lg(*data.mutex);
				data.instances.transforms.resize(instance_count * 16); // 16 floats per Transform3D
				data.instances.aabbs.resize(instance_count * 6); // 6 floats per AABB (position + size)
				data.instances.culled_objects.resize(instance_count); // 1 uint32_t per instance
				data.has_color = mmi_comp.has_color;
				data.has_data = mmi_comp.has_data;
				if(data.has_color) {
					data.instances.colors.resize(instance_count * 4); // 4 floats per Color
				}
				if(data.has_data) {
					data.instances.data.resize(instance_count * 4); // 4 floats per Vector4
				}
			}
		});
	frustum_culling_system_resize_buffer.set_name("MultiMeshRenderSystem/FrustumCulling: ResizeBuffer");
	pipeline_manager->add_to_pipeline(frustum_culling_system_resize_buffer, flecs::OnUpdate);

	flecs::system frustum_culling_system_prepare_buffer = w->system<const MultiMeshInstanceComponent, const Transform3DComponent, const VisibilityComponent>()
			.multi_threaded()
			.cached()
			.each([&](flecs::entity e, const MultiMeshInstanceComponent &mmi_comp, const Transform3DComponent &xform_comp, const VisibilityComponent &vis_comp) {
				const flecs::entity_t mm_entity_t = e.parent().id();
				auto *multimesh_map_ptr = &multimesh_data_map;

				MultiMeshData &data = (*multimesh_map_ptr)[mm_entity_t];

				// NOTE: ResizeBuffer runs in an earlier pipeline phase and
				// ensures the per-parent buffers are sized. To avoid heavy
				// lock contention we avoid locking here and perform raw
				// writes into pre-sized buffers. This assumes ResizeBuffer
				// completed before PrepareBuffer runs; if that ordering
				// changes, reintroduce per-data locking.
				const Basis b = xform_comp.transform.basis;
				const Vector3 far_origin = Vector3(100000, 100000, 100000);
				Vector3 origin = xform_comp.transform.get_origin();
				if(!vis_comp.visible){
					origin = far_origin;
				}
				// Instead of writing directly into the per-parent buffers (which
				// races with other threads), enqueue a small POD snapshot. The
				// flush system will apply these updates single-threaded under
				// the per-parent mutex before the cull step.
				PendingInstanceUpdate pu{}; // zero-init to avoid uninitialized fields
				pu.parent_entity = e.parent().id();
				pu.index = mmi_comp.index;
				pu.transform[0] = b.rows[0].x;
				pu.transform[1] = b.rows[0].y;
				pu.transform[2] = b.rows[0].z;
				pu.transform[3] = 0.0f;
				pu.transform[4] = b.rows[1].x;
				pu.transform[5] = b.rows[1].y;
				pu.transform[6] = b.rows[1].z;
				pu.transform[7] = 0.0f;
				pu.transform[8] = b.rows[2].x;
				pu.transform[9] = b.rows[2].y;
				pu.transform[10] = b.rows[2].z;
				pu.transform[11] = 0.0f;
				pu.transform[12] = origin.x;
				pu.transform[13] = origin.y;
				pu.transform[14] = origin.z;
				pu.transform[15] = 1.0f;

				const Vector3& position = mmi_comp.custom_aabb.position;
				const Vector3& size = mmi_comp.custom_aabb.size;
				(void)0; // placeholder to avoid unused-warning for earlier local
				if(mmi_comp.custom_aabb != AABB()){
					pu.aabb[0] = position.x;
					pu.aabb[1] = position.y;
					pu.aabb[2] = position.z;
					pu.aabb[3] = size.x;
					pu.aabb[4] = size.y;
					pu.aabb[5] = size.z;
				} else {
					pu.aabb[0] = -0.5f;
					pu.aabb[1] = -0.5f;
					pu.aabb[2] = -0.5f;
					pu.aabb[3] = 1.0f;
					pu.aabb[4] = 1.0f;
					pu.aabb[5] = 1.0f;
				}

				if(e.has<MultiMeshInstanceDataComponent>()) {
					const MultiMeshInstanceDataComponent &mmi_data = e.get<MultiMeshInstanceDataComponent>();
					pu.has_color = data.has_color;
					if(pu.has_color) {
						pu.color[0] = mmi_data.color.r;
						pu.color[1] = mmi_data.color.g;
						pu.color[2] = mmi_data.color.b;
						pu.color[3] = mmi_data.color.a;
					}
					pu.has_data = data.has_data;
					if(pu.has_data) {
						pu.data_vals[0] = mmi_data.data.x;
						pu.data_vals[1] = mmi_data.data.y;
						pu.data_vals[2] = mmi_data.data.z;
						pu.data_vals[3] = mmi_data.data.w;
					}
				}

				// Append to the thread-local staging vector to avoid global atomics
				ThreadStaging *ts = get_thread_staging();
				ts->items.push_back(pu);
				return;

			});
		frustum_culling_system_prepare_buffer.set_name("MultiMeshRenderSystem/FrustumCulling: PrepareBuffer");
		flecs::entity_t prepare_phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem/FrustumCulling: PrepareBuffer", "MultiMeshRenderSystem/FrustumCulling: ResizeBuffer");
		pipeline_manager->add_to_pipeline(frustum_culling_system_prepare_buffer, prepare_phase);

	// Flush system: run before the cull phase and apply all pending updates
	// under the per-parent mutex. This system is single-threaded and drains
	// the queue to avoid races with PrepareBuffer.
	flecs::system frustum_culling_system_flush_queue = w->system<>()
		.each([&]() {
			// Bulk-drain the concurrent queue into a local vector, sort by parent id,
			// and apply contiguous batches. This avoids per-item allocations and
			// eliminates the unordered_map construction/rehashes on hot paths.
			size_t approx = g_pending_updates.size_approx();
			std::vector<PendingInstanceUpdate> drained;
			drained.reserve(approx ? approx : 256);

			// Merge per-thread stagings into drained vector. This avoids global
			// atomic operations on the hot path. Reserve the approximate size
			// to avoid repeated allocations.
			size_t cap = approx ? approx : 256;
			drained.clear();
			drained.reserve(cap);
			{
				std::lock_guard<std::mutex> lg(g_staging_registry_mutex);
				for (ThreadStaging *ts : g_thread_stagings) {
					if (!ts->items.empty()) {
						drained.insert(drained.end(), ts->items.begin(), ts->items.end());
						ts->items.clear();
					}
				}
			}
			if (drained.empty()) { return; }

			// Use radix sort on indices to avoid copying whole PendingInstanceUpdate structs
			std::vector<uint32_t> sorted_idx;
			radix_sort_indices_by_parent(drained, sorted_idx);

			auto *multimesh_map_ptr = &multimesh_data_map;

			// Process contiguous ranges with the same parent_id so we lock each
			// MultiMeshData only once per batch. Iterate via sorted indices.
			const size_t n = drained.size();
			size_t i = 0;
			while (i < n) {
				const flecs::entity_t parent_id = drained[static_cast<size_t>(sorted_idx[i])].parent_entity;
				size_t j = i + 1;
				while (j < n && drained[static_cast<size_t>(sorted_idx[j])].parent_entity == parent_id) { ++j; }

				// Ensure the map entry exists
				{
					std::lock_guard<std::mutex> lg_map(g_multimesh_mutex);
					if(!(*multimesh_map_ptr).has(parent_id)){
						(*multimesh_map_ptr).insert(parent_id, MultiMeshData());
					}
				}

				MultiMeshData &data = (*multimesh_map_ptr)[parent_id];
				std::lock_guard<std::mutex> lg(*data.mutex);

				for (size_t k = i; k < j; ++k) {
					const auto &pu2 = drained[static_cast<size_t>(sorted_idx[k])];
					const size_t transform_offset = static_cast<size_t>(pu2.index) * 16;
					if (transform_offset + 16 <= data.instances.transforms.size()) {
						memcpy(data.instances.transforms.data() + transform_offset, pu2.transform, sizeof(pu2.transform));
					} else {
						ERR_PRINT_ONCE(String("MultiMeshRenderSystem::flush_queue: transform write out-of-bounds; parent_id=") + String::num_uint64(pu2.parent_entity) + String(" index=") + itos(pu2.index));
					}
					const size_t aabb_offset = static_cast<size_t>(pu2.index) * 6;
					if (aabb_offset + 6 <= data.instances.aabbs.size()) {
						memcpy(data.instances.aabbs.data() + aabb_offset, pu2.aabb, sizeof(pu2.aabb));
					} else {
						ERR_PRINT_ONCE(String("MultiMeshRenderSystem::flush_queue: aabb write out-of-bounds; parent_id=") + String::num_uint64(pu2.parent_entity) + String(" index=") + itos(pu2.index));
					}
					if (pu2.has_color && data.has_color) {
						const size_t color_offset = static_cast<size_t>(pu2.index) * 4;
						if (color_offset + 4 <= data.instances.colors.size()) {
							memcpy(data.instances.colors.data() + color_offset, pu2.color, sizeof(pu2.color));
						}
					}
					if (pu2.has_data && data.has_data) {
						const size_t data_offset = static_cast<size_t>(pu2.index) * 4;
						if (data_offset + 4 <= data.instances.data.size()) {
							memcpy(data.instances.data.data() + data_offset, pu2.data_vals, sizeof(pu2.data_vals));
						}
					}
				}

				i = j;
			}
		});
	frustum_culling_system_flush_queue.set_name("MultiMeshRenderSystem/FrustumCulling: FlushQueue");
	flecs::entity_t flush_phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem/FrustumCulling: FlushQueue", "MultiMeshRenderSystem/FrustumCulling: PrepareBuffer");
	pipeline_manager->add_to_pipeline(frustum_culling_system_flush_queue, flush_phase);

	flecs::system frustum_culling_system_cull = w->system<const MultiMeshComponent>()
			.multi_threaded()
			.cached()
			.each([&](flecs::iter& it,size_t row, const MultiMeshComponent &mmi_comp) {

			// Snapshot values used inside the deferred lambda so it doesn't depend on
		// the outer 'this' pointer or mutable members at execution time.
		// Capture a copy of the system's stored world RID atomically so the
		// render thread can safely resolve the flecs::world pointer later.
		auto world_rid_snapshot = world_rid.load();
		// Do NOT capture a flecs::entity across threads (it embeds a
		// world pointer). Capture the entity id instead and rebuild the
		// entity from the resolved world on the render thread.
		auto main_camera_entity_id = main_camera.id();
		auto *multimesh_map_ptr = &multimesh_data_map;
		bool recomp_mm_shader = false;
		const flecs::entity mm_entity =it.entity(row);
		const flecs::entity_t mm_entity_t = mm_entity.id();
			// Ensure map entry exists using a short global lock, then operate on
			// the per-parent data under its own mutex.
			{
				std::lock_guard<std::mutex> lg_map(g_multimesh_mutex);
				if(!(*multimesh_map_ptr).has(mm_entity_t)){
					(*multimesh_map_ptr).insert(mm_entity_t, MultiMeshData());
					(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.world_rid = world_rid_snapshot;
					(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.main_camera_entity_id = main_camera_entity_id;
					(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.mmi_comp = mmi_comp; // copy
					(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.shader_code = shader_code; // copy
					(*multimesh_map_ptr)[mm_entity_t].multimesh_data_buffer = RID();
					recomp_mm_shader = true;
				}
			}
			MultiMeshData &data = (*multimesh_map_ptr)[mm_entity_t];
			std::lock_guard<std::mutex> lg_data(*data.mutex);
		FrustumSnapshot& snap= (*multimesh_map_ptr)[mm_entity_t].frustum_snapshot;
		RID& mm_buff = (*multimesh_map_ptr)[mm_entity_t].multimesh_data_buffer;
		RID& shader = (*multimesh_map_ptr)[mm_entity_t].shader;
			// Acquire or create a persistent local rendering device for this
			// MultiMeshData so the same device instance is used for submit and
			// later sync/readback. Access is protected by the global mutex.
			RenderingDevice* rd_ptr = nullptr;
			{
				std::lock_guard<std::mutex> lg_device(g_multimesh_mutex);
				rd_ptr = (*multimesh_map_ptr)[mm_entity_t].rendering_device;
				if (!rd_ptr) {
					if (RS::get_singleton() && RS::get_singleton()->get_rendering_device()) {
						rd_ptr = RS::get_singleton()->get_rendering_device()->create_local_device();
						(*multimesh_map_ptr)[mm_entity_t].rendering_device = rd_ptr;
					}
				}
			}
		uint8_t& cur_frame_count = (*multimesh_map_ptr)[mm_entity_t].current_frame_count;
		const uint8_t& max_frame_count = (*multimesh_map_ptr)[mm_entity_t].max_frame_count;
		uint32_t& num_instances = (*multimesh_map_ptr)[mm_entity_t].num_instances;
		RDShaderSPIRV* frustum_cull_shader = (*multimesh_map_ptr)[mm_entity_t].frustum_cull_shader.ptr();

		// Diagnostic: print the snapshot world id and shared_ptr internals
		// right before enqueue so we can compare them with the values
		// observed on the render thread when the command executes.
		bool& submitted = (*multimesh_map_ptr)[mm_entity_t].submitted;
		bool& synced = (*multimesh_map_ptr)[mm_entity_t].synced;
		


			if(submitted && synced){
				// End of frustum culling. Ensure we have a valid local device before
				// freeing driver resources.
				if (rd_ptr) {
					rd_ptr->free(mm_buff);
				} else {
					ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: no rendering device available to free resources");
				}
			cur_frame_count = 0;
			// Free any other resources as needed	
			submitted = false;
			synced = false;
		}


		if(!submitted && !synced){
			
		(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.world_rid = world_rid_snapshot;
		(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.main_camera_entity_id = main_camera_entity_id;
		(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.mmi_comp = mmi_comp; // copy
		(*multimesh_map_ptr)[mm_entity_t].frustum_snapshot.shader_code = shader_code; // copy

				if (!rd_ptr) {
					ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: failed to acquire a rendering device for execute thread");
					return;
				}
		const flecs::world &world_snapshot = mm_entity.world().get_world();
		// Rebuild the main camera entity in the context of the
		// resolved world. This avoids using a flecs::entity copied
		// from another thread which may point to an invalid world.
		flecs::entity main_camera_entity = world_snapshot.entity(snap.main_camera_entity_id);
		if (!main_camera_entity.is_valid() || !main_camera_entity.has<CameraComponent>()) {
			ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: CameraComponent not found");
			return;
		}
		if (!world_snapshot.has<World3DComponent>()) {
			ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: World3D not found");
			return;
		}
		const auto cam_camera_ref = main_camera_entity.try_get<CameraComponent>();
		if (cam_camera_ref == nullptr) {
			ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: cam_camera_ref not found");
			return;
		}
		const auto cam_transform_ref = main_camera_entity.try_get<Transform3DComponent>();
		if (cam_transform_ref == nullptr) {
			ERR_PRINT_ONCE("MultiMeshRenderSystem::create_frustum_culling: cam_transform_ref not found");
			return;
		}

		if(num_instances != snap.mmi_comp.instance_count){
			num_instances = snap.mmi_comp.instance_count;
			recomp_mm_shader = true;
		}
		if(recomp_mm_shader){
			String error;
			Vector<uint8_t> spirv_data = rd_ptr->shader_compile_spirv_from_source(RenderingDeviceCommons::SHADER_STAGE_COMPUTE, snap.shader_code.replace("!NUM_INSTANCES!", itos(num_instances)),RenderingDevice::SHADER_LANGUAGE_GLSL, &error, true);
			if (spirv_data.is_empty()) {
				ERR_PRINT("Failed to compile shader: " + error);
				return;
			}
			frustum_cull_shader->set_stage_bytecode(RD::ShaderStage::SHADER_STAGE_COMPUTE, spirv_data);
			shader = rd_ptr->shader_create_from_spirv(frustum_cull_shader->get_stages(), "multimesh_frustum_cull");
		}
			
		// move me plz ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,---------------------
		MultiMeshData &data = (*multimesh_map_ptr)[mm_entity_t];
		// Use float buffers sized by element counts to avoid byte/float size confusion.
		data.instances.transforms.resize(num_instances * 16); // 16 floats per transform (4x4 layout)
		data.instances.aabbs.resize(num_instances * 6); // 6 floats per AABB (pos.x,y,z + size.x,y,z)

	// Store sizes in bytes
	data.array_sizes.aabbs = data.instances.aabbs.size() * sizeof(float);
	data.array_sizes.transforms = data.instances.transforms.size() * sizeof(float);
	std::vector<uint32_t> culled_objects;
	culled_objects.resize(data.num_instances);
	data.array_sizes.culled_objects = culled_objects.size() * sizeof(uint32_t);
		// Collect camera frustum planes directly from camera reference
		Plane local_camera_planes[6];
		for (int i = 0; i < 6; ++i) {
			local_camera_planes[i] = cam_camera_ref->frustum[i];
		}
	// Assemble bytes and copy into PackedByteArray
	const size_t total_bytes = data.array_sizes.total();
	std::vector<uint8_t> multimesh_data_in_assemble;
	multimesh_data_in_assemble.reserve(total_bytes);
	// Append aabbs bytes
	multimesh_data_in_assemble.insert(
		multimesh_data_in_assemble.end(),
		reinterpret_cast<uint8_t*>(data.instances.aabbs.data()),
		reinterpret_cast<uint8_t*>(data.instances.aabbs.data()) + data.array_sizes.aabbs);
	// Append transforms bytes
	multimesh_data_in_assemble.insert(
		multimesh_data_in_assemble.end(),
		reinterpret_cast<uint8_t*>(data.instances.transforms.data()),
		reinterpret_cast<uint8_t*>(data.instances.transforms.data()) + data.array_sizes.transforms);
	// Append culled objects bytes
	multimesh_data_in_assemble.insert(
		multimesh_data_in_assemble.end(),
		reinterpret_cast<uint8_t*>(data.instances.culled_objects.data()),
		reinterpret_cast<uint8_t*>(data.instances.culled_objects.data()) + data.array_sizes.culled_objects);

	PackedByteArray multimesh_data_in;
	multimesh_data_in.resize(total_bytes);
	if (total_bytes) {
	    memcpy(multimesh_data_in.ptrw(), multimesh_data_in_assemble.data(), total_bytes);
	}
	mm_buff = rd_ptr->storage_buffer_create(multimesh_data_in.size(), multimesh_data_in);
		
		PackedVector4Array camera_frustum;
		for(int i = 0; i < 6; ++i) {
			const Vector3& normal = local_camera_planes[i].normal;
			camera_frustum.push_back(Vector4(normal.x, normal.y, normal.z, local_camera_planes[i].d));
		}
		PackedByteArray& camera_frustum_data = data.camera_frustum_data;
		camera_frustum_data.clear();
		camera_frustum_data.append_array(camera_frustum.to_byte_array());

		// Construct driver-level uniforms directly (RD::Uniform) and use the
		// public RenderingDevice::uniform_set_create overload that accepts
		// a collection of RD::Uniform. This avoids accessing engine-internal
		// RDUniform wrapper internals and matches the engine-instantiated
		// template variants (Vector<RD::Uniform>).
		Vector<RD::Uniform> driver_uniforms;
		driver_uniforms.resize(2);
		RD::Uniform *uniform_ptrw = driver_uniforms.ptrw();
		uniform_ptrw[0] = RD::Uniform(RD::UNIFORM_TYPE_STORAGE_BUFFER, 0, mm_buff);

		const RID uniform_set = rd_ptr->uniform_set_create(driver_uniforms, shader, 0);
		const RID pipeline = rd_ptr->compute_pipeline_create(shader);
		const RenderingDevice::ComputeListID compute_list = rd_ptr->compute_list_begin();
		rd_ptr->compute_list_bind_compute_pipeline(compute_list, pipeline);
		
		rd_ptr->compute_list_bind_uniform_set(compute_list, uniform_set, 0);
		rd_ptr->compute_list_set_push_constant(compute_list, static_cast<void*>(camera_frustum_data.ptrw()), sizeof(Vector4) * 6);
		rd_ptr->compute_list_dispatch(compute_list, 64, 1, 1);
		rd_ptr->compute_list_end();

		rd_ptr->submit();
		submitted = true;
	}

	if(submitted && !synced){
		// Still waiting for the previous frame to sync, skip this frame
		// Still waiting for the previous frame to sync, skip this frame
		if(cur_frame_count < max_frame_count){
			cur_frame_count++;
			return;
		}else{
			rd_ptr->sync();
			MultiMeshData& data = (*multimesh_map_ptr)[mm_entity_t];
				PackedByteArray read_back = rd_ptr->buffer_get_data(data.multimesh_data_buffer);
			// 4 bytes == float32
			// 12 bytes == Vector3
			// 4 bytes == uint32
			// 16 bytes == Vector4
			// occlusion results are at (12 bytes * ((instance_count + 1) *2)) + (16 bytes * ((instance_count + 1) *2)) 
			// so offset = (12 * instance_count * 2) + (16 * instance_count * 2) = 56 * instance_count
			// size of occlusion results = (4 bytes * (instance_count + 1)))
			const size_t offset = data.array_sizes.aabbs + data.array_sizes.transforms;
			const size_t size = data.array_sizes.culled_objects;
			PackedByteArray occlusion_bin_data = read_back.slice(offset, offset + size);
			Vector<uint32_t> new_culled_objects;
			size_t byte_count = occlusion_bin_data.size();
			if (byte_count % sizeof(uint32_t) != 0) {
				ERR_PRINT("occlusion_bin_data size not divisible by 4");
			} else {
				size_t size = byte_count / sizeof(uint32_t);
				new_culled_objects.resize(size);
				const uint8_t *src = occlusion_bin_data.ptr(); // pointer to bytes

				uint32_t *dst = new_culled_objects.ptrw();
				for (size_t i = 0; i < size; ++i) {
					uint32_t v;
					memcpy(&v, src + i * sizeof(uint32_t), sizeof(uint32_t));
					dst[i] = v;
				}
			}

			mm_entity.children([&](flecs::entity e) {
				FrustumCulled &fc = e.get_mut<FrustumCulled>();
				const MultiMeshInstanceComponent &mmi = e.get<MultiMeshInstanceComponent>();
				fc.is_culled = new_culled_objects[mmi.index] == 1;
			});

			synced = true;
			
		}

	}

		





			
	
	});

	frustum_culling_system_cull.set_name("MultiMeshRenderSystem/FrustumCulling");
	flecs::entity_t cull_phase = pipeline_manager->create_custom_phase("MultiMeshRenderSystem/FrustumCulling", "MultiMeshRenderSystem/FrustumCulling: PrepareBuffer");
	pipeline_manager->add_to_pipeline(frustum_culling_system_cull, cull_phase);
}

