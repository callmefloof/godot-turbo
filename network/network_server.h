#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

/**************************************************************************/
/*  network_server.h                                                      */
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

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/templates/rid.h"
#include "core/variant/callable.h"
#include "scene/main/multiplayer_peer.h"

#include "network_types.h"
#include "components/network_components.h"

#include <atomic>
#include <mutex>

class FlecsServer;

/**
 * @class NetworkServer
 * @brief Central singleton for ECS multiplayer networking operations
 * 
 * NetworkServer provides the main API for networked multiplayer functionality
 * in Godot Turbo ECS. It manages:
 * 
 * - Connection lifecycle (host/join/disconnect)
 * - Entity network identity and replication
 * - Authority and ownership management
 * - Tick synchronization
 * - Input handling and prediction
 * - RPC-like functionality for entities
 * 
 * @section Architecture
 * NetworkServer integrates with Godot's MultiplayerPeer for transport and
 * FlecsServer for ECS operations. It maintains mappings between local entity
 * RIDs and network-unique IDs.
 * 
 * @section Usage from GDScript
 * @code
 * # Get singleton
 * var network = NetworkServer
 * 
 * # Host a game
 * network.host_game(7777, 16)  # Port 7777, max 16 players
 * 
 * # Or join a game
 * network.join_game("192.168.1.100", 7777)
 * 
 * # Register an entity for networking
 * var entity_rid = FlecsServer.create_entity(world)
 * var net_id = network.register_networked_entity(world, entity_rid)
 * 
 * # Configure replication
 * network.set_entity_replicated_components(world, entity_rid, 
 *     ["Transform3DComponent", "HealthComponent"])
 * 
 * # In game loop
 * func _physics_process(delta):
 *     network.network_process(delta)
 * @endcode
 * 
 * @section Signals
 * - peer_connected(peer_id: int)
 * - peer_disconnected(peer_id: int, reason: int)
 * - connection_succeeded()
 * - connection_failed(reason: String)
 * - server_started()
 * - server_stopped()
 * - entity_spawned_remote(network_id: int, entity_rid: RID)
 * - entity_despawned_remote(network_id: int)
 * - authority_changed(network_id: int, new_authority: int)
 * 
 * @see FlecsServer, NetworkComponents
 */
class NetworkServer : public Object {
	GDCLASS(NetworkServer, Object);

public:
	/**
	 * @enum NetworkRole
	 * @brief The role this peer plays in the network
	 */
	enum NetworkRole {
		ROLE_NONE = 0,      ///< Not connected
		ROLE_HOST = 1,      ///< Server/Host
		ROLE_CLIENT = 2,    ///< Client connected to host
	};

	/**
	 * @enum ConnectionState
	 * @brief Current connection state
	 */
	enum ConnectionState {
		STATE_DISCONNECTED = 0,
		STATE_CONNECTING = 1,
		STATE_HANDSHAKING = 2,
		STATE_CONNECTED = 3,
		STATE_DISCONNECTING = 4,
	};

protected:
	static void _bind_methods();

private:
	// Singleton
	static NetworkServer *singleton;

	// References
	FlecsServer *flecs_server = nullptr;
	Ref<MultiplayerPeer> multiplayer_peer;

	// Connection state
	NetworkRole role = ROLE_NONE;
	ConnectionState connection_state = STATE_DISCONNECTED;
	int32_t local_peer_id = 0;

	// Tick management
	uint64_t current_tick = 0;
	uint64_t server_tick = 0;  // Last known server tick (for clients)
	uint32_t tick_rate = NetworkTypes::DEFAULT_TICK_RATE;
	double tick_accumulator = 0.0;
	double tick_interval = 1.0 / NetworkTypes::DEFAULT_TICK_RATE;

	// Network ID generation (server only)
	std::atomic<uint64_t> next_network_id{1};

	// Entity mappings (per world)
	struct WorldNetworkData {
		RID world_rid;
		
		// Network ID -> Local Entity RID
		HashMap<uint64_t, RID> network_to_local;
		
		// Local Entity RID -> Network ID
		HashMap<RID, uint64_t> local_to_network;
		
		// Entities pending spawn confirmation
		HashMap<uint64_t, Dictionary> pending_spawns;
		
		// Snapshot history for delta compression
		Vector<NetworkTypes::WorldSnapshot> snapshot_history;
		int snapshot_head = 0;
	};
	HashMap<RID, WorldNetworkData> world_network_data;

	// Connected peers
	HashMap<int32_t, NetworkTypes::PeerInfo> connected_peers;

	// Pending messages
	mutable std::mutex send_mutex;
	Vector<Dictionary> outgoing_reliable;
	Vector<Dictionary> outgoing_unreliable;

