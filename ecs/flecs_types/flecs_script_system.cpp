//
// Created by Floof on 21-7-2025.
//
#include "flecs_script_system.h"
#include "core/string/string_name.h"
#include "core/templates/rid.h"
#include "core/variant/array.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#include "modules/godot_turbo/ecs/components/component_reflection.h"
#include "modules/godot_turbo/thirdparty/flecs/distr/flecs.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "core/os/os.h"

// Clean refactored implementation below

std::atomic_uint32_t FlecsScriptSystem::global_system_index = 0; // definition

namespace {
static flecs::entity resolve_component_entity(flecs::world *world, const String &component_name) {
	if (!world) {
		return flecs::entity();
	}

	const CharString cname = component_name.ascii();
	const char *cname_ptr = cname.get_data();
	if (!cname_ptr || cname_ptr[0] == '\0') {
		return flecs::entity();
	}

	const ecs_world_t *c_world = world->c_ptr();
	ecs_entity_t resolved_id = ecs_lookup_symbol(c_world, cname_ptr, true, true);
	if (resolved_id != 0) {
		return flecs::entity(c_world, resolved_id);
	}

	flecs::entity resolved;

	const String suffix_ns = String("::") + component_name;
	const String suffix_dot = String(".") + component_name;
	world->each<flecs::Component>([&](flecs::entity e, flecs::Component &) {
		if (resolved.is_valid()) {
			return;
		}
		const char *name = e.name().c_str();
		const char *symbol = e.symbol().c_str();
		if (name) {
			const String name_str(name);
			if (name_str == component_name || name_str.ends_with(suffix_ns) || name_str.ends_with(suffix_dot)) {
				resolved = e;
				return;
			}
		}
		if (symbol) {
			const String symbol_str(symbol);
			if (symbol_str == component_name || symbol_str.ends_with(suffix_ns) || symbol_str.ends_with(suffix_dot)) {
				resolved = e;
			}
		}
	});

	if (!resolved.is_valid()) {
		resolved = world->component(cname_ptr);
	}

	return resolved;
}
} // namespace

// ============================================================================
// Helper Methods for build_system()
// ============================================================================

void FlecsScriptSystem::cleanup_existing_systems() {
	if (script_system.is_alive()) { script_system.destruct(); }
	if (change_observer.is_alive()) { change_observer.destruct(); }
	if (change_observer_add.is_alive()) { change_observer_add.destruct(); }
	if (change_observer_remove.is_alive()) { change_observer_remove.destruct(); }
	if (reset_system.is_alive()) { reset_system.destruct(); }
}

Vector<flecs::entity> FlecsScriptSystem::get_component_terms() {
	Vector<flecs::entity> comp_terms;
	for (int i = 0; i < required_components.size(); ++i) {
		String cname = required_components.get(i);
		flecs::entity ce = resolve_component_entity(world, cname);
		if (!ce.is_valid()) {
			print_line(String("Invalid component name: ") + cname);
			continue;
		}
		comp_terms.push_back(ce);
	}
	return comp_terms;
}

Dictionary FlecsScriptSystem::serialize_entity_components(flecs::entity e) {
	Dictionary comp_dicts;
	for (int ci = 0; ci < required_components.size(); ++ci) {
		String cname = required_components.get(ci);
		flecs::entity ce = resolve_component_entity(world, cname);
		if (!ce.is_valid()) { continue; }
		Dictionary value;
		if (e.has(ce)) {
			value = FlecsReflection::Registry::get().serialize(e, ce.id());
		}
		comp_dicts[StringName(cname)] = value;
	}
	return comp_dicts;
}

void FlecsScriptSystem::update_instrumentation(uint64_t start_time) {
	if (!instrumentation_enabled) { return; }
	
	uint64_t dt = OS::get_singleton()->get_ticks_usec() - start_time;
	last_frame_dispatch_usec = dt;
	frame_dispatch_invocations += 1;
	frame_dispatch_accum_usec += dt;
	if (dt < frame_dispatch_min_usec) { frame_dispatch_min_usec = dt; }
	if (dt > frame_dispatch_max_usec) { frame_dispatch_max_usec = dt; }
	if (detailed_timing_enabled && frame_dispatch_samples.size() < max_sample_count) {
		frame_dispatch_samples.push_back(dt);
	}
}

void FlecsScriptSystem::dispatch_callback(const Array& data) {
	if (use_deferred_calls) {
		callback.call_deferred(data);
	} else {
		callback.call(data);
	}
}

