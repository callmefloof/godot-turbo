// component_proxy.h
#pragma once
#include "core/variant/dictionary.h"
#include "core/string/string_name.h"
#include "flecs.h"

struct CompBase {
    virtual ~CompBase() = default;
    virtual Dictionary to_dict() const = 0;
    virtual void from_dict(const Dictionary &dict) = 0;
    virtual Dictionary to_dict_with_entity(flecs::entity &entity) const = 0;
    virtual void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) = 0;
    virtual StringName get_type_name() const { return "CompBase"; }
};
