#include "flecs_pair.h"
#include "ecs/flecs_types/flecs_entity.h"

void FlecsPair::_set_first(const flecs::entity& p_first) {
    first = p_first;
}

void FlecsPair::_set_second(const flecs::entity& p_second) {
    second = p_second;
}
void FlecsPair::set_first(FlecsEntity* p_first) {
    if (!p_first) {
        ERR_PRINT("FlecsPair::set_first called with null entity");
        return;
    }
    _set_first(p_first->get_internal_entity());
    gd_first = p_first;
}

void FlecsPair::set_second(FlecsEntity* p_second) {
    if (!p_second) {
        ERR_PRINT("FlecsPair::set_second called with null entity");
        return;
    }
    _set_second(p_second->get_internal_entity());
    gd_second = p_second;
}
flecs::entity FlecsPair::_get_first() const {
    return first;
}

flecs::entity FlecsPair::_get_second() const {
    return second;
}

FlecsEntity* FlecsPair::get_first() const {
    return gd_first;
}

FlecsEntity* FlecsPair::get_second() const {
    return gd_second;
}

void FlecsPair::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_first", "first"), &FlecsPair::set_first);
    ClassDB::bind_method(D_METHOD("set_second", "second"), &FlecsPair::set_second);
    ClassDB::bind_method(D_METHOD("get_first"), &FlecsPair::get_first);
    ClassDB::bind_method(D_METHOD("get_second"), &FlecsPair::get_second);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "first", PROPERTY_HINT_RESOURCE_TYPE, "FlecsEntity"), "set_first", "get_first");
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "second", PROPERTY_HINT_RESOURCE_TYPE, "FlecsEntity"), "set_second", "get_second");
}