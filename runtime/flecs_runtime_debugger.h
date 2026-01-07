#ifndef FLECS_RUNTIME_DEBUGGER_H
#define FLECS_RUNTIME_DEBUGGER_H

#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/debugger/engine_debugger.h"

class FlecsServer;

/**
 * FlecsRuntimeDebugger handles debugger messages at runtime.
 * It responds to editor requests for world, entity, and component information.
 */
class FlecsRuntimeDebugger {
public:
	FlecsRuntimeDebugger();
	~FlecsRuntimeDebugger();

	/** Initialize the runtime debugger and register message capture */
	void initialize();

	/** Shutdown the runtime debugger */
	void shutdown();

	/** Message capture callback - static wrapper for C++ function pointers */
	static Error _capture_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured);

private:
	FlecsServer *server = nullptr;
	bool initialized = false;

	/** Handle the "flecs:request_worlds" message */
	Error _handle_request_worlds(const Array &p_args);

	/** Handle the "flecs:request_entities" message */
	Error _handle_request_entities(const Array &p_args);

	/** Handle the "flecs:request_components" message */
	Error _handle_request_components(const Array &p_args);

	/** Handle the "flecs:request_profiler_metrics" message */
	Error _handle_request_profiler_metrics(const Array &p_args);

	/** Helper: Send a message back to the editor debugger */
	void _send_debugger_message(const String &p_msg, const Dictionary &p_data);

	/** Helper: Serialize world information */
	Dictionary _serialize_world_info(const RID &p_world_rid);

	/** Helper: Serialize entity information */
	Dictionary _serialize_entity_info(const RID &p_world_rid, uint64_t p_entity_id);

	/** Helper: Serialize component information (full data) */
	Array _serialize_components(const RID &p_world_rid, uint64_t p_entity_id);

	/** Helper: Serialize component information (safe - names only, no data access) */
	Array _serialize_components_safe(const RID &p_world_rid, uint64_t p_entity_id);

	/** Static instance for callback dispatch */
	static FlecsRuntimeDebugger *singleton;
};

#endif // FLECS_RUNTIME_DEBUGGER_H