/**************************************************************************/
/*  ecs_trace_bridge.h                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT TURBO                                */
/*               https://github.com/KairosMusic/godot-turbo               */
/**************************************************************************/
/* Copyright (c) 2025 Joëlle Ubink                                        */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "core/config/engine.h"
#include "core/object/object.h"

#include <cstdint>

// ============================================================================
// ECS Trace Bridge - Runtime bridge to AxiomScript's Neural Visualizer
// ============================================================================
// This header provides a lightweight, zero-overhead bridge from godot_turbo's
// FlecsServer to axiomscript's TraceAggregator. It uses runtime singleton
// lookup to avoid hard dependencies between modules.
//
// Design principles (per Axiom Philosophy):
// - No hidden state: all tracing goes through explicit calls
// - Performance is a budget: singleton is cached, minimal overhead when disabled
// - Approximation must be honest: events are dropped rather than blocked
// - Separation of concerns: godot_turbo doesn't need to know axiomscript internals
//
// Usage:
// 1. Call ECSTraceBridge::initialize() once at startup (after Engine singletons exist)
// 2. Use ECS_TRACE_* macros at instrumentation points
// 3. Call ECSTraceBridge::shutdown() at cleanup
// ============================================================================

namespace godot_turbo {

class ECSTraceBridge {
public:
	// ========================================================================
	// Lifecycle
	// ========================================================================

	// Initialize the bridge - looks up NeuralWebSystem singleton
	// Safe to call multiple times, will only initialize once
	static void initialize() {
		if (initialized) {
			return;
		}

		// Try to find the NeuralWebSystem singleton from axiomscript
		Object *singleton = Engine::get_singleton()->get_singleton_object("NeuralWebSystem");
		if (singleton) {
			neural_web_system = singleton;
			// Cache the method binds for faster calls
			has_emit_read = singleton->has_method("_trace_emit_read");
			has_emit_write = singleton->has_method("_trace_emit_write");
			has_emit_add = singleton->has_method("_trace_emit_add");
			has_emit_remove = singleton->has_method("_trace_emit_remove");
			has_emit_query = singleton->has_method("_trace_emit_query");
			has_emit_entity_create = singleton->has_method("_trace_emit_entity_create");
			has_emit_entity_destroy = singleton->has_method("_trace_emit_entity_destroy");
			has_register_thread = singleton->has_method("_trace_register_thread");
			has_unregister_thread = singleton->has_method("_trace_unregister_thread");
			has_is_enabled = singleton->has_method("is_enabled");
		}

		initialized = true;
	}

	// Shutdown the bridge - clears cached singleton
	static void shutdown() {
		neural_web_system = nullptr;
		initialized = false;
		has_emit_read = false;
		has_emit_write = false;
		has_emit_add = false;
		has_emit_remove = false;
		has_emit_query = false;
		has_emit_entity_create = false;
		has_emit_entity_destroy = false;
		has_register_thread = false;
		has_unregister_thread = false;
		has_is_enabled = false;
	}

	// ========================================================================
	// Status
	// ========================================================================

	// Check if tracing is available (NeuralWebSystem was found)
	static bool is_available() {
		return neural_web_system != nullptr;
	}

	// Check if tracing is currently enabled
	static bool is_enabled() {
		if (!neural_web_system || !has_is_enabled) {
			return false;
		}
		return neural_web_system->call("is_enabled");
	}

	// ========================================================================
	// Thread Registration
	// ========================================================================

	// Register current thread for tracing (call from worker thread startup)
	static void register_thread() {
		if (!neural_web_system || !has_register_thread) {
			return;
		}
		neural_web_system->call("_trace_register_thread");
	}

	// Unregister current thread (call from worker thread shutdown)
	static void unregister_thread() {
		if (!neural_web_system || !has_unregister_thread) {
			return;
		}
		neural_web_system->call("_trace_unregister_thread");
	}

	// ========================================================================
	// Event Emission - Hot Path
	// ========================================================================
	// These are designed to be as fast as possible when tracing is disabled.
	// When enabled, they forward to NeuralWebSystem via Godot's call() mechanism.

	// Emit a component read event
	static void emit_read(uint64_t p_entity_id, uint64_t p_component_id, uint32_t p_field_hash = 0) {
		if (!neural_web_system || !has_emit_read) {
			return;
		}
		neural_web_system->call("_trace_emit_read", p_entity_id, p_component_id, p_field_hash);
	}

	// Emit a component write event
	static void emit_write(uint64_t p_entity_id, uint64_t p_component_id, uint32_t p_field_hash = 0) {
		if (!neural_web_system || !has_emit_write) {
			return;
		}
		neural_web_system->call("_trace_emit_write", p_entity_id, p_component_id, p_field_hash);
	}

