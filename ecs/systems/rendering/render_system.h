#pragma once
#include "flecs.h"
#include "core/math/vector2i.h"
#include "systems/commands/command.h"
#include "systems/pipeline_manager.h"
#include <functional>
#include <optional>
#include <atomic>

class RenderSystem {

    protected:
    flecs::entity main_camera;
    float far_dist = 9999;
    // Store owning world's RID in an atomic wrapper instead of a direct
    // reference to `flecs::world`. This prevents races when the world RID
    // is set on one thread while other threads (worker threads) capture
    // or read it for deferred commands. Use `resolve_world()` to obtain
    // the live `flecs::world*` on the render thread.
    struct AtomicRID {
        std::atomic<uint64_t> _id{0};
        AtomicRID() = default;
        // Copy construct/assign by loading the atomic value from the other and
        // storing it into this instance. This avoids using the deleted
        // std::atomic copy-assignment and keeps operations atomic semantics.
        AtomicRID(const AtomicRID &other) {
            _id.store(other._id.load(std::memory_order_acquire), std::memory_order_release);
        }
        AtomicRID &operator=(const AtomicRID &other) {
            if (this != &other) {
                _id.store(other._id.load(std::memory_order_acquire), std::memory_order_release);
            }
            return *this;
        }
        // Store a new RID atomically.
        void store(const RID &r) { _id.store(r.get_id(), std::memory_order_release); }
        // Load current RID as a value (0 == null).
        RID load() const { return RID::from_uint64(_id.load(std::memory_order_acquire)); }
        bool has_value() const { return _id.load(std::memory_order_acquire) != 0; }
        void clear() { _id.store(0, std::memory_order_release); }
    } world_rid;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;

    public:
    RenderSystem() = default;
    virtual ~RenderSystem() = default;
    flecs::entity get_main_camera() const { return main_camera; }
	void set_main_camera(const flecs::entity &p_main_camera) { main_camera = p_main_camera; }
    float get_far_dist() const { return far_dist; }
	void set_far_dist(const float p_far_dist) { far_dist = p_far_dist; }
    Vector2i get_window_size() const;
    void set_window_size(const Vector2i &window); 
    // Resolve the stored RID to a flecs::world* (may return nullptr).
    flecs::world *resolve_world() const;
    // Store the owning world's RID. Use a copy (RID is small POD).
    void set_world(RID p_world_rid);
};
