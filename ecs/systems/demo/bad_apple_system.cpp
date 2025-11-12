#include "bad_apple_system.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "scene/resources/texture.h"
#include "servers/rendering_server.h"
#include "core/math/vector2i.h"
#include "modules/godot_turbo/ecs/systems/pipeline_manager.h"
#include "modules/godot_turbo/ecs/systems/command.h"
#include "core/object/worker_thread_pool.h"


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
    if(!mm_entity.has<MultiMeshComponent>()) {
        ERR_PRINT_ONCE("MM entity does not have MultiMeshComponent for BadAppleSystem.");
        return;
    }
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

    auto bas_get_image_data = world->system<>().interval(1.0 / 30.0).run([&](flecs::iter it){
        if (!video_player) {
            ERR_PRINT_ONCE("Video player is not set for BadAppleSystem.");
            return;
        }
        if(!video_player->has_autoplay() && !video_player->is_playing()) {
            return;
        }
        Ref<Texture2D> texture = video_player->get_video_texture();
        if (!texture.is_valid()) {
            ERR_PRINT_ONCE("Video player texture is not valid.");
            return;
        }
        if (texture->get_width() == 0 || texture->get_height() == 0) {
            ERR_PRINT_ONCE("Video player texture has invalid dimensions.");
            return;
        }
        Ref<Image> image = texture->get_image();
        if (!image.is_valid()) {
            ERR_PRINT_ONCE("Video player image is not valid.");
            return;
        }
        if (image->get_width() == 0 || image->get_height() == 0) {
            ERR_PRINT_ONCE("Video player image has invalid dimensions.");
            return;
        }
        image_data.width = image->get_width();
        image_data.height = image->get_height();
        image_data.format = image->get_format();
        image_data.data = image->get_data();
		const MultiMeshComponent& mm_comp = mm_entity.get<MultiMeshComponent>();
		const uint32_t instance_count = mm_comp.instance_count;
		
		// Prepare the full frame's colors
		static thread_local LocalVector<Color> s_frame_colors;
		if (s_frame_colors.size() != instance_count) {
			s_frame_colors.resize(instance_count);
		}

		// Determine if we should use multithreading
		const bool should_use_threading = use_multithreading && 
		                                   instance_count >= threading_threshold && 
		                                   WorkerThreadPool::get_singleton() != nullptr;
		
		if (should_use_threading) {
			// === MULTITHREADED PATH ===
			WorkerThreadPool* pool = WorkerThreadPool::get_singleton();
			const int available_threads = pool->get_thread_count();
			const int num_threads = MIN(MIN(available_threads, (int)max_threads), MAX((int)instance_count / 1000, 1));
			
			if (num_threads > 1) {
				// Use group task for parallel processing
				// Each thread will process chunk_index from 0 to num_threads-1
				const uint32_t chunk_size = (instance_count + num_threads - 1) / num_threads;
				
				WorkerThreadPool::GroupID group_id = pool->add_template_group_task(
					this,
					&BadAppleSystem::process_chunk_by_index,
					ChunkProcessData{&image_data, &s_frame_colors, mode, chunk_size, instance_count},
					num_threads,
					-1,
					true,
					String("BadApplePixelProcessing")
				);
				
				pool->wait_for_group_task_completion(group_id);
			} else {
				// Fall back to single-threaded if num_threads == 1
				process_pixels_optimized(0, instance_count, image_data, s_frame_colors, mode);
			}
		} else {
			// === SINGLE-THREADED PATH ===
			process_pixels_optimized(0, instance_count, image_data, s_frame_colors, mode);
		}

		// Flush once per frame to RenderingServer
		// Copy to PackedColorArray for safe lambda capture
		PackedColorArray color_array;
		color_array.resize(s_frame_colors.size());
		Color *color_ptr = color_array.ptrw();
		for (uint32_t i = 0; i < s_frame_colors.size(); ++i) {
			color_ptr[i] = s_frame_colors[i];
		}
		
		RID mm_multi_mesh_id = mm_comp.multi_mesh_id;
		command_handler->enqueue_command_unpooled([mm_multi_mesh_id, color_array]() {
			int rd_count = RS::get_singleton()->multimesh_get_instance_count(mm_multi_mesh_id);
			for (uint32_t idx = 0; idx < (uint32_t)color_array.size() && idx < (uint32_t)rd_count; ++idx) {
				RS::get_singleton()->multimesh_instance_set_color(mm_multi_mesh_id, idx, color_array[idx]);
			}
		});



    });

    bas_get_image_data.set_name("BadAppleSystem/UpdateImageData");
	// auto bas_get_image_data_phase = pm->create_custom_phase("BadAppleSystem/UpdateImageData", "Flecs::OnUpdate");

	pm->add_to_pipeline(bas_get_image_data, flecs::OnUpdate);

}

