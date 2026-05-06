/**************************************************************************/
/*  flecs_os_api_traced.h                                                 */
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

#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/debug/ecs_trace_bridge.h"

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

// ============================================================================
// Flecs OS API Traced - Thread-aware wrapper for ECS trace integration
// ============================================================================
// This module provides a custom Flecs OS API that hooks thread creation and
// destruction to register/unregister threads with the Neural Web trace system.
//
// When Flecs creates worker threads (via set_threads()), these threads will
// automatically be registered for tracing, allowing multi-threaded ECS systems
// to emit trace events correctly.
//
// Usage:
// 1. Call FlecsOSApiTraced::install() ONCE before any Flecs world is created
// 2. After this, all Flecs worker threads will be auto-registered for tracing
//
// Design principles (per Axiom Philosophy):
// - No hidden state: thread registration is explicit and tracked
// - Performance is a budget: wrapper overhead is minimal (one atomic + pointer)
// - Data movement is the enemy: wrapper context is small and cache-friendly
// ============================================================================

namespace godot_turbo {

// Forward declaration of thread context wrapper
struct TracedThreadContext;

class FlecsOSApiTraced {
public:
	// ========================================================================
	// Installation
	// ========================================================================
	
	// Install the traced OS API. Call this ONCE before creating Flecs worlds.
	// Safe to call multiple times (will only install once).
	static void install() {
		if (installed.load(std::memory_order_acquire)) {
			return;
		}
		
		std::lock_guard<std::mutex> lock(install_mutex);
		if (installed.load(std::memory_order_relaxed)) {
			return;
		}
		
		// Get current defaults
		ecs_os_set_api_defaults();
		
		// Store the original API functions
		original_api = ecs_os_api;
		
		// Create our wrapped API
		ecs_os_api_t traced_api = ecs_os_api;
		
		// Override thread/task creation functions
		traced_api.thread_new_ = traced_thread_new;
		traced_api.thread_join_ = traced_thread_join;
		traced_api.task_new_ = traced_task_new;
		traced_api.task_join_ = traced_task_join;
		
		// Also wrap mutex operations for potential future use
		traced_api.mutex_new_ = traced_mutex_new;
		traced_api.mutex_free_ = traced_mutex_free;
		traced_api.mutex_lock_ = traced_mutex_lock;
		traced_api.mutex_unlock_ = traced_mutex_unlock;
		
		// Condition variables
		traced_api.cond_new_ = traced_cond_new;
		traced_api.cond_free_ = traced_cond_free;
		traced_api.cond_signal_ = traced_cond_signal;
		traced_api.cond_broadcast_ = traced_cond_broadcast;
		traced_api.cond_wait_ = traced_cond_wait;
		
		// Atomics
		traced_api.ainc_ = traced_ainc;
		traced_api.adec_ = traced_adec;
		traced_api.lainc_ = traced_lainc;
		traced_api.ladec_ = traced_ladec;
		
		// Install the traced API
		ecs_os_set_api(&traced_api);
		
		installed.store(true, std::memory_order_release);
	}
	
	// Check if traced API is installed
	static bool is_installed() {
		return installed.load(std::memory_order_acquire);
	}
	
	// Get count of currently active traced worker threads
	static int get_active_thread_count() {
		return active_thread_count.load(std::memory_order_relaxed);
	}

private:
	// ========================================================================
	// Thread Context
	// ========================================================================
	
	struct TracedThreadContext {
		ecs_os_thread_callback_t original_callback;
		void *original_arg;
		std::thread *thread_handle;
		bool is_task; // true for task threads, false for regular threads
	};
	
	// Wrapper callback that handles thread registration
	static void* traced_thread_callback_wrapper(void *arg) {
		TracedThreadContext *ctx = static_cast<TracedThreadContext*>(arg);
		
		// Register this thread for tracing
		ECS_TRACE_REGISTER_THREAD();
		active_thread_count.fetch_add(1, std::memory_order_relaxed);
		
		// Store the original callback and arg before we potentially delete context
		ecs_os_thread_callback_t callback = ctx->original_callback;
		void *callback_arg = ctx->original_arg;
		
		// Call the original Flecs thread callback
		void *result = callback(callback_arg);
		
		// Unregister this thread from tracing
		active_thread_count.fetch_sub(1, std::memory_order_relaxed);
		ECS_TRACE_UNREGISTER_THREAD();
		
		return result;
	}
	
	// ========================================================================
	// Thread API Wrappers
	// ========================================================================
	
