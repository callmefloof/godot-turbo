#ifndef NETWORK_TYPES_H
#define NETWORK_TYPES_H

/**************************************************************************/
/*  network_types.h                                                       */
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

#include "core/io/marshalls.h"
#include "core/string/string_name.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include <cstdint>

/**
 * @file network_types.h
 * @brief Network protocol definitions, packet structures, and type enumerations
 * 
 * This file defines the core networking types used by Godot Turbo's multiplayer
 * system. It includes:
 * 
 * - Packet type enumerations
 * - Network message structures
 * - Protocol constants
 * - Serialization helpers
 */

namespace NetworkTypes {

//=============================================================================
// PROTOCOL CONSTANTS
//=============================================================================

/// Protocol version - increment when breaking changes are made
constexpr uint16_t PROTOCOL_VERSION = 1;

/// Magic bytes for packet validation
constexpr uint32_t PACKET_MAGIC = 0x47544E45; // "GTNE" - Godot Turbo Network ECS

/// Maximum packet size (bytes)
constexpr uint32_t MAX_PACKET_SIZE = 65535;

/// Maximum entities per snapshot packet
constexpr uint32_t MAX_ENTITIES_PER_PACKET = 256;

/// Maximum components per entity update
constexpr uint32_t MAX_COMPONENTS_PER_UPDATE = 32;

/// Default tick rate (ticks per second)
constexpr uint32_t DEFAULT_TICK_RATE = 60;

/// Default interpolation delay (ticks)
constexpr uint32_t DEFAULT_INTERPOLATION_DELAY = 2;

/// Maximum input buffer size
constexpr uint32_t MAX_INPUT_BUFFER_SIZE = 32;

/// Snapshot history size for delta compression
constexpr uint32_t SNAPSHOT_HISTORY_SIZE = 64;

//=============================================================================
// PACKET TYPES
//=============================================================================

/**
 * @enum PacketType
 * @brief Identifies the type of network packet
 */
enum class PacketType : uint8_t {
	// Connection & Handshake (0x00 - 0x0F)
	HANDSHAKE_REQUEST = 0x00,
	HANDSHAKE_RESPONSE = 0x01,
	HANDSHAKE_COMPLETE = 0x02,
	DISCONNECT = 0x03,
	PING = 0x04,
	PONG = 0x05,
	
	// World State (0x10 - 0x1F)
	WORLD_SNAPSHOT_FULL = 0x10,
	WORLD_SNAPSHOT_DELTA = 0x11,
	WORLD_TICK_SYNC = 0x12,
	
	// Entity Operations (0x20 - 0x2F)
	ENTITY_SPAWN = 0x20,
	ENTITY_DESPAWN = 0x21,
	ENTITY_UPDATE = 0x22,
	ENTITY_UPDATE_BATCH = 0x23,
	ENTITY_AUTHORITY_CHANGE = 0x24,
	ENTITY_OWNERSHIP_CHANGE = 0x25,
	
	// Component Operations (0x30 - 0x3F)
	COMPONENT_ADD = 0x30,
	COMPONENT_REMOVE = 0x31,
	COMPONENT_UPDATE = 0x32,
	COMPONENT_UPDATE_DELTA = 0x33,
	
	// Input & Prediction (0x40 - 0x4F)
	INPUT_COMMAND = 0x40,
	INPUT_ACK = 0x41,
	PREDICTION_CORRECTION = 0x42,
	
	// RPCs (0x50 - 0x5F)
	RPC_CALL = 0x50,
	RPC_RESPONSE = 0x51,
	
	// System Messages (0xF0 - 0xFF)
	ERROR = 0xF0,
	DEBUG_INFO = 0xFE,
	CUSTOM = 0xFF
};

/**
 * @enum DisconnectReason
 * @brief Reason codes for disconnection
 */
enum class DisconnectReason : uint8_t {
	NONE = 0,
	GRACEFUL = 1,
	TIMEOUT = 2,
	KICKED = 3,
	BANNED = 4,
	SERVER_FULL = 5,
	VERSION_MISMATCH = 6,
	AUTHENTICATION_FAILED = 7,
	INVALID_PACKET = 8,
	INTERNAL_ERROR = 9
};

/**
 * @enum ErrorCode
 * @brief Network error codes
 */
enum class ErrorCode : uint16_t {
	OK = 0,
	
