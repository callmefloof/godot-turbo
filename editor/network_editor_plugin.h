#ifndef NETWORK_EDITOR_PLUGIN_H
#define NETWORK_EDITOR_PLUGIN_H

/**************************************************************************/
/*  network_editor_plugin.h                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT TURBO                                */
/*                      ECS Framework for Godot 4.x                       */
/**************************************************************************/
/* Copyright (c) 2025 Godot Turbo Contributors                            */
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

#include "editor/plugins/editor_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/graph_edit.h"
#include "scene/gui/item_list.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/tree.h"

class NetworkServer;
class Timer;

/**
 * @class NetworkEntityInspector
 * @brief Panel for inspecting networked entity properties
 * 
 * Shows detailed information about a selected networked entity including:
 * - Network ID and authority information
 * - Replicated components and their configuration
 * - Network statistics (bytes sent/received, update frequency)
 * - Interpolation buffer state
 */
class NetworkEntityInspector : public VBoxContainer {
	GDCLASS(NetworkEntityInspector, VBoxContainer);

protected:
	static void _bind_methods();

private:
	NetworkServer *network_server = nullptr;

	// Header
	Label *entity_name_label = nullptr;
	Label *network_id_label = nullptr;

	// Authority section
	PanelContainer *authority_panel = nullptr;
	Label *authority_mode_label = nullptr;
	Label *authority_peer_label = nullptr;
	Label *owner_peer_label = nullptr;
	CheckBox *has_local_authority_check = nullptr;

	// Replication section
	Tree *replicated_components_tree = nullptr;
	Button *add_component_button = nullptr;
	Button *remove_component_button = nullptr;

	// Statistics section
	Label *bytes_sent_label = nullptr;
	Label *bytes_received_label = nullptr;
	Label *updates_sent_label = nullptr;
	Label *updates_received_label = nullptr;
	Label *avg_update_interval_label = nullptr;
	ProgressBar *interpolation_buffer_bar = nullptr;

	// Current selection
	RID current_world;
	RID current_entity;

	void _build_ui();
	void _update_display();
	void _on_add_component_pressed();
	void _on_remove_component_pressed();
	void _on_component_selected();

public:
	NetworkEntityInspector();
	~NetworkEntityInspector();

	void set_entity(RID p_world, RID p_entity);
	void clear();
	void refresh();
};

/**
 * @class NetworkPeersList
 * @brief Panel showing connected peers and their status
 */
class NetworkPeersList : public VBoxContainer {
	GDCLASS(NetworkPeersList, VBoxContainer);

protected:
	static void _bind_methods();

private:
	NetworkServer *network_server = nullptr;

	Tree *peers_tree = nullptr;
	Label *connection_status_label = nullptr;
	Label *local_peer_id_label = nullptr;
	Label *role_label = nullptr;

	Button *kick_button = nullptr;
	Button *refresh_button = nullptr;

	void _build_ui();
	void _update_display();
	void _on_peer_selected();
	void _on_kick_pressed();
	void _on_refresh_pressed();

public:
	NetworkPeersList();
	~NetworkPeersList();

	void refresh();
	int32_t get_selected_peer_id() const;
};

/**
 * @class NetworkStatsPanel
 * @brief Real-time network statistics visualization
 */
class NetworkStatsPanel : public VBoxContainer {
	GDCLASS(NetworkStatsPanel, VBoxContainer);

protected:
	static void _bind_methods();

private:
	NetworkServer *network_server = nullptr;

	// Global stats
	Label *tick_label = nullptr;
	Label *tick_rate_label = nullptr;
	Label *entities_label = nullptr;
	Label *worlds_label = nullptr;

	// Bandwidth graph (simplified - using progress bars)
	Label *bandwidth_label = nullptr;
	ProgressBar *upload_bar = nullptr;
	ProgressBar *download_bar = nullptr;

	// Latency
	Label *latency_label = nullptr;
	Tree *peer_latency_tree = nullptr;

	// History for averaging
	Vector<float> upload_history;
	Vector<float> download_history;
	int history_index = 0;
	static constexpr int HISTORY_SIZE = 60;

	Timer *update_timer = nullptr;

	void _build_ui();
	void _update_display();
	void _on_timer_timeout();

public:
	NetworkStatsPanel();
	~NetworkStatsPanel();

	void start_monitoring();
	void stop_monitoring();
};

/**
 * @class NetworkWorldInspector
 * @brief Panel for inspecting networked entities in a world
 */
class NetworkWorldInspector : public VBoxContainer {
	GDCLASS(NetworkWorldInspector, VBoxContainer);

protected:
	static void _bind_methods();

private:
	NetworkServer *network_server = nullptr;

	// World selection
	OptionButton *world_selector = nullptr;
	Button *refresh_worlds_button = nullptr;

	// Entity tree
	Tree *entities_tree = nullptr;
	LineEdit *search_filter = nullptr;

	// Filter options
	CheckBox *show_local_authority_only = nullptr;
	CheckBox *show_remote_authority_only = nullptr;

	// Selected entity inspector
	NetworkEntityInspector *entity_inspector = nullptr;

	RID current_world;

