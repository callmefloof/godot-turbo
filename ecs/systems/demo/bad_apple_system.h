#pragma once
#include "core/error/error_macros.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/ecs/systems/command.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "scene/gui/video_stream_player.h"
#include "modules/godot_turbo/ecs/components/all_components.h"
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
        const uint8_t* ptr = nullptr;  // Direct pointer (no copy!)
        int width = 0;
        int height = 0;
        Image::Format format = Image::FORMAT_MAX;
    } image_data;
    MultiMeshComponent component;
    
    // Threading configuration
    bool use_multithreading = true;
    uint32_t threading_threshold = 10000; // Only use threading if pixel count exceeds this
    uint32_t max_threads = 8;
    
    // Image orientation
    bool flip_y = true; // Flip Y axis when reading image (true = correct for standard 3D Y-up multimesh layout)
    
    // Chunk-based processing component
    struct ImageProcessChunk {
        uint32_t start_index;
        uint32_t end_index;
        const ImageData* img_data;
        BASMode mode;
        Color* output_ptr;  // Direct write to shared buffer
    };
    
    // Shared output buffer (eliminates per-chunk allocation)
    PackedColorArray shared_output_buffer;
    
    // Pre-created chunk entities (reused every frame)
    LocalVector<flecs::entity> chunk_entities;
    bool chunks_initialized = false;
    
    // Fast-path pixel processing (avoids switch per pixel)
    void process_pixels_rgba8(uint32_t start_idx, uint32_t end_idx, int width, int height, 
                              const uint8_t* data, BASMode mode, Color* output, bool p_flip_y);
    void process_pixels_rgb8(uint32_t start_idx, uint32_t end_idx, int width, int height, 
                             const uint8_t* data, BASMode mode, Color* output, bool p_flip_y);
    
    // Helper methods for pixel processing

    

    
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
    
    // Image orientation
    void set_flip_y(bool p_flip) { flip_y = p_flip; }
    bool get_flip_y() const { return flip_y; }
    
    static void _bind_methods();

};
