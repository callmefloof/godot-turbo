#ifndef PROXY_DATA_H
#define PROXY_DATA_H
#include "core/templates/a_hash_map.h"

// OAHashMap<uint64_t, EntityProxyData> entity_store;  // 10k max
struct ComponentProxyData {
    uint64_t component_id;
    StringName component_name;
    AHashMap<StringName, Variant> fields;  // FieldName -> Variant
};

struct EntityProxyData {
    uint64_t entity_id;
    StringName entity_name;
    AHashMap<uint64_t, ComponentProxyData> components;  // ComponentID -> Data
    bool dirty = true;
    Vector3 gizmo_aabb_min, gizmo_aabb_max;
};

#endif // PROXY_DATA_H