void FlecsScriptSystem::build_change_observer_system() {
	Vector<flecs::entity> comp_terms = get_component_terms();
	if (comp_terms.is_empty()) {
		ERR_PRINT("FlecsScriptSystem change observer: no valid component terms");
		return;
	}
	
	auto make_observer = [this, &comp_terms](flecs::entity_t evt, uint64_t &last_counter, uint64_t &total_counter) {
		flecs::observer_builder<> ob = world->observer();
		ob.event(evt);
		for (int i = 0; i < comp_terms.size(); ++i) {
			ob.with(comp_terms[i].id());
		}
		return ob.each([this, &last_counter, &total_counter](flecs::entity e) {
			if (is_paused || !callback.is_valid()) { return; }
			
			uint64_t t0 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;
			
			FlecsServer *server = FlecsServer::get_singleton();
			if (!server) {
				ERR_PRINT("FlecsScriptSystem observer: FlecsServer null");
				return;
			}
			
			RID wid = world_id;
			if (!wid.is_valid()) {
				ERR_PRINT("FlecsScriptSystem observer: invalid world id");
				return;
			}
			
			RID rid = server->_get_or_create_rid_for_entity(wid, e);
			Dictionary comp_dicts = serialize_entity_components(e);
			
			Array arr;
			arr.resize(1);
			Dictionary row;
			row["rid"] = rid;
			row["components"] = comp_dicts;
			arr[0] = row;
			
			dispatch_callback(arr);
			
			if (instrumentation_enabled) {
				total_entities_processed += 1;
				total_callbacks_invoked += 1;
				last_frame_entity_count += 1;
				last_frame_batch_size = 1;
				update_instrumentation(t0);
				last_counter += 1;
				total_counter += 1;
			}
		});
	};
	
	// Create observers for different events
	change_observer = make_observer(flecs::OnSet, last_frame_onset, total_onset);
	if (observe_add_and_set) {
		change_observer_add = make_observer(flecs::OnAdd, last_frame_onadd, total_onadd);
	}
	if (observe_remove) {
		change_observer_remove = make_observer(flecs::OnRemove, last_frame_onremove, total_onremove);
	}
}

void FlecsScriptSystem::build_task_system() {
	script_system = world->system()
		.kind(flecs::OnUpdate)
		.run([this](flecs::iter& it) {
			if (is_paused || !callback.is_valid()) { return; }
			
			uint64_t t0 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;
			Array empty; // No entities/components to report
			
			dispatch_callback(empty);
			
			if (instrumentation_enabled) {
				total_callbacks_invoked += 1;
				last_frame_batch_size = 0;
				update_instrumentation(t0);
			}
		});
}

void FlecsScriptSystem::build_entity_iteration_system() {
	flecs::system_builder<> builder = world->system().kind(flecs::OnUpdate);
	
	for (int i = 0; i < required_components.size(); ++i) {
		String cname = required_components.get(i);
		flecs::entity ce = resolve_component_entity(world, cname);
		if (!ce.is_valid()) {
			print_line(String("Invalid component name: ") + cname);
			continue;
		}
		builder.with(ce.id());
	}
	
	// Enable multi-threading for regular entity-iterating systems
	if (required_components.size() > 0 && multi_threaded) {
		builder.multi_threaded(true);
	}
	
	script_system = builder.each([this](flecs::entity e) {
		if (is_paused || !callback.is_valid()) { return; }
		
		uint64_t t0 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;
		
		FlecsServer *server = FlecsServer::get_singleton();
		if (!server) {
			ERR_PRINT("FlecsScriptSystem system iter: FlecsServer null");
			return;
		}
		
		RID wid = world_id;
		if (!wid.is_valid()) {
			ERR_PRINT("FlecsScriptSystem system iter: invalid world id");
			return;
		}
		
		RID rid = server->_get_or_create_rid_for_entity(wid, e);
		Dictionary comp_dicts = serialize_entity_components(e);
		
		Dictionary row;
		row["rid"] = rid;
		row["components"] = comp_dicts;
		
		// Multi-threaded: accumulate and flush later
		if (multi_threaded) {
			{
				std::lock_guard<std::mutex> _l(batch_mtx);
				batch_accumulator.push_back(row);
				batch_dirty = true;
			}
			if (instrumentation_enabled) {
				std::lock_guard<std::mutex> _li(instr_mtx);
				total_entities_processed += 1;
				last_frame_entity_count += 1;
			}
			return;
		}
		
		// Per-entity dispatch
		if (dispatch_mode == DISPATCH_PER_ENTITY) {
			Array single;
			single.resize(1);
			single[0] = row;
			
			dispatch_callback(single);
			
			if (instrumentation_enabled) {
				total_callbacks_invoked += 1;
				last_frame_batch_size = 1;
				update_instrumentation(t0);
			}
		} else {
			// Batch accumulation in single-threaded mode
			batch_accumulator.push_back(row);
			batch_dirty = true;
		}
		
		if (instrumentation_enabled) {
			total_entities_processed += 1;
			last_frame_entity_count += 1;
		}
	});
	
	String base_name = system_name.is_empty() ? String("ScriptSystem" + itos(id)) : system_name;
	String unique_name = base_name;
	if (world) {
		const CharString base_name_cs = base_name.ascii();
		flecs::entity existing = world->lookup(base_name_cs.get_data());
		if (existing.is_valid() && existing != script_system) {
			unique_name = base_name + "#" + itos(id);
		}
	}
	script_system.set_name(unique_name.ascii().get_data());
}

