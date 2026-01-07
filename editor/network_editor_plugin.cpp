/**************************************************************************/
/*  network_editor_plugin.cpp                                             */
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

#include "network_editor_plugin.h"
#include "instance_manager.h"

#include "core/config/engine.h"
#include "editor/editor_node.h"
#include "scene/gui/separator.h"
#include "scene/main/timer.h"
#include "core/os/time.h"

#include "modules/godot_turbo/network/network_server.h"

// =============================================================================
// NetworkEntityInspector
// =============================================================================

void NetworkEntityInspector::_bind_methods() {
}

NetworkEntityInspector::NetworkEntityInspector() {
	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}
	_build_ui();
}

NetworkEntityInspector::~NetworkEntityInspector() {
}

void NetworkEntityInspector::_build_ui() {
	set_custom_minimum_size(Size2(300, 0));

	// Header
	entity_name_label = memnew(Label);
	entity_name_label->set_text("No Entity Selected");
	entity_name_label->add_theme_font_size_override("font_size", 16);
	add_child(entity_name_label);

	network_id_label = memnew(Label);
	network_id_label->set_text("Network ID: -");
	add_child(network_id_label);

	add_child(memnew(HSeparator));

	// Authority section
	Label *authority_header = memnew(Label);
	authority_header->set_text("Authority");
	authority_header->add_theme_font_size_override("font_size", 14);
	add_child(authority_header);

	authority_panel = memnew(PanelContainer);
	VBoxContainer *authority_vbox = memnew(VBoxContainer);
	authority_panel->add_child(authority_vbox);

	authority_mode_label = memnew(Label);
	authority_mode_label->set_text("Mode: SERVER");
	authority_vbox->add_child(authority_mode_label);

	authority_peer_label = memnew(Label);
	authority_peer_label->set_text("Authority Peer: 1");
	authority_vbox->add_child(authority_peer_label);

	owner_peer_label = memnew(Label);
	owner_peer_label->set_text("Owner Peer: 1");
	authority_vbox->add_child(owner_peer_label);

	has_local_authority_check = memnew(CheckBox);
	has_local_authority_check->set_text("Has Local Authority");
	has_local_authority_check->set_disabled(true);
	authority_vbox->add_child(has_local_authority_check);

	add_child(authority_panel);

	add_child(memnew(HSeparator));

	// Replication section
	Label *replication_header = memnew(Label);
	replication_header->set_text("Replicated Components");
	replication_header->add_theme_font_size_override("font_size", 14);
	add_child(replication_header);

	replicated_components_tree = memnew(Tree);
	replicated_components_tree->set_columns(4);
	replicated_components_tree->set_column_titles_visible(true);
	replicated_components_tree->set_column_title(0, "Component");
	replicated_components_tree->set_column_title(1, "Mode");
	replicated_components_tree->set_column_title(2, "Interpolate");
	replicated_components_tree->set_column_title(3, "Priority");
	replicated_components_tree->set_custom_minimum_size(Size2(0, 150));
	add_child(replicated_components_tree);

	HBoxContainer *component_buttons = memnew(HBoxContainer);
	add_component_button = memnew(Button);
	add_component_button->set_text("Add");
	add_component_button->connect("pressed", callable_mp(this, &NetworkEntityInspector::_on_add_component_pressed));
	component_buttons->add_child(add_component_button);

	remove_component_button = memnew(Button);
	remove_component_button->set_text("Remove");
	remove_component_button->connect("pressed", callable_mp(this, &NetworkEntityInspector::_on_remove_component_pressed));
	component_buttons->add_child(remove_component_button);

	add_child(component_buttons);

	add_child(memnew(HSeparator));

	// Statistics section
	Label *stats_header = memnew(Label);
	stats_header->set_text("Network Statistics");
	stats_header->add_theme_font_size_override("font_size", 14);
	add_child(stats_header);

	bytes_sent_label = memnew(Label);
	bytes_sent_label->set_text("Bytes Sent: 0");
	add_child(bytes_sent_label);

	bytes_received_label = memnew(Label);
	bytes_received_label->set_text("Bytes Received: 0");
	add_child(bytes_received_label);

	updates_sent_label = memnew(Label);
	updates_sent_label->set_text("Updates Sent: 0");
	add_child(updates_sent_label);

	updates_received_label = memnew(Label);
	updates_received_label->set_text("Updates Received: 0");
	add_child(updates_received_label);

	avg_update_interval_label = memnew(Label);
	avg_update_interval_label->set_text("Avg Update Interval: 0 ms");
	add_child(avg_update_interval_label);

	Label *interp_label = memnew(Label);
	interp_label->set_text("Interpolation Buffer:");
	add_child(interp_label);

	interpolation_buffer_bar = memnew(ProgressBar);
	interpolation_buffer_bar->set_max(32);
	interpolation_buffer_bar->set_value(0);
	add_child(interpolation_buffer_bar);
}

