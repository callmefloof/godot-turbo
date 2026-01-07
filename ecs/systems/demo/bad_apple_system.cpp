#include "bad_apple_system.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "scene/resources/texture.h"
#include "servers/rendering_server.h"
#include "core/math/vector2i.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "modules/godot_turbo/ecs/systems/command.h"


// OPTIMIZED: Row-based processing WITH Y-flip
// Multimesh instances: y=0 at BOTTOM, y=height-1 at TOP (3D Y-up convention)
// Image data: y=0 at TOP, y=height-1 at BOTTOM (standard image convention)
// Therefore we need to flip Y when reading from image
// Eliminates per-pixel modulo/division by processing rows
// Template implementation to avoid code duplication between RGBA8 and RGB8
template<uint32_t BytesPerPixel, bool HasAlpha>
static inline void process_pixels_impl(uint32_t start_idx, uint32_t end_idx, int width, int height,
                                        const uint8_t* data, BASMode processing_mode, Color* output, bool p_flip_y) {
	const float inv_255 = 1.0f / 255.0f;

	// Calculate which rows this chunk covers (only ~2-10 divisions total!)
	uint32_t start_row = start_idx / width;
	uint32_t end_row = (end_idx - 1) / width + 1;

	// Helper lambda to read pixel color components
	auto read_pixel = [&](uint32_t pixel_offset) -> Color {
		float r = data[pixel_offset + 0] * inv_255;
		float g = data[pixel_offset + 1] * inv_255;
		float b = data[pixel_offset + 2] * inv_255;
		float a = HasAlpha ? (data[pixel_offset + 3] * inv_255) : 1.0f;
		return Color(r, g, b, a);
	};

	// Pre-compute time seed for RANDOM mode (outside loop)
	uint32_t time_seed = (processing_mode == BASMode::RANDOM) ? OS::get_singleton()->get_ticks_msec() : 0;

	// Common row iteration - only mode-specific pixel processing differs
	for (uint32_t y = start_row; y < end_row; y++) {
		uint32_t row_start_idx = y * width;
		uint32_t row_end_idx = row_start_idx + width;

		// Clamp to chunk boundaries
		uint32_t actual_start = (row_start_idx < start_idx) ? start_idx : row_start_idx;
		uint32_t actual_end = (row_end_idx > end_idx) ? end_idx : row_end_idx;

		// Y-flip if enabled: instance y=0 is at bottom, but image y=0 is at top
		uint32_t read_y = p_flip_y ? (height - 1 - y) : y;
		uint32_t pixel_row_base = read_y * width * BytesPerPixel;

		// Fast inner loop - only additions, no modulo/division!
		// Mode check is outside the pixel loop to avoid branching per pixel
		if (processing_mode == BASMode::REGULAR) {
			for (uint32_t idx = actual_start; idx < actual_end; idx++) {
				uint32_t x = idx - row_start_idx;
				uint32_t pixel_offset = pixel_row_base + x * BytesPerPixel;
				output[idx] = read_pixel(pixel_offset);
			}
		} else if (processing_mode == BASMode::INVERTED) {
			for (uint32_t idx = actual_start; idx < actual_end; idx++) {
				uint32_t x = idx - row_start_idx;
				uint32_t pixel_offset = pixel_row_base + x * BytesPerPixel;
				Color c = read_pixel(pixel_offset);
				// Swap black and white: black (0,0,0) becomes white (1,1,1) and vice versa
				output[idx] = Color(1.0f - c.r, 1.0f - c.g, 1.0f - c.b, c.a);
			}
		} else { // BASMode::RANDOM
			for (uint32_t idx = actual_start; idx < actual_end; idx++) {
				uint32_t x = idx - row_start_idx;
				uint32_t pixel_offset = pixel_row_base + x * BytesPerPixel;
				Color c = read_pixel(pixel_offset);
				// Check if pixel is white (or close to white) - brightness > 0.5
				float brightness = (c.r + c.g + c.b) / 3.0f;
				if (brightness > 0.5f) {
					// Replace white with random color based on pixel position
					uint32_t hash = (idx * 2654435761u) ^ (time_seed * 2246822519u);
					float r = ((hash >> 0) & 0xFF) / 255.0f;
					float g = ((hash >> 8) & 0xFF) / 255.0f;
					float b = ((hash >> 16) & 0xFF) / 255.0f;
					output[idx] = Color(r, g, b, c.a);
				} else {
					// Keep black pixels as black
					output[idx] = c;
				}
			}
		}
	}
}

