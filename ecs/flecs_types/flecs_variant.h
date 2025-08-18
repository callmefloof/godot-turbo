#pragma once
#include "thirdparty/flecs/distr/flecs.h"

struct FlecsWorldVariant {
    flecs::world world;

    // Constructors
    FlecsWorldVariant(flecs::world&& world) noexcept : world(std::move(world)) {}
    FlecsWorldVariant(const flecs::world& world) : world(world) {}
    FlecsWorldVariant() = default;
    FlecsWorldVariant(const FlecsWorldVariant& other) : world(other.world) {}
    FlecsWorldVariant(FlecsWorldVariant&& other) noexcept : world(std::move(other.world)) {}

    // Getters
    flecs::world& get_world() {
        return world;
    }

};

struct FlecsEntityVariant {
    flecs::entity entity;

    // Constructors
    FlecsEntityVariant(const flecs::entity& entity) : entity(entity) {}
    FlecsEntityVariant(const FlecsEntityVariant& other) : entity(other.entity) {}
    FlecsEntityVariant(FlecsEntityVariant&& other) noexcept : entity(std::move(other.entity)) {}
    FlecsEntityVariant(flecs::entity&& entity) noexcept : entity(std::move(entity)) {}


    // Getters
    flecs::entity get_entity() const {
        return entity;
    }

    // Check if the entity is valid
    bool is_valid() const {
        return entity.is_valid();
    }
};

struct FlecsSystemVariant {
    flecs::system system;

    // Constructors
    FlecsSystemVariant(flecs::system system) : system(system) {}

    // Getters
    flecs::system get_system() const {
        return system;
    }

    // Check if the system is valid
    bool is_valid() const {
        return system.is_valid();
    }
};


struct FlecsTypeIDVariant {
    flecs::entity_t type;

    // Constructors
    FlecsTypeIDVariant(flecs::entity_t type) : type(type) {}
    FlecsTypeIDVariant(const FlecsTypeIDVariant& other) : type(other.type) {}
    FlecsTypeIDVariant(FlecsTypeIDVariant&& other) noexcept : type(std::move(other.type)) {}
    

    // Getters
    flecs::entity_t get_type() const {
        return type;
    }

    // Check if the type is valid
    bool is_valid() const {
        return type != 0;
    }
};