void NetworkEntityInspector::_update_display() {
	if (!network_server || !current_entity.is_valid()) {
		entity_name_label->set_text("No Entity Selected");
		network_id_label->set_text("Network ID: -");
		return;
	}

	uint64_t network_id = network_server->get_entity_network_id(current_world, current_entity);
	entity_name_label->set_text(vformat("Entity: %d", current_entity.get_id()));
	network_id_label->set_text(vformat("Network ID: %d", network_id));

	// Authority info
	int mode = network_server->get_entity_authority_mode(current_world, current_entity);
	int32_t auth_peer = network_server->get_entity_authority_peer(current_world, current_entity);
	int32_t owner_peer = network_server->get_entity_owner(current_world, current_entity);
	bool has_authority = network_server->has_authority(current_world, current_entity);

	String mode_str;
	switch (mode) {
		case 0: mode_str = "SERVER"; break;
		case 1: mode_str = "CLIENT"; break;
		case 2: mode_str = "TRANSFERABLE"; break;
		case 3: mode_str = "SHARED"; break;
		default: mode_str = "UNKNOWN"; break;
	}

	authority_mode_label->set_text(vformat("Mode: %s", mode_str));
	authority_peer_label->set_text(vformat("Authority Peer: %d", auth_peer));
	owner_peer_label->set_text(vformat("Owner Peer: %d", owner_peer));
	has_local_authority_check->set_pressed(has_authority);

	// Replicated components
	replicated_components_tree->clear();
	TreeItem *root = replicated_components_tree->create_item();
	root->set_text(0, "Root");

	PackedStringArray components = network_server->get_entity_replicated_components(current_world, current_entity);
	for (int i = 0; i < components.size(); i++) {
		TreeItem *item = replicated_components_tree->create_item(root);
		item->set_text(0, components[i]);
		item->set_text(1, "ON_CHANGE");
		item->set_text(2, "No");
		item->set_text(3, "128");
	}

	// Statistics
	Dictionary stats = network_server->get_entity_network_stats(current_world, current_entity);
	bytes_sent_label->set_text(vformat("Bytes Sent: %d", int64_t(stats.get("bytes_sent", 0))));
	bytes_received_label->set_text(vformat("Bytes Received: %d", int64_t(stats.get("bytes_received", 0))));
	updates_sent_label->set_text(vformat("Updates Sent: %d", int64_t(stats.get("updates_sent", 0))));
	updates_received_label->set_text(vformat("Updates Received: %d", int64_t(stats.get("updates_received", 0))));
	avg_update_interval_label->set_text(vformat("Avg Update Interval: %.2f ms", float(stats.get("avg_update_interval_ms", 0.0f))));
}

void NetworkEntityInspector::_on_add_component_pressed() {
	// TODO: Show dialog to select component to add
}

void NetworkEntityInspector::_on_remove_component_pressed() {
	// TODO: Remove selected component from replication
}

void NetworkEntityInspector::_on_component_selected() {
	// TODO: Show component configuration options
}

void NetworkEntityInspector::set_entity(RID p_world, RID p_entity) {
	current_world = p_world;
	current_entity = p_entity;
	_update_display();
}

void NetworkEntityInspector::clear() {
	current_world = RID();
	current_entity = RID();
	_update_display();
}

void NetworkEntityInspector::refresh() {
	_update_display();
}

// =============================================================================
// NetworkPeersList
// =============================================================================

void NetworkPeersList::_bind_methods() {
}

NetworkPeersList::NetworkPeersList() {
	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}
	_build_ui();
}

NetworkPeersList::~NetworkPeersList() {
}

void NetworkPeersList::_build_ui() {
	// Connection status
	connection_status_label = memnew(Label);
	connection_status_label->set_text("Status: Disconnected");
	connection_status_label->add_theme_font_size_override("font_size", 14);
	add_child(connection_status_label);

	role_label = memnew(Label);
	role_label->set_text("Role: None");
	add_child(role_label);

	local_peer_id_label = memnew(Label);
	local_peer_id_label->set_text("Local Peer ID: -");
	add_child(local_peer_id_label);

	add_child(memnew(HSeparator));

	// Peers tree
	peers_tree = memnew(Tree);
	peers_tree->set_columns(4);
	peers_tree->set_column_titles_visible(true);
	peers_tree->set_column_title(0, "Peer ID");
	peers_tree->set_column_title(1, "Name");
	peers_tree->set_column_title(2, "Latency");
	peers_tree->set_column_title(3, "Status");
	peers_tree->set_custom_minimum_size(Size2(0, 200));
	peers_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	peers_tree->connect("item_selected", callable_mp(this, &NetworkPeersList::_on_peer_selected));
	add_child(peers_tree);

	// Buttons
	HBoxContainer *buttons = memnew(HBoxContainer);

	kick_button = memnew(Button);
	kick_button->set_text("Kick");
	kick_button->set_disabled(true);
	kick_button->connect("pressed", callable_mp(this, &NetworkPeersList::_on_kick_pressed));
	buttons->add_child(kick_button);

	refresh_button = memnew(Button);
	refresh_button->set_text("Refresh");
	refresh_button->connect("pressed", callable_mp(this, &NetworkPeersList::_on_refresh_pressed));
	buttons->add_child(refresh_button);

	add_child(buttons);
}

