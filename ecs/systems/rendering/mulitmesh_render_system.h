#pragma once
#include "core/math/color.h"
#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/templates/rid.h"
#include "core/variant/variant.h"
#include "render_system.h"
#include "rendering/rendering_components.h"
#include "servers/rendering/rendering_device.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "../pipeline_manager.h"
#include "ecs/systems/commands/command.h"
#include "servers/rendering/rendering_device_binds.h"
#include "transform_3d_component.h"
#include "visibility_component.h"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <memory.h>
#include <mutex>

struct FrameCounter {
	int frame = 0;
};

class MultiMeshRenderSystem : public RenderSystem {
	String shader_code = R"<!>(
#version 450
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

layout(push_constant) uniform PushConstants {
    vec4 planes[6];
} camera_frustum;

layout(set = 0, binding = 0, std430) restrict buffer MultiMeshData {
    mat2x3 aabbs[!NUM_INSTANCES!];        // aabb[0] = pos, aabb[1] = size
    mat4 transforms[!NUM_INSTANCES!];
    uint culled_objects[!NUM_INSTANCES!];
    uint num_instances;
} multimesh_data;


// Helpers
vec3 extractPosition(mat4 m) {
    return vec3(m[3]);
}

vec3 extractScale(mat4 m) {
    return vec3(
        length(vec3(m[0])),
        length(vec3(m[1])),
        length(vec3(m[2]))
    );
}

mat2x3 transformAABB(mat2x3 aabb, mat4 m) {
    vec3 pos   = extractPosition(m);
    vec3 scale = extractScale(m);

    vec3 localPos  = aabb[0];
    vec3 localSize = aabb[1];

    vec3 localMin = localPos - 0.5 * localSize;
    vec3 localMax = localPos + 0.5 * localSize;

    // Apply scaling
    vec3 newMin = localMin * scale;
    vec3 newMax = localMax * scale;

    // Ensure ordering and apply translation
    vec3 finalMin = min(newMin, newMax) + pos;
    vec3 finalMax = max(newMin, newMax) + pos;

    return mat2x3(finalMin, finalMax);
}


bool is_aabb_outside_plane(vec4 plane, mat2x3 aabb) {
    // use the most positive vertex
    vec3 minP = aabb[0];
    vec3 maxP = aabb[1];
    vec3 positive = minP;

    if (plane.x > 0.0) positive.x = maxP.x;
    if (plane.y > 0.0) positive.y = maxP.y;
    if (plane.z > 0.0) positive.z = maxP.z;

    float d = dot(plane.xyz, positive) - plane.w;
    return d < 0.0;
}


// Kernel
void main() {
    uint gid = gl_GlobalInvocationID.x;
    if (gid >= uint(multimesh_data.num_instances)) return;
    int index = int(gid);

    mat2x3 world_aabb = transformAABB(multimesh_data.aabbs[index], multimesh_data.transforms[index]);

    bool culled = false;
    for (int i = 0; i < 6; ++i) {
        vec4 plane = camera_frustum.planes[i]; // use the declared name
        if (is_aabb_outside_plane(plane, world_aabb)) {
            culled = true;
            break;
        }
    }

    multimesh_data.culled_objects[index] = culled ? 1u : 0u; // store as uint
}


)<!>";


// Copy per-entity/context values we need and move them into a
// heap-allocated snapshot. Capturing a shared_ptr by value into
// the queued lambda ensures the data remains alive even if the
// command pool reuses memory for the lambda object itself.
struct FrustumSnapshot {
    RID world_rid;
    uint64_t main_camera_entity_id;
    uint64_t mm_entity_id;
    MultiMeshComponent mmi_comp;
    String shader_code;
};


struct MultiMeshArraySizeData{
    size_t aabbs;
    size_t transforms;
    size_t culled_objects;
    constexpr size_t total() const {
        return aabbs + transforms + culled_objects + sizeof(uint32_t); // + num_instances
    }
    
};


struct MultiMeshInstanceData{
	std::vector<float> transforms; // 16 floats = 4x4 matrix
	std::vector<float> aabbs; // 12 floats = 3 floats position, 3 floats size
    std::vector<uint32_t> culled_objects; // 1 uint32_t = 4 bytes
    std::vector<float> colors; // 4 floats = 1 color
    std::vector<float> data; // Custom data 4 floats (x, y, z, w)
};



struct MultiMeshData {
	MultiMeshInstanceData instances;
    MultiMeshArraySizeData array_sizes;
	Ref<RDShaderSPIRV> frustum_cull_shader = Ref<RDShaderSPIRV>(memnew(RDShaderSPIRV));
	uint32_t num_instances = 0;
    RenderingDevice* rendering_device = nullptr;
    // Per-instance mutex to protect concurrent access to this MultiMeshData
    // (resizing, writes, device creation/readback). Use a unique_ptr so the
    // struct remains assignable and movable while each instance gets its
    // own mutex object.
    std::unique_ptr<std::mutex> mutex = std::make_unique<std::mutex>();
    FrustumSnapshot frustum_snapshot;
    RID multimesh_data_buffer;
    PackedByteArray camera_frustum_data;
    uint8_t current_frame_count = 0;
    uint8_t max_frame_count = 2;
    bool submitted = false;
    bool synced = false;
    RID shader;
    bool has_color = false;
    bool has_data = false;

