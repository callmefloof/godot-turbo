/**************************************************************************/
/*  network_server.cpp                                                    */
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

#include "network_server.h"

#include "core/config/engine.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "core/variant/variant.h"
#include "scene/main/multiplayer_peer.h"

#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

// Global singleton pointer
NetworkServer *p_network_server = nullptr;
NetworkServer *NetworkServer::singleton = nullptr;

// =============================================================================
// BINDING
// =============================================================================

void NetworkServer::_bind_methods() {
	// Connection Management
	ClassDB::bind_method(D_METHOD("host_game", "port", "max_clients", "bind_address"), &NetworkServer::host_game, DEFVAL(16), DEFVAL(""));
	ClassDB::bind_method(D_METHOD("join_game", "address", "port"), &NetworkServer::join_game);
	ClassDB::bind_method(D_METHOD("disconnect_game", "reason"), &NetworkServer::disconnect_game, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("set_multiplayer_peer", "peer"), &NetworkServer::set_multiplayer_peer);
	ClassDB::bind_method(D_METHOD("get_multiplayer_peer"), &NetworkServer::get_multiplayer_peer);
	ClassDB::bind_method(D_METHOD("is_host"), &NetworkServer::is_host);
	ClassDB::bind_method(D_METHOD("is_connected_to_game"), &NetworkServer::is_connected_to_game);
	ClassDB::bind_method(D_METHOD("get_local_peer_id"), &NetworkServer::get_local_peer_id);
	ClassDB::bind_method(D_METHOD("get_network_role"), &NetworkServer::get_network_role);
	ClassDB::bind_method(D_METHOD("get_connection_state"), &NetworkServer::get_connection_state);

	// Peer Management
	ClassDB::bind_method(D_METHOD("get_connected_peers"), &NetworkServer::get_connected_peers);
	ClassDB::bind_method(D_METHOD("get_peer_info", "peer_id"), &NetworkServer::get_peer_info);
	ClassDB::bind_method(D_METHOD("kick_peer", "peer_id", "reason"), &NetworkServer::kick_peer, DEFVAL(3));
	ClassDB::bind_method(D_METHOD("get_peer_latency", "peer_id"), &NetworkServer::get_peer_latency);

	// World Management
	ClassDB::bind_method(D_METHOD("register_world", "world"), &NetworkServer::register_world);
	ClassDB::bind_method(D_METHOD("unregister_world", "world"), &NetworkServer::unregister_world);
	ClassDB::bind_method(D_METHOD("is_world_registered", "world"), &NetworkServer::is_world_registered);

	// Entity Networking
	ClassDB::bind_method(D_METHOD("register_networked_entity", "world", "entity", "spawn_scene", "spawn_data"), 
						 &NetworkServer::register_networked_entity, DEFVAL(""), DEFVAL(Dictionary()));
	ClassDB::bind_method(D_METHOD("unregister_networked_entity", "world", "entity"), &NetworkServer::unregister_networked_entity);
	ClassDB::bind_method(D_METHOD("get_entity_network_id", "world", "entity"), &NetworkServer::get_entity_network_id);
	ClassDB::bind_method(D_METHOD("get_entity_by_network_id", "world", "network_id"), &NetworkServer::get_entity_by_network_id);
	ClassDB::bind_method(D_METHOD("is_entity_networked", "world", "entity"), &NetworkServer::is_entity_networked);

	// Replication Configuration
	ClassDB::bind_method(D_METHOD("set_entity_replicated_components", "world", "entity", "components"), 
						 &NetworkServer::set_entity_replicated_components);
	ClassDB::bind_method(D_METHOD("get_entity_replicated_components", "world", "entity"), 
						 &NetworkServer::get_entity_replicated_components);
	ClassDB::bind_method(D_METHOD("configure_component_replication", "world", "entity", "component", "mode", "interpolate", "priority"),
						 &NetworkServer::configure_component_replication, DEFVAL(false), DEFVAL(128));

	// Authority Management
	ClassDB::bind_method(D_METHOD("set_entity_authority", "world", "entity", "mode", "authority_peer"), 
						 &NetworkServer::set_entity_authority, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("get_entity_authority_mode", "world", "entity"), &NetworkServer::get_entity_authority_mode);
	ClassDB::bind_method(D_METHOD("get_entity_authority_peer", "world", "entity"), &NetworkServer::get_entity_authority_peer);
	ClassDB::bind_method(D_METHOD("has_authority", "world", "entity"), &NetworkServer::has_authority);
	ClassDB::bind_method(D_METHOD("request_authority", "world", "entity"), &NetworkServer::request_authority);
	ClassDB::bind_method(D_METHOD("release_authority", "world", "entity"), &NetworkServer::release_authority);
	ClassDB::bind_method(D_METHOD("set_entity_owner", "world", "entity", "owner_peer"), &NetworkServer::set_entity_owner);
	ClassDB::bind_method(D_METHOD("get_entity_owner", "world", "entity"), &NetworkServer::get_entity_owner);

	// Input & Prediction
	ClassDB::bind_method(D_METHOD("send_input", "world", "entity", "input"), &NetworkServer::send_input);
	ClassDB::bind_method(D_METHOD("get_input_tick"), &NetworkServer::get_input_tick);
	ClassDB::bind_method(D_METHOD("set_interpolation_delay", "delay_ms"), &NetworkServer::set_interpolation_delay);
	ClassDB::bind_method(D_METHOD("get_interpolation_delay"), &NetworkServer::get_interpolation_delay);

	// RPC
	ClassDB::bind_method(D_METHOD("entity_rpc", "world", "entity", "method", "args", "target", "reliable"),
						 &NetworkServer::entity_rpc, DEFVAL(Array()), DEFVAL(0), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("set_rpc_callback", "callback"), &NetworkServer::set_rpc_callback);
	ClassDB::bind_method(D_METHOD("set_spawn_callback", "callback"), &NetworkServer::set_spawn_callback);
	ClassDB::bind_method(D_METHOD("set_despawn_callback", "callback"), &NetworkServer::set_despawn_callback);

	// Tick & Processing
	ClassDB::bind_method(D_METHOD("network_process", "delta"), &NetworkServer::network_process);
	ClassDB::bind_method(D_METHOD("get_current_tick"), &NetworkServer::get_current_tick);
	ClassDB::bind_method(D_METHOD("get_server_tick"), &NetworkServer::get_server_tick);
	ClassDB::bind_method(D_METHOD("set_tick_rate", "rate"), &NetworkServer::set_tick_rate);
	ClassDB::bind_method(D_METHOD("get_tick_rate"), &NetworkServer::get_tick_rate);

	// Configuration
	ClassDB::bind_method(D_METHOD("set_auto_spawn_enabled", "enabled"), &NetworkServer::set_auto_spawn_enabled);
	ClassDB::bind_method(D_METHOD("is_auto_spawn_enabled"), &NetworkServer::is_auto_spawn_enabled);
	ClassDB::bind_method(D_METHOD("set_debug_logging", "enabled"), &NetworkServer::set_debug_logging);
	ClassDB::bind_method(D_METHOD("is_debug_logging_enabled"), &NetworkServer::is_debug_logging_enabled);

	// Statistics
	ClassDB::bind_method(D_METHOD("get_network_stats"), &NetworkServer::get_network_stats);
	ClassDB::bind_method(D_METHOD("get_entity_network_stats", "world", "entity"), &NetworkServer::get_entity_network_stats);
	ClassDB::bind_method(D_METHOD("reset_network_stats"), &NetworkServer::reset_network_stats);

	// Signals
	ADD_SIGNAL(MethodInfo("peer_connected", PropertyInfo(Variant::INT, "peer_id")));
	ADD_SIGNAL(MethodInfo("peer_disconnected", PropertyInfo(Variant::INT, "peer_id"), PropertyInfo(Variant::INT, "reason")));
	ADD_SIGNAL(MethodInfo("connection_succeeded"));
	ADD_SIGNAL(MethodInfo("connection_failed", PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("server_started"));
	ADD_SIGNAL(MethodInfo("server_stopped"));
	ADD_SIGNAL(MethodInfo("entity_spawned_remote", PropertyInfo(Variant::INT, "network_id"), PropertyInfo(Variant::RID, "entity_rid")));
	ADD_SIGNAL(MethodInfo("entity_despawned_remote", PropertyInfo(Variant::INT, "network_id")));
	ADD_SIGNAL(MethodInfo("authority_changed", PropertyInfo(Variant::INT, "network_id"), PropertyInfo(Variant::INT, "new_authority")));

	// Enums
	BIND_ENUM_CONSTANT(ROLE_NONE);
	BIND_ENUM_CONSTANT(ROLE_HOST);
	BIND_ENUM_CONSTANT(ROLE_CLIENT);

	BIND_ENUM_CONSTANT(STATE_DISCONNECTED);
	BIND_ENUM_CONSTANT(STATE_CONNECTING);
	BIND_ENUM_CONSTANT(STATE_HANDSHAKING);
	BIND_ENUM_CONSTANT(STATE_CONNECTED);
	BIND_ENUM_CONSTANT(STATE_DISCONNECTING);
}

// =============================================================================
// CONSTRUCTOR / DESTRUCTOR
// =============================================================================

NetworkServer::NetworkServer() {
	singleton = this;
	
	// Get FlecsServer reference
	if (Engine::get_singleton()->has_singleton("FlecsServer")) {
		flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));
	}
}