void FlecsScriptSystem::build_batch_flush_system() {
	// No flush needed for task systems
	if (required_components.size() == 0) {
		if (batch_flush_system.is_alive()) {
			batch_flush_system.destruct();
		}
		return;
	}
	
	// Only create flush system if in batch mode or multi-threaded
	if (dispatch_mode != DISPATCH_BATCH && !multi_threaded) {
		if (batch_flush_system.is_alive()) {
			batch_flush_system.destruct();
		}
		return;
	}
	
	if (batch_flush_system.is_alive()) {
		batch_flush_system.destruct();
	}
	
	batch_flush_system = world->system()
		.kind(flecs::PostUpdate)
		.run([this](flecs::iter& it) {
			if (!batch_dirty || is_paused || !callback.is_valid()) { return; }
			
			// Respect minimum flush interval if configured
			if (min_flush_interval_usec > 0) {
				uint64_t now = OS::get_singleton()->get_ticks_usec();
				if (last_flush_time_usec != 0 && (now - last_flush_time_usec) < min_flush_interval_usec) {
					return; // skip this frame; try next
				}
			}
			
			Array buffered;
			{
				std::lock_guard<std::mutex> _l(batch_mtx);
				batch_dirty = false;
				buffered = batch_accumulator;
				batch_accumulator.clear();
			}
			
			if (buffered.is_empty()) { return; }
			
			uint64_t t0 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;
			
			// Chunked flushing if requested
			if (batch_flush_chunk_size > 0 && buffered.size() > batch_flush_chunk_size) {
				for (int i = 0; i < buffered.size(); i += batch_flush_chunk_size) {
					int len = MIN(batch_flush_chunk_size, (int)buffered.size() - i);
					Array slice;
					slice.resize(len);
					for (int j = 0; j < len; ++j) {
						slice[j] = buffered[i + j];
					}
					
					uint64_t t1 = instrumentation_enabled ? OS::get_singleton()->get_ticks_usec() : 0;
					dispatch_callback(slice);
					
					if (instrumentation_enabled) {
						std::lock_guard<std::mutex> _li(instr_mtx);
						total_callbacks_invoked += 1;
						last_frame_batch_size = slice.size();
						update_instrumentation(t1);
					}
				}
			} else {
				dispatch_callback(buffered);
				
				if (instrumentation_enabled) {
					std::lock_guard<std::mutex> _li(instr_mtx);
					total_callbacks_invoked += 1;
					last_frame_batch_size = buffered.size();
					update_instrumentation(t0);
				}
			}
			
			last_flush_time_usec = OS::get_singleton()->get_ticks_usec();
		});
}

void FlecsScriptSystem::build_auto_reset_system() {
	if (!instrumentation_enabled || !auto_reset_per_frame) {
		return;
	}
	
	reset_system = world->system()
		.kind(flecs::PreUpdate)
		.run([this](flecs::iter& it) {
			last_frame_entity_count = 0;
			last_frame_batch_size = 0;
			last_frame_dispatch_usec = 0;
			frame_dispatch_invocations = 0;
			frame_dispatch_accum_usec = 0;
			frame_dispatch_min_usec = UINT64_MAX;
			frame_dispatch_max_usec = 0;
			last_frame_onadd = 0;
			last_frame_onset = 0;
			last_frame_onremove = 0;
			frame_dispatch_samples.clear();
		});
}

// ============================================================================
// Main build_system() - Orchestrates the helper methods
// ============================================================================