    // Defaulted constructor is fine; provide copy/move operations so the
    // struct remains assignable/movable even though std::mutex is
    // non-copyable. The mutex is default-constructed for the target so
    // it protects the target's data independently.
    MultiMeshData() = default;

    // Copy constructor: deep-copy all logical members, but default-
    // construct the mutex for the destination.
            MultiMeshData(const MultiMeshData &o)
                : instances(o.instances),
                    array_sizes(o.array_sizes),
          frustum_cull_shader(o.frustum_cull_shader),
          num_instances(o.num_instances),
                        rendering_device(o.rendering_device),
          frustum_snapshot(o.frustum_snapshot),
          multimesh_data_buffer(o.multimesh_data_buffer),
          camera_frustum_data(o.camera_frustum_data),
          current_frame_count(o.current_frame_count),
          max_frame_count(o.max_frame_count),
          submitted(o.submitted),
          synced(o.synced),
          shader(o.shader),
          has_color(o.has_color),
          has_data(o.has_data) {
                    // allocate a fresh mutex for the copy so it protects the copied
                    // instance independently.
                    mutex = std::make_unique<std::mutex>();
    }

    MultiMeshData &operator=(const MultiMeshData &o) {
        if (this != &o) {
            // Lock our own mutex while assigning to avoid races on this
            // object's members. Do not attempt to lock the source's
            // mutex (could deadlock); copying while the source is
            // concurrently mutated is inherently racy and should be
            // avoided at call sites.
            std::lock_guard<std::mutex> lg(*mutex);
            instances = o.instances;
            array_sizes = o.array_sizes;
            frustum_cull_shader = o.frustum_cull_shader;
            num_instances = o.num_instances;
            rendering_device = o.rendering_device;
            frustum_snapshot = o.frustum_snapshot;
            multimesh_data_buffer = o.multimesh_data_buffer;
            camera_frustum_data = o.camera_frustum_data;
            current_frame_count = o.current_frame_count;
            max_frame_count = o.max_frame_count;
            submitted = o.submitted;
            synced = o.synced;
            shader = o.shader;
            has_color = o.has_color;
            has_data = o.has_data;
        }
        return *this;
    }

    // Move constructor: move most movable members and default-construct
    // our mutex. Take ownership of rendering_device pointer and null it
    // on the source to avoid ambiguity in ownership/cleanup.
        MultiMeshData(MultiMeshData &&o) noexcept
                : instances(std::move(o.instances)),
                    array_sizes(o.array_sizes),
          frustum_cull_shader(o.frustum_cull_shader),
          num_instances(o.num_instances),
                        rendering_device(o.rendering_device),
          frustum_snapshot(std::move(o.frustum_snapshot)),
          multimesh_data_buffer(o.multimesh_data_buffer),
          camera_frustum_data(std::move(o.camera_frustum_data)),
          current_frame_count(o.current_frame_count),
          max_frame_count(o.max_frame_count),
          submitted(o.submitted),
          synced(o.synced),
          shader(o.shader),
          has_color(o.has_color),
          has_data(o.has_data) {
                    // Transfer mutex ownership to the destination and null the source
                    // pointer to avoid double-ownership.
                    mutex = std::move(o.mutex);
                    o.mutex = std::make_unique<std::mutex>();
                    o.rendering_device = nullptr;
    }

    MultiMeshData &operator=(MultiMeshData &&o) noexcept {
        if (this != &o) {
            std::lock_guard<std::mutex> lg(*mutex);
            instances = std::move(o.instances);
            array_sizes = o.array_sizes;
            frustum_cull_shader = o.frustum_cull_shader;
            num_instances = o.num_instances;
            rendering_device = o.rendering_device;
            frustum_snapshot = std::move(o.frustum_snapshot);
            multimesh_data_buffer = o.multimesh_data_buffer;
            camera_frustum_data = std::move(o.camera_frustum_data);
            current_frame_count = o.current_frame_count;
            max_frame_count = o.max_frame_count;
            submitted = o.submitted;
            synced = o.synced;
            shader = o.shader;
            has_color = o.has_color;
            has_data = o.has_data;
            // transfer mutex ownership
            mutex = std::move(o.mutex);
            o.mutex = std::make_unique<std::mutex>();
            o.rendering_device = nullptr;
        }
        return *this;
    }
};

HashMap<flecs::entity_t, MultiMeshData> multimesh_data_map;

protected:
	flecs::entity pipeline;
public:
	MultiMeshRenderSystem() = default;
    // Construct with owning world RID so the system stores the RID rather than
    // a direct flecs::world reference (avoids cross-thread issues).
    MultiMeshRenderSystem(RID p_world_rid) {
        set_world(p_world_rid);
        flecs::world *w = resolve_world();
        if (w) {
            pipeline = w->get_pipeline();
        }
    }
	~MultiMeshRenderSystem() override = default;
	void create_rendering(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager);
    // Use the system's stored owning world RID (set via set_world) so deferred
    // commands can safely resolve the flecs::world on the render thread rather
    // than capturing an external world_id parameter.
    void create_frustum_culling(Ref<CommandHandler>& command_handler, PipelineManager& pipeline_manager);
};
