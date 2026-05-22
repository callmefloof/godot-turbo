#pragma once

#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"

class FlecsSerializer {
public:
	static Variant serialize_value(flecs::world &p_world, flecs::entity_t p_type_id, const void *p_ptr);
	static bool deserialize_value(flecs::world &p_world, flecs::entity_t p_type_id, void *p_ptr, const Variant &p_value);

	static Dictionary serialize_component(const flecs::entity &p_entity, flecs::entity_t p_component_id);
	static bool deserialize_component(flecs::entity &p_entity, flecs::entity_t p_component_id, const Dictionary &p_data);

	static String serialize_world(flecs::world &p_world);
	static bool deserialize_world(flecs::world &p_world, const String &p_state);

private:
	FlecsSerializer() = delete;
};
