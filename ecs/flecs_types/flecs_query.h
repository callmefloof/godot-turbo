//
// Created by Assistant on query variant implementation
//

#ifndef FLECS_QUERY_H
#define FLECS_QUERY_H

#include "core/templates/rid.h"
#include "core/typedefs.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include <cstdint>

/**
 * FlecsQuery - High-performance query variant for direct entity iteration
 * 
 * Unlike FlecsScriptSystem which uses callbacks, FlecsQuery allows you to:
 * 1. Build a query once with required components
 * 2. Fetch matching entities as a batch (returns array of RIDs)
 * 3. Manually iterate in GDScript with minimal overhead
 * 
 * This is ideal for performance-critical systems where callback overhead
 * is too expensive. You get the speed of manual query loops with the
 * modularity of the new ECS architecture.
 * 
 * Example usage in GDScript:
 *   var query = FlecsServer.create_query(world_rid, ["Position", "Velocity"])
 *   var entities = FlecsServer.query_get_entities(world_rid, query)
 *   for entity_rid in entities:
 *       var pos = FlecsServer.get_component_by_name(entity_rid, "Position")
 *       var vel = FlecsServer.get_component_by_name(entity_rid, "Velocity")
 *       # ... process ...
 */
class FlecsQuery {
public:
    enum FetchMode {
        FETCH_RID_ONLY = 0,      // Only return entity RIDs (fastest)
        FETCH_WITH_COMPONENTS = 1 // Return RIDs + component data dictionaries
    };

    enum CachingStrategy {
        NO_CACHE = 0,           // Rebuild entity list every fetch (safest, up-to-date)
        CACHE_ENTITIES = 1,     // Cache entity list, invalidate on component changes
        CACHE_FULL = 2          // Cache entities + component data (fastest, use with caution)
    };

private:
    RID world_id;
    flecs::world *world = nullptr;
    flecs::query<> query;
    PackedStringArray required_components;
    
    // Caching support
    CachingStrategy caching_strategy = NO_CACHE;
    Array cached_entities;          // Cached RIDs
    Array cached_full_data;         // Cached RIDs + component dicts
    bool cache_dirty = true;
    
    // Change observers for cache invalidation
    flecs::entity change_observer_set;
    flecs::entity change_observer_add;
    flecs::entity change_observer_remove;
    
    // Filter options
    bool filter_enabled = false;
    String filter_name_pattern;     // e.g., "Player*" for wildcard matching
    
    // Instrumentation
    bool instrumentation_enabled = false;
    uint64_t total_fetches = 0;
    uint64_t total_entities_returned = 0;
    uint64_t last_fetch_entity_count = 0;
    uint64_t last_fetch_usec = 0;
    uint64_t cache_hits = 0;
    uint64_t cache_misses = 0;
    
    // Internal helpers
    void build_query();
    void setup_cache_invalidation();
    void invalidate_cache();
    Array fetch_entities_internal(FetchMode mode);
    
public:
    FlecsQuery() = default;
    ~FlecsQuery();
    
    // Initialization
    void init(const RID &p_world_id, const PackedStringArray &p_required_components);
    void reset(const RID &p_world_id, const PackedStringArray &p_required_components);
    
    // Core query operations
    Array get_entities();                              // Returns Array of RIDs
    Array get_entities_with_components();              // Returns Array of Dictionaries {rid: RID, components: {name: data}}
    int get_entity_count();                            // Returns count without fetching all entities
    
    // Batched fetch with limit (for pagination or chunked processing)
    Array get_entities_limited(int max_count, int offset = 0);
    Array get_entities_with_components_limited(int max_count, int offset = 0);
    
    // Single entity check
    bool matches_entity(const RID &entity_rid);        // Check if entity matches this query
    
    // Configuration
    void set_required_components(const PackedStringArray &p_components);
    PackedStringArray get_required_components() const { return required_components; }
    
    void set_caching_strategy(CachingStrategy p_strategy);
    CachingStrategy get_caching_strategy() const { return caching_strategy; }
    
    void set_filter_name_pattern(const String &p_pattern);
    String get_filter_name_pattern() const { return filter_name_pattern; }
    void clear_filter() { filter_enabled = false; filter_name_pattern = ""; }
    
    // Cache control
    void force_cache_refresh() { invalidate_cache(); }
    bool is_cache_dirty() const { return cache_dirty; }
    
    // Instrumentation
    void set_instrumentation_enabled(bool p_enabled) { instrumentation_enabled = p_enabled; }
    bool get_instrumentation_enabled() const { return instrumentation_enabled; }
    
    Dictionary get_instrumentation_data() const;
    void reset_instrumentation();
    
    uint64_t get_total_fetches() const { return total_fetches; }
    uint64_t get_total_entities_returned() const { return total_entities_returned; }
    uint64_t get_last_fetch_entity_count() const { return last_fetch_entity_count; }
    uint64_t get_last_fetch_usec() const { return last_fetch_usec; }
    uint64_t get_cache_hits() const { return cache_hits; }
    uint64_t get_cache_misses() const { return cache_misses; }
    
    // Internal access (for FlecsServer)
    flecs::world* _get_world() const { return world; }
    void _set_world(flecs::world *p_world) { world = p_world; }
    RID get_world() const { return world_id; }
    void set_world(const RID &p_world_id);
    
    // Copy/assignment
    FlecsQuery(const FlecsQuery &other);
    FlecsQuery& operator=(const FlecsQuery &other);
};

#endif // FLECS_QUERY_H