	// Callbacks
	Callable on_spawn_request_callback;
	Callable on_despawn_request_callback;
	Callable on_rpc_callback;
	Callable on_authority_request_callback;

	// Configuration
	bool auto_spawn_replicated = true;
	float interpolation_delay_ms = 100.0f;
	int max_input_buffer_size = NetworkTypes::MAX_INPUT_BUFFER_SIZE;
	bool debug_logging = false;

	// Internal methods
	void _process_incoming_packets();
	void _send_outgoing_packets();
	void _handle_packet(int32_t p_peer_id, const PackedByteArray &p_data);
	
	// Packet handlers
	void _handle_handshake_request(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_handshake_response(const Dictionary &p_data);
	void _handle_handshake_complete(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_disconnect(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_ping(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_pong(int32_t p_peer_id, const Dictionary &p_data);
	
	void _handle_world_snapshot(const Dictionary &p_data, bool p_is_delta);
	void _handle_tick_sync(const Dictionary &p_data);
	
	void _handle_entity_spawn(const Dictionary &p_data);
	void _handle_entity_despawn(const Dictionary &p_data);
	void _handle_entity_update(const Dictionary &p_data);
	void _handle_entity_update_batch(const Dictionary &p_data);
	void _handle_authority_change(const Dictionary &p_data);
	
	void _handle_input_command(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_input_ack(const Dictionary &p_data);
	
	void _handle_rpc_call(int32_t p_peer_id, const Dictionary &p_data);
	void _handle_rpc_response(const Dictionary &p_data);
	
	// Sending helpers
	void _send_to_peer(int32_t p_peer_id, NetworkTypes::PacketType p_type, 
					   const Dictionary &p_data, NetworkTypes::TransferMode p_mode);
	void _broadcast(NetworkTypes::PacketType p_type, const Dictionary &p_data,
					NetworkTypes::TransferMode p_mode, int32_t p_exclude_peer = 0);
	void _send_to_server(NetworkTypes::PacketType p_type, const Dictionary &p_data,
						 NetworkTypes::TransferMode p_mode);
	
	PackedByteArray _serialize_packet(NetworkTypes::PacketType p_type, 
									  const Dictionary &p_data);
	Dictionary _deserialize_packet(const PackedByteArray &p_data, 
								   NetworkTypes::PacketType &r_type);
	
	// Replication
	void _tick_replication();
	void _collect_dirty_entities(RID p_world, Vector<NetworkTypes::EntityUpdateMessage> &r_updates);
	void _apply_entity_update(RID p_world, const NetworkTypes::EntityUpdateMessage &p_update);
	void _spawn_remote_entity(RID p_world, const NetworkTypes::EntitySpawnMessage &p_spawn);
	void _despawn_remote_entity(RID p_world, uint64_t p_network_id);
	
	// Authority
	bool _has_local_authority(RID p_world, RID p_entity);
	
	// Network ID management
	uint64_t _generate_network_id();
	RID _get_entity_by_network_id(RID p_world, uint64_t p_network_id);
	uint64_t _get_network_id_by_entity(RID p_world, RID p_entity);
	
	// Peer callbacks
	void _on_peer_connected(int32_t p_peer_id);
	void _on_peer_disconnected(int32_t p_peer_id);

public:
	static NetworkServer *get_singleton();

	NetworkServer();
	~NetworkServer();

	// =========================================================================
	// CONNECTION MANAGEMENT
	// =========================================================================

	/**
	 * @brief Start hosting a game server
	 * 
	 * @param p_port Port to listen on
	 * @param p_max_clients Maximum number of clients
	 * @param p_bind_address Address to bind to (empty = all interfaces)
	 * @return Error code (OK on success)
	 */
	Error host_game(int p_port, int p_max_clients = 16, const String &p_bind_address = "");

	/**
	 * @brief Join a hosted game
	 * 
	 * @param p_address Server address
	 * @param p_port Server port
	 * @return Error code (OK on success)
	 */
	Error join_game(const String &p_address, int p_port);

	/**
	 * @brief Disconnect from the current game
	 * 
	 * @param p_reason Reason for disconnection (int cast of NetworkTypes::DisconnectReason)
	 */
	void disconnect_game(int p_reason = 1); // 1 = GRACEFUL

	/**
	 * @brief Set a custom multiplayer peer (for custom transport)
	 */
	void set_multiplayer_peer(const Ref<MultiplayerPeer> &p_peer);

	/**
	 * @brief Get the current multiplayer peer
	 */
	Ref<MultiplayerPeer> get_multiplayer_peer() const;

	/**
	 * @brief Check if currently hosting
	 */
	bool is_host() const;

	/**
	 * @brief Check if connected (as client or host)
	 */
	bool is_connected_to_game() const;

	/**
	 * @brief Get the local peer ID
	 */
	int32_t get_local_peer_id() const;

	/**
	 * @brief Get current network role
	 */
	NetworkRole get_network_role() const;

	/**
	 * @brief Get current connection state
	 */
	ConnectionState get_connection_state() const;

	// =========================================================================
	// PEER MANAGEMENT
	// =========================================================================

	/**
	 * @brief Get list of connected peer IDs
	 */
	PackedInt32Array get_connected_peers() const;

	/**
	 * @brief Get information about a specific peer
	 */
	Dictionary get_peer_info(int32_t p_peer_id) const;

	/**
	 * @brief Kick a peer from the server (host only)
	 */
	void kick_peer(int32_t p_peer_id, int p_reason = 3); // 3 = KICKED

	/**
	 * @brief Get latency to a specific peer (ms)
	 */
	float get_peer_latency(int32_t p_peer_id) const;

	// =========================================================================
	// WORLD MANAGEMENT
	// =========================================================================

	/**
	 * @brief Register a world for networking
	 * 
	 * @param p_world World RID from FlecsServer
	 */
	void register_world(RID p_world);

	/**
	 * @brief Unregister a world from networking
	 */
	void unregister_world(RID p_world);

	/**
	 * @brief Check if a world is registered for networking
	 */
	bool is_world_registered(RID p_world) const;

	// =========================================================================
	// ENTITY NETWORKING
	// =========================================================================

	/**
	 * @brief Register an entity for network replication
	 * 
	 * @param p_world World the entity belongs to
	 * @param p_entity Entity RID
	 * @param p_spawn_scene Optional scene path for remote spawning
	 * @param p_spawn_data Optional custom spawn data
	 * @return Network ID assigned to the entity (0 on failure)
	 */
	uint64_t register_networked_entity(RID p_world, RID p_entity,
									   const String &p_spawn_scene = "",
									   const Dictionary &p_spawn_data = Dictionary());

	/**
	 * @brief Unregister an entity from networking
	 */
	void unregister_networked_entity(RID p_world, RID p_entity);

	/**
	 * @brief Get network ID for an entity
	 */
	uint64_t get_entity_network_id(RID p_world, RID p_entity) const;

	/**
	 * @brief Get entity RID by network ID
	 */
	RID get_entity_by_network_id(RID p_world, uint64_t p_network_id) const;

	/**
	 * @brief Check if an entity is networked
	 */
	bool is_entity_networked(RID p_world, RID p_entity) const;

	// =========================================================================
	// REPLICATION CONFIGURATION
	// =========================================================================

	/**
	 * @brief Set which components should be replicated for an entity
	 * 
	 * @param p_world World RID
	 * @param p_entity Entity RID
	 * @param p_components Array of component names to replicate
	 */
	void set_entity_replicated_components(RID p_world, RID p_entity,
										  const PackedStringArray &p_components);

	/**
	 * @brief Get replicated components for an entity
	 */
	PackedStringArray get_entity_replicated_components(RID p_world, RID p_entity) const;

	/**
	 * @brief Configure replication mode for a specific component on an entity
	 * 
	 * @param p_world World RID
	 * @param p_entity Entity RID
	 * @param p_component Component name
	 * @param p_mode Replication mode (0=CONTINUOUS, 1=ON_CHANGE, 2=RELIABLE, 3=ONCE, 4=NONE)
	 * @param p_interpolate Whether to interpolate this component
	 * @param p_priority Replication priority (higher = more important)
	 */
	void configure_component_replication(RID p_world, RID p_entity,
										 const StringName &p_component,
										 int p_mode, bool p_interpolate = false,
										 int p_priority = 128);

	// =========================================================================
	// AUTHORITY MANAGEMENT
	// =========================================================================

	/**
	 * @brief Set authority mode for an entity
	 * 
	 * @param p_world World RID
	 * @param p_entity Entity RID  
	 * @param p_mode Authority mode (0=SERVER, 1=CLIENT, 2=TRANSFERABLE, 3=SHARED)
	 * @param p_authority_peer Peer ID of authority holder
	 */
	void set_entity_authority(RID p_world, RID p_entity, int p_mode, int32_t p_authority_peer = 1);

	/**
	 * @brief Get authority mode for an entity
	 */
	int get_entity_authority_mode(RID p_world, RID p_entity) const;

	/**
	 * @brief Get the peer ID that has authority over an entity
	 */
	int32_t get_entity_authority_peer(RID p_world, RID p_entity) const;

	/**
	 * @brief Check if local peer has authority over an entity
	 */
	bool has_authority(RID p_world, RID p_entity) const;

	/**
	 * @brief Request authority transfer (for TRANSFERABLE entities)
	 */
	void request_authority(RID p_world, RID p_entity);

	/**
	 * @brief Release authority back to server
	 */
	void release_authority(RID p_world, RID p_entity);

	/**
	 * @brief Set entity owner
	 */
	void set_entity_owner(RID p_world, RID p_entity, int32_t p_owner_peer);

	/**
	 * @brief Get entity owner peer ID
	 */
	int32_t get_entity_owner(RID p_world, RID p_entity) const;

	// =========================================================================
	// INPUT & PREDICTION
	// =========================================================================

	/**
	 * @brief Send input for a client-authoritative entity
	 * 
	 * @param p_world World RID
	 * @param p_entity Entity RID
	 * @param p_input Input data dictionary
	 */
	void send_input(RID p_world, RID p_entity, const Dictionary &p_input);

	/**
	 * @brief Get the current input tick
	 */
	uint64_t get_input_tick() const;

	/**
	 * @brief Set interpolation delay
	 */
	void set_interpolation_delay(float p_delay_ms);

	/**
	 * @brief Get interpolation delay
	 */
	float get_interpolation_delay() const;

	// =========================================================================
	// REMOTE PROCEDURE CALLS
	// =========================================================================

	/**
	 * @brief Call a method on a networked entity across the network
	 * 
	 * @param p_world World RID
	 * @param p_entity Entity RID
	 * @param p_method Method name
	 * @param p_args Method arguments
	 * @param p_target Target peer (0 = all, >0 = specific peer)
	 * @param p_reliable Whether to use reliable delivery
	 */
	void entity_rpc(RID p_world, RID p_entity, const StringName &p_method,
					const Array &p_args = Array(), int32_t p_target = 0,
					bool p_reliable = true);

	/**
	 * @brief Set callback for handling incoming RPCs
	 * 
	 * Callback signature: func(world: RID, entity: RID, method: StringName, args: Array, sender: int)
	 */
	void set_rpc_callback(const Callable &p_callback);

	// =========================================================================
	// SPAWN CALLBACKS
	// =========================================================================

	/**
	 * @brief Set callback for custom entity spawning
	 * 
	 * Called when a remote entity needs to be spawned locally.
	 * Callback signature: func(world: RID, spawn_data: Dictionary) -> RID
	 */
	void set_spawn_callback(const Callable &p_callback);

	/**
	 * @brief Set callback for custom entity despawning
	 * 
	 * Called when a remote entity needs to be despawned.
	 * Callback signature: func(world: RID, entity: RID, network_id: int)
	 */
	void set_despawn_callback(const Callable &p_callback);

	// =========================================================================
	// TICK & PROCESSING
	// =========================================================================

	/**
	 * @brief Process network operations (call every physics frame)
	 * 
	 * @param p_delta Delta time since last call
	 */
	void network_process(double p_delta);

	/**
	 * @brief Get current network tick
	 */
	uint64_t get_current_tick() const;

	/**
	 * @brief Get server tick (for clients)
	 */
	uint64_t get_server_tick() const;

	/**
	 * @brief Set tick rate
	 */
	void set_tick_rate(uint32_t p_rate);

	/**
	 * @brief Get tick rate
	 */
	uint32_t get_tick_rate() const;

	// =========================================================================
	// CONFIGURATION
	// =========================================================================

	/**
	 * @brief Enable/disable automatic spawning of replicated entities
	 */
	void set_auto_spawn_enabled(bool p_enabled);

	/**
	 * @brief Check if auto spawn is enabled
	 */
	bool is_auto_spawn_enabled() const;

	/**
	 * @brief Enable/disable debug logging
	 */
	void set_debug_logging(bool p_enabled);

	/**
	 * @brief Check if debug logging is enabled
	 */
	bool is_debug_logging_enabled() const;

	// =========================================================================
	// STATISTICS
	// =========================================================================

	/**
	 * @brief Get network statistics
	 */
	Dictionary get_network_stats() const;

	/**
	 * @brief Get statistics for a specific entity
	 */
	Dictionary get_entity_network_stats(RID p_world, RID p_entity) const;

	/**
	 * @brief Reset all statistics
	 */
	void reset_network_stats();
};

// Enum binding macros
VARIANT_ENUM_CAST(NetworkServer::NetworkRole);
VARIANT_ENUM_CAST(NetworkServer::ConnectionState);

// Global pointer for singleton
extern NetworkServer *p_network_server;

#endif // NETWORK_SERVER_H