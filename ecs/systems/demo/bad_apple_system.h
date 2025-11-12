#pragma once
#include "core/error/error_macros.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/ecs/systems/command.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
#include "core/object/worker_thread_pool.h"
#include "core/templates/local_vector.h"
#include <cassert>

// SIMD support detection
#if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2) || defined(_M_X64) || defined(_M_AMD64)
    #define BAD_APPLE_SIMD_SSE2
    #include <emmintrin.h>  // SSE2
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    #define BAD_APPLE_SIMD_NEON
    #include <arm_neon.h>
#endif


enum BASMode {
    REGULAR = 0,
    INVERTED = 1,
    RANDOM = 2
};

class BadAppleSystem : public Object {
    GDCLASS(BadAppleSystem, Object);
    flecs::entity mm_entity;
    RID gd_mm_entity;
    RID world_id;
    flecs::world *world = nullptr;
    VideoStreamPlayer *video_player = nullptr;
    Ref<CommandHandler> command_handler;
    PipelineManager* pipeline_manager = nullptr;
    BASMode mode = BASMode::REGULAR;
    struct ImageData {
        PackedByteArray data;
        int width = 0;
        int height = 0;
        Image::Format format = Image::FORMAT_MAX;
    } image_data;
    MultiMeshComponent component;
    
    // Data structure for chunk processing in parallel
    struct ChunkProcessData {
        const ImageData* img_data;
        LocalVector<Color>* colors;
        BASMode mode;
        uint32_t chunk_size;
        uint32_t instance_count;
    };
    
    // Threading configuration
    bool use_multithreading = true;
    uint32_t threading_threshold = 10000; // Only use threading if pixel count exceeds this
    uint32_t max_threads = 8;
    
    // Helper methods for pixel processing
    void process_pixels_optimized(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
    void process_pixel_chunk_rgba8(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
    void process_pixel_chunk_rgb8(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
    void process_pixel_chunk_generic(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
    void process_chunk_by_index(uint32_t p_chunk_index, ChunkProcessData p_data) const;
    
    // SIMD-optimized implementations (used when available)
#ifdef BAD_APPLE_SIMD_SSE2
    void process_pixel_chunk_rgba8_sse2(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
#endif
#ifdef BAD_APPLE_SIMD_NEON
    void process_pixel_chunk_rgba8_neon(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const;
#endif
    
    // Fast hash for random mode (avoids expensive Math::randf())
    static inline float hash_to_float(uint32_t x) {
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = ((x >> 16) ^ x) * 0x45d9f3b;
        x = (x >> 16) ^ x;
        return (x & 0xFFFFFF) / 16777216.0f; // [0, 1)
    }

    public:


    BadAppleSystem() = default;
    ~BadAppleSystem() = default;
    void start();
    void set_mm_entity(const RID& mm_entity);
    void set_video_player(VideoStreamPlayer *p_video_player);
    VideoStreamPlayer *get_video_player() const;
    RID get_mm_entity() const;
    void set_world_id(const RID& p_world_id);
    RID get_world_id() const;
    Color _get_color_at_ofs(const Image::Format format, const uint8_t *ptr, uint32_t ofs) const;
    Color get_pixel(const ImageData& image_data, const int x, const int y) const;
    int get_mode() const { return static_cast<int>(mode); }
    void set_mode(int m) { ERR_FAIL_COND_MSG(m < 0 || m >= 3, "Invalid mode"); mode = static_cast<BASMode>(m); }
    
    // Threading configuration
    void set_use_multithreading(bool p_enabled) { use_multithreading = p_enabled; }
    bool get_use_multithreading() const { return use_multithreading; }
    void set_threading_threshold(uint32_t p_threshold) { threading_threshold = p_threshold; }
    uint32_t get_threading_threshold() const { return threading_threshold; }
    void set_max_threads(uint32_t p_max) { max_threads = CLAMP(p_max, 1U, 32U); }
    uint32_t get_max_threads() const { return max_threads; }
    
    static void _bind_methods();

};
