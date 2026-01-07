// ecs/editor/entity_dump.h
#ifndef WOLRD_INFO_H
#define WOLRD_INFO_H

#include "core/object/ref_counted.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include <vector>
#include <unordered_map>
#include <functional>

/**
 * @class EntityDumper
 * @brief Zero-copy entity and component introspection directly from Flecs backend
 *
 * Works directly with Flecs C++ API to iterate all entities and their components
 * without creating intermediate Dictionary/Variant overhead.
 */
 class WorldInfo : public RefCounted {
     GDCLASS(WorldInfo, RefCounted);

 protected:
     static void _bind_methods();

 public:
     /**
      * @brief Callback signature for entity visitation
      * Called for each entity with all its component data
      */
     using EntityCallback = std::function<void(
         flecs::entity entity,
         const flecs::entity_view& entity_view
     )>;

     /**
      * @brief Callback for component data
      * Called for each component on an entity
      */
     using ComponentCallback = std::function<void(
         const String& component_name,
         flecs::id component_id,
         const void* component_data
     )>;

     /**
      * @brief Dump all entities and components with low-level callback
      * Perfect for bulk operations, statistics, or direct backend access
      */
     static void dump_all_entities(
         flecs::world* world,
         EntityCallback entity_callback
     );

     /**
      * @brief Iterate all components on a single entity
      * Access component data directly without marshalling
      */
     static void dump_entity_components(
         flecs::entity entity,
         ComponentCallback component_callback
     );

     /**
      * @brief Get statistics about world state (fast, backend-only)
      */
     struct WorldStats {
         ecs_entity_t last_component_id;
         ecs_entity_t min_id;
         ecs_entity_t max_id;

         double delta_time_raw;
         double delta_time;
         double time_scale;
         double target_fps;
         double frame_time_total;
         double system_time_total;
         double emit_time_total;
         double merge_time_total;
         double rematch_time_total;
         double world_time_total;
         double world_time_total_raw;

         int64_t frame_count_total;
         int64_t merge_count_total;
         int64_t eval_comp_monitors_total;
         int64_t rematch_count_total;

         int64_t id_create_total;
         int64_t id_delete_total;
         int64_t table_create_total;
         int64_t table_delete_total;
         int64_t pipeline_build_count_total;
         int64_t systems_ran_total;
         int64_t observers_ran_total;
         int64_t queries_ran_total;

         int32_t tag_id_count;
         int32_t component_id_count;
         int32_t pair_id_count;

         int32_t table_count;

         uint32_t creation_time;

         struct {
             int64_t add_count;
             int64_t remove_count;
             int64_t delete_count;
             int64_t clear_count;
             int64_t set_count;
             int64_t ensure_count;
             int64_t modified_count;
             int64_t discard_count;
             int64_t event_count;
             int64_t other_count;
             int64_t batched_entity_count;
             int64_t batched_command_count;
         } cmd;

         const char* name_prefix;
     };
     static WorldStats get_world_stats(flecs::world* world);

     /**
      * @brief Fast entity lookup
      */
     static flecs::entity find_entity_by_name(
         flecs::world* world,
         const String& name
     );
 };

#endif // WOLRD_INFO_H