	// Connection errors (100-199)
	ERR_NOT_CONNECTED = 100,
	ERR_ALREADY_CONNECTED = 101,
	ERR_CONNECTION_FAILED = 102,
	ERR_TIMEOUT = 103,
	
	// Protocol errors (200-299)
	ERR_INVALID_PACKET = 200,
	ERR_VERSION_MISMATCH = 201,
	ERR_SEQUENCE_ERROR = 202,
	ERR_CHECKSUM_FAILED = 203,
	
	// Authority errors (300-399)
	ERR_NO_AUTHORITY = 300,
	ERR_AUTHORITY_CONFLICT = 301,
	ERR_INVALID_OWNER = 302,
	
	// Entity errors (400-499)
	ERR_ENTITY_NOT_FOUND = 400,
	ERR_ENTITY_ALREADY_EXISTS = 401,
	ERR_INVALID_NETWORK_ID = 402,
	ERR_SPAWN_FAILED = 403,
	
	// Component errors (500-599)
	ERR_COMPONENT_NOT_FOUND = 500,
	ERR_COMPONENT_NOT_REGISTERED = 501,
	ERR_SERIALIZATION_FAILED = 502,
	
	// RPC errors (600-699)
	ERR_RPC_NOT_FOUND = 600,
	ERR_RPC_INVALID_ARGS = 601,
	ERR_RPC_PERMISSION_DENIED = 602
};

//=============================================================================
// TRANSFER MODE
//=============================================================================

/**
 * @enum TransferMode
 * @brief How packets should be sent
 */
enum class TransferMode : uint8_t {
	/// Unreliable, unordered (UDP-like, fastest)
	UNRELIABLE = 0,
	
	/// Unreliable but ordered within channel
	UNRELIABLE_ORDERED = 1,
	
	/// Reliable, unordered
	RELIABLE = 2,
	
	/// Reliable and ordered (TCP-like, slowest)
	RELIABLE_ORDERED = 3
};

//=============================================================================
// PACKET HEADERS
//=============================================================================

/**
 * @struct PacketHeader
 * @brief Common header for all network packets
 * 
 * Every packet starts with this header for identification and validation.
 * Total size: 16 bytes
 */
struct PacketHeader {
	uint32_t magic = PACKET_MAGIC;       // 4 bytes - Validation
	uint16_t protocol_version = PROTOCOL_VERSION; // 2 bytes
	PacketType type = PacketType::CUSTOM; // 1 byte
	uint8_t flags = 0;                   // 1 byte - Reserved for future use
	uint64_t tick = 0;                   // 8 bytes - Server tick
	
	bool is_valid() const {
		return magic == PACKET_MAGIC && protocol_version == PROTOCOL_VERSION;
	}
	
	int serialize(uint8_t* p_buffer, int p_max_size) const {
		if (p_max_size < 16) return -1;
		
		encode_uint32(magic, p_buffer);
		encode_uint16(protocol_version, p_buffer + 4);
		p_buffer[6] = static_cast<uint8_t>(type);
		p_buffer[7] = flags;
		encode_uint64(tick, p_buffer + 8);
		
		return 16;
	}
	
