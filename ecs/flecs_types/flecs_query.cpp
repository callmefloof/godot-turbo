//
// Created by Assistant on query variant implementation
//

#include "flecs_query.h"
#include "core/os/os.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "modules/godot_turbo/ecs/components/component_reflection.h"


FlecsQuery::~FlecsQuery() {
    if (change_observer_set.is_alive()) {
        change_observer_set.destruct();
    }
    if (change_observer_add.is_alive()) {
        change_observer_add.destruct();
    }
    if (change_observer_remove.is_alive()) {
        change_observer_remove.destruct();
    }
    // Query is destructed automatically
}

void FlecsQuery::init(const RID &p_world_id, const PackedStringArray &p_required_components) {
    world_id = p_world_id;
    required_components = p_required_components;

    FlecsServer *server = FlecsServer::get_singleton();
    if (!server) {
        ERR_PRINT("FlecsQuery::init - FlecsServer singleton is null");
        return;
    }

    world = server->_get_world(world_id);
    if (!world) {
        ERR_PRINT("FlecsQuery::init - Invalid world RID");
        return;
    }

    build_query();

    if (caching_strategy != NO_CACHE) {
        setup_cache_invalidation();
    }
}

void FlecsQuery::reset(const RID &p_world_id, const PackedStringArray &p_required_components) {
    // Clean up existing query and observers
    if (change_observer_set.is_alive()) {
        change_observer_set.destruct();
    }
    if (change_observer_add.is_alive()) {
        change_observer_add.destruct();
    }
    if (change_observer_remove.is_alive()) {
        change_observer_remove.destruct();
    }
    // Query will be rebuilt

    // Reset cache
    cached_entities.clear();
    cached_full_data.clear();
    cache_dirty = true;

    // Reinitialize
    init(p_world_id, p_required_components);
}

void FlecsQuery::build_query() {
    if (!world) {
        ERR_PRINT("FlecsQuery::build_query - world is null");
        return;
    }

    // Build query with required components
    flecs::query_builder<> builder = world->query_builder<>();

    for (int i = 0; i < required_components.size(); ++i) {
        String cname = required_components[i];
        flecs::entity ce = world->component(cname.ascii().get_data());
        if (!ce.is_valid()) {
            ERR_PRINT(vformat("FlecsQuery::build_query - Invalid component name: %s", cname));
            continue;
        }
        builder.term().id(ce.id());
    }

    query = builder.build();

    invalidate_cache();
}

void FlecsQuery::setup_cache_invalidation() {
    if (!world || caching_strategy == NO_CACHE) {
        return;
    }

    // Clean up existing observers
    if (change_observer_set.is_alive()) {
        change_observer_set.destruct();
    }
    if (change_observer_add.is_alive()) {
        change_observer_add.destruct();
    }
    if (change_observer_remove.is_alive()) {
        change_observer_remove.destruct();
    }

    // Build component terms for observers
    Vector<flecs::entity> comp_terms;
    for (int i = 0; i < required_components.size(); ++i) {
        String cname = required_components[i];
        flecs::entity ce = world->component(cname.ascii().get_data());
        if (ce.is_valid()) {
            comp_terms.push_back(ce);
        }
    }

    if (comp_terms.size() == 0) {
        return;
    }

    // Create invalidation observers
    auto make_observer = [this, &comp_terms](flecs::entity_t evt) {
        flecs::observer_builder<> ob = world->observer();
        ob.event(evt);
        for (int i = 0; i < comp_terms.size(); ++i) {
            ob.term().id(comp_terms[i].id());
        }
        return ob.each([this](flecs::entity e) {
            invalidate_cache();
        });
    };

    change_observer_set = make_observer(flecs::OnSet);
    change_observer_add = make_observer(flecs::OnAdd);
    change_observer_remove = make_observer(flecs::OnRemove);
}

void FlecsQuery::invalidate_cache() {
    cache_dirty = true;
    cached_entities.clear();
    cached_full_data.clear();
}

