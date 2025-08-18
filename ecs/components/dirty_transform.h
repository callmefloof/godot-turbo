#pragma once

#include "ecs/components/component_registry.h"
#include "core/variant/dictionary.h"

struct DirtyTransform : CompBase {

    Dictionary to_dict() const override {
        Dictionary dict;
        return dict;
    }

    void from_dict(const Dictionary &p_dict) override {
    }

    Dictionary to_dict_with_entity(flecs::entity &entity) const override {
        Dictionary dict;
        if (entity.has<DirtyTransform>()) {
            // No fields to serialize
        } else {
            ERR_PRINT("DirtyTransform::to_dict: entity does not have DirtyTransform");
        }
        return dict;
    }

    void from_dict_with_entity(const Dictionary &p_dict, flecs::entity &entity) override {
        if (entity.has<DirtyTransform>()) {
            // No fields to deserialize
        } else {
            ERR_PRINT("DirtyTransform::from_dict: entity does not have DirtyTransform");
        }
    }

    StringName get_type_name() const override {
        return "DirtyTransform";
    }
};
REGISTER_COMPONENT(DirtyTransform);