	int deserialize(const uint8_t* p_buffer, int p_size) {
		if (p_size < 16) return -1;
		
		magic = decode_uint32(p_buffer);
		protocol_version = decode_uint16(p_buffer + 4);
		type = static_cast<PacketType>(p_buffer[6]);
		flags = p_buffer[7];
		tick = decode_uint64(p_buffer + 8);
		
		return 16;
	}
};

//=============================================================================
// MESSAGE STRUCTURES
//=============================================================================

/**
 * @struct HandshakeRequest
 * @brief Client -> Server handshake initiation
 */
struct HandshakeRequest {
	uint16_t client_version = PROTOCOL_VERSION;
	String client_name;
	String auth_token;
	Dictionary client_info;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["version"] = client_version;
		d["name"] = client_name;
		d["auth_token"] = auth_token;
		d["info"] = client_info;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		client_version = d.get("version", PROTOCOL_VERSION);
		client_name = d.get("name", "");
		auth_token = d.get("auth_token", "");
		client_info = d.get("info", Dictionary());
	}
};

/**
 * @struct HandshakeResponse
 * @brief Server -> Client handshake response
 */
struct HandshakeResponse {
	bool accepted = false;
	int32_t assigned_peer_id = 0;
	DisconnectReason reject_reason = DisconnectReason::NONE;
	String reject_message;
	uint64_t server_tick = 0;
	uint32_t tick_rate = DEFAULT_TICK_RATE;
	Dictionary server_info;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["accepted"] = accepted;
		d["peer_id"] = assigned_peer_id;
		d["reject_reason"] = static_cast<int>(reject_reason);
		d["reject_message"] = reject_message;
		d["server_tick"] = server_tick;
		d["tick_rate"] = tick_rate;
		d["info"] = server_info;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		accepted = d.get("accepted", false);
		assigned_peer_id = d.get("peer_id", 0);
		reject_reason = static_cast<DisconnectReason>(int(d.get("reject_reason", 0)));
		reject_message = d.get("reject_message", "");
		server_tick = d.get("server_tick", 0);
		tick_rate = d.get("tick_rate", DEFAULT_TICK_RATE);
		server_info = d.get("info", Dictionary());
	}
};

/**
 * @struct EntitySpawnMessage
 * @brief Message for spawning a networked entity
 */
struct EntitySpawnMessage {
	uint64_t network_id = 0;
	int32_t owner_peer_id = 1;
	int32_t authority_peer_id = 1;
	String spawn_scene_path;
	Dictionary spawn_data;
	Dictionary initial_components;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["network_id"] = network_id;
		d["owner"] = owner_peer_id;
		d["authority"] = authority_peer_id;
		d["scene"] = spawn_scene_path;
		d["spawn_data"] = spawn_data;
		d["components"] = initial_components;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		network_id = d.get("network_id", 0);
		owner_peer_id = d.get("owner", 1);
		authority_peer_id = d.get("authority", 1);
		spawn_scene_path = d.get("scene", "");
		spawn_data = d.get("spawn_data", Dictionary());
		initial_components = d.get("components", Dictionary());
	}
};

/**
 * @struct EntityDespawnMessage
 * @brief Message for despawning a networked entity
 */
struct EntityDespawnMessage {
	uint64_t network_id = 0;
	DisconnectReason reason = DisconnectReason::NONE;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["network_id"] = network_id;
		d["reason"] = static_cast<int>(reason);
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		network_id = d.get("network_id", 0);
		reason = static_cast<DisconnectReason>(int(d.get("reason", 0)));
	}
};

/**
 * @struct ComponentUpdate
 * @brief Single component update within an entity update
 */
struct ComponentUpdate {
	StringName component_name;
	Dictionary component_data;
	bool is_delta = false;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["name"] = component_name;
		d["data"] = component_data;
		d["delta"] = is_delta;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		component_name = d.get("name", StringName());
		component_data = d.get("data", Dictionary());
		is_delta = d.get("delta", false);
	}
};

/**
 * @struct EntityUpdateMessage
 * @brief Message for updating entity component(s)
 */
struct EntityUpdateMessage {
	uint64_t network_id = 0;
	uint64_t tick = 0;
	Vector<ComponentUpdate> components;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["network_id"] = network_id;
		d["tick"] = tick;
		
		Array comp_array;
		for (const auto &comp : components) {
			comp_array.push_back(comp.to_dict());
		}
		d["components"] = comp_array;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		network_id = d.get("network_id", 0);
		tick = d.get("tick", 0);
		