void BadAppleSystem::process_pixels_rgba8(uint32_t start_idx, uint32_t end_idx, int width, int height,
                                          const uint8_t* data, BASMode processing_mode, Color* output, bool p_flip_y) {
	process_pixels_impl<4, true>(start_idx, end_idx, width, height, data, processing_mode, output, p_flip_y);
}

void BadAppleSystem::process_pixels_rgb8(uint32_t start_idx, uint32_t end_idx, int width, int height,
                                         const uint8_t* data, BASMode processing_mode, Color* output, bool p_flip_y) {
	process_pixels_impl<3, false>(start_idx, end_idx, width, height, data, processing_mode, output, p_flip_y);
}

void BadAppleSystem::start() {
    if(!world) {
        ERR_PRINT_ONCE("World is not set for BadAppleSystem.");
        return;
    }
    if (!video_player) {
        ERR_PRINT_ONCE("Video player is not set for BadAppleSystem.");
        return;
    }
    if (!mm_entity.is_valid()) {
        ERR_PRINT_ONCE("MM entity is not set for BadAppleSystem.");
        return;
    }
    // Validate MultiMeshComponent exists before creating systems
    if(!mm_entity.is_alive()) {
        ERR_PRINT_ONCE("MM entity is not alive for BadAppleSystem.");
        return;
    }
    if(!mm_entity.has<MultiMeshComponent>()) {
        ERR_PRINT_ONCE("MM entity does not have MultiMeshComponent for BadAppleSystem.");
        return;
    }
    const MultiMeshComponent& mm_comp = mm_entity.get<MultiMeshComponent>();
    if(!command_handler.is_valid()) {
        ERR_PRINT_ONCE("CommandHandler is not set for BadAppleSystem.");
        return;
    }
	// Fetch the pipeline manager fresh from the server to avoid holding a
	// pointer to a potentially-moved map entry (AHashMap can rehash/move
	// elements which would invalidate stored pointers).
	PipelineManager* pm = FlecsServer::get_singleton()->_get_pipeline_manager(world_id);
	if(!pm) {
		ERR_PRINT_ONCE("PipelineManager is not available for BadAppleSystem.");
		return;
	}

    // Cache the multimesh RID and configuration to avoid component lookups in lambdas
    const RID cached_mm_rid = mm_comp.multi_mesh_id;
    const bool uses_colors = mm_comp.has_color;
    const bool uses_custom_data = mm_comp.has_data;
    const RS::MultimeshTransformFormat transform_format = mm_comp.transform_format;

    // Calculate multimesh buffer stride
    // Format: [Transform Data][Color (if enabled)][Custom Data (if enabled)]
    // Transform3D = 12 floats, Transform2D = 8 floats
    // Color = 4 floats, Custom Data = 4 floats
    const uint32_t transform_stride = (transform_format == RS::MULTIMESH_TRANSFORM_2D) ? 8 : 12;
    const uint32_t color_offset = transform_stride;
    const uint32_t total_stride = transform_stride + (uses_colors ? 4 : 0) + (uses_custom_data ? 4 : 0);

    // System to update image data and prepare chunks for processing
    auto bas_get_image_data = world->system<>().interval(1.0 / 30.0).run([&, cached_mm_rid](flecs::iter it){
        if (!video_player) {
            return;
        }

        // Try to start playback if not playing
        if (!video_player->is_playing()) {
            // Check if we have a valid stream first
            Ref<VideoStream> stream = video_player->get_stream();
            if (!stream.is_valid()) {
                static bool printed_no_stream = false;
                if (!printed_no_stream) {
                    ERR_PRINT("BadAppleSystem: VideoStreamPlayer has no stream set. Cannot play video.");
                    printed_no_stream = true;
                }
                return;
            }

            // Try to play
            video_player->play();

            // Verify it actually started playing
            if (!video_player->is_playing()) {
                static int retry_count = 0;
                static bool printed_cant_play = false;
                retry_count++;

                if (!printed_cant_play && retry_count > 10) {
                    ERR_PRINT("BadAppleSystem: VideoStreamPlayer.play() called but video is not playing. Check if video player is in scene tree and stream is valid.");
                    printed_cant_play = true;
                }
                return;
            }
        }

        Ref<Texture2D> texture = video_player->get_video_texture();
        if (!texture.is_valid()) {
            return;
        }
        if (texture->get_width() == 0 || texture->get_height() == 0) {
            return;
        }

        // OPTIMIZATION: Get image reference (not copy if possible)
        Ref<Image> image = texture->get_image();
        if (!image.is_valid()) {
            return;
        }
        if (image->get_width() == 0 || image->get_height() == 0) {
            return;
        }

        // Update image data with direct pointer access (zero-copy)
        image_data.width = image->get_width();
        image_data.height = image->get_height();
        image_data.format = image->get_format();
        image_data.ptr = image->get_data().ptr();  // Direct pointer, no copy!

		// Get instance count - avoid .get<>() call by using cached RID
		uint32_t instance_count = RS::get_singleton()->multimesh_get_instance_count(cached_mm_rid);
		if (instance_count == 0) {
			return;
		}

		// Allocate shared output buffer once
		if (shared_output_buffer.size() != (int)instance_count) {
			shared_output_buffer.resize(instance_count);
		}

		// Initialize chunk entities if needed
		if (!chunks_initialized) {
			// Determine optimal chunk count based on thread count
			uint32_t num_chunks = use_multithreading ? MIN(max_threads, (uint32_t)OS::get_singleton()->get_processor_count()) : 1;
			num_chunks = CLAMP(num_chunks, 1U, 32U);

			chunk_entities.clear();
			chunk_entities.resize(num_chunks);

			for (uint32_t i = 0; i < num_chunks; ++i) {
				chunk_entities[i] = world->entity();
			}

			chunks_initialized = true;
		}

		// Distribute work across chunks
		uint32_t num_chunks = chunk_entities.size();
		uint32_t pixels_per_chunk = (instance_count + num_chunks - 1) / num_chunks;
		Color* output_ptr = shared_output_buffer.ptrw();

		for (uint32_t i = 0; i < num_chunks; ++i) {
			uint32_t start = i * pixels_per_chunk;
			uint32_t end = MIN((i + 1) * pixels_per_chunk, instance_count);

			if (start >= instance_count) {
				// Clear this chunk if we don't need it this frame
				if (chunk_entities[i].has<ImageProcessChunk>()) {
					chunk_entities[i].remove<ImageProcessChunk>();
				}
				continue;
			}

			ImageProcessChunk chunk = {};
			chunk.start_index = start;
			chunk.end_index = end;
			chunk.img_data = &image_data;
			chunk.mode = mode;
			chunk.output_ptr = output_ptr;  // Shared buffer

			chunk_entities[i].set<ImageProcessChunk>(chunk);
		}
    });
    bas_get_image_data.set_name("BadAppleSystem/UpdateImageData");

	// Multi-threaded system to process pixel chunks in parallel
	auto bas_process_chunks = world->system<ImageProcessChunk>()
		.multi_threaded(use_multithreading)
		.each([this](flecs::entity e, ImageProcessChunk& chunk) {
			const ImageData* img_data = chunk.img_data;
			if (!img_data || !img_data->ptr) {
				// Fill with black
				for (uint32_t idx = chunk.start_index; idx < chunk.end_index; ++idx) {
					chunk.output_ptr[idx] = Color(0, 0, 0, 1);
				}
				return;
			}

			// Use fast-path for common formats
			if (img_data->format == Image::FORMAT_RGBA8) {
				process_pixels_rgba8(chunk.start_index, chunk.end_index, img_data->width, img_data->height,
				                     img_data->ptr, chunk.mode, chunk.output_ptr, flip_y);
			} else if (img_data->format == Image::FORMAT_RGB8) {
				process_pixels_rgb8(chunk.start_index, chunk.end_index, img_data->width, img_data->height,
				                    img_data->ptr, chunk.mode, chunk.output_ptr, flip_y);
			} else {
				// Fallback to slow path for other formats
				for (uint32_t idx = chunk.start_index; idx < chunk.end_index; ++idx) {
					int x = idx % img_data->width;
					int y = idx / img_data->width;

					Color result;
					if (x >= img_data->width || y >= img_data->height) {
						result = Color(0, 0, 0, 1);
					} else {
						// Use the generic get_pixel for uncommon formats
						result = get_pixel(*img_data, x, y);

						// Apply mode
						switch (chunk.mode) {
							case BASMode::INVERTED:
								result = Color(1.0f - result.r, 1.0f - result.g, 1.0f - result.b, result.a);
								break;
							case BASMode::RANDOM: {
								// Check if pixel is white (or close to white) - brightness > 0.5
								float brightness = (result.r + result.g + result.b) / 3.0f;
								if (brightness > 0.5f) {
									// Replace white with random color based on pixel position
									uint32_t time_seed = OS::get_singleton()->get_ticks_msec();
									uint32_t hash = (idx * 2654435761u) ^ (time_seed * 2246822519u);
									float r = ((hash >> 0) & 0xFF) / 255.0f;
									float g = ((hash >> 8) & 0xFF) / 255.0f;
									float b = ((hash >> 16) & 0xFF) / 255.0f;
									result = Color(r, g, b, result.a);
								}
								// Keep black pixels as black (no change needed)
								break;
							}
							default:
								break;
						}
					}

					chunk.output_ptr[idx] = result;
				}
			}
		});
	bas_process_chunks.set_name("BadAppleSystem/ProcessChunks");

	// OPTIMIZATION: Use direct buffer update instead of per-instance calls
	// Single-threaded flush system that sends results to RenderingServer
	auto bas_flush_results = world->system<>().run([&, cached_mm_rid, total_stride, color_offset, uses_colors](flecs::iter it) {
		// Get instance count from RenderingServer directly
		int instance_count = RS::get_singleton()->multimesh_get_instance_count(cached_mm_rid);
		if (instance_count == 0 || shared_output_buffer.size() == 0) {
			return;
		}

		// Only update if multimesh uses colors
		if (!uses_colors) {
			return;
		}

		// Get current buffer to preserve transform data
		Vector<float> current_buffer = RS::get_singleton()->multimesh_get_buffer(cached_mm_rid);

		// Update color data in buffer
		const Color* colors = shared_output_buffer.ptr();
		float* buffer_ptr = current_buffer.ptrw();

		for (int i = 0; i < instance_count && i < shared_output_buffer.size(); ++i) {
			uint32_t base_offset = i * total_stride + color_offset;
			buffer_ptr[base_offset + 0] = colors[i].r;
			buffer_ptr[base_offset + 1] = colors[i].g;
			buffer_ptr[base_offset + 2] = colors[i].b;
			buffer_ptr[base_offset + 3] = colors[i].a;
		}

		// Send entire buffer in one call (much faster!)
		command_handler->enqueue_command_unpooled([cached_mm_rid, current_buffer]() {
			RS::get_singleton()->multimesh_set_buffer(cached_mm_rid, current_buffer);
		});
	});
	bas_flush_results.set_name("BadAppleSystem/FlushResults");

	// Add all systems to pipeline in order
	pm->add_to_pipeline(bas_get_image_data, flecs::OnUpdate);
	pm->add_to_pipeline(bas_process_chunks, flecs::OnUpdate);
	pm->add_to_pipeline(bas_flush_results, flecs::OnUpdate);
}