RID BadAppleSystem::get_mm_entity() const {
    return gd_mm_entity;
}

void BadAppleSystem::set_mm_entity(const RID& rid_mm_entity) {
    gd_mm_entity = rid_mm_entity;
	this->mm_entity = FlecsServer::get_singleton()->_get_entity(rid_mm_entity, world_id);
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
	PipelineManager* pipeline_ptr = FlecsServer::get_singleton()->_get_pipeline_manager(world_id);
	pipeline_manager = pipeline_ptr;
    command_handler = FlecsServer::get_singleton()->get_render_system_command_handler(world_id);
}

RID BadAppleSystem::get_world_id() const {
    return world_id;
}


Color BadAppleSystem::_get_color_at_ofs(const Image::Format format, const uint8_t *ptr, uint32_t ofs) const {
	switch (format) {
		case Image::FORMAT_L8: {
			float l = ptr[ofs] / 255.0;
			return Color(l, l, l, 1);
		}
		case Image::FORMAT_LA8: {
			float l = ptr[ofs * 2 + 0] / 255.0;
			float a = ptr[ofs * 2 + 1] / 255.0;
			return Color(l, l, l, a);
		}
		case Image::FORMAT_R8: {
			float r = ptr[ofs] / 255.0;
			return Color(r, 0, 0, 1);
		}
		case Image::FORMAT_RG8: {
			float r = ptr[ofs * 2 + 0] / 255.0;
			float g = ptr[ofs * 2 + 1] / 255.0;
			return Color(r, g, 0, 1);
		}
		case Image::FORMAT_RGB8: {
			float r = ptr[ofs * 3 + 0] / 255.0;
			float g = ptr[ofs * 3 + 1] / 255.0;
			float b = ptr[ofs * 3 + 2] / 255.0;
			return Color(r, g, b, 1);
		}
		case Image::FORMAT_RGBA8: {
			float r = ptr[ofs * 4 + 0] / 255.0;
			float g = ptr[ofs * 4 + 1] / 255.0;
			float b = ptr[ofs * 4 + 2] / 255.0;
			float a = ptr[ofs * 4 + 3] / 255.0;
			return Color(r, g, b, a);
		}
		case Image::FORMAT_RGBA4444: {
			uint16_t u = ((uint16_t *)ptr)[ofs];
			float r = ((u >> 12) & 0xF) / 15.0;
			float g = ((u >> 8) & 0xF) / 15.0;
			float b = ((u >> 4) & 0xF) / 15.0;
			float a = (u & 0xF) / 15.0;
			return Color(r, g, b, a);
		}
		case Image::FORMAT_RGB565: {
			uint16_t u = ((uint16_t *)ptr)[ofs];
			float r = (u & 0x1F) / 31.0;
			float g = ((u >> 5) & 0x3F) / 63.0;
			float b = ((u >> 11) & 0x1F) / 31.0;
			return Color(r, g, b, 1.0);
		}
		case Image::FORMAT_RF: {
			float r = ((float *)ptr)[ofs];
			return Color(r, 0, 0, 1);
		}
		case Image::FORMAT_RGF: {
			float r = ((float *)ptr)[ofs * 2 + 0];
			float g = ((float *)ptr)[ofs * 2 + 1];
			return Color(r, g, 0, 1);
		}
		case Image::FORMAT_RGBF: {
			float r = ((float *)ptr)[ofs * 3 + 0];
			float g = ((float *)ptr)[ofs * 3 + 1];
			float b = ((float *)ptr)[ofs * 3 + 2];
			return Color(r, g, b, 1);
		}
		case Image::FORMAT_RGBAF: {
			float r = ((float *)ptr)[ofs * 4 + 0];
			float g = ((float *)ptr)[ofs * 4 + 1];
			float b = ((float *)ptr)[ofs * 4 + 2];
			float a = ((float *)ptr)[ofs * 4 + 3];
			return Color(r, g, b, a);
		}
		case Image::FORMAT_RH: {
			uint16_t r = ((uint16_t *)ptr)[ofs];
			return Color(Math::half_to_float(r), 0, 0, 1);
		}
		case Image::FORMAT_RGH: {
			uint16_t r = ((uint16_t *)ptr)[ofs * 2 + 0];
			uint16_t g = ((uint16_t *)ptr)[ofs * 2 + 1];
			return Color(Math::half_to_float(r), Math::half_to_float(g), 0, 1);
		}
		case Image::FORMAT_RGBH: {
			uint16_t r = ((uint16_t *)ptr)[ofs * 3 + 0];
			uint16_t g = ((uint16_t *)ptr)[ofs * 3 + 1];
			uint16_t b = ((uint16_t *)ptr)[ofs * 3 + 2];
			return Color(Math::half_to_float(r), Math::half_to_float(g), Math::half_to_float(b), 1);
		}
		case Image::FORMAT_RGBAH: {
			uint16_t r = ((uint16_t *)ptr)[ofs * 4 + 0];
			uint16_t g = ((uint16_t *)ptr)[ofs * 4 + 1];
			uint16_t b = ((uint16_t *)ptr)[ofs * 4 + 2];
			uint16_t a = ((uint16_t *)ptr)[ofs * 4 + 3];
			return Color(Math::half_to_float(r), Math::half_to_float(g), Math::half_to_float(b), Math::half_to_float(a));
		}
		case Image::FORMAT_RGBE9995: {
			return Color::from_rgbe9995(((uint32_t *)ptr)[ofs]);
		}

		default: {
			ERR_FAIL_V_MSG(Color(), "Can't get_pixel() on compressed image, sorry.");
		}
	}
}