void NetworkPeersList::_update_display() {
	if (!network_server) {
		connection_status_label->set_text("Status: NetworkServer not available");
		return;
	}

	// Connection status
	NetworkServer::ConnectionState state = network_server->get_connection_state();
	String state_str;
	switch (state) {
		case NetworkServer::STATE_DISCONNECTED: state_str = "Disconnected"; break;
		case NetworkServer::STATE_CONNECTING: state_str = "Connecting..."; break;
		case NetworkServer::STATE_HANDSHAKING: state_str = "Handshaking..."; break;
		case NetworkServer::STATE_CONNECTED: state_str = "Connected"; break;
		case NetworkServer::STATE_DISCONNECTING: state_str = "Disconnecting..."; break;
		default: state_str = "Unknown"; break;
	}
	connection_status_label->set_text(vformat("Status: %s", state_str));

	// Role
	NetworkServer::NetworkRole role = network_server->get_network_role();
	String role_str;
	switch (role) {
		case NetworkServer::ROLE_NONE: role_str = "None"; break;
		case NetworkServer::ROLE_HOST: role_str = "Host (Server)"; break;
		case NetworkServer::ROLE_CLIENT: role_str = "Client"; break;
		default: role_str = "Unknown"; break;
	}
	role_label->set_text(vformat("Role: %s", role_str));

	// Local peer ID
	int32_t local_id = network_server->get_local_peer_id();
	local_peer_id_label->set_text(vformat("Local Peer ID: %d", local_id));

	// Peers tree
	peers_tree->clear();
	TreeItem *root = peers_tree->create_item();
	root->set_text(0, "Peers");

	PackedInt32Array peers = network_server->get_connected_peers();
	for (int i = 0; i < peers.size(); i++) {
		int32_t peer_id = peers[i];
		Dictionary peer_info = network_server->get_peer_info(peer_id);

		TreeItem *item = peers_tree->create_item(root);
		item->set_text(0, vformat("%d", peer_id));
		item->set_text(1, peer_info.get("name", "Unknown"));
		item->set_text(2, vformat("%.1f ms", float(peer_info.get("latency_ms", 0.0f))));
		item->set_text(3, peer_info.get("is_host", false) ? "Host" : "Client");
		item->set_metadata(0, peer_id);
	}

	// Enable/disable kick button
	kick_button->set_disabled(role != NetworkServer::ROLE_HOST);
}

void NetworkPeersList::_on_peer_selected() {
	if (network_server && network_server->is_host()) {
		TreeItem *selected = peers_tree->get_selected();
		if (selected && selected->get_metadata(0).get_type() == Variant::INT) {
			int32_t peer_id = selected->get_metadata(0);
			kick_button->set_disabled(peer_id == 1); // Can't kick the host
		}
	}
}

void NetworkPeersList::_on_kick_pressed() {
	if (!network_server || !network_server->is_host()) {
		return;
	}

	TreeItem *selected = peers_tree->get_selected();
	if (selected && selected->get_metadata(0).get_type() == Variant::INT) {
		int32_t peer_id = selected->get_metadata(0);
		if (peer_id != 1) { // Don't kick host
			network_server->kick_peer(peer_id);
			_update_display();
		}
	}
}

void NetworkPeersList::_on_refresh_pressed() {
	_update_display();
}

void NetworkPeersList::refresh() {
	_update_display();
}

int32_t NetworkPeersList::get_selected_peer_id() const {
	TreeItem *selected = peers_tree->get_selected();
	if (selected && selected->get_metadata(0).get_type() == Variant::INT) {
		return selected->get_metadata(0);
	}
	return -1;
}

// =============================================================================
// NetworkStatsPanel
// =============================================================================

void NetworkStatsPanel::_bind_methods() {
}

NetworkStatsPanel::NetworkStatsPanel() {
	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}

	upload_history.resize(HISTORY_SIZE);
	download_history.resize(HISTORY_SIZE);
	upload_history.fill(0.0f);
	download_history.fill(0.0f);

	_build_ui();
}

NetworkStatsPanel::~NetworkStatsPanel() {
	stop_monitoring();
}

void NetworkStatsPanel::_build_ui() {
	// Global stats
	Label *global_header = memnew(Label);
	global_header->set_text("Global Statistics");
	global_header->add_theme_font_size_override("font_size", 14);
	add_child(global_header);

	tick_label = memnew(Label);
	tick_label->set_text("Current Tick: 0");
	add_child(tick_label);

	tick_rate_label = memnew(Label);
	tick_rate_label->set_text("Tick Rate: 60 Hz");
	add_child(tick_rate_label);

	entities_label = memnew(Label);
	entities_label->set_text("Networked Entities: 0");
	add_child(entities_label);

	worlds_label = memnew(Label);
	worlds_label->set_text("Registered Worlds: 0");
	add_child(worlds_label);

	add_child(memnew(HSeparator));

	// Bandwidth
	bandwidth_label = memnew(Label);
	bandwidth_label->set_text("Bandwidth");
	bandwidth_label->add_theme_font_size_override("font_size", 14);
	add_child(bandwidth_label);

	Label *upload_label = memnew(Label);
	upload_label->set_text("Upload:");
	add_child(upload_label);

	upload_bar = memnew(ProgressBar);
	upload_bar->set_max(100000); // 100 KB/s
	upload_bar->set_value(0);
	upload_bar->set_show_percentage(false);
	add_child(upload_bar);

	Label *download_label = memnew(Label);
	download_label->set_text("Download:");
	add_child(download_label);

	download_bar = memnew(ProgressBar);
	download_bar->set_max(100000); // 100 KB/s
	download_bar->set_value(0);
	download_bar->set_show_percentage(false);
	add_child(download_bar);

	add_child(memnew(HSeparator));

	// Latency
	latency_label = memnew(Label);
	latency_label->set_text("Peer Latency");
	latency_label->add_theme_font_size_override("font_size", 14);
	add_child(latency_label);

	peer_latency_tree = memnew(Tree);
	peer_latency_tree->set_columns(2);
	peer_latency_tree->set_column_titles_visible(true);
	peer_latency_tree->set_column_title(0, "Peer");
	peer_latency_tree->set_column_title(1, "Latency (ms)");
	peer_latency_tree->set_custom_minimum_size(Size2(0, 100));
	add_child(peer_latency_tree);

	// Timer for updates
	update_timer = memnew(Timer);
	update_timer->set_wait_time(0.1); // 10 Hz updates
	update_timer->connect("timeout", callable_mp(this, &NetworkStatsPanel::_on_timer_timeout));
	add_child(update_timer);
}

