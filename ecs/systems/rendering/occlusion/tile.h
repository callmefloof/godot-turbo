//
// Created by Floof on 29-7-2025.
//

#ifndef TILE_H
#define TILE_H
#include "core/error/error_macros.h"
#include "core/math/projection.h"
#include "core/math/vector2.h"
#include "core/math/vector2i.h"
#include "core/math/vector3.h"
#include "core/math/transform_3d.h"
#include "core/variant/variant.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include <algorithm>

constexpr int OCCLUSION_WIDTH = 320;
constexpr int OCCLUSION_HEIGHT = 180;
constexpr int TILE_SIZE = 32;
constexpr int TILE_RES = TILE_SIZE; // 1:1 sample-to-pixel in each tile

constexpr int SCREEN_TILES_X = OCCLUSION_WIDTH / TILE_SIZE;
constexpr int SCREEN_TILES_Y = OCCLUSION_HEIGHT / TILE_SIZE;

struct ScreenTriangle {

public:
	ScreenTriangle() {
		z0 = 0;
		z1 = 0;
		z2 = 0;
	}

	ScreenTriangle(const Vector2 &v0, const Vector2 &v1, const Vector2 &v2, float z0, float z1, float z2)
		: v0(v0), v1(v1), v2(v2), z0(z0), z1(z1), z2(z2) {}

	ScreenTriangle(const Dictionary &dict) {
		if(!dict.has("v0")) { WARN_PRINT("Missing v0 in ScreenTriangle dictionary"); }
		v0 = dict["v0"];
		if(!dict.has("v1")) { WARN_PRINT("Missing v1 in ScreenTriangle dictionary"); }
		v1 = dict["v1"];
		if(!dict.has("v2")) { WARN_PRINT("Missing v2 in ScreenTriangle dictionary"); }
		v2 = dict["v2"];
		if(!dict.has("z0")) { WARN_PRINT("Missing z0 in ScreenTriangle dictionary"); }
		z0 = dict["z0"];
		if(!dict.has("z1")) { WARN_PRINT("Missing z1 in ScreenTriangle dictionary"); }
		z1 = dict["z1"];
		if(!dict.has("z2")) { WARN_PRINT("Missing z2 in ScreenTriangle dictionary"); }
		z2 = dict["z2"];
	}
	Dictionary to_dict() const {
		Dictionary dict;
		dict["v0"] = v0;
		dict["v1"] = v1;
		dict["v2"] = v2;
		dict["z0"] = z0;
		dict["z1"] = z1;
		dict["z2"] = z2;
		return dict;
	}

	static Vector<ScreenTriangle> from_dict(const Dictionary &dict) {
		Vector<ScreenTriangle> triangles;
		for (const String key : dict.keys()) {
			if (dict.has(key)) {
				ScreenTriangle triangle(dict[key]);
				triangles.push_back(triangle);
			} else {
				WARN_PRINT("Missing key in ScreenTriangle dictionary: " + key);
			}
		}
		return triangles;
	}

	static Vector2 world_to_screen(const Vector3 &world_pos, const Transform3D &cam_view, const Projection &cam_proj, const Vector2 &screen_size) {
		const Vector3 view_pos = cam_view.xform(world_pos); // World to view space
		const Vector3 clip_pos = cam_proj.xform(view_pos); // View to clip space

		if (clip_pos.z <= 0) {
			// Behind camera, optionally handle or discard
		}

		const Vector2 ndc = Vector2(clip_pos.x, clip_pos.y) / clip_pos.z; // perspective divide

		// Convert from NDC [-1,1] to screen [0, width], [0, height]
		const Vector2 screen_pos = ((ndc + Vector2(1.0f, 1.0f)) * 0.5f) * screen_size;
		return screen_pos;
	}