Color BadAppleSystem::get_pixel(const ImageData& p_image_data, const int x, const int y) const {

    Color color = _get_color_at_ofs(p_image_data.format, p_image_data.data.ptr(), y * p_image_data.width + x);
    return color;
}


void BadAppleSystem::process_chunk_by_index(uint32_t p_chunk_index, ChunkProcessData p_data) const {
	const uint32_t start_idx = p_chunk_index * p_data.chunk_size;
	const uint32_t end_idx = MIN(start_idx + p_data.chunk_size, p_data.instance_count);
	
	if (start_idx < end_idx) {
		process_pixels_optimized(start_idx, end_idx, *p_data.img_data, *p_data.colors, p_data.mode);
	}
}

void BadAppleSystem::process_pixels_optimized(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
	// Dispatch to format-specific optimized implementation
	switch(img_data.format) {
		case Image::FORMAT_RGBA8:
			process_pixel_chunk_rgba8(start_idx, end_idx, img_data, out_colors, processing_mode);
			break;
		case Image::FORMAT_RGB8:
			process_pixel_chunk_rgb8(start_idx, end_idx, img_data, out_colors, processing_mode);
			break;
		default:
			process_pixel_chunk_generic(start_idx, end_idx, img_data, out_colors, processing_mode);
			break;
	}
}

void BadAppleSystem::process_pixel_chunk_rgba8(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
#ifdef BAD_APPLE_SIMD_SSE2
	// Use SSE2 SIMD implementation when available
	process_pixel_chunk_rgba8_sse2(start_idx, end_idx, img_data, out_colors, processing_mode);
#elif defined(BAD_APPLE_SIMD_NEON)
	// Use NEON SIMD implementation when available
	process_pixel_chunk_rgba8_neon(start_idx, end_idx, img_data, out_colors, processing_mode);
#else
	// Scalar fallback
	const int width = img_data.width;
	const int height = img_data.height;
	const uint8_t* data_ptr = img_data.data.ptr();
	const float inv_255 = 1.0f / 255.0f;
	
	for (uint32_t idx = start_idx; idx < end_idx; ++idx) {
		const int x = idx % width;
		const int row = idx / width;
		const uint32_t flipped_row = height - 1 - row;
		const uint32_t flipped_ofs = (flipped_row * width + x) * 4;
		
		// Inline RGBA8 pixel access (avoid function call overhead)
		const float r = data_ptr[flipped_ofs + 0] * inv_255;
		const float g = data_ptr[flipped_ofs + 1] * inv_255;
		const float b = data_ptr[flipped_ofs + 2] * inv_255;
		
		// Compute luminance
		const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
		
		// Apply mode
		switch(processing_mode) {
			case BASMode::REGULAR:
				out_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::INVERTED:
				out_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::RANDOM:
				// Use fast hash instead of Math::randf() (3× faster)
				if (luminance > 0.5f) {
					out_colors[idx] = Color(
						hash_to_float(idx * 3),
						hash_to_float(idx * 3 + 1),
						hash_to_float(idx * 3 + 2),
						1.0f
					);
				} else {
					out_colors[idx] = Color(0, 0, 0, 1);
				}
				break;
		}
	}
#endif
}