		Array comp_array = d.get("components", Array());
		components.clear();
		for (int i = 0; i < comp_array.size(); i++) {
			ComponentUpdate comp;
			comp.from_dict(comp_array[i]);
			components.push_back(comp);
		}
	}
};

/**
 * @struct EntityUpdateBatch
 * @brief Batched entity updates for efficiency
 */
struct EntityUpdateBatch {
	uint64_t tick = 0;
	Vector<EntityUpdateMessage> updates;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["tick"] = tick;
		
		Array updates_array;
		for (const auto &update : updates) {
			updates_array.push_back(update.to_dict());
		}
		d["updates"] = updates_array;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		tick = d.get("tick", 0);
		
		Array updates_array = d.get("updates", Array());
		updates.clear();
		for (int i = 0; i < updates_array.size(); i++) {
			EntityUpdateMessage msg;
			msg.from_dict(updates_array[i]);
			updates.push_back(msg);
		}
	}
};

/**
 * @struct InputCommand
 * @brief Client input sent to server
 */
struct InputCommand {
	uint64_t input_tick = 0;
	uint64_t network_id = 0;  // Which entity this input is for
	Dictionary input_data;
	
	// Include recent unacknowledged inputs for redundancy
	Vector<Dictionary> input_history;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["tick"] = input_tick;
		d["network_id"] = network_id;
		d["input"] = input_data;
		
		Array history;
		for (const auto &h : input_history) {
			history.push_back(h);
		}
		d["history"] = history;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		input_tick = d.get("tick", 0);
		network_id = d.get("network_id", 0);
		input_data = d.get("input", Dictionary());
		
		Array history = d.get("history", Array());
		input_history.clear();
		for (int i = 0; i < history.size(); i++) {
			input_history.push_back(history[i]);
		}
	}
};

/**
 * @struct InputAck
 * @brief Server acknowledgment of processed input
 */
struct InputAck {
	uint64_t acked_tick = 0;
	uint64_t network_id = 0;
	Dictionary authoritative_state;  // Server's state at this tick
	
	Dictionary to_dict() const {
		Dictionary d;
		d["tick"] = acked_tick;
		d["network_id"] = network_id;
		d["state"] = authoritative_state;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		acked_tick = d.get("tick", 0);
		network_id = d.get("network_id", 0);
		authoritative_state = d.get("state", Dictionary());
	}
};

/**
 * @struct RPCMessage
 * @brief Remote procedure call on an entity
 */
struct RPCMessage {
	uint64_t rpc_id = 0;  // For response matching
	uint64_t network_id = 0;
	StringName method_name;
	Array arguments;
	int32_t sender_peer_id = 0;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["rpc_id"] = rpc_id;
		d["network_id"] = network_id;
		d["method"] = method_name;
		d["args"] = arguments;
		d["sender"] = sender_peer_id;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		rpc_id = d.get("rpc_id", 0);
		network_id = d.get("network_id", 0);
		method_name = d.get("method", StringName());
		arguments = d.get("args", Array());
		sender_peer_id = d.get("sender", 0);
	}
};

/**
 * @struct WorldSnapshot
 * @brief Full or delta world state snapshot
 */
struct WorldSnapshot {
	uint64_t tick = 0;
	uint64_t base_tick = 0;  // For delta, which snapshot this is relative to
	bool is_delta = false;
	Vector<EntitySpawnMessage> spawned_entities;
	Vector<uint64_t> despawned_entity_ids;
	Vector<EntityUpdateMessage> entity_updates;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["tick"] = tick;
		d["base_tick"] = base_tick;
		d["is_delta"] = is_delta;
		
		Array spawned;
		for (const auto &e : spawned_entities) {
			spawned.push_back(e.to_dict());
		}
		d["spawned"] = spawned;
		
