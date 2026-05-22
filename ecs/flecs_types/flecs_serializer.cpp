#include "modules/godot_turbo/ecs/flecs_types/flecs_serializer.h"

#include "core/error/error_macros.h"
#include "core/io/json.h"

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
#ifdef FAR
#undef FAR
#endif

#include "core/variant/variant_parser.h"

#include <cstring>

static String _variant_to_text(const Variant &p_value) {
	String text;
	if (VariantWriter::write_to_string(p_value, text) != OK) {
		return String();
	}
	return text;
}

static bool _parse_variant_text(const String &p_text, Variant &r_value) {
	VariantParser::StreamString stream;
	stream.s = p_text;
	String error;
	int error_line = 0;
	return VariantParser::parse(&stream, r_value, error, error_line) == OK;
}

static bool _is_text_opaque_type(const flecs::entity &p_type) {
	if (!p_type.is_valid() || !p_type.has<EcsType>()) {
		return false;
	}

	const EcsType &ecs_type = p_type.get<EcsType>();
	if (ecs_type.kind != EcsOpaqueType) {
		return false;
	}

	flecs::string_view name_view = p_type.name();
	const char *name = name_view.c_str();
	return name == nullptr || strcmp(name, "ObjectID") != 0;
}

Variant FlecsSerializer::serialize_value(flecs::world &p_world, flecs::entity_t p_type_id, const void *p_ptr) {
	ERR_FAIL_NULL_V(p_ptr, Variant());

	char *json = ecs_ptr_to_json(p_world.c_ptr(), p_type_id, p_ptr);
	if (!json) {
		return Variant();
	}

	Variant result = JSON::parse_string(String::utf8(json));
	ecs_os_free(json);

	flecs::entity type(p_world.c_ptr(), p_type_id);
	if (_is_text_opaque_type(type) && result.get_type() == Variant::STRING) {
		Variant parsed;
		if (_parse_variant_text(result, parsed)) {
			return parsed;
		}
	}

	return result;
}

bool FlecsSerializer::deserialize_value(flecs::world &p_world, flecs::entity_t p_type_id, void *p_ptr, const Variant &p_value) {
	ERR_FAIL_NULL_V(p_ptr, false);

	Variant value = p_value;
	flecs::entity type(p_world.c_ptr(), p_type_id);
	if (_is_text_opaque_type(type)) {
		value = _variant_to_text(p_value);
	}

	String json = JSON::stringify(value);
	CharString utf8 = json.utf8();
	ecs_from_json_desc_t desc = {};
	desc.name = "FlecsSerializer::deserialize_value";
	return ecs_ptr_from_json(p_world.c_ptr(), p_type_id, p_ptr, utf8.get_data(), &desc) != nullptr;
}

Dictionary FlecsSerializer::serialize_component(const flecs::entity &p_entity, flecs::entity_t p_component_id) {
	if (!p_entity.is_valid() || !p_entity.has(p_component_id)) {
		return Dictionary();
	}

	flecs::world world = p_entity.world();
	const void *ptr = p_entity.get(p_component_id);
	Variant value = serialize_value(world, p_component_id, ptr);
	if (value.get_type() == Variant::DICTIONARY) {
		return value;
	}

	Dictionary result;
	result["value"] = value;
	return result;
}

bool FlecsSerializer::deserialize_component(flecs::entity &p_entity, flecs::entity_t p_component_id, const Dictionary &p_data) {
	if (!p_entity.is_valid()) {
		return false;
	}

	void *ptr = p_entity.ensure(p_component_id);
	if (!ptr) {
		return false;
	}

	flecs::world world = p_entity.world();
	Variant value = p_data;
	if (p_data.size() == 1 && p_data.has("value")) {
		value = p_data["value"];
	}

	const bool ok = deserialize_value(world, p_component_id, ptr, value);
	if (ok) {
		p_entity.modified(p_component_id);
	}
	return ok;
}

String FlecsSerializer::serialize_world(flecs::world &p_world) {
	ecs_world_to_json_desc_t desc = {};
	char *json = ecs_world_to_json(p_world.c_ptr(), &desc);
	if (!json) {
		return String();
	}

	String result = String::utf8(json);
	ecs_os_free(json);
	return result;
}

bool FlecsSerializer::deserialize_world(flecs::world &p_world, const String &p_state) {
	ERR_FAIL_COND_V(p_state.is_empty(), false);

	CharString state_utf8 = p_state.utf8();
	ecs_from_json_desc_t desc = {};
	desc.name = "FlecsSerializer::deserialize_world";
	return ecs_world_from_json(p_world.c_ptr(), state_utf8.get_data(), &desc) != nullptr;
}