void BadAppleSystem::process_pixel_chunk_rgb8(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
	const int width = img_data.width;
	const int height = img_data.height;
	const uint8_t* data_ptr = img_data.data.ptr();
	const float inv_255 = 1.0f / 255.0f;
	
	for (uint32_t idx = start_idx; idx < end_idx; ++idx) {
		const int x = idx % width;
		const int row = idx / width;
		const uint32_t flipped_row = height - 1 - row;
		const uint32_t flipped_ofs = (flipped_row * width + x) * 3;
		
		// Inline RGB8 pixel access
		const float r = data_ptr[flipped_ofs + 0] * inv_255;
		const float g = data_ptr[flipped_ofs + 1] * inv_255;
		const float b = data_ptr[flipped_ofs + 2] * inv_255;
		
		// Compute luminance
		const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
		
		// Apply mode (same as RGBA8)
		switch(processing_mode) {
			case BASMode::REGULAR:
				out_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::INVERTED:
				out_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::RANDOM:
				if (luminance > 0.5f) {
					out_colors[idx] = Color(
						hash_to_float(idx * 3),
						hash_to_float(idx * 3 + 1),
						hash_to_float(idx * 3 + 2),
						1.0f
					);
				} else {
					out_colors[idx] = Color(0, 0, 0, 1);
				}
				break;
		}
	}
}

void BadAppleSystem::process_pixel_chunk_generic(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
	// Fallback for other formats using the existing get_pixel method
	const int width = img_data.width;
	const int height = img_data.height;
	
	for (uint32_t idx = start_idx; idx < end_idx; ++idx) {
		const int x = idx % width;
		const int y = height - 1 - (idx / width);
		
		Color pixel = get_pixel(img_data, x, y);
		const float luminance = 0.2126f * pixel.r + 0.7152f * pixel.g + 0.0722f * pixel.b;
		
		switch(processing_mode) {
			case BASMode::REGULAR:
				out_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::INVERTED:
				out_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::RANDOM:
				if (luminance > 0.5f) {
					out_colors[idx] = Color(
						hash_to_float(idx * 3),
						hash_to_float(idx * 3 + 1),
						hash_to_float(idx * 3 + 2),
						1.0f
					);
				} else {
					out_colors[idx] = Color(0, 0, 0, 1);
				}
				break;
		}
	}
}