		Array despawned;
		for (const auto &id : despawned_entity_ids) {
			despawned.push_back(id);
		}
		d["despawned"] = despawned;
		
		Array updates;
		for (const auto &u : entity_updates) {
			updates.push_back(u.to_dict());
		}
		d["updates"] = updates;
		
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		tick = d.get("tick", 0);
		base_tick = d.get("base_tick", 0);
		is_delta = d.get("is_delta", false);
		
		Array spawned = d.get("spawned", Array());
		spawned_entities.clear();
		for (int i = 0; i < spawned.size(); i++) {
			EntitySpawnMessage msg;
			msg.from_dict(spawned[i]);
			spawned_entities.push_back(msg);
		}
		
		Array despawned = d.get("despawned", Array());
		despawned_entity_ids.clear();
		for (int i = 0; i < despawned.size(); i++) {
			despawned_entity_ids.push_back(despawned[i]);
		}
		
		Array updates = d.get("updates", Array());
		entity_updates.clear();
		for (int i = 0; i < updates.size(); i++) {
			EntityUpdateMessage msg;
			msg.from_dict(updates[i]);
			entity_updates.push_back(msg);
		}
	}
};

/**
 * @struct TickSyncMessage
 * @brief Server tick synchronization for clients
 */
struct TickSyncMessage {
	uint64_t server_tick = 0;
	uint64_t server_time_usec = 0;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["tick"] = server_tick;
		d["time"] = server_time_usec;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		server_tick = d.get("tick", 0);
		server_time_usec = d.get("time", 0);
	}
};

/**
 * @struct AuthorityChangeMessage
 * @brief Notification of authority transfer
 */
struct AuthorityChangeMessage {
	uint64_t network_id = 0;
	int32_t new_authority_peer_id = 1;
	int32_t old_authority_peer_id = 1;
	uint64_t change_tick = 0;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["network_id"] = network_id;
		d["new_authority"] = new_authority_peer_id;
		d["old_authority"] = old_authority_peer_id;
		d["tick"] = change_tick;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		network_id = d.get("network_id", 0);
		new_authority_peer_id = d.get("new_authority", 1);
		old_authority_peer_id = d.get("old_authority", 1);
		change_tick = d.get("tick", 0);
	}
};

/**
 * @struct ErrorMessage
 * @brief Network error notification
 */
struct ErrorMessage {
	ErrorCode error_code = ErrorCode::OK;
	String error_message;
	Dictionary error_data;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["code"] = static_cast<int>(error_code);
		d["message"] = error_message;
		d["data"] = error_data;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		error_code = static_cast<ErrorCode>(int(d.get("code", 0)));
		error_message = d.get("message", "");
		error_data = d.get("data", Dictionary());
	}
};

//=============================================================================
// PEER INFO
//=============================================================================

/**
 * @struct PeerInfo
 * @brief Information about a connected peer
 */
struct PeerInfo {
	int32_t peer_id = 0;
	String peer_name;
	bool is_host = false;
	bool is_authenticated = false;
	uint64_t connect_time_usec = 0;
	uint64_t last_seen_tick = 0;
	
	// Network quality metrics
	float latency_ms = 0.0f;
	float jitter_ms = 0.0f;
	float packet_loss = 0.0f;
	
	// Owned entities
	Vector<uint64_t> owned_entity_ids;
	
	Dictionary to_dict() const {
		Dictionary d;
		d["peer_id"] = peer_id;
		d["name"] = peer_name;
		d["is_host"] = is_host;
		d["is_authenticated"] = is_authenticated;
		d["connect_time"] = connect_time_usec;
		d["last_seen"] = last_seen_tick;
		d["latency_ms"] = latency_ms;
		d["jitter_ms"] = jitter_ms;
		d["packet_loss"] = packet_loss;
		
		Array owned;
		for (const auto &id : owned_entity_ids) {
			owned.push_back(id);
		}
		d["owned_entities"] = owned;
		return d;
	}
	