void NetworkStatsPanel::_update_display() {
	if (!network_server) {
		return;
	}

	Dictionary stats = network_server->get_network_stats();

	tick_label->set_text(vformat("Current Tick: %d", int64_t(stats.get("current_tick", 0))));
	tick_rate_label->set_text(vformat("Tick Rate: %d Hz", int(stats.get("tick_rate", 60))));
	entities_label->set_text(vformat("Networked Entities: %d", int(stats.get("networked_entities", 0))));
	worlds_label->set_text(vformat("Registered Worlds: %d", int(stats.get("registered_worlds", 0))));

	// Update latency tree
	peer_latency_tree->clear();
	TreeItem *root = peer_latency_tree->create_item();

	PackedInt32Array peers = network_server->get_connected_peers();
	for (int i = 0; i < peers.size(); i++) {
		int32_t peer_id = peers[i];
		float latency = network_server->get_peer_latency(peer_id);

		TreeItem *item = peer_latency_tree->create_item(root);
		item->set_text(0, vformat("Peer %d", peer_id));
		item->set_text(1, vformat("%.1f", latency));
	}
}

void NetworkStatsPanel::_on_timer_timeout() {
	_update_display();
}

void NetworkStatsPanel::start_monitoring() {
	if (update_timer) {
		update_timer->start();
	}
}

void NetworkStatsPanel::stop_monitoring() {
	if (update_timer) {
		update_timer->stop();
	}
}

// =============================================================================
// NetworkWorldInspector
// =============================================================================

void NetworkWorldInspector::_bind_methods() {
}

NetworkWorldInspector::NetworkWorldInspector() {
	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}
	_build_ui();
}

NetworkWorldInspector::~NetworkWorldInspector() {
}

void NetworkWorldInspector::_build_ui() {
	// World selection
	HBoxContainer *world_select_container = memnew(HBoxContainer);

	Label *world_label = memnew(Label);
	world_label->set_text("World:");
	world_select_container->add_child(world_label);

	world_selector = memnew(OptionButton);
	world_selector->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	world_selector->connect("item_selected", callable_mp(this, &NetworkWorldInspector::_on_world_selected));
	world_select_container->add_child(world_selector);

	refresh_worlds_button = memnew(Button);
	refresh_worlds_button->set_text("Refresh");
	refresh_worlds_button->connect("pressed", callable_mp(this, &NetworkWorldInspector::_on_refresh_pressed));
	world_select_container->add_child(refresh_worlds_button);

	add_child(world_select_container);

	// Search filter
	HBoxContainer *search_container = memnew(HBoxContainer);

	Label *search_label = memnew(Label);
	search_label->set_text("Filter:");
	search_container->add_child(search_label);

	search_filter = memnew(LineEdit);
	search_filter->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	search_filter->set_placeholder("Search entities...");
	search_filter->connect("text_changed", callable_mp(this, &NetworkWorldInspector::_on_search_changed));
	search_container->add_child(search_filter);

	add_child(search_container);

	// Filter options
	HBoxContainer *filter_options = memnew(HBoxContainer);

	show_local_authority_only = memnew(CheckBox);
	show_local_authority_only->set_text("Local Auth Only");
	filter_options->add_child(show_local_authority_only);

	show_remote_authority_only = memnew(CheckBox);
	show_remote_authority_only->set_text("Remote Auth Only");
	filter_options->add_child(show_remote_authority_only);

	add_child(filter_options);

	// Split container for tree and inspector
	HSplitContainer *split = memnew(HSplitContainer);
	split->set_v_size_flags(Control::SIZE_EXPAND_FILL);

	// Entities tree
	entities_tree = memnew(Tree);
	entities_tree->set_columns(3);
	entities_tree->set_column_titles_visible(true);
	entities_tree->set_column_title(0, "Entity");
	entities_tree->set_column_title(1, "Network ID");
	entities_tree->set_column_title(2, "Authority");
	entities_tree->set_custom_minimum_size(Size2(300, 200));
	entities_tree->connect("item_selected", callable_mp(this, &NetworkWorldInspector::_on_entity_selected));
	split->add_child(entities_tree);

	// Entity inspector
	entity_inspector = memnew(NetworkEntityInspector);
	split->add_child(entity_inspector);

	add_child(split);
}

