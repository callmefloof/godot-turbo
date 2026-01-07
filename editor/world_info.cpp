// ecs/editor/entity_dump.cpp
#include "modules/godot_turbo/editor/world_info.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"

void WorldInfo::_bind_methods() {
    // Only expose high-level methods if needed
}

void WorldInfo::dump_all_entities(
    flecs::world* world,
    EntityCallback entity_callback)
{
    if (!world) {
    	return;
    }

    // Empty query matches ALL entities
    world->query<>().run([entity_callback](flecs::iter& it) {
        while (it.next()) {
            // it.count() = number of entities in this batch
            for (int i = 0; i < it.count(); i++) {
                flecs::entity e = it.entity(i);
                flecs::entity_view view = e;
                entity_callback(e, view);
            }
        }
    });
}

void WorldInfo::dump_entity_components(
    flecs::entity entity,
    ComponentCallback component_callback)
{
    if (!entity.is_alive()) {
    	return;
    }

    // Iterate all component IDs on this entity
    entity.each([component_callback,entity](flecs::id id) {
        // id is the component identifier

        if (id.is_pair()) {
            // Handle relationship pairs if needed
            // flecs::entity rel = id.first();
            // flecs::entity tgt = id.second();
            return; // Skip pairs for now
        }

        // Get component entity
        flecs::entity comp_entity = id.entity();

        // Get the actual component data pointer
        const void* data = entity.get(id);
        if (data) {
            String comp_name = String(comp_entity.name().c_str());
            component_callback(comp_name, id, data);
        }
    });
}

WorldInfo::WorldStats WorldInfo::get_world_stats(flecs::world* world) {
    if (!world) {
     return {};
    }

    WorldStats stats{};

    const flecs::world_info_t* world_info = world->get_info();
    if (!world_info) {
        return stats;
    }

    stats.last_component_id = world_info->last_component_id;
    stats.min_id = world_info->min_id;
    stats.max_id = world_info->max_id;

    stats.delta_time_raw = world_info->delta_time_raw;
    stats.delta_time = world_info->delta_time;
    stats.time_scale = world_info->time_scale;
    stats.target_fps = world_info->target_fps;
    stats.frame_time_total = world_info->frame_time_total;
    stats.system_time_total = world_info->system_time_total;
    stats.emit_time_total = world_info->emit_time_total;
    stats.merge_time_total = world_info->merge_time_total;
    stats.rematch_time_total = world_info->rematch_time_total;
    stats.world_time_total = world_info->world_time_total;
    stats.world_time_total_raw = world_info->world_time_total_raw;

    stats.frame_count_total = world_info->frame_count_total;
    stats.merge_count_total = world_info->merge_count_total;
    stats.eval_comp_monitors_total = world_info->eval_comp_monitors_total;
    stats.rematch_count_total = world_info->rematch_count_total;

    stats.id_create_total = world_info->id_create_total;
    stats.id_delete_total = world_info->id_delete_total;
    stats.table_create_total = world_info->table_create_total;
    stats.table_delete_total = world_info->table_delete_total;
    stats.pipeline_build_count_total = world_info->pipeline_build_count_total;
    stats.systems_ran_total = world_info->systems_ran_total;
    stats.observers_ran_total = world_info->observers_ran_total;
    stats.queries_ran_total = world_info->queries_ran_total;

    stats.tag_id_count = world_info->tag_id_count;
    stats.component_id_count = world_info->component_id_count;
    stats.pair_id_count = world_info->pair_id_count;

    stats.table_count = world_info->table_count;

    stats.creation_time = world_info->creation_time;

    stats.cmd.add_count = world_info->cmd.add_count;
    stats.cmd.remove_count = world_info->cmd.remove_count;
    stats.cmd.delete_count = world_info->cmd.delete_count;
    stats.cmd.clear_count = world_info->cmd.clear_count;
    stats.cmd.set_count = world_info->cmd.set_count;
    stats.cmd.ensure_count = world_info->cmd.ensure_count;
    stats.cmd.modified_count = world_info->cmd.modified_count;
    stats.cmd.discard_count = world_info->cmd.discard_count;
    stats.cmd.event_count = world_info->cmd.event_count;
    stats.cmd.other_count = world_info->cmd.other_count;
    stats.cmd.batched_entity_count = world_info->cmd.batched_entity_count;
    stats.cmd.batched_command_count = world_info->cmd.batched_command_count;

    stats.name_prefix = world_info->name_prefix;

    return stats;
}

flecs::entity WorldInfo::find_entity_by_name(
    flecs::world* world,
    const String& name)
{
    if (!world) {
    return flecs::entity::null();
    }

    const char* c_name = name.utf8().get_data();
    return world->lookup(c_name);
}