NetworkServer::~NetworkServer() {
	if (connection_state != STATE_DISCONNECTED) {
		disconnect_game(static_cast<int>(NetworkTypes::DisconnectReason::GRACEFUL));
	}
	
	singleton = nullptr;
}

NetworkServer *NetworkServer::get_singleton() {
	return singleton;
}

// =============================================================================
// CONNECTION MANAGEMENT
// =============================================================================

Error NetworkServer::host_game(int p_port, int p_max_clients, const String &p_bind_address) {
	if (connection_state != STATE_DISCONNECTED) {
		ERR_PRINT("NetworkServer: Already connected. Disconnect first.");
		return ERR_ALREADY_IN_USE;
	}

	// Create multiplayer peer using SceneTree's multiplayer API
	// The user must set a peer before calling host_game, or we create a default one
	if (!multiplayer_peer.is_valid()) {
		// Try to create ENetMultiplayerPeer via ClassDB
		Ref<MultiplayerPeer> peer = Ref<MultiplayerPeer>(ClassDB::instantiate("ENetMultiplayerPeer"));
		if (peer.is_null()) {
			ERR_PRINT("NetworkServer: Failed to instantiate ENetMultiplayerPeer. Make sure the ENet module is enabled.");
			return ERR_UNAVAILABLE;
		}
		
		// Call create_server via Variant call
		Array args;
		args.push_back(p_port);
		args.push_back(p_max_clients);
		
		Callable create_method = Callable(peer.ptr(), "create_server");
		Variant result = create_method.callv(args);
		Error err = static_cast<Error>(int(result));
		
		if (err != OK) {
			ERR_PRINT(vformat("NetworkServer: Failed to create server on port %d: %d", p_port, (int)err));
			return err;
		}
		
		multiplayer_peer = peer;
	}

	role = ROLE_HOST;
	connection_state = STATE_CONNECTED;
	local_peer_id = 1; // Host is always peer 1
	current_tick = 0;

	// Connect peer signals
	multiplayer_peer->connect("peer_connected", callable_mp(this, &NetworkServer::_on_peer_connected));
	multiplayer_peer->connect("peer_disconnected", callable_mp(this, &NetworkServer::_on_peer_disconnected));

	if (debug_logging) {
		print_line(vformat("NetworkServer: Server started on port %d", p_port));
	}

	emit_signal(SNAME("server_started"));
	return OK;
}

Error NetworkServer::join_game(const String &p_address, int p_port) {
	if (connection_state != STATE_DISCONNECTED) {
		ERR_PRINT("NetworkServer: Already connected. Disconnect first.");
		return ERR_ALREADY_IN_USE;
	}

	// Create multiplayer peer using ClassDB
	if (!multiplayer_peer.is_valid()) {
		Ref<MultiplayerPeer> peer = Ref<MultiplayerPeer>(ClassDB::instantiate("ENetMultiplayerPeer"));
		if (peer.is_null()) {
			ERR_PRINT("NetworkServer: Failed to instantiate ENetMultiplayerPeer. Make sure the ENet module is enabled.");
			emit_signal(SNAME("connection_failed"), String("ENet module not available"));
			return ERR_UNAVAILABLE;
		}
		
		// Call create_client via Variant call
		Array args;
		args.push_back(p_address);
		args.push_back(p_port);
		
		Callable create_method = Callable(peer.ptr(), "create_client");
		Variant result = create_method.callv(args);
		Error err = static_cast<Error>(int(result));
		
		if (err != OK) {
			ERR_PRINT(vformat("NetworkServer: Failed to connect to %s:%d: %d", p_address, p_port, (int)err));
			emit_signal(SNAME("connection_failed"), vformat("Failed to connect: error %d", (int)err));
			return err;
		}
		
		multiplayer_peer = peer;
	}

	role = ROLE_CLIENT;
	connection_state = STATE_CONNECTING;

	// Connect peer signals
	multiplayer_peer->connect("peer_connected", callable_mp(this, &NetworkServer::_on_peer_connected));
	multiplayer_peer->connect("peer_disconnected", callable_mp(this, &NetworkServer::_on_peer_disconnected));

	if (debug_logging) {
		print_line(vformat("NetworkServer: Connecting to %s:%d...", p_address, p_port));
	}

	return OK;
}

void NetworkServer::disconnect_game(int p_reason) {
	if (connection_state == STATE_DISCONNECTED) {
		return;
	}

	NetworkTypes::DisconnectReason reason = static_cast<NetworkTypes::DisconnectReason>(p_reason);
	connection_state = STATE_DISCONNECTING;

	// Send disconnect message to all peers
	if (multiplayer_peer.is_valid() && multiplayer_peer->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED) {
		Dictionary disconnect_data;
		disconnect_data["reason"] = p_reason;
		_broadcast(NetworkTypes::PacketType::DISCONNECT, disconnect_data, NetworkTypes::TransferMode::RELIABLE);
	}

	// Clean up
	if (multiplayer_peer.is_valid()) {
		multiplayer_peer->close();
		multiplayer_peer.unref();
	}

	// Clear state
	connected_peers.clear();
	world_network_data.clear();
	outgoing_reliable.clear();
	outgoing_unreliable.clear();
	
	bool was_host = (role == ROLE_HOST);
	role = ROLE_NONE;
	connection_state = STATE_DISCONNECTED;
	local_peer_id = 0;
	current_tick = 0;
	server_tick = 0;

	if (debug_logging) {
		print_line("NetworkServer: Disconnected");
	}

	if (was_host) {
		emit_signal(SNAME("server_stopped"));
	}
}

void NetworkServer::set_multiplayer_peer(const Ref<MultiplayerPeer> &p_peer) {
	if (connection_state != STATE_DISCONNECTED) {
		disconnect_game();
	}
	
	multiplayer_peer = p_peer;
	
	if (multiplayer_peer.is_valid()) {
		// Connect signals
		if (!multiplayer_peer->is_connected("peer_connected", callable_mp(this, &NetworkServer::_on_peer_connected))) {
			multiplayer_peer->connect("peer_connected", callable_mp(this, &NetworkServer::_on_peer_connected));
		}
		if (!multiplayer_peer->is_connected("peer_disconnected", callable_mp(this, &NetworkServer::_on_peer_disconnected))) {
			multiplayer_peer->connect("peer_disconnected", callable_mp(this, &NetworkServer::_on_peer_disconnected));
		}
	}
}

Ref<MultiplayerPeer> NetworkServer::get_multiplayer_peer() const {
	return multiplayer_peer;
}

bool NetworkServer::is_host() const {
	return role == ROLE_HOST;
}

bool NetworkServer::is_connected_to_game() const {
	return connection_state == STATE_CONNECTED;
}

int32_t NetworkServer::get_local_peer_id() const {
	return local_peer_id;
}

NetworkServer::NetworkRole NetworkServer::get_network_role() const {
	return role;
}

NetworkServer::ConnectionState NetworkServer::get_connection_state() const {
	return connection_state;
}

// =============================================================================
// PEER MANAGEMENT
// =============================================================================

PackedInt32Array NetworkServer::get_connected_peers() const {
	PackedInt32Array result;
	for (const KeyValue<int32_t, NetworkTypes::PeerInfo> &E : connected_peers) {
		result.push_back(E.key);
	}
	return result;
}

Dictionary NetworkServer::get_peer_info(int32_t p_peer_id) const {
	if (!connected_peers.has(p_peer_id)) {
		return Dictionary();
	}
	return connected_peers[p_peer_id].to_dict();
}

void NetworkServer::kick_peer(int32_t p_peer_id, int p_reason) {
	if (role != ROLE_HOST) {
		ERR_PRINT("NetworkServer: Only host can kick peers");
		return;
	}

	if (!connected_peers.has(p_peer_id)) {
		ERR_PRINT(vformat("NetworkServer: Peer %d not found", p_peer_id));
		return;
	}

	Dictionary kick_data;
	kick_data["reason"] = p_reason;
	_send_to_peer(p_peer_id, NetworkTypes::PacketType::DISCONNECT, kick_data, NetworkTypes::TransferMode::RELIABLE);

	if (multiplayer_peer.is_valid()) {
		multiplayer_peer->disconnect_peer(p_peer_id);
	}

	connected_peers.erase(p_peer_id);
	emit_signal(SNAME("peer_disconnected"), p_peer_id, p_reason);
}

