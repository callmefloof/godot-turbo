/**************************************************************************/
/*  flecs_profiler.h                                                      */
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

#ifndef FLECS_PROFILER_H
#define FLECS_PROFILER_H

#include "scene/gui/box_container.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "scene/resources/image_texture.h"
#include "core/variant/typed_array.h"

// Forward declarations to avoid static initialization issues
class Button;
class CheckBox;
class Label;
class OptionButton;
class SpinBox;
class HSplitContainer;
class TextureRect;
class Tree;
class Timer;
class InputEvent;
class FlecsServer;
class ItemList;
class EditorDebuggerSession;
class FlecsWorldEditorPlugin;

/**
 * @class FlecsProfiler
 * @brief Profiler for Flecs ECS systems and queries
 * 
 * Displays per-frame and aggregated profiling metrics for systems and queries
 * in a selected Flecs world. Features include:
 * - Real-time frame timing visualization
 * - Per-system/query metrics (min, max, average, median, stddev)
 * - Call count and entity processing statistics
 * - System pause/resume controls
 * - CSV export of profiling data
 */
class FlecsProfiler : public VBoxContainer {
	GDCLASS(FlecsProfiler, VBoxContainer);

protected:
	static void _bind_methods();

public:
	struct SystemMetric {
		RID system_id;
		String name;
		uint64_t total_time_usec = 0;
		uint64_t min_time_usec = 0;
		uint64_t max_time_usec = 0;
		uint64_t call_count = 0;
		int entity_count = 0;
		uint64_t on_add = 0;
		uint64_t on_set = 0;
		uint64_t on_remove = 0;
		double median_usec = 0.0;
		double stddev_usec = 0.0;
		bool is_paused = false;
	};

	struct QueryMetric {
		RID query_id;
		String name;
		int entity_count = 0;
	};

	struct FrameMetric {
		uint64_t frame_number = 0;
		Vector<SystemMetric> system_metrics;
		Vector<QueryMetric> query_metrics;
		uint64_t total_frame_time_usec = 0;
	};

	enum DisplayMode {
		DISPLAY_FRAME_TIME,
		DISPLAY_AVERAGE_TIME,
		DISPLAY_FRAME_PERCENT,
	};

	enum SortColumn {
		SORT_NAME,
		SORT_TIME,
		SORT_CALLS,
		SORT_ENTITIES,
	};

private:
	FlecsServer *flecs_server = nullptr;
	RID selected_world;
	bool is_profiling = false;
	bool seeking = false;

	// World selector
	OptionButton *world_selector = nullptr;
	Vector<RID> available_worlds;
	uint64_t frame_counter = 0;
	bool waiting_for_remote_metrics = false;
	bool waiting_for_remote_worlds = false;
	bool has_requested_worlds = false;
	Timer *world_refresh_timer = nullptr;

	Button *activate_btn = nullptr;
	Button *clear_btn = nullptr;
	Button *pause_systems_btn = nullptr;
	Button *resume_systems_btn = nullptr;
	CheckBox *detailed_timing_checkbox = nullptr;
	OptionButton *display_mode_dropdown = nullptr;
	SpinBox *cursor_metric_edit = nullptr;

	TextureRect *graph = nullptr;
	Ref<ImageTexture> graph_texture;
	Vector<uint8_t> graph_image;

	Tree *metrics_tree = nullptr;
	Label *info_label = nullptr;
	HSplitContainer *h_split = nullptr;

	Vector<FrameMetric> frame_metrics;
	int last_metric = -1;
	int hover_metric = -1;
	int total_metrics = 0;

	DisplayMode current_display_mode = DISPLAY_FRAME_TIME;
	SortColumn current_sort_column = SORT_TIME;
	float graph_height = 1.0f;
	float graph_limit = 50000.0f;

	Timer *frame_delay = nullptr;
	Timer *plot_delay = nullptr;

	Dictionary system_name_cache;
	Dictionary query_name_cache;

	void _notification(int p_what);

	void _build_profiler_ui();

	void _on_activate_pressed();
	void _on_clear_pressed();
	void _on_pause_systems_pressed();
	void _on_resume_systems_pressed();
	void _on_detailed_timing_toggled(bool p_enabled);
	void _on_display_mode_changed(int p_mode);
	void _on_cursor_metric_changed(double p_value);
	void _on_world_selected(int p_index);

	void _refresh_world_list();
	void _refresh_world_list_deferred();
	void _request_remote_worlds();
	void _on_world_refresh_timer();
	void _collect_frame_metrics();
	void _request_remote_metrics();
	void _process_metrics_dictionary(const Dictionary &metrics);
	void _update_metrics_tree();
	void _update_plot();

	void _graph_tex_draw();
	void _graph_tex_input(const Ref<InputEvent> &p_ev);
	void _graph_tex_mouse_exit();

	void _update_button_text();
	String _get_time_as_text(uint64_t p_time_usec);
	String _get_metric_label(const SystemMetric &p_metric, float p_time);
	Color _get_color_from_system_id(const RID &p_system_id) const;
	int _get_cursor_index() const;

	Vector<Vector<String>> _get_metrics_as_csv() const;

public:
	FlecsProfiler();
	~FlecsProfiler();

	void set_flecs_server(FlecsServer *p_server);
	void set_selected_world(RID p_world);
	void enable_profiling(bool p_enable);
	
	// Called by FlecsWorldEditorPlugin when remote metrics arrive
	void handle_remote_metrics(const Dictionary &p_data);
	// Called by FlecsWorldEditorPlugin when remote worlds arrive
	void handle_remote_worlds(const Array &p_data);
	bool is_profiling_active() const { return is_profiling; }
	void clear_metrics();
	void add_frame_metric(const FrameMetric &p_metric);

	bool is_seeking() const { return seeking; }
	void disable_seeking();

	Vector<Vector<String>> get_data_as_csv() const;
};

#endif // FLECS_PROFILER_H