	static Vector<ScreenTriangle> convert_to_screen_triangles(
    const PackedVector3Array &vertices,
    const PackedInt32Array &indices,
    const Transform3D& cam_view,
    const Projection& cam_proj,
    const Vector2& screen_size) {
    Vector<ScreenTriangle> screen_tris;
    const size_t triangle_count = indices.size() / 3;

    for (size_t i = 0; i < triangle_count; ++i) {
        int idx0 = indices[i*3 + 0];
        int idx1 = indices[i*3 + 1];
        int idx2 = indices[i*3 + 2];

        Vector3 v0_world = vertices[idx0];
        Vector3 v1_world = vertices[idx1];
        Vector3 v2_world = vertices[idx2];

        Vector3 v0_view = cam_view.xform(v0_world);
        Vector3 v1_view = cam_view.xform(v1_world);
        Vector3 v2_view = cam_view.xform(v2_world);

        Vector3 v0_clip = cam_proj.xform(v0_view);
        Vector3 v1_clip = cam_proj.xform(v1_view);
        Vector3 v2_clip = cam_proj.xform(v2_view);

        // Skip triangles behind the camera
        if (v0_clip.z <= 0 || v1_clip.z <= 0 || v2_clip.z <= 0) { continue; }

        // Perspective divide (to get NDC)
        Vector2 v0_ndc = Vector2(v0_clip.x, v0_clip.y) / v0_clip.z;
        Vector2 v1_ndc = Vector2(v1_clip.x, v1_clip.y) / v1_clip.z;
        Vector2 v2_ndc = Vector2(v2_clip.x, v2_clip.y) / v2_clip.z;

        // To screen space coordinates
        Vector2 v0_screen = ((v0_ndc + Vector2(1,1)) * 0.5f) * screen_size;
        Vector2 v1_screen = ((v1_ndc + Vector2(1,1)) * 0.5f) * screen_size;
        Vector2 v2_screen = ((v2_ndc + Vector2(1,1)) * 0.5f) * screen_size;

        // Create ScreenTriangle
        screen_tris.push_back({v0_screen, v1_screen, v2_screen, v0_view.z, v1_view.z, v2_view.z});
    }

	return screen_tris;
}


	Vector2 v0, v1, v2;       // screen-space coordinates
	float z0, z1, z2;         // depth values
};

struct ScreenAABB {
public:
	ScreenAABB() {
		position = Vector2i(0, 0);
		size = Vector2i(0, 0);
		min_z = FLT_MAX;
		max_z = -FLT_MAX;
	}
	~ScreenAABB() = default;
	Vector2i position;
	Vector2i size;
	float min_z = FLT_MAX;
	float max_z = -FLT_MAX;
	constexpr int min_x() const {
		return position.x - size.x;
	}
	constexpr int min_y() const{
		return position.y - size.y;
	}
	constexpr int max_x() const{
		return position.x + size.x;
	}
	constexpr int max_y() const{
		return position.y + size.y;
	}

	static ScreenAABB aabb_to_screen_aabb(
		const AABB &aabb,
		const Vector2i &screen_size,
		const Projection &cam_projection,
		const Transform3D &cam_transform,
		const Vector2 &cam_view_offset
	) {
		ScreenAABB screen_aabb;

		Transform3D cam_view = cam_transform.affine_inverse();

		Vector3 min = aabb.position;
		Vector3 max = aabb.position + aabb.size;

		Vector3 corners[8] = {
			Vector3(min.x, min.y, min.z),
			Vector3(max.x, min.y, min.z),
			Vector3(min.x, max.y, min.z),
			Vector3(max.x, max.y, min.z),
			Vector3(min.x, min.y, max.z),
			Vector3(max.x, min.y, max.z),
			Vector3(min.x, max.y, max.z),
			Vector3(max.x, max.y, max.z),
		};

		Vector2 min_screen = Vector2(FLT_MAX, FLT_MAX);
		Vector2 max_screen = Vector2(-FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 8; ++i) {
			Vector3 view_pos = cam_view.xform(corners[i]);
			Vector3 clip_pos = cam_projection.xform(view_pos);

			if (clip_pos.z <= 0.0f) // Behind camera
				continue;

			screen_aabb.min_z = std::min(screen_aabb.min_z, view_pos.z);
			screen_aabb.max_z = std::max(screen_aabb.max_z, view_pos.z);

			Vector2 ndc = Vector2(clip_pos.x, clip_pos.y) / clip_pos.z;
			Vector2 screen_pos = ((ndc + Vector2(1.0f, 1.0f)) * 0.5f) * screen_size + cam_view_offset;

			min_screen = min_screen.min(screen_pos);
			max_screen = max_screen.max(screen_pos);
		}

		screen_aabb.position = min_screen.floor();
		screen_aabb.size = (max_screen - min_screen).ceil();

		return screen_aabb;
	}
};

struct TileBin {
	Vector<ScreenTriangle> triangles;
	Vector<float> depth_buffer; // tileWidth * tileHeight
	int tile_width;

	void init(const int tile_size) {
		tile_width = tile_size;
		triangles.clear();
		depth_buffer.resize(tile_size * tile_size);
		// Initialize with 0 (far depth)
		for (int i = 0; i < depth_buffer.size(); ++i) {
			depth_buffer.set(i ,0.0f);
		}
	}
};


struct TileBuffer {
	float depth[TILE_RES][TILE_RES] = {}; // Zero-initialized

	void clear() {
		for (int y = 0; y < TILE_RES; y++) {
			for (int x = 0; x < TILE_RES; x++) {
				depth[y][x] = std::numeric_limits<float>::infinity();
			}
		}
	}
};

#endif //TILE_H