float NetworkServer::get_peer_latency(int32_t p_peer_id) const {
	if (!connected_peers.has(p_peer_id)) {
		return -1.0f;
	}
	return connected_peers[p_peer_id].latency_ms;
}

// =============================================================================
// WORLD MANAGEMENT
// =============================================================================

void NetworkServer::register_world(RID p_world) {
	if (world_network_data.has(p_world)) {
		WARN_PRINT("NetworkServer: World already registered");
		return;
	}

	WorldNetworkData data;
	data.world_rid = p_world;
	world_network_data[p_world] = data;

	if (debug_logging) {
		print_line(vformat("NetworkServer: Registered world %d", p_world.get_id()));
	}
}

void NetworkServer::unregister_world(RID p_world) {
	if (!world_network_data.has(p_world)) {
		return;
	}

	// Clean up entity mappings
	world_network_data.erase(p_world);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Unregistered world %d", p_world.get_id()));
	}
}

bool NetworkServer::is_world_registered(RID p_world) const {
	return world_network_data.has(p_world);
}

// =============================================================================
// ENTITY NETWORKING
// =============================================================================

uint64_t NetworkServer::register_networked_entity(RID p_world, RID p_entity,
												  const String &p_spawn_scene,
												  const Dictionary &p_spawn_data) {
	if (!world_network_data.has(p_world)) {
		ERR_PRINT("NetworkServer: World not registered for networking");
		return 0;
	}

	WorldNetworkData &world_data = world_network_data[p_world];

	// Check if already registered
	if (world_data.local_to_network.has(p_entity)) {
		return world_data.local_to_network[p_entity];
	}

	// Generate network ID (only host can generate)
	uint64_t network_id = 0;
	if (role == ROLE_HOST || role == ROLE_NONE) {
		network_id = _generate_network_id();
	} else {
		// Clients must wait for server to assign ID
		ERR_PRINT("NetworkServer: Only host can register new networked entities");
		return 0;
	}

	// Store mappings
	world_data.network_to_local[network_id] = p_entity;
	world_data.local_to_network[p_entity] = network_id;

	// Add NetworkIdentity component to entity
	if (flecs_server) {
		Dictionary identity_data;
		identity_data["network_id"] = (int64_t)network_id;
		identity_data["is_network_spawned"] = true;
		identity_data["spawn_tick"] = (int64_t)current_tick;
		identity_data["spawn_scene_path"] = Variant(p_spawn_scene);
		identity_data["spawn_data"] = p_spawn_data;
		
		flecs_server->set_component(p_entity, "NetworkIdentity", identity_data);
		
		// Add default NetworkAuthority (server authority)
		Dictionary authority_data;
		authority_data["mode"] = static_cast<int>(NetworkComponents::AuthorityMode::SERVER);
		authority_data["authority_peer_id"] = 1;
		authority_data["owner_peer_id"] = local_peer_id;
		authority_data["is_local_authority"] = is_host();
		authority_data["authority_change_tick"] = current_tick;
		
		flecs_server->set_component(p_entity, "NetworkAuthority", authority_data);
		
		// Add empty NetworkReplicated component
		Dictionary replicated_data;
		replicated_data["is_active"] = true;
		replicated_data["relevancy_radius"] = -1.0f;
		replicated_data["replicated_components"] = Array();
		
		flecs_server->set_component(p_entity, "NetworkReplicated", replicated_data);
	}

	// Broadcast spawn to other peers (if host and connected)
	if (role == ROLE_HOST && connection_state == STATE_CONNECTED) {
		NetworkTypes::EntitySpawnMessage spawn_msg;
		spawn_msg.network_id = network_id;
		spawn_msg.owner_peer_id = local_peer_id;
		spawn_msg.authority_peer_id = 1;
		spawn_msg.spawn_scene_path = p_spawn_scene;
		spawn_msg.spawn_data = p_spawn_data;
		
		// Get initial component states
		if (flecs_server) {
			PackedStringArray comp_names = flecs_server->get_component_types_as_name(p_entity);
			Dictionary initial_components;
			for (int i = 0; i < comp_names.size(); i++) {
				Dictionary comp_data = flecs_server->get_component_by_name(p_entity, comp_names[i]);
				if (!comp_data.is_empty()) {
					initial_components[comp_names[i]] = comp_data;
				}
			}
			spawn_msg.initial_components = initial_components;
			}
		
			_broadcast(NetworkTypes::PacketType::ENTITY_SPAWN, spawn_msg.to_dict(), 
					   NetworkTypes::TransferMode::RELIABLE_ORDERED);
		}

		if (debug_logging) {
			print_line(vformat("NetworkServer: Registered entity %d with network ID %d", 
							   p_entity.get_id(), (int)network_id));
		}

	return network_id;
}

void NetworkServer::unregister_networked_entity(RID p_world, RID p_entity) {
	if (!world_network_data.has(p_world)) {
		return;
	}

	WorldNetworkData &world_data = world_network_data[p_world];

	if (!world_data.local_to_network.has(p_entity)) {
		return;
	}

	uint64_t network_id = world_data.local_to_network[p_entity];

	// Broadcast despawn (if host)
	if (role == ROLE_HOST && connection_state == STATE_CONNECTED) {
		NetworkTypes::EntityDespawnMessage despawn_msg;
		despawn_msg.network_id = network_id;
		despawn_msg.reason = NetworkTypes::DisconnectReason::GRACEFUL;
		
		_broadcast(NetworkTypes::PacketType::ENTITY_DESPAWN, despawn_msg.to_dict(),
				   NetworkTypes::TransferMode::RELIABLE_ORDERED);
	}

	// Remove mappings
	world_data.network_to_local.erase(network_id);
	world_data.local_to_network.erase(p_entity);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Unregistered entity with network ID %d", (int)network_id));
	}
}

uint64_t NetworkServer::get_entity_network_id(RID p_world, RID p_entity) const {
	if (!world_network_data.has(p_world)) {
		return 0;
	}

	const WorldNetworkData &world_data = world_network_data[p_world];
	if (!world_data.local_to_network.has(p_entity)) {
		return 0;
	}

	return world_data.local_to_network[p_entity];
}

RID NetworkServer::get_entity_by_network_id(RID p_world, uint64_t p_network_id) const {
	if (!world_network_data.has(p_world)) {
		return RID();
	}

	const WorldNetworkData &world_data = world_network_data[p_world];
	if (!world_data.network_to_local.has(p_network_id)) {
		return RID();
	}

	return world_data.network_to_local[p_network_id];
}

bool NetworkServer::is_entity_networked(RID p_world, RID p_entity) const {
	if (!world_network_data.has(p_world)) {
		return false;
	}

	const WorldNetworkData &world_data = world_network_data[p_world];
	return world_data.local_to_network.has(p_entity);
}

// =============================================================================
// REPLICATION CONFIGURATION
// =============================================================================

void NetworkServer::set_entity_replicated_components(RID p_world, RID p_entity,
													 const PackedStringArray &p_components) {
	if (!flecs_server) {
		return;
	}

	Dictionary replicated_data = flecs_server->get_component_by_name(p_entity, "NetworkReplicated");
	if (replicated_data.is_empty()) {
		replicated_data["is_active"] = true;
		replicated_data["relevancy_radius"] = -1.0f;
	}

	Array comp_configs;
	for (int i = 0; i < p_components.size(); i++) {
		Dictionary config;
		config["component_name"] = p_components[i];
		config["mode"] = static_cast<int>(NetworkComponents::ReplicationMode::ON_CHANGE);
		config["priority"] = 128;
		config["interpolate"] = false;
		config["min_update_interval_ms"] = 0;
		comp_configs.push_back(config);
	}
	
	replicated_data["replicated_components"] = comp_configs;
flecs_server->set_component(p_entity, StringName("NetworkReplicated"), replicated_data);
}

PackedStringArray NetworkServer::get_entity_replicated_components(RID p_world, RID p_entity) const {
PackedStringArray result;
	
if (!flecs_server) {
	return result;
}

Dictionary replicated_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkReplicated"));
	if (replicated_data.is_empty()) {
		return result;
	}

	Array comp_configs = replicated_data.get("replicated_components", Array());
	for (int i = 0; i < comp_configs.size(); i++) {
		Dictionary config = comp_configs[i];
		result.push_back(String(config.get("component_name", "")));
	}

	return result;
}