	// Emit a component add event
	static void emit_add(uint64_t p_entity_id, uint64_t p_component_id) {
		if (!neural_web_system || !has_emit_add) {
			return;
		}
		neural_web_system->call("_trace_emit_add", p_entity_id, p_component_id);
	}

	// Emit a component remove event
	static void emit_remove(uint64_t p_entity_id, uint64_t p_component_id) {
		if (!neural_web_system || !has_emit_remove) {
			return;
		}
		neural_web_system->call("_trace_emit_remove", p_entity_id, p_component_id);
	}

	// Emit a query iteration event
	static void emit_query(uint64_t p_entity_id, uint64_t p_component_id) {
		if (!neural_web_system || !has_emit_query) {
			return;
		}
		neural_web_system->call("_trace_emit_query", p_entity_id, p_component_id);
	}

	// Emit an entity creation event
	static void emit_entity_create(uint64_t p_entity_id) {
		if (!neural_web_system || !has_emit_entity_create) {
			return;
		}
		neural_web_system->call("_trace_emit_entity_create", p_entity_id);
	}

	// Emit an entity destruction event
	static void emit_entity_destroy(uint64_t p_entity_id) {
		if (!neural_web_system || !has_emit_entity_destroy) {
			return;
		}
		neural_web_system->call("_trace_emit_entity_destroy", p_entity_id);
	}

	// ========================================================================
	// Name Registration (for UI display)
	// ========================================================================

	// Register a human-readable name for an entity
	static void register_entity_name(uint64_t p_entity_id, const String &p_name) {
		if (!neural_web_system) {
			return;
		}
		if (neural_web_system->has_method("register_entity_name")) {
			neural_web_system->call("register_entity_name", p_entity_id, p_name);
		}
	}

	// Register a human-readable name for a component type
	static void register_component_name(uint64_t p_component_id, const String &p_name) {
		if (!neural_web_system) {
			return;
		}
		if (neural_web_system->has_method("register_component_name")) {
			neural_web_system->call("register_component_name", p_component_id, p_name);
		}
	}

private:
	// Cached singleton pointer
	static inline Object *neural_web_system = nullptr;
	static inline bool initialized = false;

	// Cached method availability flags (avoid repeated has_method calls)
	static inline bool has_emit_read = false;
	static inline bool has_emit_write = false;
	static inline bool has_emit_add = false;
	static inline bool has_emit_remove = false;
	static inline bool has_emit_query = false;
	static inline bool has_emit_entity_create = false;
	static inline bool has_emit_entity_destroy = false;
	static inline bool has_register_thread = false;
	static inline bool has_unregister_thread = false;
	static inline bool has_is_enabled = false;
};

} // namespace godot_turbo

// ============================================================================
// Instrumentation Macros
// ============================================================================
// Use these macros at instrumentation points in FlecsServer and related code.
// They check for bridge availability inline for minimal overhead when tracing
// is not active.
//
// The macros are always defined (no compile-time toggle) because:
// 1. The runtime check is very fast (null pointer check)
// 2. We want tracing to be toggleable at runtime for production debugging
// 3. The call() overhead only occurs when tracing is actually enabled

#define ECS_TRACE_INIT() \
	godot_turbo::ECSTraceBridge::initialize()

#define ECS_TRACE_SHUTDOWN() \
	godot_turbo::ECSTraceBridge::shutdown()

#define ECS_TRACE_REGISTER_THREAD() \
	godot_turbo::ECSTraceBridge::register_thread()

#define ECS_TRACE_UNREGISTER_THREAD() \
	godot_turbo::ECSTraceBridge::unregister_thread()

#define ECS_TRACE_READ(entity_id, component_id, field_hash) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_read(entity_id, component_id, field_hash); \
		} \
	} while (0)

#define ECS_TRACE_WRITE(entity_id, component_id, field_hash) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_write(entity_id, component_id, field_hash); \
		} \
	} while (0)

#define ECS_TRACE_ADD(entity_id, component_id) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_add(entity_id, component_id); \
		} \
	} while (0)

#define ECS_TRACE_REMOVE(entity_id, component_id) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_remove(entity_id, component_id); \
		} \
	} while (0)

#define ECS_TRACE_QUERY(entity_id, component_id) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_query(entity_id, component_id); \
		} \
	} while (0)

#define ECS_TRACE_ENTITY_CREATE(entity_id) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_entity_create(entity_id); \
		} \
	} while (0)

#define ECS_TRACE_ENTITY_DESTROY(entity_id) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::emit_entity_destroy(entity_id); \
		} \
	} while (0)

#define ECS_TRACE_REGISTER_ENTITY_NAME(entity_id, name) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::register_entity_name(entity_id, name); \
		} \
	} while (0)

#define ECS_TRACE_REGISTER_COMPONENT_NAME(component_id, name) \
	do { \
		if (godot_turbo::ECSTraceBridge::is_available()) { \
			godot_turbo::ECSTraceBridge::register_component_name(component_id, name); \
		} \
	} while (0)

