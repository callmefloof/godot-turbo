#pragma once
#include "core/object/object_id.h"
#include "ecs/components/component_registry.h"
#include "core/variant/variant.h"

struct ObjectInstanceComponent : CompBase {
    ObjectID object_instance_id; // Unique identifier for the object instance

    // Default constructor initializes instance_id to an invalid state
    ObjectInstanceComponent() = default;
    ~ObjectInstanceComponent() = default;

    // Constructor to initialize with a specific instance ID
    explicit ObjectInstanceComponent(ObjectID p_instance_id) : object_instance_id(p_instance_id) {}

    Dictionary to_dict() const override {
        Dictionary dict;
        dict.set("object_instance_id", object_instance_id.operator int64_t());
        return dict;
    }

    void from_dict(const Dictionary &dict) override {
        int64_t id = dict["object_instance_id"];
        object_instance_id = ObjectID(id);
    }

    Dictionary to_dict_with_entity(flecs::entity &entity) const override {
        Dictionary dict;
        if (entity.has<ObjectInstanceComponent>()) {
            const ObjectInstanceComponent &instance = entity.get<ObjectInstanceComponent>();
            dict.set("object_instance_id", instance.object_instance_id.operator int64_t());
        } else {
            ERR_PRINT("ObjectInstanceComponent::to_dict: entity does not have ObjectInstanceComponent");
        }
        return dict;
    }

    void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
        if (entity.has<ObjectInstanceComponent>()) {
            ObjectInstanceComponent &instance = entity.get_mut<ObjectInstanceComponent>();
            instance.object_instance_id = ObjectID(dict["object_instance_id"].operator int64_t());
        } else {
            ERR_PRINT("ObjectInstanceComponent::from_dict: entity does not have ObjectInstanceComponent");
        }
    }

    StringName get_type_name() const override {
        return "ObjectInstanceComponent";
    }
};
REGISTER_COMPONENT(ObjectInstanceComponent);