void NetworkServer::configure_component_replication(RID p_world, RID p_entity,
													const StringName &p_component,
													int p_mode, bool p_interpolate,
													int p_priority) {
	if (!flecs_server) {
		return;
	}

	Dictionary replicated_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkReplicated"));
	if (replicated_data.is_empty()) {
		replicated_data["is_active"] = true;
		replicated_data["relevancy_radius"] = -1.0f;
		replicated_data["replicated_components"] = Array();
	}

	Array comp_configs = replicated_data.get("replicated_components", Array());
	
	// Find or create config for this component
	bool found = false;
	for (int i = 0; i < comp_configs.size(); i++) {
		Dictionary config = comp_configs[i];
		if (StringName(config.get("component_name", "")) == p_component) {
			config["mode"] = p_mode;
			config["interpolate"] = p_interpolate;
			config["priority"] = p_priority;
			comp_configs[i] = config;
			found = true;
			break;
		}
	}

	if (!found) {
		Dictionary config;
		config["component_name"] = p_component;
		config["mode"] = p_mode;
		config["interpolate"] = p_interpolate;
		config["priority"] = p_priority;
		config["min_update_interval_ms"] = 0;
		comp_configs.push_back(config);
	}

	replicated_data["replicated_components"] = comp_configs;
	flecs_server->set_component(p_entity, StringName("NetworkReplicated"), replicated_data);
}

// =============================================================================
// AUTHORITY MANAGEMENT
// =============================================================================

