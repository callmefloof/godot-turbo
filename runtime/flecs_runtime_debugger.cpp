#include "flecs_runtime_debugger.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "core/debugger/engine_debugger.h"
#include "core/variant/typed_array.h"
#include "core/string/ustring.h"

FlecsRuntimeDebugger *FlecsRuntimeDebugger::singleton = nullptr;

FlecsRuntimeDebugger::FlecsRuntimeDebugger() {
	singleton = this;
	server = nullptr;
	initialized = false;
}

FlecsRuntimeDebugger::~FlecsRuntimeDebugger() {
	if (initialized) {
		shutdown();
	}
	singleton = nullptr;
}

void FlecsRuntimeDebugger::initialize() {
	if (initialized) {
		return;
	}

	server = FlecsServer::get_singleton();
	if (!server) {
		return;
	}

	EngineDebugger *debugger = EngineDebugger::get_singleton();
	if (!debugger) {
		return;
	}
	
	EngineDebugger::register_message_capture("flecs", EngineDebugger::Capture(this, _capture_message));
	initialized = true;
}

void FlecsRuntimeDebugger::shutdown() {
	if (!initialized) {
		return;
	}

	EngineDebugger::unregister_message_capture("flecs");
	initialized = false;
}

Error FlecsRuntimeDebugger::_capture_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured) {
	if (!p_user) {
		return FAILED;
	}

	FlecsRuntimeDebugger *debugger = static_cast<FlecsRuntimeDebugger *>(p_user);

	// NOTE: EngineDebugger strips the "flecs:" prefix before calling this handler
	// So we receive "request_worlds" not "flecs:request_worlds"
	if (p_msg == "request_worlds") {
		Error result = debugger->_handle_request_worlds(p_args);
		r_captured = (result == OK);
		return OK;
	} else if (p_msg == "request_entities") {
		r_captured = debugger->_handle_request_entities(p_args) == OK;
		return OK;
	} else if (p_msg == "request_components") {
		r_captured = debugger->_handle_request_components(p_args) == OK;
		return OK;
	} else if (p_msg == "request_profiler_metrics") {
		r_captured = debugger->_handle_request_profiler_metrics(p_args) == OK;
		return OK;
	}

	return FAILED;
}

Error FlecsRuntimeDebugger::_handle_request_worlds(const Array &p_args) {
	if (!server) {
		return FAILED;
	}

	Dictionary response;
	response["type"] = "world_list";

	Array worlds_array;

	// Get all worlds from FlecsServer
	TypedArray<RID> world_list = server->get_world_list();

	for (int i = 0; i < world_list.size(); i++) {
		RID world_rid = world_list[i];
		uint64_t world_id = world_rid.get_id();

		Dictionary world_dict;
		world_dict["id"] = world_id;
		world_dict["name"] = String("World_") + String::num_int64(world_id, 16);
		world_dict["entity_count"] = 0; // Will be updated when entities are requested

		worlds_array.push_back(world_dict);
	}

	response["worlds"] = worlds_array;

	_send_debugger_message("flecs:worlds", response);
	return OK;
}

Error FlecsRuntimeDebugger::_handle_request_entities(const Array &p_args) {
	if (!server) {
		return FAILED;
	}

	if (p_args.size() < 3) {
		return FAILED;
	}

	uint64_t world_id = p_args[0];
	int offset = p_args[1];
	int count = p_args[2];


	RID world_rid = RID::from_uint64(world_id);

	Dictionary response;
	response["type"] = "entities";
	response["world_id"] = world_id;

	Array entities_array;

	// Create a query to get all entities (empty required_components = all entities)
	PackedStringArray empty_components;
	RID query_rid = server->create_query(world_rid, empty_components);



	if (query_rid.is_valid() && count > 0) {
		Array limited_entities = server->query_get_entities_limited(world_rid, query_rid, count, offset);
	
		int entities_size = limited_entities.size();

		for (int i = 0; i < entities_size; i++) {
			// Safely access array element with bounds check
			if (i >= limited_entities.size()) {
				break;
			}
			
			Variant entity_var = limited_entities[i];
			Variant::Type var_type = entity_var.get_type();
			
			if (var_type == Variant::RID) {
				RID entity_rid = entity_var;
				if (!entity_rid.is_valid()) {
					continue;
				}
				
				uint64_t entity_id = entity_rid.get_id();
				Dictionary entity_dict = _serialize_entity_info(world_rid, entity_id);
				
				if (!entity_dict.is_empty()) {
					entities_array.push_back(entity_dict);
				}
			}
		}

		server->free_query(world_rid, query_rid);
	}

	response["entities"] = entities_array;
	response["offset"] = offset;
	response["count"] = count;

	_send_debugger_message("flecs:entities", response);
	return OK;
}