RID BadAppleSystem::get_mm_entity() const {
    return gd_mm_entity;
}

void BadAppleSystem::set_mm_entity(const RID& p_mm_entity) {
    gd_mm_entity = p_mm_entity;
    mm_entity = FlecsServer::get_singleton()->_get_entity(p_mm_entity, world_id);
}

void BadAppleSystem::set_video_player(VideoStreamPlayer *p_video_player) {
    video_player = p_video_player;
}

VideoStreamPlayer *BadAppleSystem::get_video_player() const {
    return video_player;
}

void BadAppleSystem::set_world_id(const RID& p_world_id) {
    world_id = p_world_id;
    world = FlecsServer::get_singleton()->_get_world(world_id);
    if (command_handler.is_null()) {
        command_handler = FlecsServer::get_singleton()->get_render_system_command_handler(world_id);
    }
}

RID BadAppleSystem::get_world_id() const {
    return world_id;
}

// Generic pixel access for uncommon formats
Color BadAppleSystem::_get_color_at_ofs(const Image::Format format, const uint8_t *ptr, uint32_t ofs) const {
    switch (format) {
        case Image::FORMAT_L8: {
            float l = ptr[ofs] / 255.0f;
            return Color(l, l, l);
        }
        case Image::FORMAT_LA8: {
            float l = ptr[ofs] / 255.0f;
            float a = ptr[ofs + 1] / 255.0f;
            return Color(l, l, l, a);
        }
        case Image::FORMAT_R8: {
            float r = ptr[ofs] / 255.0f;
            return Color(r, 0, 0);
        }
        case Image::FORMAT_RG8: {
            float r = ptr[ofs] / 255.0f;
            float g = ptr[ofs + 1] / 255.0f;
            return Color(r, g, 0);
        }
        case Image::FORMAT_RGB8: {
            float r = ptr[ofs] / 255.0f;
            float g = ptr[ofs + 1] / 255.0f;
            float b = ptr[ofs + 2] / 255.0f;
            return Color(r, g, b);
        }
        case Image::FORMAT_RGBA8: {
            float r = ptr[ofs] / 255.0f;
            float g = ptr[ofs + 1] / 255.0f;
            float b = ptr[ofs + 2] / 255.0f;
            float a = ptr[ofs + 3] / 255.0f;
            return Color(r, g, b, a);
        }
        default:
            return Color();
    }
}

