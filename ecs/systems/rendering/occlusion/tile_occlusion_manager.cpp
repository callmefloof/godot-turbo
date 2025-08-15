#include "tile_occlusion_manager.h"
#include "core/math/vector2i.h"
#include "core/math/vector2.h"


// Each tile is accessed by only one thread — thread-safe with write[] access.
void TileOcclusionManager::rasterize_all_bins_parallel(const int thread_count) {
    const int total_tiles = num_tiles_x * num_tiles_y;

    // Divide into ranges
    const int tiles_per_thread = (total_tiles + thread_count - 1) / thread_count;

    std::vector<std::thread> threads;

    for (int t = 0; t < thread_count; ++t) {
        const int start = t * tiles_per_thread;
        const int end = std::min(start + tiles_per_thread, total_tiles);

        threads.emplace_back([=]() {
            for (int i = start; i < end; ++i) {
                const int tx = i % num_tiles_x;
                const int ty = i / num_tiles_x;

                Vector2i tile_origin(tx * TILE_SIZE, ty * TILE_SIZE);
                if(tile_buffers.size() == 0) {
                    tile_buffers.resize(total_tiles);
                }
                tile_buffers.write[i].clear();

                for (const Ref<ScreenTriangle> &tri : tile_bins[i].triangles) {
                    rasterize_triangle_to_tile(tri, tile_buffers.write[i], tile_origin);
                }
            }
        });
    }

    // Join all threads
    for (auto &thread : threads) {
        thread.join();
    }
}

void TileOcclusionManager::initialize(const int p_screen_width, const int p_screen_height) {
    static_assert(TILE_SIZE == TILE_RES, "TILE_SIZE and TILE_RES must match for raster logic");
    screen_width = p_screen_width;
    screen_height = p_screen_height;
    num_tiles_x = (screen_width + TILE_SIZE - 1) / TILE_SIZE;
    num_tiles_y = (screen_height + TILE_SIZE - 1) / TILE_SIZE;
    tile_bins.resize(num_tiles_x * num_tiles_y);
    for (int i = 0; i < tile_bins.size(); i++) {
        tile_bins.get(i).init(TILE_SIZE);
    }
}
Ref<ScreenAABB> TileOcclusionManager::compute_2d_aabb(const Vector2& v0, const Vector2& v1, const Vector2& v2) {
    const float min_x = std::min({ v0.x, v1.x, v2.x });
    const float max_x = std::max({ v0.x, v1.x, v2.x });
    const float min_y = std::min({ v0.y, v1.y, v2.y });
    const float max_y = std::max({v0.y, v1.y, v2.y});

    Ref<ScreenAABB> aabb = memnew(ScreenAABB);
    aabb->position = Vector2i(static_cast<int>(min_x), static_cast<int>(min_y));
    aabb->size = Vector2i(static_cast<int>(max_x - min_x), static_cast<int>(max_y - min_y));
    return aabb;
}

void TileOcclusionManager::clear_bins()  {
    for (int i = 0; i < tile_bins.size(); i++) {
        tile_bins.write[i].triangles.clear();
    }
}
void TileOcclusionManager::reset()  {
    clear_bins();
    initialize(screen_width, screen_height);
}

void TileOcclusionManager::reset(const int32_t p_screen_width, const int32_t p_screen_height)  {
    clear_bins();
    initialize(p_screen_width, p_screen_height);
}

void TileOcclusionManager::bin_triangles(const Vector<Ref<ScreenTriangle>> &tris) {
    for (auto &tri : tris) {
        Ref<ScreenAABB> bb = compute_2d_aabb(tri->v0, tri->v1, tri->v2);

        const int min_tile_x = CLAMP(bb->min_x() / TILE_SIZE, 0, num_tiles_x - 1);
        const int max_tile_x = CLAMP(bb->max_x() / TILE_SIZE, 0, num_tiles_x - 1);
        const int min_tile_y = CLAMP(bb->min_y() / TILE_SIZE, 0, num_tiles_y - 1);
        const int max_tile_y = CLAMP(bb->max_y() / TILE_SIZE, 0, num_tiles_y - 1);

        for (int ty = min_tile_y; ty <= max_tile_y; ++ty) {
            for (int tx = min_tile_x; tx <= max_tile_x; ++tx) {
                tile_bins.write[ty * num_tiles_x + tx].triangles.push_back(tri);
            }
        }
    }
}

Vector3 TileOcclusionManager::compute_barycentric(const Vector2 &pix_center, const Vector2 &v0, const Vector2 &v1, const Vector2 &v2) {
    const Vector2 vb0 = v1 - v0;
    const Vector2 vb1 = v2 - v0;
    const Vector2 vb2 = pix_center - v0;
    const float d00 = vb0.dot(vb0);
    const float d01 = vb0.dot(vb1);
    const float d11 = vb1.dot(vb1);
    const float d20 = vb2.dot(vb0);
    const float d21 = vb2.dot(vb1);
    const float denom = d00 * d11 - d01 * d01;
    if (Math::is_zero_approx(denom)) {
        return Vector3(-1, -1, -1); // invalid
    }
    const float v = (d11 * d20 - d01 * d21) / denom;
    const float w = (d00 * d21 - d01 * d20) / denom;
    const float u = 1.0f - v - w;
    return Vector3(u, v, w);
}

bool TileOcclusionManager::is_inside_triangle(const Vector3 &bary){
    return bary.x >= 0.0f && bary.y >= 0.0f && bary.z >= 0.0f;
}