void FlecsScriptSystem::build_system() {
	if (!world) {
		ERR_PRINT("FlecsScriptSystem::build_system: world is null");
		return;
	}
	
	cleanup_existing_systems();
	
	// Change-only mode uses observers instead of per-frame systems
	if (change_only) {
		build_change_observer_system();
		return;
	}
	
	// Build appropriate system type
	if (required_components.size() == 0) {
		build_task_system();
	} else {
		build_entity_iteration_system();
	}
	
	// Build supporting systems
	build_batch_flush_system();
	build_auto_reset_system();
}

void FlecsScriptSystem::set_dispatch_mode(DispatchMode p_mode) {
	if (change_only && p_mode == DISPATCH_BATCH) {
		ERR_PRINT("Cannot set batch dispatch while in change-only mode. Disable change-only first.");
		return;
	}
	dispatch_mode = (int)p_mode;
    build_system();
}

void FlecsScriptSystem::set_change_only(bool p_change_only) {
	if (change_only == p_change_only) { return; }
	if (p_change_only && dispatch_mode == (int)DISPATCH_BATCH) {
		ERR_PRINT("Cannot enable change-only while in batch dispatch mode. Switch to per-entity first.");
		return;
	}
	change_only = p_change_only;
	build_system();
}

void FlecsScriptSystem::set_change_observe_add_and_set(bool p_both) {
	if (observe_add_and_set == p_both) { return; }
	observe_add_and_set = p_both;
	if (change_only) { build_system(); }
}

void FlecsScriptSystem::reset_instrumentation() {
	last_frame_entity_count = 0;
	last_frame_batch_size = 0;
	last_frame_dispatch_usec = 0;
	frame_dispatch_invocations = 0;
	frame_dispatch_accum_usec = 0;
	frame_dispatch_min_usec = UINT64_MAX;
	frame_dispatch_max_usec = 0;
	last_frame_onadd = last_frame_onset = last_frame_onremove = 0;
	frame_dispatch_samples.clear();
}

void FlecsScriptSystem::set_change_observe_remove(bool p_remove) {
	if (observe_remove == p_remove) { return; }
	observe_remove = p_remove;
	if (change_only) { build_system(); }
}

void FlecsScriptSystem::init(const RID &p_world_id, const PackedStringArray &req_comps, const Callable &p_callable) {
	set_world(p_world_id);
	required_components = req_comps;
	callback = p_callable;
	build_system();
}

void FlecsScriptSystem::reset(const RID &p_world_id, const PackedStringArray &req_comps, const Callable &p_callable) { init(p_world_id, req_comps, p_callable); }

void FlecsScriptSystem::set_required_components(const PackedStringArray &p_required_components) { required_components = p_required_components; build_system(); }
PackedStringArray FlecsScriptSystem::get_required_components() const { return required_components; }
void FlecsScriptSystem::set_callback(const Callable &p_callback) { callback = p_callback; build_system(); }
Callable FlecsScriptSystem::get_callback() const { return callback; }
PackedStringArray FlecsScriptSystem::get_required_components() { return required_components; }

flecs::world *FlecsScriptSystem::_get_world() const { return world; }
void FlecsScriptSystem::_set_world(flecs::world *p_world) { world = p_world; }

RID FlecsScriptSystem::get_world() {
	if (!world || !world_id.is_valid()) { ERR_PRINT("FlecsScriptSystem::get_world: world not set"); return RID(); }
	return world_id;
}

void FlecsScriptSystem::set_world(const RID &p_world_id) {
	world_id = p_world_id;
	world = FlecsServer::get_singleton()->_get_world(p_world_id);
	if (!world) { ERR_PRINT("FlecsScriptSystem::set_world: invalid world"); return; }
	build_system();
}

void FlecsScriptSystem::set_system_dependency(uint32_t p_system_id) {
	if (p_system_id == id) { ERR_PRINT("FlecsScriptSystem::set_system_dependency: self"); return; }
	depends_on_system_id = p_system_id;
}

