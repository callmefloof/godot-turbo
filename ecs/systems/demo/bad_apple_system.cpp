#include "bad_apple_system.h"
#include "flecs.h"
#include "flecs_server.h"
#include "scene/resources/texture.h"
#include "ecs/components/rendering/rendering_components.h"
#include "servers/rendering_server.h"
#include "core/math/vector2i.h"
#include "systems/pipeline_manager.h"
#include <optional>


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

    auto bas_get_image_data = world->system().interval(1.0 / 30.0).run([&](flecs::iter it){
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
		// At system tick: prepare the full frame's colors
		static thread_local std::vector<Color> s_frame_colors;
		s_frame_colors.resize(mm_comp.instance_count);

		for (uint32_t idx = 0; idx < mm_comp.instance_count; ++idx) {
			const int x = idx % image_data.width;
			const int y = image_data.height - 1 - (idx / image_data.width) ;
			Color pixel = get_pixel(image_data, x, y);

			// Luminance -> black/white
			float luminance = 0.2126f * pixel.r + 0.7152f * pixel.g + 0.0722f * pixel.b;
			switch(mode) {
				case BASMode::REGULAR:
					s_frame_colors[idx] = (luminance > 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::INVERTED:
					s_frame_colors[idx] = (luminance < 0.5f) ? Color(1, 1, 1, 1) : Color(0, 0, 0, 1);
					break;
				case BASMode::RANDOM:
					s_frame_colors[idx] = luminance > 0.5f ? Color(Math::randf(), Math::randf(), Math::randf(), 1) : Color(0, 0, 0, 1);
					break;
			}
		}

		// Flush once per frame
		RID mm_multi_mesh_id = mm_comp.multi_mesh_id;
		command_handler->enqueue_command_unpooled([mm_multi_mesh_id, frame_colors = std::move(s_frame_colors)]() mutable {
			int rd_count = RS::get_singleton()->multimesh_get_instance_count(mm_multi_mesh_id);
			for (uint32_t idx = 0; idx < frame_colors.size() && idx < (uint32_t)rd_count; ++idx) {
				RS::get_singleton()->multimesh_instance_set_color(mm_multi_mesh_id, idx, frame_colors[idx]);
			}
			frame_colors.clear(); // free memory after use
		});



    });

    bas_get_image_data.set_name("BadAppleSystem/UpdateImageData");
	// auto bas_get_image_data_phase = pm->create_custom_phase("BadAppleSystem/UpdateImageData", "Flecs::OnUpdate");

	pm->add_to_pipeline(bas_get_image_data, flecs::OnUpdate);

}

RID BadAppleSystem::get_mm_entity() const {
    return gd_mm_entity;
}

void BadAppleSystem::set_mm_entity(const RID& mm_entity) {
    gd_mm_entity = mm_entity;
	this->mm_entity = FlecsServer::get_singleton()->_get_entity(mm_entity, world_id);
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

Color BadAppleSystem::get_pixel(const ImageData& image_data, const int x, const int y) const {

    Color color = _get_color_at_ofs(image_data.format, image_data.data.ptr(), y * image_data.width + x);
    return color;
}


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

	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "Regular,Inverted,Random"), "set_mode", "get_mode");

}