	void from_dict(const Dictionary &d) {
		peer_id = d.get("peer_id", 0);
		peer_name = d.get("name", "");
		is_host = d.get("is_host", false);
		is_authenticated = d.get("is_authenticated", false);
		connect_time_usec = d.get("connect_time", 0);
		last_seen_tick = d.get("last_seen", 0);
		latency_ms = d.get("latency_ms", 0.0f);
		jitter_ms = d.get("jitter_ms", 0.0f);
		packet_loss = d.get("packet_loss", 0.0f);
		
		Array owned = d.get("owned_entities", Array());
		owned_entity_ids.clear();
		for (int i = 0; i < owned.size(); i++) {
			owned_entity_ids.push_back(owned[i]);
		}
	}
};

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

/**
 * @brief Get human-readable name for packet type
 */
inline String packet_type_to_string(PacketType p_type) {
	switch (p_type) {
		case PacketType::HANDSHAKE_REQUEST: return "HANDSHAKE_REQUEST";
		case PacketType::HANDSHAKE_RESPONSE: return "HANDSHAKE_RESPONSE";
		case PacketType::HANDSHAKE_COMPLETE: return "HANDSHAKE_COMPLETE";
		case PacketType::DISCONNECT: return "DISCONNECT";
		case PacketType::PING: return "PING";
		case PacketType::PONG: return "PONG";
		case PacketType::WORLD_SNAPSHOT_FULL: return "WORLD_SNAPSHOT_FULL";
		case PacketType::WORLD_SNAPSHOT_DELTA: return "WORLD_SNAPSHOT_DELTA";
		case PacketType::WORLD_TICK_SYNC: return "WORLD_TICK_SYNC";
		case PacketType::ENTITY_SPAWN: return "ENTITY_SPAWN";
		case PacketType::ENTITY_DESPAWN: return "ENTITY_DESPAWN";
		case PacketType::ENTITY_UPDATE: return "ENTITY_UPDATE";
		case PacketType::ENTITY_UPDATE_BATCH: return "ENTITY_UPDATE_BATCH";
		case PacketType::ENTITY_AUTHORITY_CHANGE: return "ENTITY_AUTHORITY_CHANGE";
		case PacketType::ENTITY_OWNERSHIP_CHANGE: return "ENTITY_OWNERSHIP_CHANGE";
		case PacketType::COMPONENT_ADD: return "COMPONENT_ADD";
		case PacketType::COMPONENT_REMOVE: return "COMPONENT_REMOVE";
		case PacketType::COMPONENT_UPDATE: return "COMPONENT_UPDATE";
		case PacketType::COMPONENT_UPDATE_DELTA: return "COMPONENT_UPDATE_DELTA";
		case PacketType::INPUT_COMMAND: return "INPUT_COMMAND";
		case PacketType::INPUT_ACK: return "INPUT_ACK";
		case PacketType::PREDICTION_CORRECTION: return "PREDICTION_CORRECTION";
		case PacketType::RPC_CALL: return "RPC_CALL";
		case PacketType::RPC_RESPONSE: return "RPC_RESPONSE";
		case PacketType::ERROR: return "ERROR";
		case PacketType::DEBUG_INFO: return "DEBUG_INFO";
		case PacketType::CUSTOM: return "CUSTOM";
		default: return "UNKNOWN";
	}
}

/**
 * @brief Get human-readable name for error code
 */
