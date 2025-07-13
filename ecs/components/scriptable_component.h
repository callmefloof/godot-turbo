//
// Created by Floof on 13-7-2025.
//

#ifndef SCRIPTABLE_COMPONENT_H
#define SCRIPTABLE_COMPONENT_H
#include "../../../../core/variant/variant.h"
#include "script_visible_component.h"
#include "single_component_module.h"

struct ScriptableComponent : ScriptVisibleComponent {
	Dictionary component_data;
	Dictionary to_dict() const override {
		return component_data;
	}
	void from_json(Dictionary dict) {
		SET_SCRIPT_COMPONENT_VALUE(dict, "component_data", component_data, Variant::DICTIONARY);
	}
};

using ScriptComponentModule = SingleComponentModule<ScriptableComponent>;


#endif //SCRIPTABLE_COMPONENT_H