	static ecs_os_thread_t traced_thread_new(
		ecs_os_thread_callback_t callback,
		void *arg)
	{
		// Create context for the wrapper
		TracedThreadContext *ctx = new TracedThreadContext();
		ctx->original_callback = callback;
		ctx->original_arg = arg;
		ctx->is_task = false;
		
		// Create the thread with our wrapper callback
		std::thread *thread = new std::thread([ctx]() {
			traced_thread_callback_wrapper(ctx);
			delete ctx;
		});
		
		return reinterpret_cast<ecs_os_thread_t>(thread);
	}
	
	static void* traced_thread_join(ecs_os_thread_t thread) {
		std::thread *thr = reinterpret_cast<std::thread*>(thread);
		if (thr && thr->joinable()) {
			thr->join();
		}
		delete thr;
		return nullptr;
	}
	
	static ecs_os_thread_t traced_task_new(
		ecs_os_thread_callback_t callback,
		void *arg)
	{
		// Tasks use the same mechanism as threads in Flecs
		TracedThreadContext *ctx = new TracedThreadContext();
		ctx->original_callback = callback;
		ctx->original_arg = arg;
		ctx->is_task = true;
		
		std::thread *thread = new std::thread([ctx]() {
			traced_thread_callback_wrapper(ctx);
			delete ctx;
		});
		
		return reinterpret_cast<ecs_os_thread_t>(thread);
	}
	
	static void* traced_task_join(ecs_os_thread_t thread) {
		return traced_thread_join(thread);
	}
	
	// ========================================================================
	// Mutex API Wrappers (pass-through to std::mutex)
	// ========================================================================
	
	static ecs_os_mutex_t traced_mutex_new(void) {
		return reinterpret_cast<ecs_os_mutex_t>(new std::mutex());
	}
	
	static void traced_mutex_free(ecs_os_mutex_t m) {
		delete reinterpret_cast<std::mutex*>(m);
	}
	
	static void traced_mutex_lock(ecs_os_mutex_t m) {
		reinterpret_cast<std::mutex*>(m)->lock();
	}
	
	static void traced_mutex_unlock(ecs_os_mutex_t m) {
		reinterpret_cast<std::mutex*>(m)->unlock();
	}
	
	// ========================================================================
	// Condition Variable API Wrappers
	// ========================================================================
	
	static ecs_os_cond_t traced_cond_new(void) {
		return reinterpret_cast<ecs_os_cond_t>(new std::condition_variable_any());
	}
	
	static void traced_cond_free(ecs_os_cond_t c) {
		delete reinterpret_cast<std::condition_variable_any*>(c);
	}
	
	static void traced_cond_signal(ecs_os_cond_t c) {
		reinterpret_cast<std::condition_variable_any*>(c)->notify_one();
	}
	
	static void traced_cond_broadcast(ecs_os_cond_t c) {
		reinterpret_cast<std::condition_variable_any*>(c)->notify_all();
	}
	
	static void traced_cond_wait(ecs_os_cond_t c, ecs_os_mutex_t m) {
		std::condition_variable_any *cond = reinterpret_cast<std::condition_variable_any*>(c);
		std::mutex *mutex = reinterpret_cast<std::mutex*>(m);
		cond->wait(*mutex);
	}
	
	// ========================================================================
	// Atomic API Wrappers
	// ========================================================================
	
	static int32_t traced_ainc(int32_t *count) {
#ifdef __GNUC__
		return __sync_add_and_fetch(count, 1);
#elif defined(_MSC_VER)
		return InterlockedIncrement(reinterpret_cast<volatile long*>(count));
#else
		return ++(*count); // Fallback, not thread-safe
#endif
	}
	
	static int32_t traced_adec(int32_t *count) {
#ifdef __GNUC__
		return __sync_sub_and_fetch(count, 1);
#elif defined(_MSC_VER)
		return InterlockedDecrement(reinterpret_cast<volatile long*>(count));
#else
		return --(*count); // Fallback, not thread-safe
#endif
	}
	
	static int64_t traced_lainc(int64_t *count) {
#ifdef __GNUC__
		return __sync_add_and_fetch(count, 1);
#elif defined(_MSC_VER)
		return InterlockedIncrement64(count);
#else
		return ++(*count); // Fallback, not thread-safe
#endif
	}
	
	static int64_t traced_ladec(int64_t *count) {
#ifdef __GNUC__
		return __sync_sub_and_fetch(count, 1);
#elif defined(_MSC_VER)
		return InterlockedDecrement64(count);
#else
		return --(*count); // Fallback, not thread-safe
#endif
	}
	
	// ========================================================================
	// State
	// ========================================================================
	
	static inline std::atomic<bool> installed{false};
	static inline std::mutex install_mutex;
	static inline ecs_os_api_t original_api;
	static inline std::atomic<int> active_thread_count{0};
};

} // namespace godot_turbo

// ============================================================================
// Convenience Macro
// ============================================================================

#define FLECS_OS_API_TRACED_INSTALL() \
	godot_turbo::FlecsOSApiTraced::install()