Error FlecsRuntimeDebugger::_handle_request_components(const Array &p_args) {
	if (!server) {
		return FAILED;
	}

	if (p_args.size() < 2) {
		return FAILED;
	}

	uint64_t world_id = p_args[0];
	uint64_t entity_id = p_args[1];

	if (entity_id == 0) {
		return FAILED;
	}

	RID world_rid = RID::from_uint64(world_id);
	if (!world_rid.is_valid()) {
		return FAILED;
	}

	Dictionary response;
	response["type"] = "components";
	response["world_id"] = world_id;
	response["entity_id"] = entity_id;

	// Get components for the entity - use simplified serialization to avoid crashes
	// during multithreaded world progression
	Array components_array = _serialize_components_safe(world_rid, entity_id);

	response["components"] = components_array;

	_send_debugger_message("flecs:components", response);
	return OK;
}

void FlecsRuntimeDebugger::_send_debugger_message(const String &p_msg, const Dictionary &p_data) {
	EngineDebugger *debugger = EngineDebugger::get_singleton();
	if (!debugger) {
		return;
	}

	Array args;
	args.push_back(p_data);

	debugger->send_message(p_msg, args);
}

Dictionary FlecsRuntimeDebugger::_serialize_world_info(const RID &p_world_rid) {
	Dictionary world_dict;

	if (!server) {
		return world_dict;
	}

	world_dict["id"] = p_world_rid.get_id();
	world_dict["name"] = String("World_") + String::num_int64(p_world_rid.get_id(), 16);

	// Get entity count from a query (empty components = all entities)
	int entity_count = 0;
	PackedStringArray empty_components;
	RID query_rid = server->create_query(p_world_rid, empty_components);

	if (query_rid.is_valid()) {
		entity_count = server->query_get_entity_count(p_world_rid, query_rid);
		server->free_query(p_world_rid, query_rid);
	}

	world_dict["entity_count"] = entity_count;

	return world_dict;
}

Dictionary FlecsRuntimeDebugger::_serialize_entity_info(const RID &p_world_rid, uint64_t p_entity_id) {
	Dictionary entity_dict;

	if (!server || p_entity_id == 0 || !p_world_rid.is_valid()) {
		return entity_dict;
	}

	RID entity_rid = RID::from_uint64(p_entity_id);
	if (!entity_rid.is_valid()) {
		return entity_dict;
	}

	// Verify the entity belongs to a valid world before proceeding
	RID entity_world = server->get_world_of_entity(entity_rid);
	if (!entity_world.is_valid()) {
		return entity_dict;
	}

	entity_dict["id"] = p_entity_id;

	// Get entity name if available
	String entity_name = server->get_entity_name(entity_rid);
	if (entity_name.is_empty() || entity_name == "ERROR") {
		entity_name = String("Entity_") + String::num_int64(p_entity_id, 16);
	}
	entity_dict["name"] = entity_name;

	// Get component count
	PackedStringArray component_names = server->get_component_types_as_name(entity_rid);
	entity_dict["component_count"] = component_names.size();

	return entity_dict;
}