void NetworkWorldInspector::_update_worlds_list() {
	world_selector->clear();

	if (!network_server) {
		return;
	}

	// Get stats which contains world info
	Dictionary stats = network_server->get_network_stats();
	int world_count = stats.get("registered_worlds", 0);

	if (world_count == 0) {
		world_selector->add_item("No worlds registered", 0);
		world_selector->set_disabled(true);
	} else {
		world_selector->set_disabled(false);
		// Note: We'd need to expose world list from NetworkServer
		// For now, show placeholder
		world_selector->add_item("World 0", 0);
	}
}

void NetworkWorldInspector::_update_entities_tree() {
	entities_tree->clear();
	TreeItem *root = entities_tree->create_item();
	root->set_text(0, "Entities");

	if (!network_server || !current_world.is_valid()) {
		return;
	}

	// Note: We'd need to expose entity list from NetworkServer
	// This is a placeholder showing the structure
}

void NetworkWorldInspector::_on_world_selected(int p_index) {
	// Update current_world based on selection
	_update_entities_tree();
}

void NetworkWorldInspector::_on_entity_selected() {
	TreeItem *selected = entities_tree->get_selected();
	if (selected) {
		Variant meta_world = selected->get_metadata(0);
		Variant meta_entity = selected->get_metadata(1);

		if (meta_world.get_type() == Variant::RID && meta_entity.get_type() == Variant::RID) {
			entity_inspector->set_entity(meta_world, meta_entity);
		}
	}
}

void NetworkWorldInspector::_on_search_changed(const String &p_text) {
	_apply_filter();
}

void NetworkWorldInspector::_on_refresh_pressed() {
	_update_worlds_list();
	_update_entities_tree();
}

void NetworkWorldInspector::_apply_filter() {
	// Filter tree items based on search text
}

void NetworkWorldInspector::refresh() {
	_update_worlds_list();
	_update_entities_tree();
}

void NetworkWorldInspector::set_world(RID p_world) {
	current_world = p_world;
	_update_entities_tree();
}

// =============================================================================
// NetworkConfigPanel
// =============================================================================

void NetworkConfigPanel::_bind_methods() {
}

NetworkConfigPanel::NetworkConfigPanel() {
	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}
	_build_ui();
}

NetworkConfigPanel::~NetworkConfigPanel() {
}

void NetworkConfigPanel::_build_ui() {
	// Connection section
	Label *connection_header = memnew(Label);
	connection_header->set_text("Connection");
	connection_header->add_theme_font_size_override("font_size", 14);
	add_child(connection_header);

	// Address
	HBoxContainer *address_container = memnew(HBoxContainer);
	Label *address_label = memnew(Label);
	address_label->set_text("Address:");
	address_label->set_custom_minimum_size(Size2(100, 0));
	address_container->add_child(address_label);

	address_edit = memnew(LineEdit);
	address_edit->set_text("127.0.0.1");
	address_edit->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	address_container->add_child(address_edit);
	add_child(address_container);

	// Port
	HBoxContainer *port_container = memnew(HBoxContainer);
	Label *port_label = memnew(Label);
	port_label->set_text("Port:");
	port_label->set_custom_minimum_size(Size2(100, 0));
	port_container->add_child(port_label);

	port_spinbox = memnew(SpinBox);
	port_spinbox->set_min(1024);
	port_spinbox->set_max(65535);
	port_spinbox->set_value(7777);
	port_spinbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	port_container->add_child(port_spinbox);
	add_child(port_container);

	// Max clients
	HBoxContainer *max_clients_container = memnew(HBoxContainer);
	Label *max_clients_label = memnew(Label);
	max_clients_label->set_text("Max Clients:");
	max_clients_label->set_custom_minimum_size(Size2(100, 0));
	max_clients_container->add_child(max_clients_label);

	max_clients_spinbox = memnew(SpinBox);
	max_clients_spinbox->set_min(1);
	max_clients_spinbox->set_max(128);
	max_clients_spinbox->set_value(16);
	max_clients_spinbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	max_clients_container->add_child(max_clients_spinbox);
	add_child(max_clients_container);

	// Connection buttons
	HBoxContainer *connection_buttons = memnew(HBoxContainer);

	host_button = memnew(Button);
	host_button->set_text("Host Game");
	host_button->connect("pressed", callable_mp(this, &NetworkConfigPanel::_on_host_pressed));
	connection_buttons->add_child(host_button);

	join_button = memnew(Button);
	join_button->set_text("Join Game");
	join_button->connect("pressed", callable_mp(this, &NetworkConfigPanel::_on_join_pressed));
	connection_buttons->add_child(join_button);

	disconnect_button = memnew(Button);
	disconnect_button->set_text("Disconnect");
	disconnect_button->set_disabled(true);
	disconnect_button->connect("pressed", callable_mp(this, &NetworkConfigPanel::_on_disconnect_pressed));
	connection_buttons->add_child(disconnect_button);

	add_child(connection_buttons);

	add_child(memnew(HSeparator));

	// Runtime settings
	Label *settings_header = memnew(Label);
	settings_header->set_text("Settings");
	settings_header->add_theme_font_size_override("font_size", 14);
	add_child(settings_header);

	// Tick rate
	HBoxContainer *tick_rate_container = memnew(HBoxContainer);
	Label *tick_rate_label = memnew(Label);
	tick_rate_label->set_text("Tick Rate:");
	tick_rate_label->set_custom_minimum_size(Size2(120, 0));
	tick_rate_container->add_child(tick_rate_label);

	tick_rate_spinbox = memnew(SpinBox);
	tick_rate_spinbox->set_min(1);
	tick_rate_spinbox->set_max(128);
	tick_rate_spinbox->set_value(60);
	tick_rate_spinbox->set_suffix(" Hz");
	tick_rate_spinbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	tick_rate_spinbox->connect("value_changed", callable_mp(this, &NetworkConfigPanel::_on_tick_rate_changed));
	tick_rate_container->add_child(tick_rate_spinbox);
	add_child(tick_rate_container);

	// Interpolation delay
	HBoxContainer *interp_container = memnew(HBoxContainer);
	Label *interp_label = memnew(Label);
	interp_label->set_text("Interp. Delay:");
	interp_label->set_custom_minimum_size(Size2(120, 0));
	interp_container->add_child(interp_label);

	interpolation_delay_spinbox = memnew(SpinBox);
	interpolation_delay_spinbox->set_min(0);
	interpolation_delay_spinbox->set_max(500);
	interpolation_delay_spinbox->set_value(100);
	interpolation_delay_spinbox->set_suffix(" ms");
	interpolation_delay_spinbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	interpolation_delay_spinbox->connect("value_changed", callable_mp(this, &NetworkConfigPanel::_on_interpolation_delay_changed));
	interp_container->add_child(interpolation_delay_spinbox);
	add_child(interp_container);

	// Checkboxes
	auto_spawn_checkbox = memnew(CheckBox);
	auto_spawn_checkbox->set_text("Auto Spawn Replicated Entities");
	auto_spawn_checkbox->set_pressed(true);
	auto_spawn_checkbox->connect("toggled", callable_mp(this, &NetworkConfigPanel::_on_auto_spawn_toggled));
	add_child(auto_spawn_checkbox);

	debug_logging_checkbox = memnew(CheckBox);
	debug_logging_checkbox->set_text("Debug Logging");
	debug_logging_checkbox->connect("toggled", callable_mp(this, &NetworkConfigPanel::_on_debug_logging_toggled));
	add_child(debug_logging_checkbox);

	_update_ui_state();
}

