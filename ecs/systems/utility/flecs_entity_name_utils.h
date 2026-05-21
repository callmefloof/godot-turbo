#ifndef FLECS_ENTITY_NAME_UTILS_H
#define FLECS_ENTITY_NAME_UTILS_H

#include "core/string/ustring.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"

namespace FlecsEntityNameUtils {

inline String make_unique_name(flecs::world *p_world, const String &p_name, flecs::entity p_entity = flecs::entity()) {
	if (!p_world || p_name.is_empty()) {
		return p_name;
	}

	String candidate = p_name;
	for (int suffix = 1; suffix < 1024; suffix++) {
		CharString candidate_utf8 = candidate.utf8();
		flecs::entity existing = p_world->lookup(candidate_utf8.get_data());
		if (!existing.is_valid() || !existing.is_alive() || existing == p_entity) {
			return candidate;
		}

		candidate = vformat("%s_%d", p_name, suffix);
	}

	return vformat("%s_%llu", p_name, (unsigned long long)p_entity.id());
}

inline void set_unique_name(flecs::world *p_world, flecs::entity p_entity, const String &p_name) {
	if (!p_entity.is_valid() || p_name.is_empty()) {
		return;
	}

	String deferred_safe_name = p_name + "_" + String::num_uint64(p_entity.id());
	String unique_name = make_unique_name(p_world, deferred_safe_name, p_entity);
	CharString unique_name_utf8 = unique_name.utf8();
	p_entity.set_name(unique_name_utf8.get_data());
}

} // namespace FlecsEntityNameUtils

#endif // FLECS_ENTITY_NAME_UTILS_H
