#ifndef FLECS_RUNTIME_DEBUGGER_H
#define FLECS_RUNTIME_DEBUGGER_H

#include "core/string/string_name.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/debugger/engine_debugger.h"
#include "core/templates/rid.h"
#include "core/object/object.h"
#include "core/object/class_db.h"
#include "scene/main/scene_tree.h"

class FlecsServer;
class Timer;

/**
 * FlecsRuntimeDebugger handles debugger messages at runtime.
 * It responds to editor requests for world, entity, and component information.
 *
 * This class extends Object and uses a Timer to retry debugger registration
 * when running from the editor. The debugger connection is not immediately
 * available during module initialization, so we need to retry until it connects.
 */
class FlecsRuntimeDebugger : public Object {
	GDCLASS(FlecsRuntimeDebugger, Object);

public:
	FlecsRuntimeDebugger();
	~FlecsRuntimeDebugger();

	/** Initialize the runtime debugger and register message capture */
	void initialize();

	/** Shutdown the runtime debugger */
	void shutdown();

	/** Get singleton instance */
	static FlecsRuntimeDebugger *get_singleton() { return singleton; }

	/** Message capture callback - static wrapper for C++ function pointers */
	static Error _capture_message(void *p_user, const String &p_msg, const Array &p_args, bool &r_captured);

protected:
	static void _bind_methods();

private:
	FlecsServer *server = nullptr;
	ObjectID retry_timer_id;
	bool initialized = false;
	bool capture_registered = false;
	int retry_count = 0;
	static constexpr int MAX_RETRY_COUNT = 50; // ~5 seconds at 100ms intervals

	/** Timer callback to retry registration */
	void _on_retry_timer();

	/** Setup the retry timer (called deferred after scene tree is ready) */
	void _setup_retry_timer();

	/** Attach the retry timer on the next process frame once the root is stable. */
	void _attach_retry_timer();

	/** Resolve retry timer from ObjectDB safely. */
	Timer *_get_retry_timer() const;

	/** Stop and queue-delete retry timer when still valid. */
	void _dispose_retry_timer();

	/** Attempt to register the message capture with the debugger */
	bool _try_register_capture();

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