void NetworkConfigPanel::_update_ui_state() {
	if (!network_server) {
		host_button->set_disabled(true);
		join_button->set_disabled(true);
		disconnect_button->set_disabled(true);
		return;
	}

	bool connected = network_server->is_connected_to_game();

	host_button->set_disabled(connected);
	join_button->set_disabled(connected);
	disconnect_button->set_disabled(!connected);

	address_edit->set_editable(!connected);
	port_spinbox->set_editable(!connected);
	max_clients_spinbox->set_editable(!connected);

	// Update settings from server
	tick_rate_spinbox->set_value(network_server->get_tick_rate());
	interpolation_delay_spinbox->set_value(network_server->get_interpolation_delay());
	auto_spawn_checkbox->set_pressed(network_server->is_auto_spawn_enabled());
	debug_logging_checkbox->set_pressed(network_server->is_debug_logging_enabled());
}

void NetworkConfigPanel::_on_host_pressed() {
	if (!network_server) {
		return;
	}

	int port = static_cast<int>(port_spinbox->get_value());
	int max_clients = static_cast<int>(max_clients_spinbox->get_value());

	Error err = network_server->host_game(port, max_clients);
	if (err != OK) {
		ERR_PRINT(vformat("Failed to host game: %d", err));
	}

	_update_ui_state();
}

void NetworkConfigPanel::_on_join_pressed() {
	if (!network_server) {
		return;
	}

	String address = address_edit->get_text();
	int port = static_cast<int>(port_spinbox->get_value());

	Error err = network_server->join_game(address, port);
	if (err != OK) {
		ERR_PRINT(vformat("Failed to join game: %d", err));
	}

	_update_ui_state();
}

void NetworkConfigPanel::_on_disconnect_pressed() {
	if (!network_server) {
		return;
	}

	network_server->disconnect_game();
	_update_ui_state();
}

void NetworkConfigPanel::_on_tick_rate_changed(double p_value) {
	if (network_server) {
		network_server->set_tick_rate(static_cast<uint32_t>(p_value));
	}
}

void NetworkConfigPanel::_on_interpolation_delay_changed(double p_value) {
	if (network_server) {
		network_server->set_interpolation_delay(static_cast<float>(p_value));
	}
}

void NetworkConfigPanel::_on_auto_spawn_toggled(bool p_enabled) {
	if (network_server) {
		network_server->set_auto_spawn_enabled(p_enabled);
	}
}

void NetworkConfigPanel::_on_debug_logging_toggled(bool p_enabled) {
	if (network_server) {
		network_server->set_debug_logging(p_enabled);
	}
}

void NetworkConfigPanel::refresh() {
	_update_ui_state();
}

// =============================================================================
// NetworkLogPanel
// =============================================================================

void NetworkLogPanel::_bind_methods() {
}