void NetworkServer::set_entity_authority(RID p_world, RID p_entity, int p_mode, int32_t p_authority_peer) {
	if (!flecs_server) {
		return;
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	if (authority_data.is_empty()) {
		authority_data["owner_peer_id"] = local_peer_id;
	}

	int32_t old_authority = authority_data.get("authority_peer_id", 1);

	authority_data["mode"] = p_mode;
	authority_data["authority_peer_id"] = p_authority_peer;
	authority_data["is_local_authority"] = (p_authority_peer == local_peer_id);
	authority_data["authority_change_tick"] = current_tick;

	flecs_server->set_component(p_entity, StringName("NetworkAuthority"), authority_data);

	// Broadcast authority change (if host)
	if (role == ROLE_HOST && connection_state == STATE_CONNECTED) {
		uint64_t network_id = get_entity_network_id(p_world, p_entity);
		if (network_id != 0) {
			NetworkTypes::AuthorityChangeMessage auth_msg;
			auth_msg.network_id = network_id;
			auth_msg.new_authority_peer_id = p_authority_peer;
			auth_msg.old_authority_peer_id = old_authority;
			auth_msg.change_tick = current_tick;
			
			_broadcast(NetworkTypes::PacketType::ENTITY_AUTHORITY_CHANGE, auth_msg.to_dict(),
					   NetworkTypes::TransferMode::RELIABLE_ORDERED);
		}
	}
}

int NetworkServer::get_entity_authority_mode(RID p_world, RID p_entity) const {
	if (!flecs_server) {
		return static_cast<int>(NetworkComponents::AuthorityMode::SERVER);
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	return authority_data.get("mode", static_cast<int>(NetworkComponents::AuthorityMode::SERVER));
}

int32_t NetworkServer::get_entity_authority_peer(RID p_world, RID p_entity) const {
	if (!flecs_server) {
		return 1;
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	return authority_data.get("authority_peer_id", 1);
}

bool NetworkServer::has_authority(RID p_world, RID p_entity) const {
	if (role == ROLE_NONE) {
		return true; // Single-player mode
	}

	if (!flecs_server) {
		return false;
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	return authority_data.get("is_local_authority", false);
}

void NetworkServer::request_authority(RID p_world, RID p_entity) {
	if (role != ROLE_CLIENT) {
		return;
	}

	uint64_t network_id = get_entity_network_id(p_world, p_entity);
	if (network_id == 0) {
		return;
	}

	// TODO: Implement authority request to server
	Dictionary request;
	request["network_id"] = network_id;
	request["requesting_peer"] = local_peer_id;
	
	_send_to_server(NetworkTypes::PacketType::ENTITY_AUTHORITY_CHANGE, request,
					NetworkTypes::TransferMode::RELIABLE);
}

void NetworkServer::release_authority(RID p_world, RID p_entity) {
	if (!has_authority(p_world, p_entity)) {
		return;
	}

	// Transfer authority back to server
	set_entity_authority(p_world, p_entity, 
						 static_cast<int>(NetworkComponents::AuthorityMode::SERVER), 1);
}

void NetworkServer::set_entity_owner(RID p_world, RID p_entity, int32_t p_owner_peer) {
	if (!flecs_server) {
		return;
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	authority_data["owner_peer_id"] = p_owner_peer;
	flecs_server->set_component(p_entity, StringName("NetworkAuthority"), authority_data);
}

int32_t NetworkServer::get_entity_owner(RID p_world, RID p_entity) const {
	if (!flecs_server) {
		return 1;
	}

	Dictionary authority_data = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	return authority_data.get("owner_peer_id", 1);
}

// =============================================================================
// INPUT & PREDICTION
// =============================================================================

void NetworkServer::send_input(RID p_world, RID p_entity, const Dictionary &p_input) {
	if (role != ROLE_CLIENT) {
		return;
	}

	uint64_t network_id = get_entity_network_id(p_world, p_entity);
	if (network_id == 0) {
		return;
	}

	NetworkTypes::InputCommand cmd;
	cmd.input_tick = current_tick;
	cmd.network_id = network_id;
	cmd.input_data = p_input;

	_send_to_server(NetworkTypes::PacketType::INPUT_COMMAND, cmd.to_dict(),
					NetworkTypes::TransferMode::UNRELIABLE_ORDERED);
}

uint64_t NetworkServer::get_input_tick() const {
	return current_tick;
}

void NetworkServer::set_interpolation_delay(float p_delay_ms) {
	interpolation_delay_ms = p_delay_ms;
}

float NetworkServer::get_interpolation_delay() const {
	return interpolation_delay_ms;
}

// =============================================================================
// RPC
// =============================================================================

void NetworkServer::entity_rpc(RID p_world, RID p_entity, const StringName &p_method,
							   const Array &p_args, int32_t p_target, bool p_reliable) {
	uint64_t network_id = get_entity_network_id(p_world, p_entity);
	if (network_id == 0) {
		ERR_PRINT("NetworkServer: Entity not networked, cannot RPC");
		return;
	}

	NetworkTypes::RPCMessage rpc_msg;
	rpc_msg.rpc_id = current_tick; // Use tick as simple ID
	rpc_msg.network_id = network_id;
	rpc_msg.method_name = p_method;
	rpc_msg.arguments = p_args;
	rpc_msg.sender_peer_id = local_peer_id;

	NetworkTypes::TransferMode mode = p_reliable ? 
		NetworkTypes::TransferMode::RELIABLE_ORDERED : 
		NetworkTypes::TransferMode::UNRELIABLE;

	if (p_target == 0) {
		// Broadcast to all
		if (role == ROLE_HOST) {
			_broadcast(NetworkTypes::PacketType::RPC_CALL, rpc_msg.to_dict(), mode);
		} else {
			_send_to_server(NetworkTypes::PacketType::RPC_CALL, rpc_msg.to_dict(), mode);
		}
	} else {
		// Specific peer
		_send_to_peer(p_target, NetworkTypes::PacketType::RPC_CALL, rpc_msg.to_dict(), mode);
	}
}

void NetworkServer::set_rpc_callback(const Callable &p_callback) {
	on_rpc_callback = p_callback;
}

void NetworkServer::set_spawn_callback(const Callable &p_callback) {
	on_spawn_request_callback = p_callback;
}

void NetworkServer::set_despawn_callback(const Callable &p_callback) {
	on_despawn_request_callback = p_callback;
}

// =============================================================================
// TICK & PROCESSING
// =============================================================================

void NetworkServer::network_process(double p_delta) {
	if (connection_state == STATE_DISCONNECTED) {
		return;
	}

	// Update tick
	tick_accumulator += p_delta;
	while (tick_accumulator >= tick_interval) {
		tick_accumulator -= tick_interval;
		current_tick++;
	}

	// Process incoming packets
	_process_incoming_packets();

	// Tick replication (for host)
	if (role == ROLE_HOST && connection_state == STATE_CONNECTED) {
		_tick_replication();
	}

	// Send outgoing packets
	_send_outgoing_packets();
}

uint64_t NetworkServer::get_current_tick() const {
	return current_tick;
}

uint64_t NetworkServer::get_server_tick() const {
	return role == ROLE_HOST ? current_tick : server_tick;
}

void NetworkServer::set_tick_rate(uint32_t p_rate) {
	tick_rate = p_rate;
	tick_interval = 1.0 / tick_rate;
}

uint32_t NetworkServer::get_tick_rate() const {
	return tick_rate;
}

// =============================================================================
// CONFIGURATION
// =============================================================================

void NetworkServer::set_auto_spawn_enabled(bool p_enabled) {
	auto_spawn_replicated = p_enabled;
}

bool NetworkServer::is_auto_spawn_enabled() const {
	return auto_spawn_replicated;
}

void NetworkServer::set_debug_logging(bool p_enabled) {
	debug_logging = p_enabled;
}

bool NetworkServer::is_debug_logging_enabled() const {
	return debug_logging;
}

// =============================================================================
// STATISTICS
// =============================================================================

Dictionary NetworkServer::get_network_stats() const {
	Dictionary stats;
	stats["role"] = static_cast<int>(role);
	stats["connection_state"] = static_cast<int>(connection_state);
	stats["local_peer_id"] = local_peer_id;
	stats["current_tick"] = current_tick;
	stats["server_tick"] = server_tick;
	stats["tick_rate"] = tick_rate;
	stats["connected_peers"] = static_cast<int>(connected_peers.size());
	stats["registered_worlds"] = static_cast<int>(world_network_data.size());
	
	int total_entities = 0;
	for (const KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		total_entities += E.value.local_to_network.size();
	}
	stats["networked_entities"] = total_entities;
	
	return stats;
}

Dictionary NetworkServer::get_entity_network_stats(RID p_world, RID p_entity) const {
	Dictionary stats;
	
	if (!flecs_server) {
		return stats;
	}

	Dictionary net_stats = flecs_server->get_component_by_name(p_entity, "NetworkStats");
	if (!net_stats.is_empty()) {
		return net_stats;
	}

	return stats;
}

void NetworkServer::reset_network_stats() {
	// TODO: Reset per-entity stats
}

// =============================================================================
// INTERNAL METHODS
// =============================================================================

void NetworkServer::_process_incoming_packets() {
	if (!multiplayer_peer.is_valid()) {
		return;
	}

	multiplayer_peer->poll();

	while (multiplayer_peer->get_available_packet_count() > 0) {
		int32_t sender_id = multiplayer_peer->get_packet_peer();
		
		// Get packet using PacketPeer API
		const uint8_t *buffer = nullptr;
		int buffer_size = 0;
		Error err = multiplayer_peer->get_packet(&buffer, buffer_size);
		
		if (err != OK || buffer == nullptr || buffer_size <= 0) {
			continue;
		}
		
		PackedByteArray packet;
		packet.resize(buffer_size);
		memcpy(packet.ptrw(), buffer, buffer_size);
		
		_handle_packet(sender_id, packet);
	}
}

void NetworkServer::_send_outgoing_packets() {
	if (!multiplayer_peer.is_valid()) {
		return;
	}

	std::lock_guard<std::mutex> lock(send_mutex);

	// Send reliable packets
	for (const Dictionary &packet : outgoing_reliable) {
		// The packet already contains target and data info
		int32_t target = packet.get("_target", 0);
		PackedByteArray data = packet.get("_data", PackedByteArray());
		
		if (data.size() == 0) {
			continue;
		}
		
		if (target == 0) {
			// Broadcast
			multiplayer_peer->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_RELIABLE);
			multiplayer_peer->set_target_peer(MultiplayerPeer::TARGET_PEER_BROADCAST);
			multiplayer_peer->put_packet(data.ptr(), data.size());
		} else {
			multiplayer_peer->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_RELIABLE);
			multiplayer_peer->set_target_peer(target);
			multiplayer_peer->put_packet(data.ptr(), data.size());
		}
	}
	outgoing_reliable.clear();

	// Send unreliable packets
	for (const Dictionary &packet : outgoing_unreliable) {
		int32_t target = packet.get("_target", 0);
		PackedByteArray data = packet.get("_data", PackedByteArray());
		
		if (data.size() == 0) {
			continue;
		}
		
		if (target == 0) {
			multiplayer_peer->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_UNRELIABLE);
			multiplayer_peer->set_target_peer(MultiplayerPeer::TARGET_PEER_BROADCAST);
			multiplayer_peer->put_packet(data.ptr(), data.size());
		} else {
			multiplayer_peer->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_UNRELIABLE);
			multiplayer_peer->set_target_peer(target);
			multiplayer_peer->put_packet(data.ptr(), data.size());
		}
	}
	outgoing_unreliable.clear();
}

void NetworkServer::_handle_packet(int32_t p_peer_id, const PackedByteArray &p_data) {
	NetworkTypes::PacketType type;
	Dictionary data = _deserialize_packet(p_data, type);
	
	if (data.is_empty()) {
		if (debug_logging) {
			print_line(vformat("NetworkServer: Failed to deserialize packet from peer %d", p_peer_id));
		}
		return;
	}

	switch (type) {
		case NetworkTypes::PacketType::HANDSHAKE_REQUEST:
			_handle_handshake_request(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::HANDSHAKE_RESPONSE:
			_handle_handshake_response(data);
			break;
		case NetworkTypes::PacketType::HANDSHAKE_COMPLETE:
			_handle_handshake_complete(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::DISCONNECT:
			_handle_disconnect(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::PING:
			_handle_ping(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::PONG:
			_handle_pong(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::WORLD_SNAPSHOT_FULL:
			_handle_world_snapshot(data, false);
			break;
		case NetworkTypes::PacketType::WORLD_SNAPSHOT_DELTA:
			_handle_world_snapshot(data, true);
			break;
		case NetworkTypes::PacketType::WORLD_TICK_SYNC:
			_handle_tick_sync(data);
			break;
		case NetworkTypes::PacketType::ENTITY_SPAWN:
			_handle_entity_spawn(data);
			break;
		case NetworkTypes::PacketType::ENTITY_DESPAWN:
			_handle_entity_despawn(data);
			break;
		case NetworkTypes::PacketType::ENTITY_UPDATE:
			_handle_entity_update(data);
			break;
		case NetworkTypes::PacketType::ENTITY_UPDATE_BATCH:
			_handle_entity_update_batch(data);
			break;
		case NetworkTypes::PacketType::ENTITY_AUTHORITY_CHANGE:
			_handle_authority_change(data);
			break;
		case NetworkTypes::PacketType::INPUT_COMMAND:
			_handle_input_command(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::INPUT_ACK:
			_handle_input_ack(data);
			break;
		case NetworkTypes::PacketType::RPC_CALL:
			_handle_rpc_call(p_peer_id, data);
			break;
		case NetworkTypes::PacketType::RPC_RESPONSE:
			_handle_rpc_response(data);
			break;
		default:
			if (debug_logging) {
				print_line(vformat("NetworkServer: Unknown packet type %d from peer %d", 
								   static_cast<int>(type), p_peer_id));
			}
			break;
	}
}

// =============================================================================
// PACKET HANDLERS
// =============================================================================

void NetworkServer::_handle_handshake_request(int32_t p_peer_id, const Dictionary &p_data) {
	if (role != ROLE_HOST) {
		return;
	}

	NetworkTypes::HandshakeRequest request;
	request.from_dict(p_data);

	// Check version
	if (request.client_version != NetworkTypes::PROTOCOL_VERSION) {
		NetworkTypes::HandshakeResponse response;
		response.accepted = false;
		response.reject_reason = NetworkTypes::DisconnectReason::VERSION_MISMATCH;
		response.reject_message = "Protocol version mismatch";
		
		_send_to_peer(p_peer_id, NetworkTypes::PacketType::HANDSHAKE_RESPONSE, 
					  response.to_dict(), NetworkTypes::TransferMode::RELIABLE_ORDERED);
		return;
	}

	// Accept connection
	NetworkTypes::PeerInfo peer_info;
	peer_info.peer_id = p_peer_id;
	peer_info.peer_name = request.client_name;
	peer_info.is_host = false;
	peer_info.is_authenticated = true;
	peer_info.connect_time_usec = OS::get_singleton()->get_ticks_usec();
	peer_info.last_seen_tick = current_tick;
	
	connected_peers[p_peer_id] = peer_info;

	NetworkTypes::HandshakeResponse response;
	response.accepted = true;
	response.assigned_peer_id = p_peer_id;
	response.server_tick = current_tick;
	response.tick_rate = tick_rate;
	
	_send_to_peer(p_peer_id, NetworkTypes::PacketType::HANDSHAKE_RESPONSE,
				  response.to_dict(), NetworkTypes::TransferMode::RELIABLE_ORDERED);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Peer %d connected: %s", p_peer_id, request.client_name));
	}

	emit_signal(SNAME("peer_connected"), p_peer_id);
}

void NetworkServer::_handle_handshake_response(const Dictionary &p_data) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::HandshakeResponse response;
	response.from_dict(p_data);

	if (!response.accepted) {
		if (debug_logging) {
			print_line(vformat("NetworkServer: Connection rejected: %s", response.reject_message));
		}
		emit_signal(SNAME("connection_failed"), response.reject_message);
		disconnect_game(static_cast<int>(response.reject_reason));
		return;
	}

	local_peer_id = response.assigned_peer_id;
	server_tick = response.server_tick;
	current_tick = response.server_tick;
	tick_rate = response.tick_rate;
	tick_interval = 1.0 / tick_rate;
	connection_state = STATE_CONNECTED;

	// Send handshake complete
	Dictionary complete_data;
	complete_data["peer_id"] = local_peer_id;
	_send_to_server(NetworkTypes::PacketType::HANDSHAKE_COMPLETE, complete_data,
					NetworkTypes::TransferMode::RELIABLE_ORDERED);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Connected as peer %d", local_peer_id));
	}

	emit_signal(SNAME("connection_succeeded"));
}

void NetworkServer::_handle_handshake_complete(int32_t p_peer_id, const Dictionary &p_data) {
	// Client confirmed handshake, send full world snapshot
	if (role != ROLE_HOST) {
		return;
	}

	// Send full world snapshot to new peer
	for (const KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		NetworkTypes::WorldSnapshot snapshot;
		snapshot.tick = current_tick;
		snapshot.is_delta = false;

		// Add all entities
		for (const KeyValue<uint64_t, RID> &entity_entry : E.value.network_to_local) {
			NetworkTypes::EntitySpawnMessage spawn_msg;
			spawn_msg.network_id = entity_entry.key;
			
			if (flecs_server) {
				Dictionary identity = flecs_server->get_component_by_name(entity_entry.value, "NetworkIdentity");
				Dictionary authority = flecs_server->get_component_by_name(entity_entry.value, "NetworkAuthority");
				
				spawn_msg.owner_peer_id = authority.get("owner_peer_id", 1);
				spawn_msg.authority_peer_id = authority.get("authority_peer_id", 1);
				spawn_msg.spawn_scene_path = identity.get("spawn_scene_path", "");
				spawn_msg.spawn_data = identity.get("spawn_data", Dictionary());
				
				// Get component data
				PackedStringArray comp_names = flecs_server->get_component_types_as_name(entity_entry.value);
				Dictionary components;
				for (int i = 0; i < comp_names.size(); i++) {
					Dictionary comp_data = flecs_server->get_component_by_name(entity_entry.value, comp_names[i]);
					if (!comp_data.is_empty()) {
						components[comp_names[i]] = comp_data;
					}
				}
				spawn_msg.initial_components = components;
			}
			
			snapshot.spawned_entities.push_back(spawn_msg);
		}

		_send_to_peer(p_peer_id, NetworkTypes::PacketType::WORLD_SNAPSHOT_FULL,
					  snapshot.to_dict(), NetworkTypes::TransferMode::RELIABLE_ORDERED);
	}
}

void NetworkServer::_handle_disconnect(int32_t p_peer_id, const Dictionary &p_data) {
	NetworkTypes::DisconnectReason reason = static_cast<NetworkTypes::DisconnectReason>(
		int(p_data.get("reason", 0))
	);

	connected_peers.erase(p_peer_id);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Peer %d disconnected (reason: %d)", p_peer_id, static_cast<int>(reason)));
	}

	emit_signal(SNAME("peer_disconnected"), p_peer_id, static_cast<int>(reason));
}

void NetworkServer::_handle_ping(int32_t p_peer_id, const Dictionary &p_data) {
	// Respond with pong
	Dictionary pong_data;
	pong_data["timestamp"] = p_data.get("timestamp", 0);
	pong_data["server_time"] = OS::get_singleton()->get_ticks_usec();
	
	_send_to_peer(p_peer_id, NetworkTypes::PacketType::PONG, pong_data,
				  NetworkTypes::TransferMode::UNRELIABLE);
}

void NetworkServer::_handle_pong(int32_t p_peer_id, const Dictionary &p_data) {
	uint64_t sent_time = p_data.get("timestamp", 0);
	uint64_t current_time = OS::get_singleton()->get_ticks_usec();
	float rtt_ms = (current_time - sent_time) / 1000.0f;

	if (connected_peers.has(p_peer_id)) {
		// Exponential moving average for latency
		float old_latency = connected_peers[p_peer_id].latency_ms;
		connected_peers[p_peer_id].latency_ms = old_latency * 0.8f + (rtt_ms / 2.0f) * 0.2f;
	}
}

void NetworkServer::_handle_world_snapshot(const Dictionary &p_data, bool p_is_delta) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::WorldSnapshot snapshot;
	snapshot.from_dict(p_data);
	server_tick = snapshot.tick;

	// Process spawns
	for (const auto &spawn : snapshot.spawned_entities) {
		// Find or create world - for now assume first registered world
		for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
			_spawn_remote_entity(E.key, spawn);
			break;
		}
	}

	// Process despawns
	for (const auto &network_id : snapshot.despawned_entity_ids) {
		for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
			_despawn_remote_entity(E.key, network_id);
			break;
		}
	}

	// Process updates
	for (const auto &update : snapshot.entity_updates) {
		for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
			_apply_entity_update(E.key, update);
			break;
		}
	}
}