Color BadAppleSystem::get_pixel(const ImageData& image_data, const int x, const int y) const {
    uint32_t ofs = 0;
    switch (image_data.format) {
        case Image::FORMAT_L8:
        case Image::FORMAT_R8:
            ofs = y * image_data.width + x;
            break;
        case Image::FORMAT_LA8:
        case Image::FORMAT_RG8:
            ofs = (y * image_data.width + x) * 2;
            break;
        case Image::FORMAT_RGB8:
            ofs = (y * image_data.width + x) * 3;
            break;
        case Image::FORMAT_RGBA8:
            ofs = (y * image_data.width + x) * 4;
            break;
        default:
            return Color();
    }
    return _get_color_at_ofs(image_data.format, image_data.ptr, ofs);
}


void BadAppleSystem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("start"), &BadAppleSystem::start);
    ClassDB::bind_method(D_METHOD("set_mm_entity", "mm_entity"), &BadAppleSystem::set_mm_entity);
    ClassDB::bind_method(D_METHOD("get_mm_entity"), &BadAppleSystem::get_mm_entity);
    ClassDB::bind_method(D_METHOD("set_video_player", "video_player"), &BadAppleSystem::set_video_player);
    ClassDB::bind_method(D_METHOD("get_video_player"), &BadAppleSystem::get_video_player);
    ClassDB::bind_method(D_METHOD("set_world_id", "world_id"), &BadAppleSystem::set_world_id);
    ClassDB::bind_method(D_METHOD("get_world_id"), &BadAppleSystem::get_world_id);
    ClassDB::bind_method(D_METHOD("set_mode", "mode"), &BadAppleSystem::set_mode);
    ClassDB::bind_method(D_METHOD("get_mode"), &BadAppleSystem::get_mode);

    // Threading configuration
    ClassDB::bind_method(D_METHOD("set_use_multithreading", "enabled"), &BadAppleSystem::set_use_multithreading);
    ClassDB::bind_method(D_METHOD("get_use_multithreading"), &BadAppleSystem::get_use_multithreading);
    ClassDB::bind_method(D_METHOD("set_threading_threshold", "threshold"), &BadAppleSystem::set_threading_threshold);
    ClassDB::bind_method(D_METHOD("get_threading_threshold"), &BadAppleSystem::get_threading_threshold);
    ClassDB::bind_method(D_METHOD("set_max_threads", "max_threads"), &BadAppleSystem::set_max_threads);
    ClassDB::bind_method(D_METHOD("get_max_threads"), &BadAppleSystem::get_max_threads);

    ADD_PROPERTY(PropertyInfo(Variant::RID, "mm_entity"), "set_mm_entity", "get_mm_entity");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "video_player", PROPERTY_HINT_RESOURCE_TYPE, "VideoStreamPlayer"), "set_video_player", "get_video_player");
    ADD_PROPERTY(PropertyInfo(Variant::RID, "world_id"), "set_world_id", "get_world_id");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "mode"), "set_mode", "get_mode");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_multithreading"), "set_use_multithreading", "get_use_multithreading");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "threading_threshold"), "set_threading_threshold", "get_threading_threshold");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_threads"), "set_max_threads", "get_max_threads");

    ClassDB::bind_method(D_METHOD("set_flip_y", "flip"), &BadAppleSystem::set_flip_y);
    ClassDB::bind_method(D_METHOD("get_flip_y"), &BadAppleSystem::get_flip_y);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flip_y"), "set_flip_y", "get_flip_y");
}
