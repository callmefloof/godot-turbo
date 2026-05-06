/**************************************************************************/
/*  turbo_debug_panel.h                                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#ifndef TURBO_DEBUG_PANEL_H
#define TURBO_DEBUG_PANEL_H

#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/label.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/tree.h"
#include "scene/main/canvas_layer.h"

class FlecsServer;
class TurboDebugDraw;
class TurboEntityPicker;
class TurboDebugSystem;
class Viewport;
class Camera3D;

// ============================================================================
// Debug Panel Configuration
// ============================================================================

struct TurboDebugPanelConfig {
	// Visibility
	bool visible = false;
	bool start_minimized = true;
	bool show_fps = true;
	bool show_entity_count = true;
	bool show_memory_stats = false;

	// Position/Size
	int anchor = 1; // 0=TopLeft, 1=TopRight, 2=BottomLeft, 3=BottomRight
	Vector2 offset = Vector2(10, 10);
	Vector2 min_size = Vector2(350, 400);
	Vector2 max_size = Vector2(600, 800);
	float opacity = 0.95f;

	// Auto-refresh
	float refresh_interval = 0.1f; // Seconds between data refreshes
	bool auto_refresh = true;

	// Entity inspector
	bool show_all_components = true;
	bool show_component_types = true;
	int max_component_depth = 4;
	int max_array_items = 50;

	// Hotkey
	String toggle_hotkey = "F3"; // Key to toggle panel visibility

	Dictionary to_dictionary() const;
	static TurboDebugPanelConfig from_dictionary(const Dictionary &p_dict);
};

// ============================================================================
// Performance Stats Data
// ============================================================================

struct TurboPerformanceStats {
	// Frame timing
	double fps = 0.0;
	double frame_time_ms = 0.0;
	double physics_time_ms = 0.0;
	double render_time_ms = 0.0;

	// ECS stats
	int total_entities = 0;
	int active_entities = 0;
	int dormant_entities = 0;
	int worlds_count = 0;

	// Memory
	uint64_t memory_static = 0;
	uint64_t memory_dynamic = 0;

	// Debug system stats
	int gizmos_drawn = 0;
	int lines_drawn = 0;
	int picks_this_frame = 0;

	Dictionary to_dictionary() const {
		Dictionary d;
		d["fps"] = fps;
		d["frame_time_ms"] = frame_time_ms;
		d["physics_time_ms"] = physics_time_ms;
		d["render_time_ms"] = render_time_ms;
		d["total_entities"] = total_entities;
		d["active_entities"] = active_entities;
		d["dormant_entities"] = dormant_entities;
		d["worlds_count"] = worlds_count;
		d["memory_static"] = memory_static;
		d["memory_dynamic"] = memory_dynamic;
		d["gizmos_drawn"] = gizmos_drawn;
		d["lines_drawn"] = lines_drawn;
		d["picks_this_frame"] = picks_this_frame;
		return d;
	}
};

// ============================================================================
// Visualization Toggle
// ============================================================================

struct TurboVisualizationToggle {
	String name;
	String tooltip;
	bool enabled;
	String category;
	Callable on_changed;

	TurboVisualizationToggle() :
			enabled(false) {}
	TurboVisualizationToggle(const String &p_name, const String &p_tooltip, bool p_enabled, const String &p_category = "General") :
			name(p_name), tooltip(p_tooltip), enabled(p_enabled), category(p_category) {}
};

// ============================================================================
// TurboDebugPanel - In-game debug HUD
// ============================================================================
// Provides a comprehensive debug interface for runtime inspection:
// - Selected entity component viewer
// - Performance statistics
// - Visualization toggles (gizmos, bounds, axiom violations, etc.)
// - Debug commands
// - Log viewer
//
// This panel is designed to work at runtime, not just in the editor.
// It uses a CanvasLayer to overlay on top of the game.
// ============================================================================

class TurboDebugPanel : public CanvasLayer {
	GDCLASS(TurboDebugPanel, CanvasLayer);

private:
	// Configuration
	TurboDebugPanelConfig config;

	// External references
	FlecsServer *flecs_server = nullptr;
	Ref<TurboDebugDraw> debug_draw;
	Ref<TurboEntityPicker> entity_picker;
	TurboDebugSystem *debug_system = nullptr;

	// Currently selected entity
	RID selected_entity;
	RID selected_world;
	String selected_entity_name;

	// Performance stats
	TurboPerformanceStats stats;

	// Visualization toggles
	LocalVector<TurboVisualizationToggle> visualization_toggles;
	HashMap<String, int> toggle_name_to_index;

	// Log buffer
	LocalVector<String> log_buffer;
	static constexpr int MAX_LOG_LINES = 500;

	// Timing
	double last_refresh_time = 0.0;
	double current_time = 0.0;
	uint64_t frame_count = 0;

	// UI State
	bool is_minimized = false;
	bool is_dragging = false;
	Vector2 drag_offset;

	// ========================================================================
	// UI Components
	// ========================================================================

	// Root container
	PanelContainer *root_panel = nullptr;
	VBoxContainer *main_container = nullptr;

	// Header bar
	HBoxContainer *header_bar = nullptr;
	Label *title_label = nullptr;
	Button *minimize_button = nullptr;
	Button *close_button = nullptr;

	// Tab container (for different views)
	TabContainer *tab_container = nullptr;

	// === Entity Tab ===
	VBoxContainer *entity_tab = nullptr;
	Label *entity_header = nullptr;
	Label *entity_name_label = nullptr;
	Label *entity_rid_label = nullptr;
	ScrollContainer *components_scroll = nullptr;
	VBoxContainer *components_container = nullptr;
	Tree *component_tree = nullptr;

	// === Stats Tab ===
	VBoxContainer *stats_tab = nullptr;
	Label *fps_label = nullptr;
	Label *frame_time_label = nullptr;
	Label *entity_count_label = nullptr;
	Label *worlds_count_label = nullptr;
	Label *memory_label = nullptr;
	Label *debug_draw_stats_label = nullptr;

	// === Visualization Tab ===
	VBoxContainer *visualization_tab = nullptr;
	ScrollContainer *toggles_scroll = nullptr;
	VBoxContainer *toggles_container = nullptr;

	// === Log Tab ===
	VBoxContainer *log_tab = nullptr;
	RichTextLabel *log_text = nullptr;
	Button *clear_log_button = nullptr;

	// === Commands Tab ===
	VBoxContainer *commands_tab = nullptr;
	VBoxContainer *commands_container = nullptr;

	// ========================================================================
	// Internal Methods
	// ========================================================================

	// UI Building
	void _build_ui();
	void _build_header();
	void _build_entity_tab();
	void _build_stats_tab();
	void _build_visualization_tab();
	void _build_log_tab();
	void _build_commands_tab();
	void _apply_theme();

	// UI Updates
	void _refresh_entity_display();
	void _refresh_stats_display();
	void _refresh_visualization_toggles();
	void _update_component_tree(const Dictionary &p_entity_data);

	// Tree building helpers
	void _add_component_to_tree(TreeItem *p_parent, const String &p_component_name, const Dictionary &p_data);
	TreeItem *_add_property_item(TreeItem *p_parent, const String &p_key, const Variant &p_value, int p_depth);
	String _format_value(const Variant &p_value) const;
	String _get_type_name(Variant::Type p_type) const;
	Color _get_type_color(Variant::Type p_type) const;

	// Event handlers
	void _on_minimize_pressed();
	void _on_close_pressed();
	void _on_toggle_changed(bool p_enabled, int p_index);
	void _on_clear_log_pressed();
	void _on_header_gui_input(const Ref<InputEvent> &p_event);
	void _on_component_tree_item_selected();

	// Data collection
	void _collect_performance_stats();
	Dictionary _get_entity_data(const RID &p_entity, const RID &p_world) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	// ========================================================================
	// Initialization
	// ========================================================================

	void initialize(TurboDebugSystem *p_debug_system);
	void shutdown();
	bool is_initialized() const { return root_panel != nullptr; }

	void set_flecs_server(FlecsServer *p_server);
	void set_debug_draw(const Ref<TurboDebugDraw> &p_debug_draw);
	void set_entity_picker(const Ref<TurboEntityPicker> &p_picker);

	// ========================================================================
	// Configuration
	// ========================================================================

	void set_config(const TurboDebugPanelConfig &p_config);
	TurboDebugPanelConfig get_config() const { return config; }
	Dictionary get_config_dict() const { return config.to_dictionary(); }
	void set_config_from_dict(const Dictionary &p_dict);

	// ========================================================================
	// Visibility
	// ========================================================================

	void show_panel();
	void hide_panel();
	void toggle_panel();
	bool is_panel_visible() const;

	void set_minimized(bool p_minimized);
	bool is_panel_minimized() const { return is_minimized; }

	void set_anchor(int p_anchor);
	int get_anchor() const { return config.anchor; }

	void set_opacity(float p_opacity);
	float get_opacity() const { return config.opacity; }

	// ========================================================================
	// Entity Selection
	// ========================================================================

	void set_selected_entity(const RID &p_entity, const RID &p_world);
	void clear_selected_entity();
	RID get_selected_entity() const { return selected_entity; }
	RID get_selected_world() const { return selected_world; }

	// ========================================================================
	// Visualization Toggles
	// ========================================================================

	void register_toggle(const String &p_name, const String &p_tooltip, bool p_default, const String &p_category = "General");
	void register_toggle_with_callback(const String &p_name, const String &p_tooltip, bool p_default, const Callable &p_callback, const String &p_category = "General");
	void set_toggle_enabled(const String &p_name, bool p_enabled);
	bool is_toggle_enabled(const String &p_name) const;
	void remove_toggle(const String &p_name);
	void clear_toggles();

	PackedStringArray get_toggle_names() const;
	PackedStringArray get_toggle_categories() const;

	// ========================================================================
	// Logging
	// ========================================================================

	void log_message(const String &p_message);
	void log_warning(const String &p_message);
	void log_error(const String &p_message);
	void log_debug(const String &p_message);
	void clear_log();

	PackedStringArray get_log_buffer() const;

	// ========================================================================
	// Commands
	// ========================================================================

	void register_command(const String &p_name, const String &p_tooltip, const Callable &p_callback);
	void unregister_command(const String &p_name);
	void clear_commands();

	// ========================================================================
	// Performance Stats
	// ========================================================================

	TurboPerformanceStats get_performance_stats() const { return stats; }
	Dictionary get_performance_stats_dict() const { return stats.to_dictionary(); }

	// ========================================================================
	// Update
	// ========================================================================

	void update(double p_delta);
	void force_refresh();

	// ========================================================================
	// Input Handling
	// ========================================================================

	virtual void _input(const Ref<InputEvent> &p_event);
	void set_toggle_hotkey(const String &p_key);
	String get_toggle_hotkey() const { return config.toggle_hotkey; }

	// ========================================================================
	// Constructors
	// ========================================================================

	TurboDebugPanel();
	~TurboDebugPanel();
};

#endif // TURBO_DEBUG_PANEL_H