void NetworkServer::_handle_tick_sync(const Dictionary &p_data) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::TickSyncMessage sync_msg;
	sync_msg.from_dict(p_data);
	server_tick = sync_msg.server_tick;
}

void NetworkServer::_handle_entity_spawn(const Dictionary &p_data) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::EntitySpawnMessage spawn_msg;
	spawn_msg.from_dict(p_data);

	// Spawn in first registered world
	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		_spawn_remote_entity(E.key, spawn_msg);
		break;
	}
}

void NetworkServer::_handle_entity_despawn(const Dictionary &p_data) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::EntityDespawnMessage despawn_msg;
	despawn_msg.from_dict(p_data);

	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		_despawn_remote_entity(E.key, despawn_msg.network_id);
		break;
	}
}

void NetworkServer::_handle_entity_update(const Dictionary &p_data) {
	NetworkTypes::EntityUpdateMessage update;
	update.from_dict(p_data);

	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		_apply_entity_update(E.key, update);
		break;
	}
}

void NetworkServer::_handle_entity_update_batch(const Dictionary &p_data) {
	NetworkTypes::EntityUpdateBatch batch;
	batch.from_dict(p_data);

	for (const auto &update : batch.updates) {
		for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
			_apply_entity_update(E.key, update);
			break;
		}
	}
}

void NetworkServer::_handle_authority_change(const Dictionary &p_data) {
	NetworkTypes::AuthorityChangeMessage auth_msg;
	auth_msg.from_dict(p_data);

	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		RID entity = _get_entity_by_network_id(E.key, auth_msg.network_id);
		if (entity.is_valid() && flecs_server) {
			Dictionary authority_data = flecs_server->get_component_by_name(entity, "NetworkAuthority");
			authority_data["authority_peer_id"] = auth_msg.new_authority_peer_id;
			authority_data["is_local_authority"] = (auth_msg.new_authority_peer_id == local_peer_id);
			authority_data["authority_change_tick"] = auth_msg.change_tick;
			flecs_server->set_component(entity, "NetworkAuthority", authority_data);
		}
		break;
	}

	emit_signal(SNAME("authority_changed"), (int64_t)auth_msg.network_id, auth_msg.new_authority_peer_id);
}

void NetworkServer::_handle_input_command(int32_t p_peer_id, const Dictionary &p_data) {
	if (role != ROLE_HOST) {
		return;
	}

	NetworkTypes::InputCommand cmd;
	cmd.from_dict(p_data);

	// Find entity and check if peer has authority
	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		RID entity = _get_entity_by_network_id(E.key, cmd.network_id);
		if (entity.is_valid() && flecs_server) {
			Dictionary authority = flecs_server->get_component_by_name(entity, StringName("NetworkAuthority"));
			if (int(authority.get("authority_peer_id", 1)) == p_peer_id) {
				// Process input - store it on the entity for game logic to use
				Dictionary input_comp;
				input_comp["input_tick"] = (int64_t)cmd.input_tick;
				input_comp["input_data"] = cmd.input_data;
				flecs_server->set_component(entity, StringName("NetworkInput"), input_comp);
				
				// Send acknowledgment
				NetworkTypes::InputAck ack;
				ack.acked_tick = cmd.input_tick;
				ack.network_id = cmd.network_id;
				// Include authoritative state for reconciliation
				ack.authoritative_state = flecs_server->get_component_by_name(entity, StringName("Transform3DComponent"));
				
				_send_to_peer(p_peer_id, NetworkTypes::PacketType::INPUT_ACK, ack.to_dict(),
							  NetworkTypes::TransferMode::UNRELIABLE_ORDERED);
			}
		}
		break;
	}
}