Array FlecsQuery::fetch_entities_internal(FetchMode mode) {
    if (!world) {
        ERR_PRINT("FlecsQuery::fetch_entities_internal - Invalid world");
        return Array();
    }

    uint64_t t0 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;

    FlecsServer *server = FlecsServer::get_singleton();
    if (!server) {
        ERR_PRINT("FlecsQuery::fetch_entities_internal - FlecsServer singleton is null");
        return Array();
    }

    Array result;
    uint64_t entity_count = 0;

    // Iterate query and collect entities
    query.each([&](flecs::entity e) {
        // Apply name filter if enabled
        if (filter_enabled && !filter_name_pattern.is_empty()) {
            const char *entity_name = e.name();
            if (!entity_name) {
                return; // Skip unnamed entities
            }

            String ename(entity_name);
            // Simple wildcard matching (* at end)
            if (filter_name_pattern.ends_with("*")) {
                String prefix = filter_name_pattern.substr(0, filter_name_pattern.length() - 1);
                if (!ename.begins_with(prefix)) {
                    return; // Skip non-matching
                }
            } else if (ename != filter_name_pattern) {
                return; // Skip non-matching
            }
        }

        RID entity_rid = server->_get_or_create_rid_for_entity(world_id, e);

        if (mode == FETCH_RID_ONLY) {
            result.push_back(entity_rid);
        } else { // FETCH_WITH_COMPONENTS
            Dictionary entity_data;
            entity_data["rid"] = entity_rid;

            Dictionary components;
            for (int i = 0; i < required_components.size(); ++i) {
                String cname = required_components[i];
                flecs::entity ce = world->component(cname.ascii().get_data());
                if (!ce.is_valid()) {
                    continue;
                }

                if (e.has(ce)) {
                    components[StringName(cname)] = FlecsReflection::Registry::get().serialize(e, ce.id());
                } else {
                    components[StringName(cname)] = Dictionary(); // Empty dict for missing component
                }
            }

            entity_data["components"] = components;
            result.push_back(entity_data);
        }

        entity_count++;
    });

    // Update instrumentation
    if (instrumentation_enabled) {
        uint64_t dt = OS::get_singleton()->get_ticks_usec() - t0;
        total_fetches++;
        total_entities_returned += entity_count;
        last_fetch_entity_count = entity_count;
        last_fetch_usec = dt;

        if (cache_dirty) {
            cache_misses++;
        } else {
            cache_hits++;
        }
    }

    return result;
}

Array FlecsQuery::get_entities() {
    if (caching_strategy != NO_CACHE && !cache_dirty) {
        if (instrumentation_enabled) {
            cache_hits++;
        }
        return cached_entities;
    }

    Array result = fetch_entities_internal(FETCH_RID_ONLY);

    if (caching_strategy == CACHE_ENTITIES || caching_strategy == CACHE_FULL) {
        cached_entities = result;
        cache_dirty = false;
    }

    return result;
}

Array FlecsQuery::get_entities_with_components() {
    if (caching_strategy == CACHE_FULL && !cache_dirty) {
        if (instrumentation_enabled) {
            cache_hits++;
        }
        return cached_full_data;
    }

    Array result = fetch_entities_internal(FETCH_WITH_COMPONENTS);

    if (caching_strategy == CACHE_FULL) {
        cached_full_data = result;
        cache_dirty = false;
    }

    return result;
}

int FlecsQuery::get_entity_count() {
    if (!world) {
        return 0;
    }

    int count = 0;
    query.each([&count](flecs::entity e) {
        count++;
    });

    return count;
}

Array FlecsQuery::get_entities_limited(int max_count, int offset) {
    if (!world) {
        return Array();
    }

    FlecsServer *server = FlecsServer::get_singleton();
    if (!server) {
        return Array();
    }

    Array result;
    int current_index = 0;
    int collected = 0;

    query.each([&](flecs::entity e) {
        if (current_index < offset) {
            current_index++;
            return;
        }

        if (collected >= max_count) {
            return;
        }

        RID entity_rid = server->_get_or_create_rid_for_entity(world_id, e);
        result.push_back(entity_rid);

        current_index++;
        collected++;
    });

    return result;
}

Array FlecsQuery::get_entities_with_components_limited(int max_count, int offset) {
    if (!world) {
        return Array();
    }

    FlecsServer *server = FlecsServer::get_singleton();
    if (!server) {
        return Array();
    }

    Array result;
    int current_index = 0;
    int collected = 0;

    query.each([&](flecs::entity e) {
        if (current_index < offset) {
            current_index++;
            return;
        }

        if (collected >= max_count) {
            return;
        }

        RID entity_rid = server->_get_or_create_rid_for_entity(world_id, e);

        Dictionary entity_data;
        entity_data["rid"] = entity_rid;

        Dictionary components;
        for (int i = 0; i < required_components.size(); ++i) {
            String cname = required_components[i];
            flecs::entity ce = world->component(cname.ascii().get_data());
            if (!ce.is_valid()) {
                continue;
            }

            if (e.has(ce)) {
                components[StringName(cname)] = FlecsReflection::Registry::get().serialize(e, ce.id());
            } else {
                components[StringName(cname)] = Dictionary();
            }
        }

        entity_data["components"] = components;
        result.push_back(entity_data);

        current_index++;
        collected++;
    });

    return result;
}