inline String error_code_to_string(ErrorCode p_code) {
	switch (p_code) {
		case ErrorCode::OK: return "OK";
		case ErrorCode::ERR_NOT_CONNECTED: return "ERR_NOT_CONNECTED";
		case ErrorCode::ERR_ALREADY_CONNECTED: return "ERR_ALREADY_CONNECTED";
		case ErrorCode::ERR_CONNECTION_FAILED: return "ERR_CONNECTION_FAILED";
		case ErrorCode::ERR_TIMEOUT: return "ERR_TIMEOUT";
		case ErrorCode::ERR_INVALID_PACKET: return "ERR_INVALID_PACKET";
		case ErrorCode::ERR_VERSION_MISMATCH: return "ERR_VERSION_MISMATCH";
		case ErrorCode::ERR_SEQUENCE_ERROR: return "ERR_SEQUENCE_ERROR";
		case ErrorCode::ERR_CHECKSUM_FAILED: return "ERR_CHECKSUM_FAILED";
		case ErrorCode::ERR_NO_AUTHORITY: return "ERR_NO_AUTHORITY";
		case ErrorCode::ERR_AUTHORITY_CONFLICT: return "ERR_AUTHORITY_CONFLICT";
		case ErrorCode::ERR_INVALID_OWNER: return "ERR_INVALID_OWNER";
		case ErrorCode::ERR_ENTITY_NOT_FOUND: return "ERR_ENTITY_NOT_FOUND";
		case ErrorCode::ERR_ENTITY_ALREADY_EXISTS: return "ERR_ENTITY_ALREADY_EXISTS";
		case ErrorCode::ERR_INVALID_NETWORK_ID: return "ERR_INVALID_NETWORK_ID";
		case ErrorCode::ERR_SPAWN_FAILED: return "ERR_SPAWN_FAILED";
		case ErrorCode::ERR_COMPONENT_NOT_FOUND: return "ERR_COMPONENT_NOT_FOUND";
		case ErrorCode::ERR_COMPONENT_NOT_REGISTERED: return "ERR_COMPONENT_NOT_REGISTERED";
		case ErrorCode::ERR_SERIALIZATION_FAILED: return "ERR_SERIALIZATION_FAILED";
		case ErrorCode::ERR_RPC_NOT_FOUND: return "ERR_RPC_NOT_FOUND";
		case ErrorCode::ERR_RPC_INVALID_ARGS: return "ERR_RPC_INVALID_ARGS";
		case ErrorCode::ERR_RPC_PERMISSION_DENIED: return "ERR_RPC_PERMISSION_DENIED";
		default: return "UNKNOWN_ERROR";
	}
}

/**
 * @brief Get default transfer mode for packet type
 */
inline TransferMode get_default_transfer_mode(PacketType p_type) {
	switch (p_type) {
		// Critical packets that must arrive
		case PacketType::HANDSHAKE_REQUEST:
		case PacketType::HANDSHAKE_RESPONSE:
		case PacketType::HANDSHAKE_COMPLETE:
		case PacketType::DISCONNECT:
		case PacketType::ENTITY_SPAWN:
		case PacketType::ENTITY_DESPAWN:
		case PacketType::ENTITY_AUTHORITY_CHANGE:
		case PacketType::ENTITY_OWNERSHIP_CHANGE:
		case PacketType::COMPONENT_ADD:
		case PacketType::COMPONENT_REMOVE:
		case PacketType::RPC_CALL:
		case PacketType::RPC_RESPONSE:
		case PacketType::ERROR:
			return TransferMode::RELIABLE_ORDERED;
		
		// Important but can handle some loss
		case PacketType::WORLD_SNAPSHOT_FULL:
		case PacketType::INPUT_COMMAND:
		case PacketType::INPUT_ACK:
		case PacketType::PREDICTION_CORRECTION:
			return TransferMode::RELIABLE;
		
		// Frequent updates, okay to lose some
		case PacketType::ENTITY_UPDATE:
		case PacketType::ENTITY_UPDATE_BATCH:
		case PacketType::COMPONENT_UPDATE:
		case PacketType::COMPONENT_UPDATE_DELTA:
		case PacketType::WORLD_SNAPSHOT_DELTA:
			return TransferMode::UNRELIABLE_ORDERED;
		
		// Timing-sensitive, unreliable is fine
		case PacketType::PING:
		case PacketType::PONG:
		case PacketType::WORLD_TICK_SYNC:
			return TransferMode::UNRELIABLE;
		
		default:
			return TransferMode::RELIABLE;
	}
}

} // namespace NetworkTypes

#endif // NETWORK_TYPES_H