void NetworkServer::_handle_input_ack(const Dictionary &p_data) {
	if (role != ROLE_CLIENT) {
		return;
	}

	NetworkTypes::InputAck ack;
	ack.from_dict(p_data);

	// Mark input as acknowledged and check for reconciliation
	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		RID entity = _get_entity_by_network_id(E.key, ack.network_id);
		if (entity.is_valid() && flecs_server) {
			Dictionary input_comp = flecs_server->get_component_by_name(entity, StringName("NetworkInput"));
			if (!input_comp.is_empty()) {
				// Acknowledge inputs up to this tick
				// Game logic should handle reconciliation if needed
			}
			
			// Store authoritative state for prediction comparison
			Dictionary prediction = flecs_server->get_component_by_name(entity, StringName("NetworkPrediction"));
			if (prediction.is_empty()) {
				prediction["last_confirmed_tick"] = (int64_t)ack.acked_tick;
				prediction["last_confirmed_state"] = ack.authoritative_state;
				prediction["is_predicting"] = true;
			} else {
				prediction["last_confirmed_tick"] = (int64_t)ack.acked_tick;
				prediction["last_confirmed_state"] = ack.authoritative_state;
			}
			flecs_server->set_component(entity, StringName("NetworkPrediction"), prediction);
		}
		break;
	}
}

void NetworkServer::_handle_rpc_call(int32_t p_peer_id, const Dictionary &p_data) {
	NetworkTypes::RPCMessage rpc_msg;
	rpc_msg.from_dict(p_data);
	rpc_msg.sender_peer_id = p_peer_id;

	// Find entity
	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		RID entity = _get_entity_by_network_id(E.key, rpc_msg.network_id);
		if (entity.is_valid()) {
			// Call the RPC callback if set
			if (on_rpc_callback.is_valid()) {
				on_rpc_callback.call(E.key, entity, rpc_msg.method_name, 
									 rpc_msg.arguments, rpc_msg.sender_peer_id);
			}
			
			// If host, forward to other peers (except sender)
			if (role == ROLE_HOST) {
				_broadcast(NetworkTypes::PacketType::RPC_CALL, rpc_msg.to_dict(),
						   NetworkTypes::TransferMode::RELIABLE_ORDERED, p_peer_id);
			}
		}
		break;
	}
}

void NetworkServer::_handle_rpc_response(const Dictionary &p_data) {
	// RPC responses are for future two-way RPC support
	// Currently RPCs are fire-and-forget
}

// =============================================================================
// SENDING HELPERS
// =============================================================================

void NetworkServer::_send_to_peer(int32_t p_peer_id, NetworkTypes::PacketType p_type,
								  const Dictionary &p_data, NetworkTypes::TransferMode p_mode) {
	if (!multiplayer_peer.is_valid()) {
		return;
	}

	PackedByteArray serialized = _serialize_packet(p_type, p_data);
	
	Dictionary packet;
	packet["_target"] = p_peer_id;
	packet["_data"] = serialized;

	std::lock_guard<std::mutex> lock(send_mutex);
	
	if (p_mode == NetworkTypes::TransferMode::RELIABLE || 
		p_mode == NetworkTypes::TransferMode::RELIABLE_ORDERED) {
		outgoing_reliable.push_back(packet);
	} else {
		outgoing_unreliable.push_back(packet);
	}
}

void NetworkServer::_broadcast(NetworkTypes::PacketType p_type, const Dictionary &p_data,
							   NetworkTypes::TransferMode p_mode, int32_t p_exclude_peer) {
	if (!multiplayer_peer.is_valid()) {
		return;
	}

	PackedByteArray serialized = _serialize_packet(p_type, p_data);

	// Send to all connected peers except excluded
	for (const KeyValue<int32_t, NetworkTypes::PeerInfo> &E : connected_peers) {
		if (E.key == p_exclude_peer) {
			continue;
		}

		Dictionary packet;
		packet["_target"] = E.key;
		packet["_data"] = serialized;

		std::lock_guard<std::mutex> lock(send_mutex);
		
		if (p_mode == NetworkTypes::TransferMode::RELIABLE || 
			p_mode == NetworkTypes::TransferMode::RELIABLE_ORDERED) {
			outgoing_reliable.push_back(packet);
		} else {
			outgoing_unreliable.push_back(packet);
		}
	}
}

void NetworkServer::_send_to_server(NetworkTypes::PacketType p_type, const Dictionary &p_data,
									NetworkTypes::TransferMode p_mode) {
	_send_to_peer(1, p_type, p_data, p_mode); // Server is always peer 1
}

PackedByteArray NetworkServer::_serialize_packet(NetworkTypes::PacketType p_type,
												 const Dictionary &p_data) {
	// Create packet with header + variant-encoded data
	PackedByteArray result;
	
	// Header (16 bytes)
	NetworkTypes::PacketHeader header;
	header.type = p_type;
	header.tick = current_tick;
	
	result.resize(16);
	header.serialize(result.ptrw(), 16);
	
	// Serialize data using Godot's variant encoding
	int len = 0;
	Error err = encode_variant(p_data, nullptr, len, false, false);
	if (err != OK) {
		ERR_PRINT("NetworkServer: Failed to calculate variant size");
		return result;
	}
	
	int old_size = result.size();
	result.resize(old_size + len);
	err = encode_variant(p_data, result.ptrw() + old_size, len, false, false);
	if (err != OK) {
		ERR_PRINT("NetworkServer: Failed to encode variant");
	}
	
	return result;
}

Dictionary NetworkServer::_deserialize_packet(const PackedByteArray &p_data,
											  NetworkTypes::PacketType &r_type) {
	if (p_data.size() < 16) {
		return Dictionary();
	}

	// Read header
	NetworkTypes::PacketHeader header;
	header.deserialize(p_data.ptr(), p_data.size());
	
	if (!header.is_valid()) {
		if (debug_logging) {
			print_line("NetworkServer: Invalid packet header");
		}
		return Dictionary();
	}
	
	r_type = header.type;
	
	// Decode variant data
	Variant data;
	int len = 0;
	Error err = decode_variant(data, p_data.ptr() + 16, p_data.size() - 16, &len, false, false);
	if (err != OK) {
		if (debug_logging) {
			print_line("NetworkServer: Failed to decode packet data");
		}
		return Dictionary();
	}
	
	if (data.get_type() != Variant::DICTIONARY) {
		return Dictionary();
	}
	
	return data;
}

// =============================================================================
// REPLICATION
// =============================================================================

void NetworkServer::_tick_replication() {
	if (role != ROLE_HOST) {
		return;
	}

	// Collect updates from all worlds
	for (KeyValue<RID, WorldNetworkData> &E : world_network_data) {
		Vector<NetworkTypes::EntityUpdateMessage> updates;
		_collect_dirty_entities(E.key, updates);
		
		if (updates.is_empty()) {
			continue;
		}

		// Batch updates
		NetworkTypes::EntityUpdateBatch batch;
		batch.tick = current_tick;
		batch.updates = updates;
		
		_broadcast(NetworkTypes::PacketType::ENTITY_UPDATE_BATCH, batch.to_dict(),
				   NetworkTypes::TransferMode::UNRELIABLE_ORDERED);
	}

	// Periodic tick sync
	if (current_tick % 60 == 0) { // Every second at 60 tick rate
		NetworkTypes::TickSyncMessage sync;
		sync.server_tick = current_tick;
		sync.server_time_usec = OS::get_singleton()->get_ticks_usec();
		
		_broadcast(NetworkTypes::PacketType::WORLD_TICK_SYNC, sync.to_dict(),
				   NetworkTypes::TransferMode::UNRELIABLE);
	}
}

void NetworkServer::_collect_dirty_entities(RID p_world, Vector<NetworkTypes::EntityUpdateMessage> &r_updates) {
	if (!flecs_server) {
		return;
	}

	WorldNetworkData &world_data = world_network_data[p_world];

	// Iterate through all networked entities
	for (const KeyValue<RID, uint64_t> &entity_entry : world_data.local_to_network) {
		RID entity = entity_entry.key;
		uint64_t network_id = entity_entry.value;

		// Check if entity has NetworkReplicated component
		Dictionary replicated = flecs_server->get_component_by_name(entity, "NetworkReplicated");
		if (replicated.is_empty() || !replicated.get("is_active", false)) {
			continue;
		}

		// Get replicated components
		Array comp_configs = replicated.get("replicated_components", Array());
		if (comp_configs.is_empty()) {
			continue;
		}

		NetworkTypes::EntityUpdateMessage update;
		update.network_id = network_id;
		update.tick = current_tick;

		for (int i = 0; i < comp_configs.size(); i++) {
			Dictionary config = comp_configs[i];
			StringName comp_name = config.get("component_name", StringName());
			int mode = config.get("mode", static_cast<int>(NetworkComponents::ReplicationMode::ON_CHANGE));
			
			// Skip NONE mode
			if (mode == static_cast<int>(NetworkComponents::ReplicationMode::NONE)) {
				continue;
			}

			// For ON_CHANGE, we'd need proper change detection
			// For now, send CONTINUOUS components every tick
			if (mode == static_cast<int>(NetworkComponents::ReplicationMode::CONTINUOUS)) {
				Dictionary comp_data = flecs_server->get_component_by_name(entity, StringName(comp_name));
				if (!comp_data.is_empty()) {
					NetworkTypes::ComponentUpdate comp_update;
					comp_update.component_name = comp_name;
					comp_update.component_data = comp_data;
					comp_update.is_delta = false;
					update.components.push_back(comp_update);
				}
			}
		}

		if (!update.components.is_empty()) {
			r_updates.push_back(update);
		}
	}
}