bool FlecsQuery::matches_entity(const RID &entity_rid) {
    if (!world) {
        return false;
    }

    FlecsServer *server = FlecsServer::get_singleton();
    if (!server) {
        return false;
    }

    flecs::entity e = server->_get_entity(entity_rid, world_id);
    if (!e.is_valid()) {
        return false;
    }

    // Check if entity has all required components
    for (int i = 0; i < required_components.size(); ++i) {
        String cname = required_components[i];
        flecs::entity ce = world->component(cname.ascii().get_data());
        if (!ce.is_valid() || !e.has(ce)) {
            return false;
        }
    }

    return true;
}

void FlecsQuery::set_required_components(const PackedStringArray &p_components) {
    required_components = p_components;
    build_query();

    if (caching_strategy != NO_CACHE) {
        setup_cache_invalidation();
    }
}

void FlecsQuery::set_caching_strategy(CachingStrategy p_strategy) {
    if (caching_strategy == p_strategy) {
        return;
    }

    caching_strategy = p_strategy;
    invalidate_cache();

    if (caching_strategy != NO_CACHE) {
        setup_cache_invalidation();
    } else {
        // Clean up observers when caching is disabled
        if (change_observer_set.is_alive()) {
            change_observer_set.destruct();
        }
        if (change_observer_add.is_alive()) {
            change_observer_add.destruct();
        }
        if (change_observer_remove.is_alive()) {
            change_observer_remove.destruct();
        }
    }
}

void FlecsQuery::set_filter_name_pattern(const String &p_pattern) {
    filter_name_pattern = p_pattern;
    filter_enabled = !p_pattern.is_empty();
    invalidate_cache();
}

void FlecsQuery::set_world(const RID &p_world_id) {
    if (world_id == p_world_id) {
        return;
    }

    world_id = p_world_id;

    FlecsServer *server = FlecsServer::get_singleton();
    if (server) {
        world = server->_get_world(world_id);
        build_query();

        if (caching_strategy != NO_CACHE) {
            setup_cache_invalidation();
        }
    }
}

Dictionary FlecsQuery::get_instrumentation_data() const {
    Dictionary data;
    data["total_fetches"] = total_fetches;
    data["total_entities_returned"] = total_entities_returned;
    data["last_fetch_entity_count"] = last_fetch_entity_count;
    data["last_fetch_usec"] = last_fetch_usec;
    data["cache_hits"] = cache_hits;
    data["cache_misses"] = cache_misses;
    data["cache_hit_rate"] = (cache_hits + cache_misses) > 0
        ? (double)cache_hits / (cache_hits + cache_misses)
        : 0.0;
    return data;
}

void FlecsQuery::reset_instrumentation() {
    total_fetches = 0;
    total_entities_returned = 0;
    last_fetch_entity_count = 0;
    last_fetch_usec = 0;
    cache_hits = 0;
    cache_misses = 0;
}

FlecsQuery::FlecsQuery(const FlecsQuery &other) {
    world_id = other.world_id;
    world = other.world;
    required_components = other.required_components;
    caching_strategy = other.caching_strategy;
    filter_enabled = other.filter_enabled;
    filter_name_pattern = other.filter_name_pattern;
    instrumentation_enabled = other.instrumentation_enabled;

    // Don't copy cached data or instrumentation stats
    cache_dirty = true;

    // Rebuild query and observers
    if (world) {
        build_query();
        if (caching_strategy != NO_CACHE) {
            setup_cache_invalidation();
        }
    }
}

FlecsQuery& FlecsQuery::operator=(const FlecsQuery &other) {
    if (this == &other) {
        return *this;
    }

    // Clean up existing resources
    if (change_observer_set.is_alive()) {
        change_observer_set.destruct();
    }
    if (change_observer_add.is_alive()) {
        change_observer_add.destruct();
    }
    if (change_observer_remove.is_alive()) {
        change_observer_remove.destruct();
    }
    // Query will be rebuilt

    // Copy data
    world_id = other.world_id;
    world = other.world;
    required_components = other.required_components;
    caching_strategy = other.caching_strategy;
    filter_enabled = other.filter_enabled;
    filter_name_pattern = other.filter_name_pattern;
    instrumentation_enabled = other.instrumentation_enabled;

    // Don't copy cached data or instrumentation stats
    cached_entities.clear();
    cached_full_data.clear();
    cache_dirty = true;

    // Rebuild query and observers
    if (world) {
        build_query();
        if (caching_strategy != NO_CACHE) {
            setup_cache_invalidation();
        }
    }

    return *this;
}
