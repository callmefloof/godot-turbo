/**************************************************************************/
/*  flecs_profiler.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

#include "flecs_profiler.h"
#include "flecs_editor_plugin.h"
#include "instance_manager.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "core/string/print_string.h"
#include "editor/debugger/editor_debugger_plugin.h"

#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/box_container.h"
#include "scene/gui/tree.h"
#include "scene/gui/split_container.h"
#include "scene/gui/check_box.h"
#include "scene/gui/option_button.h"
#include "scene/gui/separator.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/texture_rect.h"
#include "scene/main/timer.h"
#include "core/string/print_string.h"
#include "core/object/callable_method_pointer.h"
#include "core/io/image.h"
#include "scene/resources/image_texture.h"
#include "core/input/input_event.h"

void FlecsProfiler::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_refresh_world_list_deferred"), &FlecsProfiler::_refresh_world_list_deferred);
}

void FlecsProfiler::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Initialize instance manager if needed
			InstanceManager::get_singleton()->initialize();
			
			if (!frame_delay) {
				_build_profiler_ui();
			}
			// Refresh world list on enter
			_refresh_world_list();
			
			if (frame_delay) {
				frame_delay->connect("timeout", callable_mp(this, &FlecsProfiler::_collect_frame_metrics));
				// Only start if already profiling (don't auto-start)
				if (is_profiling) {
					frame_delay->start();
				}
			}
			if (plot_delay) {
				plot_delay->connect("timeout", callable_mp(this, &FlecsProfiler::_update_plot));
				// Only start if already profiling
				if (is_profiling) {
					plot_delay->start();
				}
			}
			// World refresh timer always runs to keep the world list updated
			if (world_refresh_timer) {
				world_refresh_timer->connect("timeout", callable_mp(this, &FlecsProfiler::_on_world_refresh_timer));
				world_refresh_timer->start();
			}
		} break;
		case NOTIFICATION_EXIT_TREE: {
			if (frame_delay && frame_delay->is_connected("timeout", callable_mp(this, &FlecsProfiler::_collect_frame_metrics))) {
				frame_delay->disconnect("timeout", callable_mp(this, &FlecsProfiler::_collect_frame_metrics));
				frame_delay->stop();
			}
			if (plot_delay && plot_delay->is_connected("timeout", callable_mp(this, &FlecsProfiler::_update_plot))) {
				plot_delay->disconnect("timeout", callable_mp(this, &FlecsProfiler::_update_plot));
				plot_delay->stop();
			}
			if (world_refresh_timer && world_refresh_timer->is_connected("timeout", callable_mp(this, &FlecsProfiler::_on_world_refresh_timer))) {
				world_refresh_timer->disconnect("timeout", callable_mp(this, &FlecsProfiler::_on_world_refresh_timer));
				world_refresh_timer->stop();
			}
		} break;
	}
}

FlecsProfiler::FlecsProfiler() {
}

FlecsProfiler::~FlecsProfiler() {
}

void FlecsProfiler::_build_profiler_ui() {
	set_anchors_preset(Control::PRESET_FULL_RECT);

	HBoxContainer *toolbar = memnew(HBoxContainer);
	toolbar->set_custom_minimum_size(Vector2(0, 36));
	add_child(toolbar);

	// World selector
	Label *world_label = memnew(Label);
	world_label->set_text("World:");
	toolbar->add_child(world_label);

	world_selector = memnew(OptionButton);
	world_selector->set_custom_minimum_size(Vector2(150, 0));
	world_selector->connect("item_selected", callable_mp(this, &FlecsProfiler::_on_world_selected));
	toolbar->add_child(world_selector);

	toolbar->add_child(memnew(VSeparator));

	activate_btn = memnew(Button);
	activate_btn->set_text("Start Profiling");
	activate_btn->set_toggle_mode(true);
	activate_btn->set_custom_minimum_size(Vector2(120, 0));
	activate_btn->connect("pressed", callable_mp(this, &FlecsProfiler::_on_activate_pressed));
	toolbar->add_child(activate_btn);

	clear_btn = memnew(Button);
	clear_btn->set_text("Clear");
	clear_btn->set_custom_minimum_size(Vector2(80, 0));
	clear_btn->connect("pressed", callable_mp(this, &FlecsProfiler::_on_clear_pressed));
	toolbar->add_child(clear_btn);

	pause_systems_btn = memnew(Button);
	pause_systems_btn->set_text("Pause All");
	pause_systems_btn->set_custom_minimum_size(Vector2(80, 0));
	pause_systems_btn->connect("pressed", callable_mp(this, &FlecsProfiler::_on_pause_systems_pressed));
	toolbar->add_child(pause_systems_btn);

	resume_systems_btn = memnew(Button);
	resume_systems_btn->set_text("Resume All");
	resume_systems_btn->set_custom_minimum_size(Vector2(80, 0));
	resume_systems_btn->connect("pressed", callable_mp(this, &FlecsProfiler::_on_resume_systems_pressed));
	toolbar->add_child(resume_systems_btn);

	toolbar->add_child(memnew(VSeparator));

	detailed_timing_checkbox = memnew(CheckBox);
	detailed_timing_checkbox->set_text("Detailed Timing");
	detailed_timing_checkbox->set_pressed(false);
	detailed_timing_checkbox->connect("toggled", callable_mp(this, &FlecsProfiler::_on_detailed_timing_toggled));
	toolbar->add_child(detailed_timing_checkbox);

	toolbar->add_child(memnew(VSeparator));

	Label *mode_label = memnew(Label);
	mode_label->set_text("Display:");
	toolbar->add_child(mode_label);

	display_mode_dropdown = memnew(OptionButton);
	display_mode_dropdown->add_item("Frame Time");
	display_mode_dropdown->add_item("Average Time");
	display_mode_dropdown->add_item("Frame Percent");
	display_mode_dropdown->set_custom_minimum_size(Vector2(120, 0));
	display_mode_dropdown->connect("item_selected", callable_mp(this, &FlecsProfiler::_on_display_mode_changed));
	toolbar->add_child(display_mode_dropdown);

	Label *cursor_label = memnew(Label);
	cursor_label->set_text("Frame:");
	toolbar->add_child(cursor_label);

	cursor_metric_edit = memnew(SpinBox);
	cursor_metric_edit->set_min(0);
	cursor_metric_edit->set_max(1000);
	cursor_metric_edit->set_value(0);
	cursor_metric_edit->set_custom_minimum_size(Vector2(80, 0));
	cursor_metric_edit->connect("value_changed", callable_mp(this, &FlecsProfiler::_on_cursor_metric_changed));
	toolbar->add_child(cursor_metric_edit);

	toolbar->add_spacer(false);

	h_split = memnew(HSplitContainer);
	h_split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	h_split->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(h_split);

	VBoxContainer *left_panel = memnew(VBoxContainer);
	left_panel->set_custom_minimum_size(Vector2(400, 0));
	h_split->add_child(left_panel);

	Label *graph_label = memnew(Label);
	graph_label->set_text("Frame Timing");
	graph_label->add_theme_font_size_override("font_size", 12);
	left_panel->add_child(graph_label);

	graph = memnew(TextureRect);
	graph->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	graph->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	graph->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
	graph->set_mouse_filter(Control::MOUSE_FILTER_PASS);
	graph->connect("draw", callable_mp(this, &FlecsProfiler::_graph_tex_draw));
	graph->connect("gui_input", callable_mp(this, &FlecsProfiler::_graph_tex_input));
	graph->connect("mouse_exited", callable_mp(this, &FlecsProfiler::_graph_tex_mouse_exit));
	left_panel->add_child(graph);

	info_label = memnew(Label);
	info_label->set_text("No profiling data");
	info_label->set_custom_minimum_size(Vector2(0, 30));
	left_panel->add_child(info_label);

	VBoxContainer *right_panel = memnew(VBoxContainer);
	right_panel->set_custom_minimum_size(Vector2(400, 0));
	h_split->add_child(right_panel);

	Label *metrics_label = memnew(Label);
	metrics_label->set_text("System Metrics");
	metrics_label->add_theme_font_size_override("font_size", 12);
	right_panel->add_child(metrics_label);

	metrics_tree = memnew(Tree);
	metrics_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	metrics_tree->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	metrics_tree->set_columns(6);
	metrics_tree->set_column_titles_visible(true);
	metrics_tree->set_column_title(0, "System/Query");
	metrics_tree->set_column_title(1, "Time (µs)");
	metrics_tree->set_column_title(2, "Calls");
	metrics_tree->set_column_title(3, "Entities");
	metrics_tree->set_column_title(4, "Min (µs)");
	metrics_tree->set_column_title(5, "Max (µs)");
	metrics_tree->set_column_expand(0, true);
	for (int i = 1; i < 6; i++) {
		metrics_tree->set_column_expand(i, false);
	}
	right_panel->add_child(metrics_tree);

	h_split->set_split_offset(400);

	frame_delay = memnew(Timer);
	add_child(frame_delay);
	frame_delay->set_wait_time(0.1);

	plot_delay = memnew(Timer);
	add_child(plot_delay);
	plot_delay->set_wait_time(0.05);

	// World refresh timer - runs independently to keep world list updated
	world_refresh_timer = memnew(Timer);
	add_child(world_refresh_timer);
	world_refresh_timer->set_wait_time(2.0); // Refresh every 2 seconds
}

void FlecsProfiler::set_flecs_server(FlecsServer *p_server) {
	flecs_server = p_server;
}

void FlecsProfiler::set_selected_world(RID p_world) {
	selected_world = p_world;
}

void FlecsProfiler::enable_profiling(bool p_enable) {
	is_profiling = p_enable;
	_update_button_text();

	if (is_profiling) {
		if (frame_delay) {
			frame_delay->start();
		}
		if (plot_delay) {
			plot_delay->start();
		}
	} else {
		if (frame_delay) {
			frame_delay->stop();
		}
		if (plot_delay) {
			plot_delay->stop();
		}
	}
}

void FlecsProfiler::_on_activate_pressed() {
	enable_profiling(!is_profiling);
}

void FlecsProfiler::_on_clear_pressed() {
	clear_metrics();
}

void FlecsProfiler::_on_pause_systems_pressed() {
	if (flecs_server && selected_world.is_valid()) {
		flecs_server->pause_all_systems(selected_world);
	}
}

void FlecsProfiler::_on_resume_systems_pressed() {
	if (flecs_server && selected_world.is_valid()) {
		flecs_server->resume_all_systems(selected_world);
	}
}

void FlecsProfiler::_on_detailed_timing_toggled(bool p_enabled) {
	// Placeholder for detailed timing collection
}

void FlecsProfiler::_on_display_mode_changed(int p_mode) {
	if (p_mode >= 0 && p_mode < 3) {
		current_display_mode = (DisplayMode)p_mode;
		_update_plot();
	}
}

void FlecsProfiler::_on_cursor_metric_changed(double p_value) {
	int index = int(p_value);
	if (index >= 0 && index < frame_metrics.size()) {
		last_metric = index;
		seeking = true;
		_update_metrics_tree();
		_update_plot();
	}
}

void FlecsProfiler::_refresh_world_list() {
	if (!world_selector) {
		return;
	}

	// Get current selection to restore it
	RID previously_selected = selected_world;
	int restore_index = -1;

	// Clear and rebuild
	world_selector->clear();
	available_worlds.clear();

	// Check if we're in remote mode - if so, request worlds independently
	FlecsWorldEditorPlugin *world_plugin = FlecsWorldEditorPlugin::get_singleton();
	bool is_remote = world_plugin && world_plugin->is_remote_mode();

	// Check for multi-instance conflicts
	InstanceManager *instance_mgr = InstanceManager::get_singleton();
	if (instance_mgr->has_other_instance() && is_remote) {
		// In remote mode with multiple instances, only primary should request
		if (!instance_mgr->is_primary_instance()) {
			// Try to acquire the profiler resource
			if (!instance_mgr->try_acquire_resource("profiler_remote")) {
				// Another instance has the profiler, use local mode only
				is_remote = false;
			}
		}
	}

	// Debug: Log refresh attempt periodically
	static int refresh_count = 0;
	refresh_count++;
	if (refresh_count % 30 == 1) { // Log every 30th refresh (~1 minute at 2s interval)
		print_line(vformat("FlecsProfiler: _refresh_world_list (flecs_server=%s, is_remote=%s, instance=%s)", 
			flecs_server ? "valid" : "null", 
			is_remote ? "true" : "false",
			instance_mgr->is_primary_instance() ? "primary" : "secondary"));
	}

	TypedArray<RID> worlds;

	if (is_remote) {
		// In remote mode, request worlds directly if we haven't yet or periodically
		if (!has_requested_worlds || frame_counter % 100 == 0) {
			_request_remote_worlds();
		}
		// Try to get worlds from FlecsWorldEditorPlugin (may have been populated by our request or Entity Inspector)
		if (world_plugin) {
			worlds = world_plugin->get_available_worlds();
		}
	} else {
		// Local mode - get worlds directly from FlecsServer
		if (flecs_server) {
			worlds = flecs_server->get_world_list();
		}
	}

	// Only log when we find worlds (to reduce spam)
	// Removed verbose logging - worlds found

	if (worlds.is_empty()) {
		if (refresh_count % 30 == 1) {
			print_line("FlecsProfiler: No worlds available");
		}
		world_selector->add_item("No worlds available");
		world_selector->set_disabled(true);
		selected_world = RID();
		return;
	}

	if (refresh_count % 30 == 1) {
		print_line(vformat("FlecsProfiler: Found %d worlds", worlds.size()));
	}

	world_selector->set_disabled(false);

	for (int i = 0; i < worlds.size(); i++) {
		RID world_rid = worlds[i];
		available_worlds.push_back(world_rid);

		// Format world name using RID (no get_world_name API available)
		String world_name = "World [" + String::num_int64(world_rid.get_id(), 16).to_upper() + "]";
		world_selector->add_item(world_name);

		if (world_rid == previously_selected) {
			restore_index = i;
		}
	}

	// Restore selection or select first world
	if (restore_index >= 0) {
		world_selector->select(restore_index);
		selected_world = previously_selected;
	} else if (!available_worlds.is_empty()) {
		world_selector->select(0);
		selected_world = available_worlds[0];
	}
}

void FlecsProfiler::_request_remote_worlds() {
	if (waiting_for_remote_worlds) {
		return; // Already waiting for a response
	}

	FlecsWorldEditorPlugin *world_plugin = FlecsWorldEditorPlugin::get_singleton();
	if (!world_plugin) {
		return;
	}

	Ref<EditorDebuggerSession> session = world_plugin->get_active_session();
	if (!session.is_valid() || !session->is_active()) {
		// Session changed or became invalid - reset state
		if (has_requested_worlds) {
			has_requested_worlds = false;
			waiting_for_remote_worlds = false;
		}
		return;
	}

	Array args;
	session->send_message("flecs:request_worlds", args);
	waiting_for_remote_worlds = true;
	has_requested_worlds = true;
}

void FlecsProfiler::_on_world_refresh_timer() {
	// Periodically refresh the world list even when not profiling
	// This ensures the world dropdown is populated when user wants to start profiling
	_refresh_world_list();
}

void FlecsProfiler::handle_remote_worlds(const Array &p_data) {
	waiting_for_remote_worlds = false;
	
	if (p_data.is_empty()) {
		return;
	}
	
	Dictionary response = p_data[0];
	Array worlds_array = response.get("worlds", Array());
	
	// The worlds will be added to world_dirty in the world plugin's handler
	// which will be called before or alongside this handler.
	// Just trigger a refresh of our world list to pick up any new worlds.
	// Use call_deferred to avoid potential re-entrancy issues
	call_deferred("_refresh_world_list_deferred");
}

void FlecsProfiler::_refresh_world_list_deferred() {
	_refresh_world_list();
}

void FlecsProfiler::_on_world_selected(int p_index) {
	if (p_index >= 0 && p_index < available_worlds.size()) {
		selected_world = available_worlds[p_index];
		clear_metrics();
	}
}

void FlecsProfiler::_collect_frame_metrics() {
	frame_counter++;

	// Check if we're in remote mode
	FlecsWorldEditorPlugin *world_plugin = FlecsWorldEditorPlugin::get_singleton();
	bool is_remote = world_plugin && world_plugin->is_remote_mode();

	// Handle multi-instance conflicts for remote mode
	InstanceManager *instance_mgr = InstanceManager::get_singleton();
	if (is_remote && instance_mgr->has_other_instance()) {
		// Only primary instance should use remote debugging
		if (!instance_mgr->is_primary_instance() && 
			!instance_mgr->is_resource_available("profiler_remote")) {
			// Fall back to local mode
			is_remote = false;
		}
	}

	// Debug logging every 60 frames (~6 seconds at 10Hz)
	if (frame_counter % 60 == 0) {
		print_line(vformat("FlecsProfiler: _collect_frame_metrics (frame=%d, is_profiling=%s, flecs_server=%s, selected_world=%s, is_remote=%s, instance=%s)",
			frame_counter,
			is_profiling ? "true" : "false",
			flecs_server ? "valid" : "null",
			selected_world.is_valid() ? "valid" : "invalid",
			is_remote ? "true" : "false",
			instance_mgr->is_primary_instance() ? "primary" : "secondary"));
	}

	if (!selected_world.is_valid()) {
		// Try to refresh and select a world if none selected
		// In remote mode, request more frequently initially (every 10 frames = ~1 second)
		// In local mode, every 50 frames (~5 seconds)
		int refresh_interval = is_remote ? 10 : 50;
		if (frame_counter % refresh_interval == 0) {
			_refresh_world_list();
		}
		// Only log once every 30 seconds to avoid spam
		// Log only once per minute (600 frames at 10Hz timer)
		if (frame_counter % 600 == 0) {
			print_line(vformat("FlecsProfiler: Waiting for world (remote_mode=%s)", is_remote ? "true" : "false"));
		}
		return;
	}

	// Refresh world list periodically (every ~30 seconds)
	if (frame_counter % 300 == 0) {
		_refresh_world_list();
	}

	if (!is_profiling) {
		return;
	}

	if (is_remote) {
		// Use remote debugging to get metrics
		_request_remote_metrics();
		return;
	}

	// Local mode - get metrics directly from FlecsServer
	if (!flecs_server) {
		if (frame_counter % 60 == 0) {
			print_line("FlecsProfiler: flecs_server is null, cannot collect metrics");
		}
		return;
	}

	Dictionary metrics = flecs_server->get_system_metrics(selected_world);
	if (frame_counter % 60 == 0) {
		print_line(vformat("FlecsProfiler: Got metrics from FlecsServer, systems count: %d", 
			metrics.has("systems") ? Array(metrics["systems"]).size() : 0));
	}
	_process_metrics_dictionary(metrics);
}

void FlecsProfiler::_request_remote_metrics() {
	if (waiting_for_remote_metrics) {
		return; // Already waiting for a response
	}

	FlecsWorldEditorPlugin *world_plugin = FlecsWorldEditorPlugin::get_singleton();
	if (!world_plugin) {
		return;
	}

	Ref<EditorDebuggerSession> session = world_plugin->get_active_session();
	if (!session.is_valid() || !session->is_active()) {
		return;
	}

	Array args;
	args.push_back(selected_world.get_id());
	session->send_message("flecs:request_profiler_metrics", args);
	waiting_for_remote_metrics = true;
}

void FlecsProfiler::handle_remote_metrics(const Dictionary &p_data) {
	waiting_for_remote_metrics = false;

	if (!is_profiling) {
		return;
	}

	_process_metrics_dictionary(p_data);
}

void FlecsProfiler::_process_metrics_dictionary(const Dictionary &metrics) {
	if (metrics.is_empty()) {
		return;
	}
	
	if (!metrics.has("systems")) {
		return;
	}

	FrameMetric frame;
	frame.frame_number = total_metrics;
	frame.total_frame_time_usec = 0;

	// Process all systems - get_system_metrics returns them in "systems" array
	Array systems = metrics["systems"];
	for (int i = 0; i < systems.size(); i++) {
		Dictionary sys = systems[i];
		SystemMetric metric;
		metric.system_id = sys.get("rid", RID());
		
		// Get system type and append suffix for clarity
		String sys_type = sys.get("type", "unknown");
		String name = sys.get("name", "Unknown");
		if (sys_type == "cpp" || sys_type == "native") {
			name += " [C++]";
		}
		metric.name = name;
		
		metric.total_time_usec = sys.get("time_usec", 0);
		metric.call_count = sys.get("call_count", 0);
		metric.entity_count = sys.get("entity_count", 0);
		metric.min_time_usec = sys.get("min_time_usec", 0);
		metric.max_time_usec = sys.get("max_time_usec", 0);
		metric.on_add = sys.get("onadd_count", 0);
		metric.on_set = sys.get("onset_count", 0);
		metric.on_remove = sys.get("onremove_count", 0);
		metric.is_paused = sys.get("paused", false);
		
		// Optional detailed timing
		if (sys.has("median_usec")) {
			metric.median_usec = sys.get("median_usec", 0.0);
		}
		if (sys.has("stddev_usec")) {
			metric.stddev_usec = sys.get("stddev_usec", 0.0);
		}
		
		frame.system_metrics.push_back(metric);
		frame.total_frame_time_usec += metric.total_time_usec;
	}

	// Use total from server if available
	if (metrics.has("total_time_usec")) {
		frame.total_frame_time_usec = metrics["total_time_usec"];
	}

	add_frame_metric(frame);
	_update_metrics_tree();
}

void FlecsProfiler::_update_metrics_tree() {
	if (!metrics_tree) {
		return;
	}

	metrics_tree->clear();

	if (frame_metrics.is_empty()) {
		InstanceManager *instance_mgr = InstanceManager::get_singleton();
		if (instance_mgr->has_other_instance()) {
			if (instance_mgr->is_primary_instance()) {
				info_label->set_text("No profiling data (primary instance)");
			} else {
				info_label->set_text("No profiling data (secondary instance - remote debugging limited)");
			}
		} else {
			info_label->set_text("No profiling data");
		}
		return;
	}

	int current_frame = _get_cursor_index();
	if (current_frame < 0 || current_frame >= frame_metrics.size()) {
		return;
	}

	const FrameMetric &frame = frame_metrics[current_frame];
	InstanceManager *instance_mgr = InstanceManager::get_singleton();
	String instance_info = "";
	if (instance_mgr->has_other_instance() && !instance_mgr->is_primary_instance()) {
		instance_info = " [secondary]";
	}
	info_label->set_text(vformat("Frame %d - Total: %.2f ms%s", frame.frame_number, frame.total_frame_time_usec / 1000.0, instance_info));

	TreeItem *root = metrics_tree->create_item();

	for (const SystemMetric &sys : frame.system_metrics) {
		TreeItem *item = metrics_tree->create_item(root);
		item->set_text(0, sys.name);
		item->set_text(1, vformat("%.1f", sys.total_time_usec / 1000.0));
		item->set_text(2, itos(sys.call_count));
		item->set_text(3, vformat("%d", sys.entity_count));
		item->set_text(4, vformat("%.1f", sys.min_time_usec / 1000.0));
		item->set_text(5, vformat("%.1f", sys.max_time_usec / 1000.0));
	}

	for (const QueryMetric &qry : frame.query_metrics) {
		TreeItem *item = metrics_tree->create_item(root);
		item->set_text(0, qry.name);
		item->set_text(3, vformat("%d", qry.entity_count));
	}
}

void FlecsProfiler::_update_plot() {
	if (!graph || frame_metrics.is_empty()) {
		return;
	}

	int width = int(graph->get_size().width);
	int height = int(graph->get_size().height);

	if (width <= 0 || height <= 0) {
		return;
	}

	graph_image.resize(width * height * 4);
	uint8_t *image_data = graph_image.ptrw();

	// Clear background to dark gray
	for (int i = 0; i < width * height; i++) {
		image_data[i * 4 + 0] = 30;
		image_data[i * 4 + 1] = 30;
		image_data[i * 4 + 2] = 30;
		image_data[i * 4 + 3] = 255;
	}

	int frame_count = MIN(frame_metrics.size(), width);
	
	// Adaptive scaling: find actual max time in recent frames for better visualization
	float actual_max_time = 0.0f;
	float total_time = 0.0f;
	int sample_count = MIN(frame_count, 100); // Sample last 100 frames for scaling
	int start_sample = MAX(0, frame_count - sample_count);
	
	for (int i = start_sample; i < frame_count; i++) {
		float frame_time = float(frame_metrics[i].total_frame_time_usec);
		actual_max_time = MAX(actual_max_time, frame_time);
		total_time += frame_time;
	}
	
	// Use adaptive max: either 1.5x the actual max, or 2x the average, whichever is smaller
	// This reduces wild swings while still showing peaks
	float avg_time = sample_count > 0 ? total_time / sample_count : 0.0f;
	float adaptive_limit = MIN(actual_max_time * 1.5f, avg_time * 3.0f);
	
	// Ensure minimum scale and use user-set limit as ceiling
	float max_time = CLAMP(adaptive_limit, 1000.0f, graph_limit);
	
	// If actual max is very small, use a smaller scale
	if (actual_max_time > 0 && actual_max_time < 1000.0f) {
		max_time = 2000.0f; // 2ms minimum scale for small values
	}

	for (int i = 0; i < frame_count; i++) {
		int x = (width * i) / frame_count;
		int bar_width = MAX(1, width / frame_count); // Ensure bars have at least 1px width
		const FrameMetric &frame = frame_metrics[i];
		float time_percent = CLAMP(float(frame.total_frame_time_usec) / max_time, 0.0f, 1.0f);
		int bar_height = MAX(1, int(height * time_percent)); // Ensure at least 1px height if non-zero

		// Color based on performance: green = good, yellow = warning, red = bad
		Color bar_color;
		if (i == last_metric) {
			bar_color = Color(1, 1, 0); // Yellow for selected
		} else if (time_percent < 0.5f) {
			bar_color = Color(0.2, 0.8, 0.2); // Green
		} else if (time_percent < 0.75f) {
			bar_color = Color(0.8, 0.8, 0.2); // Yellow-ish
		} else {
			bar_color = Color(0.8, 0.3, 0.2); // Red-ish
		}
		
		// Draw bar with proper width
		for (int bx = 0; bx < bar_width && (x + bx) < width; bx++) {
			for (int y = height - bar_height; y < height; y++) {
				if (y >= 0 && y < height) {
					uint8_t *pixel = image_data + (y * width + (x + bx)) * 4;
					pixel[0] = uint8_t(bar_color.r * 255);
					pixel[1] = uint8_t(bar_color.g * 255);
					pixel[2] = uint8_t(bar_color.b * 255);
					pixel[3] = 255;
				}
			}
		}
	}

	if (!graph_texture.is_valid()) {
		Ref<Image> img = Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, graph_image);
		graph_texture = ImageTexture::create_from_image(img);
	} else {
		Ref<Image> img = Image::create_from_data(width, height, false, Image::FORMAT_RGBA8, graph_image);
		graph_texture->set_image(img);
	}

	if (graph && graph_texture.is_valid()) {
		graph->set_texture(graph_texture);
	}
}

void FlecsProfiler::_graph_tex_draw() {
	_update_plot();
}

void FlecsProfiler::_graph_tex_input(const Ref<InputEvent> &p_ev) {
	const Ref<InputEventMouseButton> mb = p_ev;
	if (!mb.is_valid() || !mb->is_pressed()) {
		return;
	}

	if (mb->get_button_index() == MouseButton::LEFT) {
		int width = int(graph->get_size().width);
		float click_percent = CLAMP(mb->get_position().x / width, 0.0f, 1.0f);
		int frame_index = int(frame_metrics.size() * click_percent);
		frame_index = CLAMP(frame_index, 0, int(frame_metrics.size()) - 1);
		cursor_metric_edit->set_value(frame_index);
	}
}

void FlecsProfiler::_graph_tex_mouse_exit() {
	hover_metric = -1;
	_update_plot();
}

void FlecsProfiler::_update_button_text() {
	if (activate_btn) {
		activate_btn->set_text(is_profiling ? "Stop Profiling" : "Start Profiling");
		activate_btn->set_pressed(is_profiling);
	}
}

String FlecsProfiler::_get_time_as_text(uint64_t p_time_usec) {
	if (p_time_usec < 1000) {
		return vformat("%.1f µs", float(p_time_usec));
	} else if (p_time_usec < 1000000) {
		return vformat("%.2f ms", float(p_time_usec) / 1000.0);
	} else {
		return vformat("%.3f s", float(p_time_usec) / 1000000.0);
	}
}

String FlecsProfiler::_get_metric_label(const SystemMetric &p_metric, float p_time) {
	return vformat("%s: %.2f ms", p_metric.name, p_time);
}

Color FlecsProfiler::_get_color_from_system_id(const RID &p_system_id) const {
	// Simple deterministic color generation based on RID
	uint64_t hash = p_system_id.get_local_index();
	float r = (float)((hash >> 16) & 0xFF) / 255.0f;
	float g = (float)((hash >> 8) & 0xFF) / 255.0f;
	float b = (float)(hash & 0xFF) / 255.0f;
	return Color(r, g, b, 1.0f);
}

int FlecsProfiler::_get_cursor_index() const {
	if (cursor_metric_edit) {
		return int(cursor_metric_edit->get_value());
	}
	return last_metric;
}

void FlecsProfiler::clear_metrics() {
	frame_metrics.clear();
	last_metric = -1;
	hover_metric = -1;
	total_metrics = 0;
	if (cursor_metric_edit) {
		cursor_metric_edit->set_max(1000);
		cursor_metric_edit->set_value(0);
	}
	_update_metrics_tree();
	_update_plot();
}

void FlecsProfiler::add_frame_metric(const FrameMetric &p_metric) {
	frame_metrics.push_back(p_metric);
	total_metrics++;

	// Keep only last 1000 frames
	if (frame_metrics.size() > 1000) {
		frame_metrics.remove_at(0);
	}

	if (cursor_metric_edit) {
		cursor_metric_edit->set_max(frame_metrics.size() - 1);
	}
}

void FlecsProfiler::disable_seeking() {
	seeking = false;
}

Vector<Vector<String>> FlecsProfiler::_get_metrics_as_csv() const {
	Vector<Vector<String>> csv_data;

	// Header row
	Vector<String> header;
	header.push_back("Frame");
	header.push_back("Total Time (µs)");
	header.push_back("System Name");
	header.push_back("System Time (µs)");
	header.push_back("Call Count");
	header.push_back("Entity Count");
	csv_data.push_back(header);

	// Data rows
	for (const FrameMetric &frame : frame_metrics) {
		Vector<String> frame_row;
		frame_row.push_back(String::num_uint64(frame.frame_number));
		frame_row.push_back(String::num_uint64(frame.total_frame_time_usec));

		if (frame.system_metrics.is_empty()) {
			frame_row.push_back("");
			frame_row.push_back("");
			frame_row.push_back("");
			frame_row.push_back("");
			csv_data.push_back(frame_row);
		} else {
			for (size_t i = 0; i < frame.system_metrics.size(); i++) {
				const SystemMetric &sys = frame.system_metrics[i];
				Vector<String> sys_row = frame_row;
				sys_row.push_back(sys.name);
				sys_row.push_back(String::num_uint64(sys.total_time_usec));
				sys_row.push_back(String::num_uint64(sys.call_count));
				sys_row.push_back(String::num_int64(sys.entity_count));
				csv_data.push_back(sys_row);
			}
		}
	}

	return csv_data;
}

Vector<Vector<String>> FlecsProfiler::get_data_as_csv() const {
	return _get_metrics_as_csv();
}