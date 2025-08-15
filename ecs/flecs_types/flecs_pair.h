#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "core/io/resource.h"

class FlecsEntity;

class FlecsPair : public Resource {
    GDCLASS(FlecsPair, Resource);
    flecs::entity first;
    flecs::entity second;
    FlecsEntity* gd_first = nullptr;
    FlecsEntity* gd_second = nullptr;

    void _set_first(const flecs::entity& p_first);
    void _set_second(const flecs::entity& p_second);
public:
    FlecsPair() = default;
    ~FlecsPair() = default;

    flecs::entity _get_first() const;
    flecs::entity _get_second() const;
    FlecsEntity * get_first() const;
    FlecsEntity * get_second() const;
    void set_first(FlecsEntity* p_first);
    void set_second(FlecsEntity* p_second);
    static void _bind_methods();
    

};

