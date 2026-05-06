/**************************************************************************/
/*  turbo_debug_panel.cpp                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#include "turbo_debug_panel.h"

#include "core/config/engine.h"
#include "core/input/input_event.h"
#include "core/os/memory.h"
#include "core/os/os.h"
#include "core/os/time.h"
#include "core/templates/rid.h"
#include "scene/gui/separator.h"
#include "scene/main/viewport.h"
#include "scene/resources/style_box_flat.h"
#include "scene/theme/theme_db.h"

#include "../ecs/flecs_types/flecs_server.h"
#include "turbo_debug_system.h"

// ============================================================================
// TurboDebugPanelConfig
// ============================================================================

Dictionary TurboDebugPanelConfig::to_dictionary() const {
	Dictionary d;
	d["visible"] = visible;
	d["start_minimized"] = start_minimized;
	d["show_fps"] = show_fps;
	d["show_entity_count"] = show_entity_count;
	d["show_memory_stats"] = show_memory_stats;
	d["anchor"] = anchor;
	d["offset"] = offset;
	d["min_size"] = min_size;
	d["max_size"] = max_size;
	d["opacity"] = opacity;
	d["refresh_interval"] = refresh_interval;
	d["auto_refresh"] = auto_refresh;
	d["show_all_components"] = show_all_components;
	d["show_component_types"] = show_component_types;
	d["max_component_depth"] = max_component_depth;
	d["max_array_items"] = max_array_items;
	d["toggle_hotkey"] = toggle_hotkey;
	return d;
}

TurboDebugPanelConfig TurboDebugPanelConfig::from_dictionary(const Dictionary &p_dict) {
	TurboDebugPanelConfig cfg;
	cfg.visible = p_dict.get("visible", cfg.visible);
	cfg.start_minimized = p_dict.get("start_minimized", cfg.start_minimized);
	cfg.show_fps = p_dict.get("show_fps", cfg.show_fps);
	cfg.show_entity_count = p_dict.get("show_entity_count", cfg.show_entity_count);
	cfg.show_memory_stats = p_dict.get("show_memory_stats", cfg.show_memory_stats);
	cfg.anchor = p_dict.get("anchor", cfg.anchor);
	cfg.offset = p_dict.get("offset", cfg.offset);
	cfg.min_size = p_dict.get("min_size", cfg.min_size);
	cfg.max_size = p_dict.get("max_size", cfg.max_size);
	cfg.opacity = p_dict.get("opacity", cfg.opacity);
	cfg.refresh_interval = p_dict.get("refresh_interval", cfg.refresh_interval);
	cfg.auto_refresh = p_dict.get("auto_refresh", cfg.auto_refresh);
	cfg.show_all_components = p_dict.get("show_all_components", cfg.show_all_components);
	cfg.show_component_types = p_dict.get("show_component_types", cfg.show_component_types);
	cfg.max_component_depth = p_dict.get("max_component_depth", cfg.max_component_depth);
	cfg.max_array_items = p_dict.get("max_array_items", cfg.max_array_items);
	cfg.toggle_hotkey = p_dict.get("toggle_hotkey", cfg.toggle_hotkey);
	return cfg;
}

// ============================================================================
// TurboDebugPanel Implementation
// ============================================================================

void TurboDebugPanel::_bind_methods() {
	// Initialization
	// NOTE: initialize() is not bound because it takes a TurboDebugSystem* pointer,
	// and TurboDebugSystem is a singleton which ClassDB validation does not allow as arguments.
	// Use TurboDebugSystem.get_debug_panel() instead to get an initialized panel.
	ClassDB::bind_method(D_METHOD("shutdown"), &TurboDebugPanel::shutdown);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TurboDebugPanel::is_initialized);

	// Configuration
	ClassDB::bind_method(D_METHOD("set_config_from_dict", "config"), &TurboDebugPanel::set_config_from_dict);
	ClassDB::bind_method(D_METHOD("get_config_dict"), &TurboDebugPanel::get_config_dict);

	// Visibility
	ClassDB::bind_method(D_METHOD("show_panel"), &TurboDebugPanel::show_panel);
	ClassDB::bind_method(D_METHOD("hide_panel"), &TurboDebugPanel::hide_panel);
	ClassDB::bind_method(D_METHOD("toggle_panel"), &TurboDebugPanel::toggle_panel);
	ClassDB::bind_method(D_METHOD("is_panel_visible"), &TurboDebugPanel::is_panel_visible);
	ClassDB::bind_method(D_METHOD("set_minimized", "minimized"), &TurboDebugPanel::set_minimized);
	ClassDB::bind_method(D_METHOD("is_panel_minimized"), &TurboDebugPanel::is_panel_minimized);
	ClassDB::bind_method(D_METHOD("set_anchor", "anchor"), &TurboDebugPanel::set_anchor);
	ClassDB::bind_method(D_METHOD("get_anchor"), &TurboDebugPanel::get_anchor);
	ClassDB::bind_method(D_METHOD("set_opacity", "opacity"), &TurboDebugPanel::set_opacity);
	ClassDB::bind_method(D_METHOD("get_opacity"), &TurboDebugPanel::get_opacity);

	// Entity Selection
	ClassDB::bind_method(D_METHOD("set_selected_entity", "entity", "world"), &TurboDebugPanel::set_selected_entity);
	ClassDB::bind_method(D_METHOD("clear_selected_entity"), &TurboDebugPanel::clear_selected_entity);
	ClassDB::bind_method(D_METHOD("get_selected_entity"), &TurboDebugPanel::get_selected_entity);
	ClassDB::bind_method(D_METHOD("get_selected_world"), &TurboDebugPanel::get_selected_world);

	// Visualization Toggles
	ClassDB::bind_method(D_METHOD("register_toggle", "name", "tooltip", "default_value", "category"), &TurboDebugPanel::register_toggle, DEFVAL("General"));
	ClassDB::bind_method(D_METHOD("set_toggle_enabled", "name", "enabled"), &TurboDebugPanel::set_toggle_enabled);
	ClassDB::bind_method(D_METHOD("is_toggle_enabled", "name"), &TurboDebugPanel::is_toggle_enabled);
	ClassDB::bind_method(D_METHOD("remove_toggle", "name"), &TurboDebugPanel::remove_toggle);
	ClassDB::bind_method(D_METHOD("clear_toggles"), &TurboDebugPanel::clear_toggles);
	ClassDB::bind_method(D_METHOD("get_toggle_names"), &TurboDebugPanel::get_toggle_names);
	ClassDB::bind_method(D_METHOD("get_toggle_categories"), &TurboDebugPanel::get_toggle_categories);

	// Logging
	ClassDB::bind_method(D_METHOD("log_message", "message"), &TurboDebugPanel::log_message);
	ClassDB::bind_method(D_METHOD("log_warning", "message"), &TurboDebugPanel::log_warning);
	ClassDB::bind_method(D_METHOD("log_error", "message"), &TurboDebugPanel::log_error);
	ClassDB::bind_method(D_METHOD("log_debug", "message"), &TurboDebugPanel::log_debug);
	ClassDB::bind_method(D_METHOD("clear_log"), &TurboDebugPanel::clear_log);
	ClassDB::bind_method(D_METHOD("get_log_buffer"), &TurboDebugPanel::get_log_buffer);

	// Commands
	ClassDB::bind_method(D_METHOD("register_command", "name", "tooltip", "callback"), &TurboDebugPanel::register_command);
	ClassDB::bind_method(D_METHOD("unregister_command", "name"), &TurboDebugPanel::unregister_command);
	ClassDB::bind_method(D_METHOD("clear_commands"), &TurboDebugPanel::clear_commands);

	// Performance
	ClassDB::bind_method(D_METHOD("get_performance_stats_dict"), &TurboDebugPanel::get_performance_stats_dict);

	// Update
	ClassDB::bind_method(D_METHOD("update", "delta"), &TurboDebugPanel::update);
	ClassDB::bind_method(D_METHOD("force_refresh"), &TurboDebugPanel::force_refresh);

	// Input
	ClassDB::bind_method(D_METHOD("set_toggle_hotkey", "key"), &TurboDebugPanel::set_toggle_hotkey);
	ClassDB::bind_method(D_METHOD("get_toggle_hotkey"), &TurboDebugPanel::get_toggle_hotkey);

	// Internal callbacks
	ClassDB::bind_method(D_METHOD("_on_minimize_pressed"), &TurboDebugPanel::_on_minimize_pressed);
	ClassDB::bind_method(D_METHOD("_on_close_pressed"), &TurboDebugPanel::_on_close_pressed);
	ClassDB::bind_method(D_METHOD("_on_clear_log_pressed"), &TurboDebugPanel::_on_clear_log_pressed);
}

TurboDebugPanel::TurboDebugPanel() {
	set_layer(100); // High layer to be on top of game
}

TurboDebugPanel::~TurboDebugPanel() {
	shutdown();
}

void TurboDebugPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			if (!is_initialized() && debug_system) {
				_build_ui();
			}
		} break;

		case NOTIFICATION_PROCESS: {
			if (config.auto_refresh) {
				update(get_process_delta_time());
			}
		} break;
	}
}

// ============================================================================
// Initialization
// ============================================================================

void TurboDebugPanel::initialize(TurboDebugSystem *p_debug_system) {
	if (is_initialized()) {
		return;
	}

	debug_system = p_debug_system;
	flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));

	_build_ui();

	set_process(true);

	if (config.start_minimized) {
		is_minimized = true;
	}

	if (!config.visible) {
		hide_panel();
	}
}

void TurboDebugPanel::shutdown() {
	if (root_panel && root_panel->get_parent()) {
		root_panel->get_parent()->remove_child(root_panel);
		memdelete(root_panel);
	}

	root_panel = nullptr;
	main_container = nullptr;
	tab_container = nullptr;
	component_tree = nullptr;
	log_text = nullptr;

	visualization_toggles.clear();
	toggle_name_to_index.clear();
	log_buffer.clear();

	set_process(false);
}

void TurboDebugPanel::set_flecs_server(FlecsServer *p_server) {
	flecs_server = p_server;
}

void TurboDebugPanel::set_debug_draw(const Ref<TurboDebugDraw> &p_debug_draw) {
	debug_draw = p_debug_draw;
}

void TurboDebugPanel::set_entity_picker(const Ref<TurboEntityPicker> &p_picker) {
	entity_picker = p_picker;
}

// ============================================================================
// UI Building
// ============================================================================

void TurboDebugPanel::_build_ui() {
	// Root panel
	root_panel = memnew(PanelContainer);
	root_panel->set_custom_minimum_size(config.min_size);
	root_panel->set_anchors_and_offsets_preset(Control::PRESET_TOP_RIGHT);
	root_panel->set_offset(SIDE_RIGHT, -config.offset.x);
	root_panel->set_offset(SIDE_TOP, config.offset.y);
	add_child(root_panel);

	main_container = memnew(VBoxContainer);
	root_panel->add_child(main_container);

	_build_header();

	// Tab container
	tab_container = memnew(TabContainer);
	tab_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_container->add_child(tab_container);

	_build_entity_tab();
	_build_stats_tab();
	_build_visualization_tab();
	_build_log_tab();
	_build_commands_tab();

	_apply_theme();
}

void TurboDebugPanel::_build_header() {
	header_bar = memnew(HBoxContainer);
	main_container->add_child(header_bar);

	title_label = memnew(Label);
	title_label->set_text("Turbo Debug");
	title_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	header_bar->add_child(title_label);

	minimize_button = memnew(Button);
	minimize_button->set_text("_");
	minimize_button->set_tooltip_text("Minimize");
	minimize_button->connect("pressed", callable_mp(this, &TurboDebugPanel::_on_minimize_pressed));
	header_bar->add_child(minimize_button);

	close_button = memnew(Button);
	close_button->set_text("X");
	close_button->set_tooltip_text("Close");
	close_button->connect("pressed", callable_mp(this, &TurboDebugPanel::_on_close_pressed));
	header_bar->add_child(close_button);

	// Make header draggable
	header_bar->connect("gui_input", callable_mp(this, &TurboDebugPanel::_on_header_gui_input));
}

void TurboDebugPanel::_build_entity_tab() {
	entity_tab = memnew(VBoxContainer);
	entity_tab->set_name("Entity");
	tab_container->add_child(entity_tab);

	// Entity header
	entity_header = memnew(Label);
	entity_header->set_text("Selected Entity");
	entity_tab->add_child(entity_header);

	entity_name_label = memnew(Label);
	entity_name_label->set_text("None");
	entity_tab->add_child(entity_name_label);

	entity_rid_label = memnew(Label);
	entity_rid_label->set_text("RID: -");
	entity_tab->add_child(entity_rid_label);

	entity_tab->add_child(memnew(HSeparator));

	// Components scroll
	components_scroll = memnew(ScrollContainer);
	components_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	components_scroll->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	entity_tab->add_child(components_scroll);

	components_container = memnew(VBoxContainer);
	components_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	components_scroll->add_child(components_container);

	// Component tree
	component_tree = memnew(Tree);
	component_tree->set_hide_root(true);
	component_tree->set_columns(2);
	component_tree->set_column_titles_visible(true);
	component_tree->set_column_title(0, "Property");
	component_tree->set_column_title(1, "Value");
	component_tree->set_column_expand(0, true);
	component_tree->set_column_expand(1, true);
	component_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	component_tree->connect("item_selected", callable_mp(this, &TurboDebugPanel::_on_component_tree_item_selected));
	components_container->add_child(component_tree);
}

void TurboDebugPanel::_build_stats_tab() {
	stats_tab = memnew(VBoxContainer);
	stats_tab->set_name("Stats");
	tab_container->add_child(stats_tab);

	fps_label = memnew(Label);
	fps_label->set_text("FPS: --");
	stats_tab->add_child(fps_label);

	frame_time_label = memnew(Label);
	frame_time_label->set_text("Frame Time: -- ms");
	stats_tab->add_child(frame_time_label);

	stats_tab->add_child(memnew(HSeparator));

	entity_count_label = memnew(Label);
	entity_count_label->set_text("Entities: --");
	stats_tab->add_child(entity_count_label);

	worlds_count_label = memnew(Label);
	worlds_count_label->set_text("Worlds: --");
	stats_tab->add_child(worlds_count_label);

	stats_tab->add_child(memnew(HSeparator));

	memory_label = memnew(Label);
	memory_label->set_text("Memory: -- MB");
	stats_tab->add_child(memory_label);

	debug_draw_stats_label = memnew(Label);
	debug_draw_stats_label->set_text("Debug Draw: -- lines, -- gizmos");
	stats_tab->add_child(debug_draw_stats_label);

	// Spacer
	Control *spacer = memnew(Control);
	spacer->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	stats_tab->add_child(spacer);
}

void TurboDebugPanel::_build_visualization_tab() {
	visualization_tab = memnew(VBoxContainer);
	visualization_tab->set_name("Visualization");
	tab_container->add_child(visualization_tab);

	Label *viz_header = memnew(Label);
	viz_header->set_text("Visualization Toggles");
	visualization_tab->add_child(viz_header);

	toggles_scroll = memnew(ScrollContainer);
	toggles_scroll->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	toggles_scroll->set_horizontal_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	visualization_tab->add_child(toggles_scroll);

	toggles_container = memnew(VBoxContainer);
	toggles_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	toggles_scroll->add_child(toggles_container);
}

void TurboDebugPanel::_build_log_tab() {
	log_tab = memnew(VBoxContainer);
	log_tab->set_name("Log");
	tab_container->add_child(log_tab);

	log_text = memnew(RichTextLabel);
	log_text->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	log_text->set_scroll_follow(true);
	log_text->set_selection_enabled(true);
	log_tab->add_child(log_text);

	clear_log_button = memnew(Button);
	clear_log_button->set_text("Clear Log");
	clear_log_button->connect("pressed", callable_mp(this, &TurboDebugPanel::_on_clear_log_pressed));
	log_tab->add_child(clear_log_button);
}

void TurboDebugPanel::_build_commands_tab() {
	commands_tab = memnew(VBoxContainer);
	commands_tab->set_name("Commands");
	tab_container->add_child(commands_tab);

	Label *cmd_header = memnew(Label);
	cmd_header->set_text("Debug Commands");
	commands_tab->add_child(cmd_header);

	commands_container = memnew(VBoxContainer);
	commands_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	commands_tab->add_child(commands_container);
}

void TurboDebugPanel::_apply_theme() {
	if (!root_panel) {
		return;
	}

	// Create a dark semi-transparent style
	Ref<StyleBoxFlat> panel_style;
	panel_style.instantiate();
	panel_style->set_bg_color(Color(0.1f, 0.1f, 0.12f, config.opacity));
	panel_style->set_border_color(Color(0.3f, 0.3f, 0.35f, 1.0f));
	panel_style->set_border_width_all(1);
	panel_style->set_corner_radius_all(4);
	panel_style->set_content_margin_all(8);

	root_panel->add_theme_style_override("panel", panel_style);
}

// ============================================================================
// Configuration
// ============================================================================

void TurboDebugPanel::set_config(const TurboDebugPanelConfig &p_config) {
	config = p_config;
	_apply_theme();
}

void TurboDebugPanel::set_config_from_dict(const Dictionary &p_dict) {
	set_config(TurboDebugPanelConfig::from_dictionary(p_dict));
}

// ============================================================================
// Visibility
// ============================================================================

void TurboDebugPanel::show_panel() {
	config.visible = true;
	if (root_panel) {
		root_panel->set_visible(true);
	}
}

void TurboDebugPanel::hide_panel() {
	config.visible = false;
	if (root_panel) {
		root_panel->set_visible(false);
	}
}

void TurboDebugPanel::toggle_panel() {
	if (is_panel_visible()) {
		hide_panel();
	} else {
		show_panel();
	}
}

bool TurboDebugPanel::is_panel_visible() const {
	return config.visible && root_panel && root_panel->is_visible();
}

void TurboDebugPanel::set_minimized(bool p_minimized) {
	is_minimized = p_minimized;
	if (tab_container) {
		tab_container->set_visible(!is_minimized);
	}
	if (minimize_button) {
		minimize_button->set_text(is_minimized ? "+" : "_");
	}
}

void TurboDebugPanel::set_anchor(int p_anchor) {
	config.anchor = p_anchor;

	if (!root_panel) {
		return;
	}

	switch (p_anchor) {
		case 0: // Top Left
			root_panel->set_anchors_and_offsets_preset(Control::PRESET_TOP_LEFT);
			root_panel->set_offset(SIDE_LEFT, config.offset.x);
			root_panel->set_offset(SIDE_TOP, config.offset.y);
			break;
		case 1: // Top Right
			root_panel->set_anchors_and_offsets_preset(Control::PRESET_TOP_RIGHT);
			root_panel->set_offset(SIDE_RIGHT, -config.offset.x);
			root_panel->set_offset(SIDE_TOP, config.offset.y);
			break;
		case 2: // Bottom Left
			root_panel->set_anchors_and_offsets_preset(Control::PRESET_BOTTOM_LEFT);
			root_panel->set_offset(SIDE_LEFT, config.offset.x);
			root_panel->set_offset(SIDE_BOTTOM, -config.offset.y);
			break;
		case 3: // Bottom Right
			root_panel->set_anchors_and_offsets_preset(Control::PRESET_BOTTOM_RIGHT);
			root_panel->set_offset(SIDE_RIGHT, -config.offset.x);
			root_panel->set_offset(SIDE_BOTTOM, -config.offset.y);
			break;
	}
}

void TurboDebugPanel::set_opacity(float p_opacity) {
	config.opacity = CLAMP(p_opacity, 0.0f, 1.0f);
	_apply_theme();
}

// ============================================================================
// Entity Selection
// ============================================================================

void TurboDebugPanel::set_selected_entity(const RID &p_entity, const RID &p_world) {
	selected_entity = p_entity;
	selected_world = p_world;
	selected_entity_name = "";

	_refresh_entity_display();
}

void TurboDebugPanel::clear_selected_entity() {
	selected_entity = RID();
	selected_world = RID();
	selected_entity_name = "";

	if (entity_name_label) {
		entity_name_label->set_text("None");
	}
	if (entity_rid_label) {
		entity_rid_label->set_text("RID: -");
	}
	if (component_tree) {
		component_tree->clear();
	}
}

// ============================================================================
// Visualization Toggles
// ============================================================================

void TurboDebugPanel::register_toggle(const String &p_name, const String &p_tooltip, bool p_default, const String &p_category) {
	TurboVisualizationToggle toggle(p_name, p_tooltip, p_default, p_category);
	toggle_name_to_index[p_name] = visualization_toggles.size();
	visualization_toggles.push_back(toggle);

	_refresh_visualization_toggles();
}

void TurboDebugPanel::register_toggle_with_callback(const String &p_name, const String &p_tooltip, bool p_default, const Callable &p_callback, const String &p_category) {
	TurboVisualizationToggle toggle(p_name, p_tooltip, p_default, p_category);
	toggle.on_changed = p_callback;
	toggle_name_to_index[p_name] = visualization_toggles.size();
	visualization_toggles.push_back(toggle);

	_refresh_visualization_toggles();
}

void TurboDebugPanel::set_toggle_enabled(const String &p_name, bool p_enabled) {
	if (toggle_name_to_index.has(p_name)) {
		int idx = toggle_name_to_index[p_name];
		visualization_toggles[idx].enabled = p_enabled;

		if (visualization_toggles[idx].on_changed.is_valid()) {
			visualization_toggles[idx].on_changed.call(p_enabled);
		}
	}
}

bool TurboDebugPanel::is_toggle_enabled(const String &p_name) const {
	if (toggle_name_to_index.has(p_name)) {
		int idx = toggle_name_to_index[p_name];
		return visualization_toggles[idx].enabled;
	}
	return false;
}

void TurboDebugPanel::remove_toggle(const String &p_name) {
	if (toggle_name_to_index.has(p_name)) {
		int idx = toggle_name_to_index[p_name];
		visualization_toggles.remove_at(idx);
		toggle_name_to_index.erase(p_name);

		// Rebuild index
		toggle_name_to_index.clear();
		for (int i = 0; i < (int)visualization_toggles.size(); i++) {
			toggle_name_to_index[visualization_toggles[i].name] = i;
		}

		_refresh_visualization_toggles();
	}
}

void TurboDebugPanel::clear_toggles() {
	visualization_toggles.clear();
	toggle_name_to_index.clear();
	_refresh_visualization_toggles();
}

PackedStringArray TurboDebugPanel::get_toggle_names() const {
	PackedStringArray names;
	for (const TurboVisualizationToggle &toggle : visualization_toggles) {
		names.push_back(toggle.name);
	}
	return names;
}

PackedStringArray TurboDebugPanel::get_toggle_categories() const {
	PackedStringArray categories;
	for (const TurboVisualizationToggle &toggle : visualization_toggles) {
		if (!categories.has(toggle.category)) {
			categories.push_back(toggle.category);
		}
	}
	return categories;
}

// ============================================================================
// Logging
// ============================================================================

void TurboDebugPanel::log_message(const String &p_message) {
	String formatted = vformat("[%s] %s", Time::get_singleton()->get_time_string_from_system(), p_message);
	log_buffer.push_back(formatted);

	if ((int)log_buffer.size() > MAX_LOG_LINES) {
		log_buffer.remove_at(0);
	}

	if (log_text) {
		log_text->add_text(formatted + "\n");
	}
}

void TurboDebugPanel::log_warning(const String &p_message) {
	String formatted = vformat("[%s] [WARNING] %s", Time::get_singleton()->get_time_string_from_system(), p_message);
	log_buffer.push_back(formatted);

	if ((int)log_buffer.size() > MAX_LOG_LINES) {
		log_buffer.remove_at(0);
	}

	if (log_text) {
		log_text->push_color(Color(1.0f, 0.8f, 0.0f));
		log_text->add_text(formatted + "\n");
		log_text->pop();
	}
}

void TurboDebugPanel::log_error(const String &p_message) {
	String formatted = vformat("[%s] [ERROR] %s", Time::get_singleton()->get_time_string_from_system(), p_message);
	log_buffer.push_back(formatted);

	if ((int)log_buffer.size() > MAX_LOG_LINES) {
		log_buffer.remove_at(0);
	}

	if (log_text) {
		log_text->push_color(Color(1.0f, 0.3f, 0.3f));
		log_text->add_text(formatted + "\n");
		log_text->pop();
	}
}

void TurboDebugPanel::log_debug(const String &p_message) {
	String formatted = vformat("[%s] [DEBUG] %s", Time::get_singleton()->get_time_string_from_system(), p_message);
	log_buffer.push_back(formatted);

	if ((int)log_buffer.size() > MAX_LOG_LINES) {
		log_buffer.remove_at(0);
	}

	if (log_text) {
		log_text->push_color(Color(0.6f, 0.6f, 0.7f));
		log_text->add_text(formatted + "\n");
		log_text->pop();
	}
}

void TurboDebugPanel::clear_log() {
	log_buffer.clear();
	if (log_text) {
		log_text->clear();
	}
}

PackedStringArray TurboDebugPanel::get_log_buffer() const {
	PackedStringArray result;
	for (const String &line : log_buffer) {
		result.push_back(line);
	}
	return result;
}

// ============================================================================
// Commands
// ============================================================================

void TurboDebugPanel::register_command(const String &p_name, const String &p_tooltip, const Callable &p_callback) {
	if (!commands_container) {
		return;
	}

	Button *cmd_button = memnew(Button);
	cmd_button->set_text(p_name);
	cmd_button->set_tooltip_text(p_tooltip);
	cmd_button->connect("pressed", p_callback);
	cmd_button->set_meta("command_name", p_name);
	commands_container->add_child(cmd_button);
}

void TurboDebugPanel::unregister_command(const String &p_name) {
	if (!commands_container) {
		return;
	}

	for (int i = 0; i < commands_container->get_child_count(); i++) {
		Button *btn = Object::cast_to<Button>(commands_container->get_child(i));
		if (btn && btn->get_meta("command_name", "") == p_name) {
			commands_container->remove_child(btn);
			memdelete(btn);
			return;
		}
	}
}

void TurboDebugPanel::clear_commands() {
	if (!commands_container) {
		return;
	}

	while (commands_container->get_child_count() > 0) {
		Node *child = commands_container->get_child(0);
		commands_container->remove_child(child);
		memdelete(child);
	}
}

// ============================================================================
// UI Updates
// ============================================================================

void TurboDebugPanel::_refresh_entity_display() {
	if (!selected_entity.is_valid()) {
		clear_selected_entity();
		return;
	}

	if (entity_rid_label) {
		entity_rid_label->set_text(vformat("RID: %s", selected_entity.get_id()));
	}

	// Get entity data
	Dictionary entity_data = _get_entity_data(selected_entity, selected_world);

	if (entity_data.has("name")) {
		selected_entity_name = entity_data["name"];
	} else {
		selected_entity_name = vformat("Entity_%d", selected_entity.get_id());
	}

	if (entity_name_label) {
		entity_name_label->set_text(selected_entity_name);
	}

	_update_component_tree(entity_data);
}

void TurboDebugPanel::_refresh_stats_display() {
	_collect_performance_stats();

	if (fps_label) {
		fps_label->set_text(vformat("FPS: %.1f", stats.fps));
	}

	if (frame_time_label) {
		frame_time_label->set_text(vformat("Frame Time: %.2f ms", stats.frame_time_ms));
	}

	if (entity_count_label) {
		entity_count_label->set_text(vformat("Entities: %d (active: %d)", stats.total_entities, stats.active_entities));
	}

	if (worlds_count_label) {
		worlds_count_label->set_text(vformat("Worlds: %d", stats.worlds_count));
	}

	if (memory_label) {
		float memory_mb = (float)(stats.memory_static + stats.memory_dynamic) / (1024.0f * 1024.0f);
		memory_label->set_text(vformat("Memory: %.1f MB", memory_mb));
	}

	if (debug_draw_stats_label && debug_draw.is_valid()) {
		debug_draw_stats_label->set_text(vformat("Debug Draw: %d lines, %d gizmos",
				debug_draw->get_lines_drawn(), debug_draw->get_gizmos_drawn()));
	}
}

void TurboDebugPanel::_refresh_visualization_toggles() {
	if (!toggles_container) {
		return;
	}

	// Clear existing toggles
	while (toggles_container->get_child_count() > 0) {
		Node *child = toggles_container->get_child(0);
		toggles_container->remove_child(child);
		memdelete(child);
	}

	// Group by category
	PackedStringArray categories = get_toggle_categories();

	for (int c = 0; c < categories.size(); c++) {
		const String &category = categories[c];

		// Category label
		Label *cat_label = memnew(Label);
		cat_label->set_text(category);
		toggles_container->add_child(cat_label);

		// Add toggles for this category
		for (int i = 0; i < (int)visualization_toggles.size(); i++) {
			const TurboVisualizationToggle &toggle = visualization_toggles[i];

			if (toggle.category != category) {
				continue;
			}

			CheckBox *checkbox = memnew(CheckBox);
			checkbox->set_text(toggle.name);
			checkbox->set_tooltip_text(toggle.tooltip);
			checkbox->set_pressed(toggle.enabled);
			checkbox->connect("toggled", callable_mp(this, &TurboDebugPanel::_on_toggle_changed).bind(i));
			toggles_container->add_child(checkbox);
		}

		if (c < categories.size() - 1) {
			toggles_container->add_child(memnew(HSeparator));
		}
	}
}

void TurboDebugPanel::_update_component_tree(const Dictionary &p_entity_data) {
	if (!component_tree) {
		return;
	}

	component_tree->clear();
	TreeItem *root = component_tree->create_item();

	if (!p_entity_data.has("components")) {
		return;
	}

	Dictionary components = p_entity_data["components"];
	Array keys = components.keys();

	for (int i = 0; i < keys.size(); i++) {
		String comp_name = keys[i];
		Variant comp_data = components[comp_name];

		_add_component_to_tree(root, comp_name, comp_data);
	}
}

void TurboDebugPanel::_add_component_to_tree(TreeItem *p_parent, const String &p_component_name, const Dictionary &p_data) {
	TreeItem *comp_item = component_tree->create_item(p_parent);
	comp_item->set_text(0, p_component_name);
	comp_item->set_text(1, vformat("(%d fields)", p_data.size()));
	comp_item->set_selectable(0, true);
	comp_item->set_selectable(1, false);

	Array keys = p_data.keys();
	for (int i = 0; i < keys.size(); i++) {
		String key = keys[i];
		Variant value = p_data[key];

		_add_property_item(comp_item, key, value, 0);
	}
}

TreeItem *TurboDebugPanel::_add_property_item(TreeItem *p_parent, const String &p_key, const Variant &p_value, int p_depth) {
	if (p_depth > config.max_component_depth) {
		return nullptr;
	}

	TreeItem *item = component_tree->create_item(p_parent);
	item->set_text(0, p_key);

	if (config.show_component_types) {
		String type_str = _get_type_name(p_value.get_type());
		item->set_text(0, vformat("%s [%s]", p_key, type_str));
	}

	String value_str = _format_value(p_value);
	item->set_text(1, value_str);
	item->set_custom_color(1, _get_type_color(p_value.get_type()));

	// Recursively add nested dictionaries
	if (p_value.get_type() == Variant::DICTIONARY && p_depth < config.max_component_depth) {
		Dictionary dict = p_value;
		Array keys = dict.keys();
		for (int i = 0; i < MIN(keys.size(), config.max_array_items); i++) {
			String sub_key = keys[i];
			_add_property_item(item, sub_key, dict[sub_key], p_depth + 1);
		}
	} else if (p_value.get_type() == Variant::ARRAY && p_depth < config.max_component_depth) {
		Array arr = p_value;
		for (int i = 0; i < MIN(arr.size(), config.max_array_items); i++) {
			_add_property_item(item, vformat("[%d]", i), arr[i], p_depth + 1);
		}
	}

	return item;
}

String TurboDebugPanel::_format_value(const Variant &p_value) const {
	switch (p_value.get_type()) {
		case Variant::NIL:
			return "null";
		case Variant::BOOL:
			return p_value.operator bool() ? "true" : "false";
		case Variant::INT:
			return vformat("%d", p_value.operator int64_t());
		case Variant::FLOAT:
			return vformat("%.4f", p_value.operator double());
		case Variant::STRING:
			return vformat("\"%s\"", p_value.operator String());
		case Variant::VECTOR2:
			return vformat("(%.2f, %.2f)", p_value.operator Vector2().x, p_value.operator Vector2().y);
		case Variant::VECTOR3:
			return vformat("(%.2f, %.2f, %.2f)",
					p_value.operator Vector3().x,
					p_value.operator Vector3().y,
					p_value.operator Vector3().z);
		case Variant::COLOR:
			return vformat("rgba(%.2f, %.2f, %.2f, %.2f)",
					p_value.operator Color().r,
					p_value.operator Color().g,
					p_value.operator Color().b,
					p_value.operator Color().a);
		case Variant::DICTIONARY:
			return vformat("{...} (%d)", p_value.operator Dictionary().size());
		case Variant::ARRAY:
			return vformat("[...] (%d)", p_value.operator Array().size());
		case Variant::RID:
			return vformat("RID(%d)", p_value.operator ::RID().get_id());
		default:
			return p_value.stringify();
	}
}

String TurboDebugPanel::_get_type_name(Variant::Type p_type) const {
	switch (p_type) {
		case Variant::NIL:
			return "null";
		case Variant::BOOL:
			return "bool";
		case Variant::INT:
			return "int";
		case Variant::FLOAT:
			return "float";
		case Variant::STRING:
			return "string";
		case Variant::VECTOR2:
			return "vec2";
		case Variant::VECTOR3:
			return "vec3";
		case Variant::QUATERNION:
			return "quat";
		case Variant::TRANSFORM3D:
			return "transform";
		case Variant::COLOR:
			return "color";
		case Variant::DICTIONARY:
			return "dict";
		case Variant::ARRAY:
			return "array";
		case Variant::RID:
			return "rid";
		default:
			return Variant::get_type_name(p_type);
	}
}

Color TurboDebugPanel::_get_type_color(Variant::Type p_type) const {
	switch (p_type) {
		case Variant::BOOL:
			return Color(0.8f, 0.5f, 0.8f); // Purple
		case Variant::INT:
			return Color(0.4f, 0.7f, 1.0f); // Light blue
		case Variant::FLOAT:
			return Color(0.4f, 0.9f, 0.8f); // Cyan
		case Variant::STRING:
			return Color(1.0f, 0.8f, 0.4f); // Yellow
		case Variant::VECTOR2:
		case Variant::VECTOR3:
			return Color(0.6f, 0.9f, 0.6f); // Green
		case Variant::COLOR:
			return Color(1.0f, 0.6f, 0.6f); // Light red
		default:
			return Color(0.8f, 0.8f, 0.8f); // Gray
	}
}

// ============================================================================
// Data Collection
// ============================================================================

void TurboDebugPanel::_collect_performance_stats() {
	// Frame timing
	stats.fps = Engine::get_singleton()->get_frames_per_second();
	stats.frame_time_ms = 1000.0 / MAX(stats.fps, 0.001);

	// ECS stats from FlecsServer
	if (flecs_server) {
		TypedArray<RID> worlds = flecs_server->get_world_list();
		stats.worlds_count = worlds.size();

		// Note: FlecsServer doesn't have a direct get_entity_count per world
		// We'd need to query or track this separately
		stats.total_entities = 0;
		stats.active_entities = 0;
	}

	// Memory stats
	stats.memory_static = OS::get_singleton()->get_static_memory_usage();
	stats.memory_dynamic = OS::get_singleton()->get_static_memory_peak_usage();

	// Debug draw stats
	if (debug_draw.is_valid()) {
		stats.lines_drawn = debug_draw->get_lines_drawn();
		stats.gizmos_drawn = debug_draw->get_gizmos_drawn();
	}

	if (entity_picker.is_valid()) {
		stats.picks_this_frame = entity_picker->get_statistics()["picks_this_frame"];
	}
}

Dictionary TurboDebugPanel::_get_entity_data(const RID &p_entity, const RID &p_world) const {
	Dictionary result;

	if (!flecs_server || !p_entity.is_valid()) {
		return result;
	}

	// Get entity name
	String name = flecs_server->get_entity_name(p_entity);
	result["name"] = name.is_empty() ? vformat("Entity_%d", p_entity.get_id()) : name;
	result["rid"] = p_entity;
	result["world"] = p_world;

	// Get all components
	Dictionary components;
	PackedStringArray comp_names = flecs_server->get_component_types_as_name(p_entity);
	for (int i = 0; i < comp_names.size(); i++) {
		Dictionary comp_data = flecs_server->get_component_by_name(p_entity, comp_names[i]);
		if (!comp_data.is_empty()) {
			components[comp_names[i]] = comp_data;
		}
	}
	result["components"] = components;

	return result;
}

// ============================================================================
// Event Handlers
// ============================================================================

void TurboDebugPanel::_on_minimize_pressed() {
	set_minimized(!is_minimized);
}

void TurboDebugPanel::_on_close_pressed() {
	hide_panel();
}

void TurboDebugPanel::_on_toggle_changed(bool p_enabled, int p_index) {
	if (p_index >= 0 && p_index < (int)visualization_toggles.size()) {
		visualization_toggles[p_index].enabled = p_enabled;

		if (visualization_toggles[p_index].on_changed.is_valid()) {
			visualization_toggles[p_index].on_changed.call(p_enabled);
		}
	}
}

void TurboDebugPanel::_on_clear_log_pressed() {
	clear_log();
}

void TurboDebugPanel::_on_header_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->get_button_index() == MouseButton::LEFT) {
			if (mb->is_pressed()) {
				is_dragging = true;
				drag_offset = root_panel->get_global_position() - mb->get_global_position();
			} else {
				is_dragging = false;
			}
		}
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && is_dragging && root_panel) {
		root_panel->set_global_position(mm->get_global_position() + drag_offset);
	}
}

void TurboDebugPanel::_on_component_tree_item_selected() {
	// Could implement component value editing here in the future
}

// ============================================================================
// Update
// ============================================================================

void TurboDebugPanel::update(double p_delta) {
	current_time += p_delta;
	frame_count++;

	// Throttle refresh rate
	if (current_time - last_refresh_time < config.refresh_interval) {
		return;
	}
	last_refresh_time = current_time;

	// Refresh stats if on stats tab
	if (tab_container && tab_container->get_current_tab_control() == stats_tab) {
		_refresh_stats_display();
	}

	// Refresh entity if on entity tab
	if (tab_container && tab_container->get_current_tab_control() == entity_tab && selected_entity.is_valid()) {
		_refresh_entity_display();
	}
}

void TurboDebugPanel::force_refresh() {
	_refresh_stats_display();
	_refresh_entity_display();
	_refresh_visualization_toggles();
}

// ============================================================================
// Input Handling
// ============================================================================

void TurboDebugPanel::_input(const Ref<InputEvent> &p_event) {
	if (!config.visible) {
		return;
	}

	Ref<InputEventKey> key = p_event;
	if (key.is_valid() && key->is_pressed() && !key->is_echo()) {
		// Check toggle hotkey
		// This is a simple implementation - could be enhanced with proper input map
		if (key->get_keycode() == Key::F3) {
			toggle_panel();
		}
	}
}

void TurboDebugPanel::set_toggle_hotkey(const String &p_key) {
	config.toggle_hotkey = p_key;
}