	void _build_ui();
	void _update_worlds_list();
	void _update_entities_tree();
	void _on_world_selected(int p_index);
	void _on_entity_selected();
	void _on_search_changed(const String &p_text);
	void _on_refresh_pressed();
	void _apply_filter();

public:
	NetworkWorldInspector();
	~NetworkWorldInspector();

	void refresh();
	void set_world(RID p_world);
};

/**
 * @class NetworkConfigPanel
 * @brief Panel for configuring network settings
 */
class NetworkConfigPanel : public VBoxContainer {
	GDCLASS(NetworkConfigPanel, VBoxContainer);

protected:
	static void _bind_methods();

private:
	NetworkServer *network_server = nullptr;

	// Connection settings
	LineEdit *address_edit = nullptr;
	SpinBox *port_spinbox = nullptr;
	SpinBox *max_clients_spinbox = nullptr;
	Button *host_button = nullptr;
	Button *join_button = nullptr;
	Button *disconnect_button = nullptr;

	// Runtime settings
	SpinBox *tick_rate_spinbox = nullptr;
	SpinBox *interpolation_delay_spinbox = nullptr;
	CheckBox *auto_spawn_checkbox = nullptr;
	CheckBox *debug_logging_checkbox = nullptr;

	void _build_ui();
	void _update_ui_state();
	void _on_host_pressed();
	void _on_join_pressed();
	void _on_disconnect_pressed();
	void _on_tick_rate_changed(double p_value);
	void _on_interpolation_delay_changed(double p_value);
	void _on_auto_spawn_toggled(bool p_enabled);
	void _on_debug_logging_toggled(bool p_enabled);

public:
	NetworkConfigPanel();
	~NetworkConfigPanel();

	void refresh();
};

/**
 * @class NetworkLogPanel
 * @brief Panel showing network event log
 */
class NetworkLogPanel : public VBoxContainer {
	GDCLASS(NetworkLogPanel, VBoxContainer);

protected:
	static void _bind_methods();

private:
	RichTextLabel *log_text = nullptr;
	Button *clear_button = nullptr;
	CheckBox *auto_scroll_checkbox = nullptr;

	// Filter checkboxes
	CheckBox *show_connections_checkbox = nullptr;
	CheckBox *show_spawns_checkbox = nullptr;
	CheckBox *show_updates_checkbox = nullptr;
	CheckBox *show_rpcs_checkbox = nullptr;
	CheckBox *show_errors_checkbox = nullptr;

	void _build_ui();
	void _on_clear_pressed();

public:
	NetworkLogPanel();
	~NetworkLogPanel();

	void add_log(const String &p_message, const Color &p_color = Color(1, 1, 1));
	void add_connection_log(const String &p_message);
	void add_spawn_log(const String &p_message);
	void add_update_log(const String &p_message);
	void add_rpc_log(const String &p_message);
	void add_error_log(const String &p_message);
	void clear();
};

/**
 * @class NetworkEditorPlugin
 * @brief Main editor plugin providing network debugging tools
 * 
 * Features:
 * - Network configuration panel for hosting/joining
 * - Connected peers list with kick functionality
 * - Networked entity inspector per world
 * - Real-time network statistics
 * - Network event log
 */
class NetworkEditorPlugin : public EditorPlugin {
	GDCLASS(NetworkEditorPlugin, EditorPlugin);

protected:
	static void _bind_methods();
	void _notification(int p_what);

private:
	// Singleton
	static NetworkEditorPlugin *singleton;

	// References
	NetworkServer *network_server = nullptr;

	// Main dock
	TabContainer *main_dock = nullptr;

	// Tab panels
	NetworkConfigPanel *config_panel = nullptr;
	NetworkPeersList *peers_panel = nullptr;
	NetworkWorldInspector *world_inspector = nullptr;
	NetworkStatsPanel *stats_panel = nullptr;
	NetworkLogPanel *log_panel = nullptr;

	// Update timer
	Timer *refresh_timer = nullptr;

	void _build_dock();
	void _on_enter_tree();
	void _on_exit_tree();
	void _on_refresh_timer_timeout();

	// Signal handlers
	void _on_peer_connected(int32_t p_peer_id);
	void _on_peer_disconnected(int32_t p_peer_id, int p_reason);
	void _on_connection_succeeded();
	void _on_connection_failed(const String &p_reason);
	void _on_server_started();
	void _on_server_stopped();
	void _on_entity_spawned_remote(int64_t p_network_id, RID p_entity);
	void _on_entity_despawned_remote(int64_t p_network_id);
	void _on_authority_changed(int64_t p_network_id, int32_t p_new_authority);

public:
	static NetworkEditorPlugin *get_singleton();

	NetworkEditorPlugin();
	~NetworkEditorPlugin();

	virtual String get_plugin_name() const override { return "Network Inspector"; }
	virtual bool has_main_screen() const override { return false; }

	// Access to sub-panels
	NetworkConfigPanel *get_config_panel() const { return config_panel; }
	NetworkPeersList *get_peers_panel() const { return peers_panel; }
	NetworkWorldInspector *get_world_inspector() const { return world_inspector; }
	NetworkStatsPanel *get_stats_panel() const { return stats_panel; }
	NetworkLogPanel *get_log_panel() const { return log_panel; }

	void refresh_all();
};

#endif // NETWORK_EDITOR_PLUGIN_H