FlecsScriptSystem::FlecsScriptSystem(const FlecsScriptSystem &other) {
	callback = other.callback;
	required_components = other.required_components;
	world_id = other.world_id;
	world = other.world;
	dispatch_mode = other.dispatch_mode;
	batch_flush_chunk_size = other.batch_flush_chunk_size;
	min_flush_interval_usec = other.min_flush_interval_usec;
	change_only = other.change_only;
	observe_add_and_set = other.observe_add_and_set;
	observe_remove = other.observe_remove;
	auto_reset_per_frame = other.auto_reset_per_frame;
	is_paused = other.is_paused;
	multi_threaded = other.multi_threaded;
	use_deferred_calls = other.use_deferred_calls;
	instrumentation_enabled = other.instrumentation_enabled;
	detailed_timing_enabled = other.detailed_timing_enabled;
	max_sample_count = other.max_sample_count;
	depends_on_system_id = other.depends_on_system_id;
	system_name = other.system_name;

	batch_accumulator.clear();
	batch_dirty = false;
	last_flush_time_usec = 0;
	reset_instrumentation();
	build_system();
}
FlecsScriptSystem &FlecsScriptSystem::operator=(const FlecsScriptSystem &other) {
	if (this != &other) {
		callback = other.callback;
		required_components = other.required_components;
		world_id = other.world_id;
		world = other.world;
		dispatch_mode = other.dispatch_mode;
		batch_flush_chunk_size = other.batch_flush_chunk_size;
		min_flush_interval_usec = other.min_flush_interval_usec;
		change_only = other.change_only;
		observe_add_and_set = other.observe_add_and_set;
		observe_remove = other.observe_remove;
		auto_reset_per_frame = other.auto_reset_per_frame;
		is_paused = other.is_paused;
		multi_threaded = other.multi_threaded;
		use_deferred_calls = other.use_deferred_calls;
		instrumentation_enabled = other.instrumentation_enabled;
		detailed_timing_enabled = other.detailed_timing_enabled;
		max_sample_count = other.max_sample_count;
		depends_on_system_id = other.depends_on_system_id;
		system_name = other.system_name;

		batch_accumulator.clear();
		batch_dirty = false;
		last_flush_time_usec = 0;
		reset_instrumentation();
		build_system();
	}
	return *this;
}
FlecsScriptSystem::~FlecsScriptSystem() {
	if (script_system.is_alive()) { script_system.destruct(); }
	if (change_observer.is_alive()) { change_observer.destruct(); }
	if (change_observer_add.is_alive()) { change_observer_add.destruct(); }
	if (change_observer_remove.is_alive()) { change_observer_remove.destruct(); }
	if (reset_system.is_alive()) { reset_system.destruct(); }
	if (batch_flush_system.is_alive()) { batch_flush_system.destruct(); }
}

double FlecsScriptSystem::get_frame_dispatch_median_usec() const {
	if (!detailed_timing_enabled || frame_dispatch_samples.is_empty()) { return 0.0; }
	// Copy then nth_element for median to avoid sorting full vector
	Vector<uint64_t> tmp = frame_dispatch_samples; // copy
	int n = tmp.size();
	int mid = n / 2;
	// Simple insertion sort for small n fallback; nth_element not available, so sort.
	tmp.sort();
	if (n % 2 == 1) { return (double)tmp[mid]; }
	return (double)(tmp[mid - 1] + tmp[mid]) / 2.0;
}

double FlecsScriptSystem::get_frame_dispatch_percentile_usec(double p) const {
	if (!detailed_timing_enabled || frame_dispatch_samples.is_empty()) { return 0.0; }
	if (p <= 0.0) { return (double)frame_dispatch_min_usec; }
	if (p >= 100.0) { return (double)frame_dispatch_max_usec; }
	Vector<uint64_t> tmp = frame_dispatch_samples;
	tmp.sort();
	double rank = (p / 100.0) * (double)(tmp.size() - 1);
	int low = (int)rank;
	int high = MIN(low + 1, tmp.size() - 1);
	double frac = rank - (double)low;
	return (double)tmp[low] + (double)(tmp[high] - tmp[low]) * frac;
}

double FlecsScriptSystem::get_frame_dispatch_stddev_usec() const {
	if (!detailed_timing_enabled || frame_dispatch_samples.size() < 2) { return 0.0; }
	// Compute mean
	long double sum = 0.0L;
	for (int i = 0; i < frame_dispatch_samples.size(); ++i) { sum += (long double)frame_dispatch_samples[i]; }
	long double mean = sum / (long double)frame_dispatch_samples.size();
	long double var_acc = 0.0L;
	for (int i = 0; i < frame_dispatch_samples.size(); ++i) {
		long double diff = (long double)frame_dispatch_samples[i] - mean;
		var_acc += diff * diff;
	}
	long double variance = var_acc / (long double)(frame_dispatch_samples.size() - 1); // sample variance
	double stddev = Math::sqrt((double)variance);
	return stddev;
}