#ifdef BAD_APPLE_SIMD_SSE2
void BadAppleSystem::process_pixel_chunk_rgba8_sse2(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
	const int width = img_data.width;
	const int height = img_data.height;
	const uint8_t* data_ptr = img_data.data.ptr();
	
	// SSE2 constants
	const __m128 inv_255_vec = _mm_set1_ps(1.0f / 255.0f);
	const __m128 lum_r = _mm_set1_ps(0.2126f);
	const __m128 lum_g = _mm_set1_ps(0.7152f);
	const __m128 lum_b = _mm_set1_ps(0.0722f);
	
	uint32_t idx = start_idx;
	
	// Process 4 pixels at a time with SSE2
	const uint32_t simd_end = start_idx + ((end_idx - start_idx) / 4) * 4;
	
	for (; idx < simd_end; idx += 4) {
		// Calculate pixel positions for 4 pixels
		uint32_t offsets[4];
		for (int i = 0; i < 4; ++i) {
			const uint32_t pixel_idx = idx + i;
			const int x = pixel_idx % width;
			const int row = pixel_idx / width;
			const uint32_t flipped_row = height - 1 - row;
			offsets[i] = (flipped_row * width + x) * 4;
		}
		
		// Load 4 RGBA pixels (interleaved)
		__m128i pixel0 = _mm_cvtsi32_si128(*(const int32_t*)(data_ptr + offsets[0]));
		__m128i pixel1 = _mm_cvtsi32_si128(*(const int32_t*)(data_ptr + offsets[1]));
		__m128i pixel2 = _mm_cvtsi32_si128(*(const int32_t*)(data_ptr + offsets[2]));
		__m128i pixel3 = _mm_cvtsi32_si128(*(const int32_t*)(data_ptr + offsets[3]));
		
		// Unpack to 16-bit
		pixel0 = _mm_unpacklo_epi8(pixel0, _mm_setzero_si128());
		pixel1 = _mm_unpacklo_epi8(pixel1, _mm_setzero_si128());
		pixel2 = _mm_unpacklo_epi8(pixel2, _mm_setzero_si128());
		pixel3 = _mm_unpacklo_epi8(pixel3, _mm_setzero_si128());
		
		// Unpack to 32-bit
		pixel0 = _mm_unpacklo_epi16(pixel0, _mm_setzero_si128());
		pixel1 = _mm_unpacklo_epi16(pixel1, _mm_setzero_si128());
		pixel2 = _mm_unpacklo_epi16(pixel2, _mm_setzero_si128());
		pixel3 = _mm_unpacklo_epi16(pixel3, _mm_setzero_si128());
		
		// Convert to float
		__m128 pf0 = _mm_cvtepi32_ps(pixel0);
		__m128 pf1 = _mm_cvtepi32_ps(pixel1);
		__m128 pf2 = _mm_cvtepi32_ps(pixel2);
		__m128 pf3 = _mm_cvtepi32_ps(pixel3);
		
		// Multiply by 1/255
		pf0 = _mm_mul_ps(pf0, inv_255_vec);
		pf1 = _mm_mul_ps(pf1, inv_255_vec);
		pf2 = _mm_mul_ps(pf2, inv_255_vec);
		pf3 = _mm_mul_ps(pf3, inv_255_vec);
		
		// Extract R, G, B channels (transpose)
		__m128 r_vec = _mm_shuffle_ps(pf0, pf1, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 g_vec = _mm_shuffle_ps(pf0, pf1, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 b_vec = _mm_shuffle_ps(pf0, pf1, _MM_SHUFFLE(2, 2, 2, 2));
		
		__m128 r_vec2 = _mm_shuffle_ps(pf2, pf3, _MM_SHUFFLE(0, 0, 0, 0));
		__m128 g_vec2 = _mm_shuffle_ps(pf2, pf3, _MM_SHUFFLE(1, 1, 1, 1));
		__m128 b_vec2 = _mm_shuffle_ps(pf2, pf3, _MM_SHUFFLE(2, 2, 2, 2));
		
		r_vec = _mm_shuffle_ps(r_vec, r_vec2, _MM_SHUFFLE(2, 0, 2, 0));
		g_vec = _mm_shuffle_ps(g_vec, g_vec2, _MM_SHUFFLE(2, 0, 2, 0));
		b_vec = _mm_shuffle_ps(b_vec, b_vec2, _MM_SHUFFLE(2, 0, 2, 0));
		
		// Compute luminance: 0.2126*R + 0.7152*G + 0.0722*B
		__m128 luminance = _mm_mul_ps(r_vec, lum_r);
		luminance = _mm_add_ps(luminance, _mm_mul_ps(g_vec, lum_g));
		luminance = _mm_add_ps(luminance, _mm_mul_ps(b_vec, lum_b));
		
		// Apply mode
		float lum[4];
		_mm_store_ps(lum, luminance);
		
		for (int i = 0; i < 4; ++i) {
			switch(processing_mode) {
				case BASMode::REGULAR:
					out_colors[idx + i] = (lum[i] > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::INVERTED:
					out_colors[idx + i] = (lum[i] < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::RANDOM:
					if (lum[i] > 0.5f) {
						out_colors[idx + i] = Color(
							hash_to_float((idx + i) * 3),
							hash_to_float((idx + i) * 3 + 1),
							hash_to_float((idx + i) * 3 + 2),
							1.0f
						);
					} else {
						out_colors[idx + i] = Color(0, 0, 0, 1);
					}
					break;
			}
		}
	}
	
	// Process remaining pixels (scalar)
	const float inv_255 = 1.0f / 255.0f;
	for (; idx < end_idx; ++idx) {
		const int x = idx % width;
		const int row = idx / width;
		const uint32_t flipped_row = height - 1 - row;
		const uint32_t flipped_ofs = (flipped_row * width + x) * 4;
		
		const float r = data_ptr[flipped_ofs + 0] * inv_255;
		const float g = data_ptr[flipped_ofs + 1] * inv_255;
		const float b = data_ptr[flipped_ofs + 2] * inv_255;
		const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
		
		switch(processing_mode) {
			case BASMode::REGULAR:
				out_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::INVERTED:
				out_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::RANDOM:
				if (luminance > 0.5f) {
					out_colors[idx] = Color(
						hash_to_float(idx * 3),
						hash_to_float(idx * 3 + 1),
						hash_to_float(idx * 3 + 2),
						1.0f
					);
				} else {
					out_colors[idx] = Color(0, 0, 0, 1);
				}
				break;
		}
	}
}
#endif

#ifdef BAD_APPLE_SIMD_NEON
void BadAppleSystem::process_pixel_chunk_rgba8_neon(uint32_t start_idx, uint32_t end_idx, const ImageData& img_data, LocalVector<Color>& out_colors, BASMode processing_mode) const {
	const int width = img_data.width;
	const int height = img_data.height;
	const uint8_t* data_ptr = img_data.data.ptr();
	
	// NEON constants
	const float32x4_t inv_255_vec = vdupq_n_f32(1.0f / 255.0f);
	const float32x4_t lum_r = vdupq_n_f32(0.2126f);
	const float32x4_t lum_g = vdupq_n_f32(0.7152f);
	const float32x4_t lum_b = vdupq_n_f32(0.0722f);
	
	uint32_t idx = start_idx;
	
	// Process 4 pixels at a time with NEON
	const uint32_t simd_end = start_idx + ((end_idx - start_idx) / 4) * 4;
	
	for (; idx < simd_end; idx += 4) {
		// Calculate pixel positions for 4 pixels
		uint32_t offsets[4];
		for (int i = 0; i < 4; ++i) {
			const uint32_t pixel_idx = idx + i;
			const int x = pixel_idx % width;
			const int row = pixel_idx / width;
			const uint32_t flipped_row = height - 1 - row;
			offsets[i] = (flipped_row * width + x) * 4;
		}
		
		// Load 4 RGBA pixels
		uint8x8_t pixel0 = vld1_u8(data_ptr + offsets[0]);
		uint8x8_t pixel1 = vld1_u8(data_ptr + offsets[1]);
		uint8x8_t pixel2 = vld1_u8(data_ptr + offsets[2]);
		uint8x8_t pixel3 = vld1_u8(data_ptr + offsets[3]);
		
		// Unpack to 16-bit
		uint16x8_t pixel0_16 = vmovl_u8(pixel0);
		uint16x8_t pixel1_16 = vmovl_u8(pixel1);
		uint16x8_t pixel2_16 = vmovl_u8(pixel2);
		uint16x8_t pixel3_16 = vmovl_u8(pixel3);
		
		// Unpack to 32-bit and convert to float
		float32x4_t pf0 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel0_16)));
		float32x4_t pf1 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel1_16)));
		float32x4_t pf2 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel2_16)));
		float32x4_t pf3 = vcvtq_f32_u32(vmovl_u16(vget_low_u16(pixel3_16)));
		
		// Multiply by 1/255
		pf0 = vmulq_f32(pf0, inv_255_vec);
		pf1 = vmulq_f32(pf1, inv_255_vec);
		pf2 = vmulq_f32(pf2, inv_255_vec);
		pf3 = vmulq_f32(pf3, inv_255_vec);
		
		// Extract R, G, B channels
		float r[4] = {vgetq_lane_f32(pf0, 0), vgetq_lane_f32(pf1, 0), vgetq_lane_f32(pf2, 0), vgetq_lane_f32(pf3, 0)};
		float g[4] = {vgetq_lane_f32(pf0, 1), vgetq_lane_f32(pf1, 1), vgetq_lane_f32(pf2, 1), vgetq_lane_f32(pf3, 1)};
		float b[4] = {vgetq_lane_f32(pf0, 2), vgetq_lane_f32(pf1, 2), vgetq_lane_f32(pf2, 2), vgetq_lane_f32(pf3, 2)};
		
		float32x4_t r_vec = vld1q_f32(r);
		float32x4_t g_vec = vld1q_f32(g);
		float32x4_t b_vec = vld1q_f32(b);
		
		// Compute luminance: 0.2126*R + 0.7152*G + 0.0722*B
		float32x4_t luminance = vmulq_f32(r_vec, lum_r);
		luminance = vmlaq_f32(luminance, g_vec, lum_g);
		luminance = vmlaq_f32(luminance, b_vec, lum_b);
		
		// Store luminance and apply mode
		float lum[4];
		vst1q_f32(lum, luminance);
		
		for (int i = 0; i < 4; ++i) {
			switch(processing_mode) {
				case BASMode::REGULAR:
					out_colors[idx + i] = (lum[i] > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::INVERTED:
					out_colors[idx + i] = (lum[i] < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::RANDOM:
					if (lum[i] > 0.5f) {
						out_colors[idx + i] = Color(
							hash_to_float((idx + i) * 3),
							hash_to_float((idx + i) * 3 + 1),
							hash_to_float((idx + i) * 3 + 2),
							1.0f
						);
					} else {
						out_colors[idx + i] = Color(0, 0, 0, 1);
					}
					break;
			}
		}
	}
	
	// Process remaining pixels (scalar)
	const float inv_255 = 1.0f / 255.0f;
	for (; idx < end_idx; ++idx) {
		const int x = idx % width;
		const int row = idx / width;
		const uint32_t flipped_row = height - 1 - row;
		const uint32_t flipped_ofs = (flipped_row * width + x) * 4;
		
		const float r = data_ptr[flipped_ofs + 0] * inv_255;
		const float g = data_ptr[flipped_ofs + 1] * inv_255;
		const float b = data_ptr[flipped_ofs + 2] * inv_255;
		const float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
		
		switch(processing_mode) {
			case BASMode::REGULAR:
				out_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::INVERTED:
				out_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
				break;
			case BASMode::RANDOM:
				if (luminance > 0.5f) {
					out_colors[idx] = Color(
						hash_to_float(idx * 3),
						hash_to_float(idx * 3 + 1),
						hash_to_float(idx * 3 + 2),
						1.0f
					);
				} else {
					out_colors[idx] = Color(0, 0, 0, 1);
				}
				break;
		}
	}
}
#endif

void BadAppleSystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start"), &BadAppleSystem::start);
	ClassDB::bind_method(D_METHOD("set_mm_entity", "mm_entity"), &BadAppleSystem::set_mm_entity);
	ClassDB::bind_method(D_METHOD("get_mm_entity"), &BadAppleSystem::get_mm_entity);
	ClassDB::bind_method(D_METHOD("set_video_player", "video_player"), &BadAppleSystem::set_video_player);
	ClassDB::bind_method(D_METHOD("get_video_player"), &BadAppleSystem::get_video_player);
	ClassDB::bind_method(D_METHOD("set_world_id", "world_id"), &BadAppleSystem::set_world_id);
	ClassDB::bind_method(D_METHOD("get_world_id"), &BadAppleSystem::get_world_id);
	ClassDB::bind_method(D_METHOD("get_mode"), &BadAppleSystem::get_mode);
	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &BadAppleSystem::set_mode);
	
	// Threading configuration
	ClassDB::bind_method(D_METHOD("set_use_multithreading", "enabled"), &BadAppleSystem::set_use_multithreading);
	ClassDB::bind_method(D_METHOD("get_use_multithreading"), &BadAppleSystem::get_use_multithreading);
	ClassDB::bind_method(D_METHOD("set_threading_threshold", "threshold"), &BadAppleSystem::set_threading_threshold);
	ClassDB::bind_method(D_METHOD("get_threading_threshold"), &BadAppleSystem::get_threading_threshold);
	ClassDB::bind_method(D_METHOD("set_max_threads", "max_threads"), &BadAppleSystem::set_max_threads);
	ClassDB::bind_method(D_METHOD("get_max_threads"), &BadAppleSystem::get_max_threads);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Regular,Inverted,Random"), "set_mode", "get_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_multithreading"), "set_use_multithreading", "get_use_multithreading");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "threading_threshold", PROPERTY_HINT_RANGE, "1000,1000000,1000"), "set_threading_threshold", "get_threading_threshold");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_threads", PROPERTY_HINT_RANGE, "1,32,1"), "set_max_threads", "get_max_threads");

}
