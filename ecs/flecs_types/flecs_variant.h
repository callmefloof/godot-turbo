/**
 * @file flecs_variant.h
 * @brief Wrapper types for Flecs ECS objects compatible with Godot's RID system
 * 
 * This header defines variant wrappers around Flecs C++ types to enable storage
 * in Godot's RID_Owner containers. These wrappers provide proper copy/move semantics
 * and validation methods while maintaining compatibility with both Flecs and Godot.
 * 
 * @note These types are used internally by FlecsServer to manage ECS objects
 * @see FlecsServer, RID_Owner
 */

#pragma once
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"

/**
 * @struct FlecsWorldVariant
 * @brief Wrapper for flecs::world to enable RID storage
 * 
 * Encapsulates a Flecs ECS world instance for use with Godot's RID system.
 * Each world is an independent ECS instance with its own entities, components,
 * and systems.
 * 
 * @note Supports multiple worlds for scene isolation or parallel processing
 * 
 * Example usage:
 * @code
 * FlecsWorldVariant world_var(flecs::world{});
 * flecs::world& world = world_var.get_world();
 * world.entity("Player").set<Position>({0, 0});
 * @endcode
 */
struct FlecsWorldVariant {
    flecs::world world; ///< The wrapped Flecs world instance

    // Constructors
    
    /** @brief Construct from moved world (efficient, no copy) */
    FlecsWorldVariant(flecs::world&& world) noexcept : world(std::move(world)) {}
    
    /** @brief Construct from world reference (creates shared world handle) */
    FlecsWorldVariant(const flecs::world& world) : world(world) {}
    
    /** @brief Default constructor - creates new empty world */
    FlecsWorldVariant() = default;
    
    /** @brief Copy constructor - creates shared world handle */
    FlecsWorldVariant(const FlecsWorldVariant& other) : world(other.world) {}
    
    /** @brief Move constructor - transfers ownership */
    FlecsWorldVariant(FlecsWorldVariant&& other) noexcept : world(std::move(other.world)) {}

    /**
     * @brief Get mutable reference to the wrapped world
     * @return Reference to flecs::world for ECS operations
     */
    flecs::world& get_world() {
        return world;
    }
};

/**
 * @struct FlecsEntityVariant
 * @brief Wrapper for flecs::entity to enable RID storage
 * 
 * Encapsulates a Flecs entity handle for use with Godot's RID system.
 * Entities are the fundamental building blocks of the ECS architecture,
 * representing game objects, actors, or any discrete element.
 * 
 * @note Entity handles are lightweight (just an ID) and can be copied freely
 * 
 * Example usage:
 * @code
 * flecs::entity e = world.entity("Player");
 * FlecsEntityVariant entity_var(e);
 * if (entity_var.is_valid()) {
 *     flecs::entity entity = entity_var.get_entity();
 *     entity.set<Health>({100});
 * }
 * @endcode
 */
struct FlecsEntityVariant {
    flecs::entity entity; ///< The wrapped Flecs entity handle

    // Constructors
    
    /** @brief Construct from entity reference */
    FlecsEntityVariant(const flecs::entity& entity) : entity(entity) {}
    
    /** @brief Copy constructor */
    FlecsEntityVariant(const FlecsEntityVariant& other) : entity(other.entity) {}
    
    /** @brief Move constructor */
    FlecsEntityVariant(FlecsEntityVariant&& other) noexcept : entity(std::move(other.entity)) {}
    
    /** @brief Construct from moved entity */
    FlecsEntityVariant(flecs::entity&& entity) noexcept : entity(std::move(entity)) {}

    /**
     * @brief Get the wrapped entity handle
     * @return Copy of flecs::entity handle
     */
    flecs::entity get_entity() const {
        return entity;
    }

    /**
     * @brief Check if the entity is valid and alive
     * @return true if entity exists in the world and hasn't been deleted
     * @note Always check validity before operating on entities from external storage
     */
    bool is_valid() const {
        return entity.is_valid();
    }
};

/**
 * @struct FlecsSystemVariant
 * @brief Wrapper for flecs::system to enable RID storage
 * 
 * Encapsulates a Flecs system handle for use with Godot's RID system.
 * Systems are logic units that process entities matching specific component patterns.
 * 
 * @note Systems run automatically during world.progress() based on their pipeline phase
 * 
 * Example usage:
 * @code
 * flecs::system sys = world.system<Position, Velocity>()
 *     .each([](Position& p, Velocity& v) { p.x += v.x; });
 * FlecsSystemVariant sys_var(sys);
 * if (sys_var.is_valid()) {
 *     // System is registered and active
 * }
 * @endcode
 */
struct FlecsSystemVariant {
    flecs::system system; ///< The wrapped Flecs system handle

    // Constructors
    
    /** @brief Construct from system handle */
    FlecsSystemVariant(flecs::system system) : system(system) {}

    /**
     * @brief Get the wrapped system handle
     * @return Copy of flecs::system handle
     */
    flecs::system get_system() const {
        return system;
    }

    /**
     * @brief Check if the system is valid
     * @return true if system exists and is registered
     */
    bool is_valid() const {
        return system.is_valid();
    }
};

/**
 * @struct FlecsTypeIDVariant
 * @brief Wrapper for flecs::entity_t component type IDs
 * 
 * Encapsulates a Flecs component type identifier for use with Godot's RID system.
 * Type IDs uniquely identify component types within a world and are used for
 * runtime component lookup and dynamic type registration.
 * 
 * @note Type IDs are just entity IDs - components are entities in Flecs
 * 
 * Example usage:
 * @code
 * flecs::entity_t comp_id = world.component<Position>().id();
 * FlecsTypeIDVariant type_var(comp_id);
 * if (type_var.is_valid()) {
 *     // Can use this ID to get/set components dynamically
 *     entity.add(type_var.get_type());
 * }
 * @endcode
 */
struct FlecsTypeIDVariant {
    flecs::entity_t type; ///< The component type ID (entity ID in Flecs)

    // Constructors
    
    /** @brief Construct from type ID */
    FlecsTypeIDVariant(flecs::entity_t type) : type(type) {}
    
    /** @brief Copy constructor */
    FlecsTypeIDVariant(const FlecsTypeIDVariant& other) : type(other.type) {}
    
    /** @brief Move constructor */
    FlecsTypeIDVariant(FlecsTypeIDVariant&& other) noexcept : type(std::move(other.type)) {}

    /**
     * @brief Get the wrapped type ID
     * @return Component type ID (flecs::entity_t)
     */
    flecs::entity_t get_type() const {
        return type;
    }

    /**
     * @brief Check if the type ID is valid
     * @return true if type ID is non-zero (zero = invalid in Flecs)
     */
    bool is_valid() const {
        return type != 0;
    }
};

