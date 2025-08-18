//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPT_VISIBLE_COMPONENT_H
#define SCRIPT_VISIBLE_COMPONENT_H
#include "core/string/string_name.h"
#include "core/templates/a_hash_map.h"
#include "core/typedefs.h"
#include "core/variant/variant.h"
#include "ecs/components/component_registry.h"
#include "core/variant/dictionary.h"

struct ScriptVisibleComponent : CompBase {
	StringName name;
	AHashMap<StringName, Variant> fields;
	
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["name"] = name;
		Dictionary fields_dict;
		for (const auto &pair : fields) {
			fields_dict[pair.key] = pair.value;
		}
		dict["fields"] = fields_dict;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		name = dict["name"];
		Dictionary fields_dict = dict["fields"];
		for (const auto &key : fields_dict.keys()) {
			fields[key] = fields_dict[key];
		}
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ScriptVisibleComponent>()) {
			const ScriptVisibleComponent &script = entity.get<ScriptVisibleComponent>();
			dict["name"] = script.name;
			Dictionary fields_dict;
			for (const auto &pair : script.fields) {
				fields_dict[pair.key] = pair.value;
			}
			dict["fields"] = fields_dict;
		} else {
			ERR_PRINT("ScriptVisibleComponent::to_dict: entity does not have ScriptVisibleComponent");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ScriptVisibleComponent>()) {
			ScriptVisibleComponent &script = entity.get_mut<ScriptVisibleComponent>();
			script.name = dict["name"];
			Dictionary fields_dict = dict["fields"];
			for (const auto &key : fields_dict.keys()) {
				script.fields[key] = fields_dict[key];
			}
		} else {
			ERR_PRINT("ScriptVisibleComponent::from_dict: entity does not have ScriptVisibleComponent");
		}
	}

	StringName get_type_name() const override {
		return "ScriptVisibleComponent";
	}

	ScriptVisibleComponent() = default;
	ScriptVisibleComponent(const ScriptVisibleComponent& rhs) {
		name = rhs.name;
		fields = rhs.fields;
	}
	ScriptVisibleComponent operator =(const ScriptVisibleComponent &rhs) {
		name = rhs.name;
		fields = rhs.fields;
		return *this;
	}




};

#endif //SCRIPT_VISIBLE_COMPONENT_H
