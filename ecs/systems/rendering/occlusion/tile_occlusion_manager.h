//
// Created by Floof on 29-7-2025.
//
#ifndef TILE_OCCLUSION_MANAGER_H
#define TILE_OCCLUSION_MANAGER_H
#include "../../../../../../core/templates/vector.h"
#include "../../thirdparty/concurrentqueue/concurrentqueue.h"
#include "tile.h"
#include <atomic>
#include <future>
#include <thread>
#include <vector>

class TileOcclusionManager {
	int num_tiles_x = 0, num_tiles_y = 0;
	Vector<TileBin> tile_bins;
	Vector<TileBuffer> tile_buffers;
	int screen_width = 0, screen_height = 0;
public:
	// Each tile is accessed by only one thread — thread-safe with write[] access.
	void rasterize_all_bins_parallel(const int thread_count);
	void initialize(const int p_screen_width, const int p_screen_height);
	static ScreenAABB compute_2d_aabb(const Vector2& v0, const Vector2& v1, const Vector2& v2);
	void clear_bins();
	void reset();
	void reset(const int p_screen_width, const int p_screen_height);
	void bin_triangles(const Vector<ScreenTriangle> &tris);

	static constexpr float min3(const float a, const float b, const float c) {
		if (a < b && a < c) {
			return a;
		}
		if (b < a && b < c) {
			return b;
		}
		return c;
	}

	static constexpr float max3(const float a, const float b, const float c) {
		if (a > b && a > c) {
			return a;
		}
		if (b > a && b > c) {
			return b;
		}
		return c;
	}

	static Vector3 compute_barycentric(const Vector2 &pix_center, const Vector2 &v0, const Vector2 &v1, const Vector2 &v2);

	static bool is_inside_triangle(const Vector3 &bary);

	static void rasterize_triangle_to_tile(const ScreenTriangle& tri, TileBuffer& tile, const Vector2i & tile_origin);

	void rasterize_all_bins();

	static bool test_visibility(const ScreenAABB &box, const float *occlusion_buffer, const int buffer_width, const int buffer_height);
	bool is_visible(const ScreenAABB &aabb) const;
	std::vector<bool> test_all_visibility_concurrent(const std::vector<ScreenAABB> &boxes, unsigned int thread_count) const;

};
#endif //TILE_OCCLUSION_MANAGER_H