Array FlecsRuntimeDebugger::_serialize_components_safe(const RID &p_world_rid, uint64_t p_entity_id) {
	// Safe component serialization that only returns component names without data
	// This avoids potential crashes from accessing component data during multithreaded
	// world progression. The inspector will show component names but may not show values.
	Array components_array;

	static constexpr int MAX_COMPONENTS_TO_SERIALIZE = 100;

	if (!server || p_entity_id == 0) {
		return components_array;
	}

	RID entity_rid = RID::from_uint64(p_entity_id);
	if (!entity_rid.is_valid()) {
		return components_array;
	}

	// Verify entity exists
	RID entity_world = server->get_world_of_entity(entity_rid);
	if (!entity_world.is_valid()) {
		return components_array;
	}

	// Get component names only - this is safer than accessing component data
	PackedStringArray component_names = server->get_component_types_as_name(entity_rid);
	int total_components = MIN(component_names.size(), MAX_COMPONENTS_TO_SERIALIZE);

	for (int i = 0; i < total_components; i++) {
		String component_name = component_names[i];
		
		if (component_name.is_empty()) {
			continue;
		}

		Dictionary component_dict;
		component_dict["name"] = component_name;
		
		// Mark pairs differently
		if (component_name.begins_with("(")) {
			component_dict["type"] = "pair";
			component_dict["data"] = Dictionary();
		} else {
			component_dict["type"] = "component";
			// For safety, don't try to serialize component data during potential world progression
			// Just return empty data - the UI will show the component exists but without values
			component_dict["data"] = Dictionary();
		}

		components_array.push_back(component_dict);
	}

	return components_array;
}

Array FlecsRuntimeDebugger::_serialize_components(const RID &p_world_rid, uint64_t p_entity_id) {
	Array components_array;

	// Maximum number of components to serialize to prevent runaway iteration
	static constexpr int MAX_COMPONENTS_TO_SERIALIZE = 100;

	if (!server) {
		return components_array;
	}

	if (p_entity_id == 0) {
		return components_array;
	}

	RID entity_rid = RID::from_uint64(p_entity_id);
	if (!entity_rid.is_valid()) {
		return components_array;
	}

	// Verify entity exists by checking its world
	RID entity_world = server->get_world_of_entity(entity_rid);
	if (!entity_world.is_valid()) {
		return components_array;
	}

	// Get all component names
	PackedStringArray component_names = server->get_component_types_as_name(entity_rid);
	int total_components = component_names.size();

	// Sanity check: if too many components, something might be wrong
	if (total_components > MAX_COMPONENTS_TO_SERIALIZE) {
		total_components = MAX_COMPONENTS_TO_SERIALIZE;
	}

	// Re-validate entity is still valid before iterating components
	RID recheck_world = server->get_world_of_entity(entity_rid);
	if (!recheck_world.is_valid()) {
		return components_array;
	}

	for (int i = 0; i < total_components; i++) {
		String component_name = component_names[i];
		
		// Skip empty component names
		if (component_name.is_empty()) {
			continue;
		}

		// Skip pair/relationship components - they start with "(" and can't be looked up by name
		// These are formatted as "(First, Second)" and world.lookup() will fail on them
		if (component_name.begins_with("(")) {
			// Still add them to the list but mark as pair type with no data
			Dictionary component_dict;
			component_dict["name"] = component_name;
			component_dict["type"] = "pair";
			component_dict["data"] = Dictionary();
			components_array.push_back(component_dict);
			continue;
		}

		Dictionary component_dict;
		component_dict["name"] = component_name;
		component_dict["type"] = "component";

		// Try to get component data
		Dictionary component_data = server->get_component_by_name(entity_rid, component_name);
		
		if (!component_data.is_empty()) {
			component_dict["data"] = component_data;
		} else {
			// Still add the component, just without data
			component_dict["data"] = Dictionary();
		}

		components_array.push_back(component_dict);
	}

	return components_array;
}

Error FlecsRuntimeDebugger::_handle_request_profiler_metrics(const Array &p_args) {
	if (!server) {
		return FAILED;
	}

	if (p_args.size() < 1) {
		return FAILED;
	}

	uint64_t world_id = p_args[0];
	RID world_rid = RID::from_uint64(world_id);

	Dictionary response;
	response["type"] = "profiler_metrics";
	response["world_id"] = world_id;

	// Get system metrics from FlecsServer
	Dictionary metrics = server->get_system_metrics(world_rid);
	
	if (metrics.is_empty()) {
		response["error"] = "Failed to retrieve system metrics";
		response["systems"] = Array();
	} else {
		Array systems = metrics.get("systems", Array());
		response["systems"] = systems;
		response["total_time_usec"] = metrics.get("total_time_usec", 0);
		response["system_count"] = metrics.get("system_count", 0);
	}

	_send_debugger_message("flecs:profiler_metrics", response);
	return OK;
}