void TileOcclusionManager::rasterize_triangle_to_tile(const Ref<ScreenTriangle>& tri, TileBuffer& tile, const Vector2i & tile_origin) {
    const Vector2 v0 = tri->v0 - tile_origin;
    const Vector2 v1 = tri->v1 - tile_origin;
    const Vector2 v2 = tri->v2 - tile_origin;

    // 2D bounding box in tile space
    int min_x = floor(min3(v0.x, v1.x, v2.x));
    int max_x = ceil(max3(v0.x, v1.x, v2.x));
    int min_y = floor(min3(v0.y, v1.y, v2.y));
    int max_y = ceil(max3(v0.y, v1.y, v2.y));

    // Clamp to tile bounds
    min_x = CLAMP(min_x, 0, TILE_RES - 1);
    max_x = CLAMP(max_x, 0, TILE_RES - 1);
    min_y = CLAMP(min_y, 0, TILE_RES - 1);
    max_y = CLAMP(max_y, 0, TILE_RES - 1);

    for (int y = min_y; y <= max_y; y++) {

        for (int x = min_x; x <= max_x; x++) {
            auto p = Vector2(x + 0.5f, y + 0.5f); // Pixel center
            if (Vector3 bary = compute_barycentric(p, v0, v1, v2); is_inside_triangle(bary)) {
                // ReSharper disable once CppDFAUnreachableCode
                if (const float z = bary.x * tri->z0 + bary.y * tri->z1 + bary.z * tri->z2; z < tile.depth[y][x]) {
                    tile.depth[y][x] = z;
                }
            }
        }
    }
}

void TileOcclusionManager::rasterize_all_bins() {
    for (int tile_y = 0; tile_y < num_tiles_y; ++tile_y) {
        for (int tile_x = 0; tile_x < num_tiles_x; ++tile_x) {
            const int idx = tile_y * num_tiles_x + tile_x;
            tile_buffers.write[idx].clear();
            Vector2i tile_origin(tile_x * TILE_SIZE, tile_y * TILE_SIZE);
            auto &tris = tile_bins[idx].triangles;
            for (const Ref<ScreenTriangle> &tri : tris) {
                rasterize_triangle_to_tile(tri, tile_buffers.write[idx], tile_origin);
            }
        }
    }
}

bool TileOcclusionManager::test_visibility(const Ref<ScreenAABB> &box, const float *occlusion_buffer, const int buffer_width, const int buffer_height) {
    const int min_x = std::clamp(box->min_x(), 0, buffer_width - 1);
    const int max_x = std::clamp(box->max_x(), 0, buffer_width - 1);
    const int min_y = std::clamp(box->min_y(), 0, buffer_height - 1);
    const int max_y = std::clamp(box->max_y(), 0, buffer_height - 1);

    // Sample some points (e.g. corners and center) for a coarse test
    constexpr int sample_count = 5;

    for (int i = 0; i < sample_count; ++i) {
        const Vector2 sample_offsets[sample_count] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
            { 0.5f, 0.5f }
        };
        const int sx = min_x + static_cast<int>((max_x - min_x) * sample_offsets[i].x);
        const int sy = min_y + static_cast<int>((max_y - min_y) * sample_offsets[i].y);

        const int index = sy * buffer_width + sx;
        if (index < 0 || index >= buffer_width * buffer_height) {
            continue;
        }

        // If our object is closer than what's already drawn, it's visible
        if (const float occluder_depth = occlusion_buffer[index]; box->min_z < occluder_depth - 0.01f) { // Add epsilon to prevent z-fighting errors
            return true;
        }
    }

    return false; // Fully occluded
}

bool TileOcclusionManager::is_visible(const Ref<ScreenAABB> &aabb) const {
    for (int ty = aabb->min_y() / TILE_RES; ty <= aabb->max_y() / TILE_RES; ++ty) {
        for (int tx = aabb->min_x() / TILE_RES; tx <= aabb->max_x() / TILE_RES; ++tx) {
            const int tile_index = ty * num_tiles_x + tx;
            const TileBuffer &tile_buffer = tile_buffers[tile_index];
            if (test_visibility(aabb ,&tile_buffer.depth[0][0], TILE_RES, TILE_RES)) {return true;}
        }
    }
    return false;
}

std::vector<bool> TileOcclusionManager::test_all_visibility_concurrent(const std::vector<Ref<ScreenAABB>> &boxes, unsigned int thread_count) const {
    const int total = static_cast<int>(boxes.size());
    std::vector<bool> results(total, false);

    moodycamel::ConcurrentQueue<int> work_queue;
    std::atomic<int> processed_count = 0;

    // Push all work indices into the queue
    for (int i = 0; i < total; ++i) {
        work_queue.enqueue(i);
    }

    // Worker function
    auto worker = [&]() {
        int idx;
        while (work_queue.try_dequeue(idx)) {
            results[idx] = is_visible(boxes[idx]);
            processed_count.fetch_add(1, std::memory_order_relaxed);
        }
    };

    // Launch worker threads
    thread_count = std::clamp(thread_count, 1U, std::thread::hardware_concurrency());
    std::vector<std::thread> workers;
    for (unsigned int i = 0; i < thread_count; ++i) {
        workers.emplace_back(worker);
    }

    // Join all workers
    for (auto& w : workers) {
        w.join();
    }

    return results;
}