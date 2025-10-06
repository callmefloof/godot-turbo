#pragma once

#include "core/variant/dictionary.h"
#include "flecs.h"
#include "ecs/components/comp_base.h"
#include "core/error/error_macros.h"
#include <memory>

struct ComponentRegistry {
    using CreateFn = std::unique_ptr<CompBase> (*)();
    using CreateFnWE = std::unique_ptr<CompBase> (*)(const flecs::entity &);
    using ToDictFn = Dictionary (*)(const flecs::entity &);
    using FromDictFn = void (*)(const flecs::entity &, const Dictionary&);
    using ToDictFnWorld = Dictionary (*)(flecs::world*);
    using FromDictFnWorld = void (*)(flecs::world*, const Dictionary&);


    struct Entry {
        CreateFn create = nullptr;
        CreateFnWE create_with_entity = nullptr;
        ToDictFn to_dict = nullptr;
        FromDictFn from_dict = nullptr;
        ToDictFnWorld to_dict_world = nullptr;
        FromDictFnWorld from_dict_world = nullptr;
        flecs::entity_t comp_id{};  // Flecs component ID in this world
    };

    static HashMap<String, Entry>& get_map() {
        static HashMap<String, Entry> map;
        return map;
    }

    static void register_type(const String& name, Entry entry) {
        get_map()[name] = entry;
    }

    static Entry* lookup(const String& name) {
        return get_map().getptr(name);
    }

    static void bind_to_world(const String& name, flecs::entity_t id) {
        if (Entry* e = lookup(name)) {
            e->comp_id = id;
        }
    }

    static Dictionary to_dict(const flecs::entity& entity, const String& type_name) {
        if (const Entry* entry = lookup(type_name)) {
            return entry->to_dict(entity);
        }
        ERR_PRINT("ComponentRegistry::to_dict: type_name not found or entity does not have component");
        return Dictionary();
    }

    static Dictionary to_dict(flecs::world* world, const String& type_name) {
        if (const Entry* entry = lookup(type_name)) {
            flecs::entity comp_type = world->component(entry->comp_id);
            if (comp_type.is_valid()) {
                return entry->to_dict(comp_type);
            }
        }
        ERR_PRINT("ComponentRegistry::to_dict: type_name not found or component type is not valid");
        return Dictionary();
    }

     static Dictionary to_dict(const flecs::entity& entity, const flecs::entity_t& type_id) {
        for (const auto& [name, entry] : get_map()) {
            if (entry.comp_id == type_id) {
                return entry.to_dict(entity);
            }
        }
        ERR_PRINT("ComponentRegistry::to_dict: type_id not found or entity does not have component");
        return Dictionary();
    }

    static Dictionary to_dict(flecs::world* world, const flecs::entity_t& type_id) {
        for (const auto& [name, entry] : get_map()) {
            if (entry.comp_id == type_id) {
                return entry.to_dict_world(world);
            }
        }
        ERR_PRINT("ComponentRegistry::to_dict: type_id not found or entity does not have component");
        return Dictionary();
    }

    static void from_dict(flecs::entity& entity, const Dictionary& dict, const String& type_name) {
        if (Entry* entry = lookup(type_name)) {
            entry->from_dict(entity, dict);
        } else {
            ERR_PRINT("ComponentRegistry::from_dict: type_name not found");
        }
    }

    static void from_dict(flecs::entity& entity, const Dictionary& dict, const flecs::entity_t& type_id) {
        for (const auto& [name, entry] : get_map()) {
            if (entry.comp_id == type_id) {
                entry.from_dict(entity, dict);
                return;
            }
        }
        ERR_PRINT("ComponentRegistry::from_dict: type_id not found or entity does not have component");
    }

    static void from_dict(flecs::world* world, const Dictionary& dict, const String& type_name) {
        if (const Entry* entry = lookup(type_name)) {
            flecs::entity comp_type = world->component(entry->comp_id);
            if (comp_type.is_valid()) {
                entry->from_dict(comp_type, dict);
            } else {
                ERR_PRINT("ComponentRegistry::from_dict: component type is not valid");
            }
        } else {
            ERR_PRINT("ComponentRegistry::from_dict: type_name not found");
        }
    }

    static void from_dict(flecs::world* world, const Dictionary& dict, const flecs::entity_t& type_id) {
        for (const auto& [name, entry] : get_map()) {
            if (entry.comp_id == type_id) {
                flecs::entity comp_type = world->component(entry.comp_id);
                if (comp_type.is_valid()) {
                    entry.from_dict_world(world, dict);
                } else {
                    ERR_PRINT("ComponentRegistry::from_dict: component type is not valid");
                }
                return;
            }
        }
        ERR_PRINT("ComponentRegistry::from_dict: type_id not found or entity does not have component");
    }
};

#define REGISTER_COMPONENT(TYPE)                                                        \
                                                                                        \
    static Dictionary TYPE##_to_dict_singleton(flecs::world *w) {                       \
        if (w->has<TYPE>()) {                                                           \
            return w->get<TYPE>().to_dict();                                            \
        }                                                                               \
        ERR_PRINT("ComponentRegistry::to_dict: type_name not found");                   \
        return Dictionary();                                                            \
    }                                                                                   \
    static void TYPE##_from_dict_singleton(flecs::world *w, const Dictionary& d) {      \
        if (w->has<TYPE>()) {                                                           \
            w->get_mut<TYPE>().from_dict(d);                                            \
        } else {                                                                        \
            TYPE comp;                                                                  \
            comp.from_dict(d);                                                          \
            w->set<TYPE>(comp);                                                         \
        }                                                                               \
    }                                                                                   \
                                                                                        \
    static Dictionary TYPE##_to_dict(const flecs::entity & e) {                         \
        if (e.has<TYPE>()) {                                                            \
            return e.get<TYPE>().to_dict();                                             \
        }                                                                               \
        ERR_PRINT("ComponentRegistry::to_dict: type_name not found");                   \
        return Dictionary();                                                            \
    }                                                                                   \
                                                                                        \
    static void TYPE##_from_dict(const flecs::entity &e, const Dictionary& d) {         \
        if (e.has<TYPE>()) {                                                            \
            e.get_mut<TYPE>().from_dict(d);                                             \
        } else {                                                                        \
            TYPE comp;                                                                  \
            comp.from_dict(d);                                                          \
            e.set<TYPE>(comp);                                                          \
        }                                                                               \
    }                                                                                   \
    struct TYPE##_AutoRegister {                                                        \
        TYPE##_AutoRegister() {                                                         \
            ComponentRegistry::register_type(                                           \
                String(#TYPE),                                                          \
                ComponentRegistry::Entry{                                               \
                    []() -> std::unique_ptr<CompBase> { return std::make_unique<TYPE>(TYPE()); },       \
                    [](const flecs::entity &e) -> std::unique_ptr<CompBase> {           \
                        if(e.has<TYPE>()) {                                             \
                            return std::make_unique<TYPE>(e.get_mut<TYPE>()); \
                        } \
                        ERR_FAIL_COND_V_MSG(!e.has<TYPE>(), nullptr,"Entity does not have component"); \
                        return nullptr; \
                    },\
                    TYPE##_to_dict,                                   \
                    TYPE##_from_dict,                                 \
                    TYPE##_to_dict_singleton,                        \
                    TYPE##_from_dict_singleton,                        \
                    {} /* world_id will be set later */               \
                }                                                     \
            );                                                        \
        }                                                             \
    };                                                                \
    static TYPE##_AutoRegister TYPE##_auto_register_instance;