NetworkLogPanel::NetworkLogPanel() {
	_build_ui();
}

NetworkLogPanel::~NetworkLogPanel() {
}

void NetworkLogPanel::_build_ui() {
	// Filter checkboxes
	HBoxContainer *filter_container = memnew(HBoxContainer);

	show_connections_checkbox = memnew(CheckBox);
	show_connections_checkbox->set_text("Connections");
	show_connections_checkbox->set_pressed(true);
	filter_container->add_child(show_connections_checkbox);

	show_spawns_checkbox = memnew(CheckBox);
	show_spawns_checkbox->set_text("Spawns");
	show_spawns_checkbox->set_pressed(true);
	filter_container->add_child(show_spawns_checkbox);

	show_updates_checkbox = memnew(CheckBox);
	show_updates_checkbox->set_text("Updates");
	show_updates_checkbox->set_pressed(false);
	filter_container->add_child(show_updates_checkbox);

	show_rpcs_checkbox = memnew(CheckBox);
	show_rpcs_checkbox->set_text("RPCs");
	show_rpcs_checkbox->set_pressed(true);
	filter_container->add_child(show_rpcs_checkbox);

	show_errors_checkbox = memnew(CheckBox);
	show_errors_checkbox->set_text("Errors");
	show_errors_checkbox->set_pressed(true);
	filter_container->add_child(show_errors_checkbox);

	add_child(filter_container);

	// Log text
	log_text = memnew(RichTextLabel);
	log_text->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	log_text->set_custom_minimum_size(Size2(0, 200));
	log_text->set_scroll_follow(true);
	log_text->set_selection_enabled(true);
	add_child(log_text);

	// Buttons
	HBoxContainer *buttons = memnew(HBoxContainer);

	clear_button = memnew(Button);
	clear_button->set_text("Clear");
	clear_button->connect("pressed", callable_mp(this, &NetworkLogPanel::_on_clear_pressed));
	buttons->add_child(clear_button);

	auto_scroll_checkbox = memnew(CheckBox);
	auto_scroll_checkbox->set_text("Auto Scroll");
	auto_scroll_checkbox->set_pressed(true);
	buttons->add_child(auto_scroll_checkbox);

	add_child(buttons);
}

void NetworkLogPanel::_on_clear_pressed() {
	clear();
}

void NetworkLogPanel::add_log(const String &p_message, const Color &p_color) {
	if (!log_text) {
		return;
	}

	log_text->push_color(p_color);
	log_text->add_text(vformat("[%s] ", Time::get_singleton()->get_time_string_from_system(false)));
	log_text->add_text(p_message);
	log_text->add_text("\n");
	log_text->pop();

	if (auto_scroll_checkbox && auto_scroll_checkbox->is_pressed()) {
		log_text->scroll_to_line(log_text->get_line_count() - 1);
	}
}

void NetworkLogPanel::add_connection_log(const String &p_message) {
	if (show_connections_checkbox && show_connections_checkbox->is_pressed()) {
		add_log(p_message, Color(0.5, 1.0, 0.5)); // Green
	}
}

void NetworkLogPanel::add_spawn_log(const String &p_message) {
	if (show_spawns_checkbox && show_spawns_checkbox->is_pressed()) {
		add_log(p_message, Color(0.5, 0.5, 1.0)); // Blue
	}
}

void NetworkLogPanel::add_update_log(const String &p_message) {
	if (show_updates_checkbox && show_updates_checkbox->is_pressed()) {
		add_log(p_message, Color(0.8, 0.8, 0.8)); // Gray
	}
}

void NetworkLogPanel::add_rpc_log(const String &p_message) {
	if (show_rpcs_checkbox && show_rpcs_checkbox->is_pressed()) {
		add_log(p_message, Color(1.0, 0.8, 0.5)); // Orange
	}
}

void NetworkLogPanel::add_error_log(const String &p_message) {
	if (show_errors_checkbox && show_errors_checkbox->is_pressed()) {
		add_log(p_message, Color(1.0, 0.3, 0.3)); // Red
	}
}

void NetworkLogPanel::clear() {
	if (log_text) {
		log_text->clear();
	}
}

// =============================================================================
// NetworkEditorPlugin
// =============================================================================

NetworkEditorPlugin *NetworkEditorPlugin::singleton = nullptr;

void NetworkEditorPlugin::_bind_methods() {
}

void NetworkEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_on_enter_tree();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_on_exit_tree();
		} break;
	}
}

NetworkEditorPlugin::NetworkEditorPlugin() {
	singleton = this;

	if (Engine::get_singleton()->has_singleton("NetworkServer")) {
		network_server = Object::cast_to<NetworkServer>(
			Engine::get_singleton()->get_singleton_object("NetworkServer"));
	}
}

NetworkEditorPlugin::~NetworkEditorPlugin() {
	singleton = nullptr;
}

NetworkEditorPlugin *NetworkEditorPlugin::get_singleton() {
	return singleton;
}