void NetworkServer::_apply_entity_update(RID p_world, const NetworkTypes::EntityUpdateMessage &p_update) {
	RID entity = _get_entity_by_network_id(p_world, p_update.network_id);
	if (!entity.is_valid() || !flecs_server) {
		return;
	}

	// Check authority - don't apply updates to locally authoritative entities
	if (_has_local_authority(p_world, entity)) {
		return;
	}

	// Apply component updates
	for (const auto &comp_update : p_update.components) {
		// Check if we should interpolate
			Dictionary replicated = flecs_server->get_component_by_name(entity, StringName("NetworkReplicated"));
		bool should_interpolate = false;
		
		if (!replicated.is_empty()) {
			Array configs = replicated.get("replicated_components", Array());
			for (int i = 0; i < configs.size(); i++) {
				Dictionary config = configs[i];
				if (StringName(config.get("component_name", "")) == comp_update.component_name) {
					should_interpolate = config.get("interpolate", false);
					break;
				}
			}
		}

		if (should_interpolate) {
			// Add to interpolation buffer instead of applying directly
			// For Transform3D, use specialized interpolation
			if (comp_update.component_name == StringName("Transform3DComponent")) {
				Dictionary interp = flecs_server->get_component_by_name(entity, StringName("NetworkTransformInterpolation3D"));
				if (interp.is_empty()) {
					interp["interpolation_delay_ticks"] = 2;
					interp["buffer"] = Array();
				}
				
				Array buffer = interp.get("buffer", Array());
				Dictionary state;
				state["tick"] = (int64_t)p_update.tick;
				state["received_time"] = (int64_t)OS::get_singleton()->get_ticks_usec();
				state["transform"] = comp_update.component_data.get("transform", Transform3D());
				buffer.push_back(state);
				
				// Limit buffer size
				while (buffer.size() > 32) {
					buffer.remove_at(0);
				}
				
				interp["buffer"] = buffer;
				flecs_server->set_component(entity, StringName("NetworkTransformInterpolation3D"), interp);
			}
		} else {
			// Apply directly
			flecs_server->set_component(entity, comp_update.component_name, comp_update.component_data);
		}
	}
}

void NetworkServer::_spawn_remote_entity(RID p_world, const NetworkTypes::EntitySpawnMessage &p_spawn) {
	WorldNetworkData &world_data = world_network_data[p_world];

	// Check if already exists
	if (world_data.network_to_local.has(p_spawn.network_id)) {
		return;
	}

	RID entity;

	// Try custom spawn callback first
	if (on_spawn_request_callback.is_valid()) {
		Variant result = on_spawn_request_callback.call(p_world, p_spawn.spawn_data);
		if (result.get_type() == Variant::RID) {
			entity = result;
		}
	}

	// Default spawn if no callback or callback didn't return entity
	if (!entity.is_valid() && flecs_server && auto_spawn_replicated) {
		entity = flecs_server->create_entity(p_world);
	}

	if (!entity.is_valid()) {
		if (debug_logging) {
			print_line(vformat("NetworkServer: Failed to spawn remote entity %d", p_spawn.network_id));
		}
		return;
	}

	// Store mapping
	world_data.network_to_local[p_spawn.network_id] = entity;
	world_data.local_to_network[entity] = p_spawn.network_id;

	// Apply initial components
	if (flecs_server) {
		// Set NetworkIdentity
		Dictionary identity;
		identity["network_id"] = (int64_t)p_spawn.network_id;
		identity["is_network_spawned"] = true;
		identity["spawn_tick"] = (int64_t)current_tick;
		identity["spawn_scene_path"] = Variant(p_spawn.spawn_scene_path);
		identity["spawn_data"] = p_spawn.spawn_data;
		flecs_server->set_component(entity, StringName("NetworkIdentity"), identity);

		// Set NetworkAuthority
		Dictionary authority;
		authority["mode"] = static_cast<int>(NetworkComponents::AuthorityMode::SERVER);
		authority["authority_peer_id"] = p_spawn.authority_peer_id;
		authority["owner_peer_id"] = p_spawn.owner_peer_id;
		authority["is_local_authority"] = (p_spawn.authority_peer_id == local_peer_id);
		flecs_server->set_component(entity, StringName("NetworkAuthority"), authority);

		// Apply initial component data
		Array keys = p_spawn.initial_components.keys();
		for (int i = 0; i < keys.size(); i++) {
			String comp_name = keys[i];
			Dictionary comp_data = p_spawn.initial_components[comp_name];
			flecs_server->set_component(entity, StringName(comp_name), comp_data);
		}
	}

	if (debug_logging) {
		print_line(vformat("NetworkServer: Spawned remote entity %d as %d", 
						   (int)p_spawn.network_id, entity.get_id()));
	}

	emit_signal(SNAME("entity_spawned_remote"), (int64_t)p_spawn.network_id, entity);
}

void NetworkServer::_despawn_remote_entity(RID p_world, uint64_t p_network_id) {
	WorldNetworkData &world_data = world_network_data[p_world];

	if (!world_data.network_to_local.has(p_network_id)) {
		return;
	}

	RID entity = world_data.network_to_local[p_network_id];

	// Call despawn callback
	if (on_despawn_request_callback.is_valid()) {
		on_despawn_request_callback.call(p_world, entity, p_network_id);
	}

	// Remove from flecs
	if (flecs_server) {
		flecs_server->free_entity(p_world, entity, true);
	}

	// Remove mappings
	world_data.network_to_local.erase(p_network_id);
	world_data.local_to_network.erase(entity);

	if (debug_logging) {
		print_line(vformat("NetworkServer: Despawned remote entity %d", (int)p_network_id));
	}

	emit_signal(SNAME("entity_despawned_remote"), (int64_t)p_network_id);
}

bool NetworkServer::_has_local_authority(RID p_world, RID p_entity) {
	if (!flecs_server) {
		return true;
	}

	Dictionary authority = flecs_server->get_component_by_name(p_entity, StringName("NetworkAuthority"));
	return authority.get("is_local_authority", false);
}

uint64_t NetworkServer::_generate_network_id() {
	return next_network_id.fetch_add(1);
}

RID NetworkServer::_get_entity_by_network_id(RID p_world, uint64_t p_network_id) {
	if (!world_network_data.has(p_world)) {
		return RID();
	}

	const WorldNetworkData &data = world_network_data[p_world];
	if (!data.network_to_local.has(p_network_id)) {
		return RID();
	}

	return data.network_to_local[p_network_id];
}

uint64_t NetworkServer::_get_network_id_by_entity(RID p_world, RID p_entity) {
	if (!world_network_data.has(p_world)) {
		return 0;
	}

	const WorldNetworkData &data = world_network_data[p_world];
	if (!data.local_to_network.has(p_entity)) {
		return 0;
	}

	return data.local_to_network[p_entity];
}

void NetworkServer::_on_peer_connected(int32_t p_peer_id) {
	if (role == ROLE_HOST) {
		// New client connected, wait for handshake
		if (debug_logging) {
			print_line(vformat("NetworkServer: Peer %d connecting...", p_peer_id));
		}
	} else if (role == ROLE_CLIENT && p_peer_id == 1) {
		// Connected to server, send handshake
		connection_state = STATE_HANDSHAKING;
		
		NetworkTypes::HandshakeRequest request;
		request.client_version = NetworkTypes::PROTOCOL_VERSION;
		request.client_name = OS::get_singleton()->get_unique_id();
		
		_send_to_server(NetworkTypes::PacketType::HANDSHAKE_REQUEST, request.to_dict(),
						NetworkTypes::TransferMode::RELIABLE_ORDERED);
	}
}

void NetworkServer::_on_peer_disconnected(int32_t p_peer_id) {
	if (connected_peers.has(p_peer_id)) {
		connected_peers.erase(p_peer_id);
	}

	if (debug_logging) {
		print_line(vformat("NetworkServer: Peer %d disconnected", p_peer_id));
	}

	emit_signal(SNAME("peer_disconnected"), p_peer_id, static_cast<int>(NetworkTypes::DisconnectReason::GRACEFUL));

	if (role == ROLE_CLIENT && p_peer_id == 1) {
		// Lost connection to server
		disconnect_game(static_cast<int>(NetworkTypes::DisconnectReason::TIMEOUT));
	}
}