void NetworkEditorPlugin::_build_dock() {
	main_dock = memnew(TabContainer);
	main_dock->set_name("Network Inspector");
	main_dock->set_custom_minimum_size(Size2(400, 300));

	// Config tab
	config_panel = memnew(NetworkConfigPanel);
	config_panel->set_name("Config");
	main_dock->add_child(config_panel);

	// Peers tab
	peers_panel = memnew(NetworkPeersList);
	peers_panel->set_name("Peers");
	main_dock->add_child(peers_panel);

	// World Inspector tab
	world_inspector = memnew(NetworkWorldInspector);
	world_inspector->set_name("Entities");
	main_dock->add_child(world_inspector);

	// Stats tab
	stats_panel = memnew(NetworkStatsPanel);
	stats_panel->set_name("Stats");
	main_dock->add_child(stats_panel);

	// Log tab
	log_panel = memnew(NetworkLogPanel);
	log_panel->set_name("Log");
	main_dock->add_child(log_panel);

	add_control_to_dock(DOCK_SLOT_RIGHT_UL, main_dock);
}

void NetworkEditorPlugin::_on_enter_tree() {
	_build_dock();

	// Connect to NetworkServer signals
	if (network_server) {
		network_server->connect("peer_connected", callable_mp(this, &NetworkEditorPlugin::_on_peer_connected));
		network_server->connect("peer_disconnected", callable_mp(this, &NetworkEditorPlugin::_on_peer_disconnected));
		network_server->connect("connection_succeeded", callable_mp(this, &NetworkEditorPlugin::_on_connection_succeeded));
		network_server->connect("connection_failed", callable_mp(this, &NetworkEditorPlugin::_on_connection_failed));
		network_server->connect("server_started", callable_mp(this, &NetworkEditorPlugin::_on_server_started));
		network_server->connect("server_stopped", callable_mp(this, &NetworkEditorPlugin::_on_server_stopped));
		network_server->connect("entity_spawned_remote", callable_mp(this, &NetworkEditorPlugin::_on_entity_spawned_remote));
		network_server->connect("entity_despawned_remote", callable_mp(this, &NetworkEditorPlugin::_on_entity_despawned_remote));
		network_server->connect("authority_changed", callable_mp(this, &NetworkEditorPlugin::_on_authority_changed));
	}

	// Create refresh timer
	refresh_timer = memnew(Timer);
	refresh_timer->set_wait_time(1.0);
	refresh_timer->set_autostart(true);
	refresh_timer->connect("timeout", callable_mp(this, &NetworkEditorPlugin::_on_refresh_timer_timeout));
	add_child(refresh_timer);

	stats_panel->start_monitoring();
}

void NetworkEditorPlugin::_on_exit_tree() {
	stats_panel->stop_monitoring();

	if (refresh_timer) {
		refresh_timer->stop();
		refresh_timer->queue_free();
		refresh_timer = nullptr;
	}

	if (main_dock) {
		remove_control_from_docks(main_dock);
		main_dock->queue_free();
		main_dock = nullptr;
	}
}

void NetworkEditorPlugin::_on_refresh_timer_timeout() {
	if (config_panel) {
		config_panel->refresh();
	}
	if (peers_panel) {
		peers_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_peer_connected(int32_t p_peer_id) {
	if (log_panel) {
		log_panel->add_connection_log(vformat("Peer %d connected", p_peer_id));
	}
	if (peers_panel) {
		peers_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_peer_disconnected(int32_t p_peer_id, int p_reason) {
	if (log_panel) {
		log_panel->add_connection_log(vformat("Peer %d disconnected (reason: %d)", p_peer_id, p_reason));
	}
	if (peers_panel) {
		peers_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_connection_succeeded() {
	if (log_panel) {
		log_panel->add_connection_log("Connection succeeded!");
	}
	if (config_panel) {
		config_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_connection_failed(const String &p_reason) {
	if (log_panel) {
		log_panel->add_error_log(vformat("Connection failed: %s", p_reason));
	}
	if (config_panel) {
		config_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_server_started() {
	if (log_panel) {
		log_panel->add_connection_log("Server started");
	}
	if (config_panel) {
		config_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_server_stopped() {
	if (log_panel) {
		log_panel->add_connection_log("Server stopped");
	}
	if (config_panel) {
		config_panel->refresh();
	}
}

void NetworkEditorPlugin::_on_entity_spawned_remote(int64_t p_network_id, RID p_entity) {
	if (log_panel) {
		log_panel->add_spawn_log(vformat("Remote entity spawned: Network ID %d", p_network_id));
	}
	if (world_inspector) {
		world_inspector->refresh();
	}
}

void NetworkEditorPlugin::_on_entity_despawned_remote(int64_t p_network_id) {
	if (log_panel) {
		log_panel->add_spawn_log(vformat("Remote entity despawned: Network ID %d", p_network_id));
	}
	if (world_inspector) {
		world_inspector->refresh();
	}
}

void NetworkEditorPlugin::_on_authority_changed(int64_t p_network_id, int32_t p_new_authority) {
	if (log_panel) {
		log_panel->add_log(vformat("Authority changed: Entity %d -> Peer %d", p_network_id, p_new_authority), 
						   Color(1.0, 1.0, 0.5)); // Yellow
	}
}

void NetworkEditorPlugin::refresh_all() {
	if (config_panel) {
		config_panel->refresh();
	}
	if (peers_panel) {
		peers_panel->refresh();
	}
	if (world_inspector) {
		world_inspector->refresh();
	}
}