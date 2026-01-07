#ifndef NETWORK_COMPONENTS_H
#define NETWORK_COMPONENTS_H

/**************************************************************************/
/*  network_components.h                                                  */
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

#include "core/math/transform_2d.h"
#include "core/math/transform_3d.h"
#include "core/string/string_name.h"
#include "core/templates/hash_set.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"

#include <cstdint>

/**
 * @file network_components.h
 * @brief ECS components for networking and multiplayer synchronization
 * 
 * This file defines all ECS components required for networked multiplayer
 * functionality in Godot Turbo. These components work together with the
 * NetworkServer singleton and network systems to provide:
 * 
 * - Entity identification across network peers
 * - Authority and ownership management
 * - Component replication configuration
 * - Transform interpolation for smooth networked movement
 * - Network state change tracking
 */

namespace NetworkComponents {

//=============================================================================
// CORE NETWORK IDENTITY
//=============================================================================

/**
 * @struct NetworkIdentity
 * @brief Uniquely identifies an entity across the network
 * 
 * Every networked entity must have a NetworkIdentity component. This provides
 * a network-unique ID that is consistent across all peers, unlike local
 * entity IDs which may differ between client and server.
 * 
 * @note Network IDs are assigned by the server/host and synchronized to clients
 * 
 * @example
 * ```cpp
 * // Server spawns a networked entity
 * auto entity = world.entity()
 *     .set<NetworkIdentity>({ network_server->generate_network_id(), true });
 * 
 * // Later, on client, find entity by network ID
 * auto* identity = entity.get<NetworkIdentity>();
 * uint64_t net_id = identity->network_id;
 * ```
 */
struct NetworkIdentity {
	/// Globally unique network identifier (assigned by server)
	uint64_t network_id = 0;
	
	/// Whether this entity was spawned by the network system
	/// (vs pre-existing scene entities)
	bool is_network_spawned = false;
	
	/// Spawn tick - when this entity was created (for late-join sync)
	uint64_t spawn_tick = 0;
	
	/// Scene/prefab path for spawning on remote peers (empty = custom spawn)
	String spawn_scene_path;
	
	/// Custom spawn data for reconstruction on remote peers
	Dictionary spawn_data;
	
	NetworkIdentity() = default;
	NetworkIdentity(uint64_t p_id, bool p_spawned = true) 
		: network_id(p_id), is_network_spawned(p_spawned) {}
};

//=============================================================================
// AUTHORITY & OWNERSHIP
//=============================================================================

/**
 * @enum AuthorityMode
 * @brief Defines who has control over a networked entity
 */
enum class AuthorityMode : uint8_t {
	/// Server has full authority (default, most secure)
	SERVER = 0,
	
	/// Specific client has authority (for player-controlled entities)
	CLIENT = 1,
	
	/// Authority can be transferred between peers
	TRANSFERABLE = 2,
	
	/// No specific authority - last write wins (risky, use sparingly)
	SHARED = 3
};

/**
 * @struct NetworkAuthority
 * @brief Defines who controls/owns a networked entity
 * 
 * Authority determines which peer's updates are considered authoritative.
 * This is crucial for preventing conflicts and ensuring consistent state.
 * 
 * For player-controlled entities, the owning client typically has authority.
 * For NPCs/world objects, the server typically has authority.
 * 
 * @example
 * ```cpp
 * // Server-authoritative NPC
 * entity.set<NetworkAuthority>({ AuthorityMode::SERVER, 1, 1 });
 * 
 * // Client-authoritative player (owned by peer 2)
 * entity.set<NetworkAuthority>({ AuthorityMode::CLIENT, 2, 2 });
 * ```
 */
struct NetworkAuthority {
	/// Who has authority over this entity
	AuthorityMode mode = AuthorityMode::SERVER;
	
	/// Peer ID of the authority holder (1 = server/host)
	int32_t authority_peer_id = 1;
	
	/// Peer ID of the "owner" (may differ from authority)
	/// Owner is who spawned/requested the entity
	int32_t owner_peer_id = 1;
	
	/// Whether local peer has authority over this entity
	/// (Updated by network system based on local peer ID)
	bool is_local_authority = false;
	
	/// Tick when authority was last changed (for conflict resolution)
	uint64_t authority_change_tick = 0;
	
	NetworkAuthority() = default;
	NetworkAuthority(AuthorityMode p_mode, int32_t p_auth_peer = 1, int32_t p_owner_peer = 1)
		: mode(p_mode), authority_peer_id(p_auth_peer), owner_peer_id(p_owner_peer) {}
};

//=============================================================================
// REPLICATION CONFIGURATION
//=============================================================================

/**
 * @enum ReplicationMode
 * @brief How frequently/reliably a component should be replicated
 */
enum class ReplicationMode : uint8_t {
	/// Replicate every tick (high bandwidth, lowest latency)
	CONTINUOUS = 0,
	
	/// Only replicate when changed (default, efficient)
	ON_CHANGE = 1,
	
	/// Replicate on change with reliable delivery (important state)
	RELIABLE = 2,
	
	/// One-time replication (spawn data, configuration)
	ONCE = 3,
	
	/// Never replicate (local-only component)
	NONE = 4
};

/**
 * @struct ComponentReplicationConfig
 * @brief Configuration for how a single component type should replicate
 */
struct ComponentReplicationConfig {
	/// Component type name (e.g., "Transform3DComponent")
	StringName component_name;
	
	/// Replication mode for this component
	ReplicationMode mode = ReplicationMode::ON_CHANGE;
	
	/// Priority (higher = replicated first when bandwidth limited)
	uint8_t priority = 128;
	
	/// Interpolation enabled for this component
	bool interpolate = false;
	
	/// Minimum time between updates (rate limiting) in milliseconds
	uint32_t min_update_interval_ms = 0;
	
	/// Custom serialization function name (empty = use default)
	StringName custom_serializer;
	
	/// Last tick this component was sent (internal use)
	uint64_t last_replicated_tick = 0;
};

/**
 * @struct NetworkReplicated
 * @brief Marks an entity for network replication and configures what to sync
 * 
 * Add this component to any entity that should be synchronized across the
 * network. Configure which components to replicate and how.
 * 
 * @example
 * ```cpp
 * NetworkReplicated replicated;
 * replicated.add_component("Transform3DComponent", ReplicationMode::CONTINUOUS, true);
 * replicated.add_component("HealthComponent", ReplicationMode::RELIABLE);
 * entity.set<NetworkReplicated>(replicated);
 * ```
 */
struct NetworkReplicated {
	/// Per-component replication configuration
	Vector<ComponentReplicationConfig> replicated_components;
	
	/// Whether this entity is currently being replicated
	bool is_active = true;
	
	/// Relevancy radius (-1 = always relevant, 0 = never, >0 = distance)
	float relevancy_radius = -1.0f;
	
	/// Last tick any component was replicated
	uint64_t last_replication_tick = 0;
	
	/// Add a component to replicate
	void add_component(const StringName &p_name, 
					   ReplicationMode p_mode = ReplicationMode::ON_CHANGE,
					   bool p_interpolate = false,
					   uint8_t p_priority = 128) {
		ComponentReplicationConfig config;
		config.component_name = p_name;
		config.mode = p_mode;
		config.interpolate = p_interpolate;
		config.priority = p_priority;
		replicated_components.push_back(config);
	}
	
	/// Check if a component is configured for replication
	bool has_component(const StringName &p_name) const {
		for (const auto &config : replicated_components) {
			if (config.component_name == p_name) {
				return true;
			}
		}
		return false;
	}
	
	/// Get configuration for a specific component
	ComponentReplicationConfig* get_component_config(const StringName &p_name) {
		for (auto &config : replicated_components) {
			if (config.component_name == p_name) {
				return &config;
			}
		}
		return nullptr;
	}
	
	NetworkReplicated() = default;
};

//=============================================================================
// CHANGE TRACKING
//=============================================================================

/**
 * @struct NetworkDirty
 * @brief Tag component indicating entity has changes to replicate
 * 
 * This is a marker/tag component added by the change detection system
 * when a replicated component is modified. The replication system
 * processes entities with this tag and removes it after sending.
 */
struct NetworkDirty {
	/// Bitmask or set of dirty component indices
	HashSet<StringName> dirty_components;
	
	/// Tick when entity became dirty
	uint64_t dirty_since_tick = 0;
	
	void mark_dirty(const StringName &p_component, uint64_t p_tick) {
		dirty_components.insert(p_component);
		if (dirty_since_tick == 0) {
			dirty_since_tick = p_tick;
		}
	}
	
	void clear() {
		dirty_components.clear();
		dirty_since_tick = 0;
	}
	
	bool is_dirty() const {
		return !dirty_components.is_empty();
	}
};

/**
 * @struct NetworkPendingSpawn
 * @brief Tag for entities waiting to be spawned on remote peers
 */
struct NetworkPendingSpawn {};

/**
 * @struct NetworkPendingDestroy
 * @brief Tag for entities waiting to be destroyed on remote peers
 */
struct NetworkPendingDestroy {
	/// Tick when destruction was requested
	uint64_t destroy_tick = 0;
};

//=============================================================================
// INTERPOLATION & PREDICTION
//=============================================================================

/**
 * @struct InterpolationState
 * @brief Generic interpolation buffer entry
 */
struct InterpolationState {
	/// Server tick this state is from
	uint64_t tick = 0;
	
	/// Local timestamp when received
	uint64_t received_time_usec = 0;
	
	/// Serialized state data
	Dictionary state_data;
};

/**
 * @struct NetworkInterpolation
 * @brief Manages interpolation buffer for smooth networked movement
 * 
 * Stores recent states received from the network to interpolate between,
 * providing smooth visual movement despite network latency and jitter.
 * 
 * @note Only used on non-authoritative peers (clients for server-auth entities)
 */
struct NetworkInterpolation {
	/// Circular buffer of recent states
	static constexpr int BUFFER_SIZE = 32;
	InterpolationState buffer[BUFFER_SIZE];
	int buffer_head = 0;
	int buffer_count = 0;
	
	/// Interpolation delay in ticks (higher = smoother but more latency)
	int interpolation_delay_ticks = 2;
	
	/// Current interpolation position (0.0 - 1.0 between two states)
	float interpolation_t = 0.0f;
	
	/// Indices of states being interpolated between
	int from_index = -1;
	int to_index = -1;
	
	/// Whether interpolation is active
	bool is_interpolating = false;
	
	/// Push a new state into the buffer
	void push_state(uint64_t p_tick, uint64_t p_time, const Dictionary &p_data) {
		buffer[buffer_head] = { p_tick, p_time, p_data };
		buffer_head = (buffer_head + 1) % BUFFER_SIZE;
		if (buffer_count < BUFFER_SIZE) {
			buffer_count++;
		}
	}
	
	/// Get state at buffer index (0 = oldest)
	const InterpolationState* get_state(int p_index) const {
		if (p_index < 0 || p_index >= buffer_count) {
			return nullptr;
		}
		int actual_index = (buffer_head - buffer_count + p_index + BUFFER_SIZE) % BUFFER_SIZE;
		return &buffer[actual_index];
	}
	
	/// Clear the buffer
	void clear() {
		buffer_head = 0;
		buffer_count = 0;
		interpolation_t = 0.0f;
		from_index = -1;
		to_index = -1;
		is_interpolating = false;
	}
};

/**
 * @struct NetworkTransformInterpolation3D
 * @brief Specialized 3D transform interpolation for common case
 * 
 * Optimized interpolation specifically for Transform3D, the most common
 * interpolated component. Uses native types instead of Dictionary.
 */
struct NetworkTransformInterpolation3D {
	/// Recent transform states
	struct TransformState {
		uint64_t tick = 0;
		uint64_t received_time_usec = 0;
		Transform3D transform;
	};
	
	static constexpr int BUFFER_SIZE = 32;
	TransformState buffer[BUFFER_SIZE];
	int buffer_head = 0;
	int buffer_count = 0;
	
	/// Interpolation parameters
	int interpolation_delay_ticks = 2;
	float interpolation_t = 0.0f;
	
	/// Interpolated result (updated each frame)
	Transform3D interpolated_transform;
	
	/// Extrapolation limit in ticks (0 = no extrapolation)
	int max_extrapolation_ticks = 3;
	
	void push_state(uint64_t p_tick, uint64_t p_time, const Transform3D &p_transform) {
		buffer[buffer_head] = { p_tick, p_time, p_transform };
		buffer_head = (buffer_head + 1) % BUFFER_SIZE;
		if (buffer_count < BUFFER_SIZE) {
			buffer_count++;
		}
	}
	
	const TransformState* get_state(int p_index) const {
		if (p_index < 0 || p_index >= buffer_count) {
			return nullptr;
		}
		int actual_index = (buffer_head - buffer_count + p_index + BUFFER_SIZE) % BUFFER_SIZE;
		return &buffer[actual_index];
	}
	
	void clear() {
		buffer_head = 0;
		buffer_count = 0;
		interpolation_t = 0.0f;
		interpolated_transform = Transform3D();
	}
};

/**
 * @struct NetworkTransformInterpolation2D
 * @brief Specialized 2D transform interpolation
 */
struct NetworkTransformInterpolation2D {
	struct TransformState {
		uint64_t tick = 0;
		uint64_t received_time_usec = 0;
		Transform2D transform;
	};
	
	static constexpr int BUFFER_SIZE = 32;
	TransformState buffer[BUFFER_SIZE];
	int buffer_head = 0;
	int buffer_count = 0;
	
	int interpolation_delay_ticks = 2;
	float interpolation_t = 0.0f;
	Transform2D interpolated_transform;
	int max_extrapolation_ticks = 3;
	
	void push_state(uint64_t p_tick, uint64_t p_time, const Transform2D &p_transform) {
		buffer[buffer_head] = { p_tick, p_time, p_transform };
		buffer_head = (buffer_head + 1) % BUFFER_SIZE;
		if (buffer_count < BUFFER_SIZE) {
			buffer_count++;
		}
	}
	
	const TransformState* get_state(int p_index) const {
		if (p_index < 0 || p_index >= buffer_count) {
			return nullptr;
		}
		int actual_index = (buffer_head - buffer_count + p_index + BUFFER_SIZE) % BUFFER_SIZE;
		return &buffer[actual_index];
	}
	
	void clear() {
		buffer_head = 0;
		buffer_count = 0;
		interpolation_t = 0.0f;
		interpolated_transform = Transform2D();
	}
};

//=============================================================================
// CLIENT-SIDE PREDICTION
//=============================================================================

/**
 * @struct NetworkPrediction
 * @brief Client-side prediction state for responsive input
 * 
 * Stores predicted states for client-authoritative input handling.
 * When server confirms/corrects, we can reconcile.
 */
struct NetworkPrediction {
	/// Circular buffer of predicted states
	struct PredictedState {
		uint64_t input_tick = 0;
		Dictionary predicted_state;
		Dictionary input_data;
	};
	
	static constexpr int BUFFER_SIZE = 64;
	PredictedState buffer[BUFFER_SIZE];
	int buffer_head = 0;
	int buffer_count = 0;
	
	/// Last confirmed tick from server
	uint64_t last_confirmed_tick = 0;
	
	/// Last confirmed state from server
	Dictionary last_confirmed_state;
	
	/// Whether prediction is active
	bool is_predicting = false;
	
	/// Number of mispredictions detected
	uint32_t misprediction_count = 0;
	
	void push_prediction(uint64_t p_tick, const Dictionary &p_state, const Dictionary &p_input) {
		buffer[buffer_head] = { p_tick, p_state, p_input };
		buffer_head = (buffer_head + 1) % BUFFER_SIZE;
		if (buffer_count < BUFFER_SIZE) {
			buffer_count++;
		}
	}
	
	/// Get predicted state for a specific tick
	const PredictedState* get_prediction(uint64_t p_tick) const {
		for (int i = 0; i < buffer_count; i++) {
			int idx = (buffer_head - 1 - i + BUFFER_SIZE) % BUFFER_SIZE;
			if (buffer[idx].input_tick == p_tick) {
				return &buffer[idx];
			}
		}
		return nullptr;
	}
	
	/// Clear predictions older than a tick
	void clear_before_tick(uint64_t p_tick) {
		// Keep only predictions at or after p_tick
		int new_count = 0;
		for (int i = 0; i < buffer_count; i++) {
			int idx = (buffer_head - buffer_count + i + BUFFER_SIZE) % BUFFER_SIZE;
			if (buffer[idx].input_tick >= p_tick) {
				new_count = buffer_count - i;
				break;
			}
		}
		buffer_count = new_count;
	}
};

//=============================================================================
// NETWORK INPUT
//=============================================================================

/**
 * @struct NetworkInput
 * @brief Stores input to be sent to server for processing
 * 
 * Used for client-authoritative entities where input is processed locally
 * and also sent to server for validation/replication.
 */
struct NetworkInput {
	/// Current input tick
	uint64_t input_tick = 0;
	
	/// Input data (game-specific, e.g., movement direction, actions)
	Dictionary input_data;
	
	/// Buffer of recent inputs for retransmission
	struct InputFrame {
		uint64_t tick = 0;
		Dictionary data;
		bool acknowledged = false;
	};
	
	static constexpr int BUFFER_SIZE = 32;
	InputFrame buffer[BUFFER_SIZE];
	int buffer_head = 0;
	int buffer_count = 0;
	
	void push_input(uint64_t p_tick, const Dictionary &p_data) {
		buffer[buffer_head] = { p_tick, p_data, false };
		buffer_head = (buffer_head + 1) % BUFFER_SIZE;
		if (buffer_count < BUFFER_SIZE) {
			buffer_count++;
		}
		input_tick = p_tick;
		input_data = p_data;
	}
	
	void acknowledge_input(uint64_t p_tick) {
		for (int i = 0; i < buffer_count; i++) {
			int idx = (buffer_head - 1 - i + BUFFER_SIZE) % BUFFER_SIZE;
			if (buffer[idx].tick <= p_tick) {
				buffer[idx].acknowledged = true;
			}
		}
	}
	
	/// Get unacknowledged inputs for retransmission
	Vector<InputFrame> get_unacknowledged() const {
		Vector<InputFrame> result;
		for (int i = 0; i < buffer_count; i++) {
			int idx = (buffer_head - buffer_count + i + BUFFER_SIZE) % BUFFER_SIZE;
			if (!buffer[idx].acknowledged) {
				result.push_back(buffer[idx]);
			}
		}
		return result;
	}
};

//=============================================================================
// NETWORK STATISTICS
//=============================================================================

/**
 * @struct NetworkStats
 * @brief Per-entity network statistics for debugging/monitoring
 */
struct NetworkStats {
	/// Bytes sent for this entity (total)
	uint64_t bytes_sent = 0;
	
	/// Bytes received for this entity (total)
	uint64_t bytes_received = 0;
	
	/// Number of updates sent
	uint64_t updates_sent = 0;
	
	/// Number of updates received
	uint64_t updates_received = 0;
	
	/// Last update timestamp
	uint64_t last_update_time_usec = 0;
	
	/// Average update interval (milliseconds)
	float avg_update_interval_ms = 0.0f;
	
	/// Number of interpolation underruns (buffer exhausted)
	uint32_t interpolation_underruns = 0;
	
	/// Number of prediction corrections
	uint32_t prediction_corrections = 0;
	
	void record_send(uint64_t p_bytes) {
		bytes_sent += p_bytes;
		updates_sent++;
	}
	
	void record_receive(uint64_t p_bytes, uint64_t p_time) {
		bytes_received += p_bytes;
		updates_received++;
		
		if (last_update_time_usec > 0) {
			float interval = (p_time - last_update_time_usec) / 1000.0f;
			avg_update_interval_ms = avg_update_interval_ms * 0.9f + interval * 0.1f;
		}
		last_update_time_usec = p_time;
	}
};

//=============================================================================
// RELEVANCY & INTEREST MANAGEMENT
//=============================================================================

/**
 * @struct NetworkRelevancy
 * @brief Controls which peers receive updates for this entity
 * 
 * Used for interest management / area of interest systems to reduce
 * bandwidth by only sending updates to peers that care about this entity.
 */
struct NetworkRelevancy {
	/// Set of peer IDs this entity is relevant to (empty = all peers)
	HashSet<int32_t> relevant_peers;
	
	/// Whether to use distance-based relevancy
	bool use_distance_relevancy = false;
	
	/// Maximum distance for relevancy (if use_distance_relevancy)
	float max_relevancy_distance = 1000.0f;
	
	/// Priority boost for nearby peers (0.0 - 1.0)
	float distance_priority_scale = 1.0f;
	
	/// Force always relevant to owner
	bool always_relevant_to_owner = true;
	
	bool is_relevant_to(int32_t p_peer_id) const {
		if (relevant_peers.is_empty()) {
			return true; // Relevant to all
		}
		return relevant_peers.has(p_peer_id);
	}
	
	void set_relevant_to(int32_t p_peer_id, bool p_relevant) {
		if (p_relevant) {
			relevant_peers.insert(p_peer_id);
		} else {
			relevant_peers.erase(p_peer_id);
		}
	}
};

//=============================================================================
// NETWORK EVENTS
//=============================================================================

/**
 * @struct NetworkRPCQueue
 * @brief Queue of pending RPC calls for this entity
 */
struct NetworkRPCQueue {
	struct RPCCall {
		StringName method_name;
		Array arguments;
		int32_t target_peer = 0; // 0 = all, >0 = specific peer
		bool reliable = true;
	};
	
	Vector<RPCCall> pending_calls;
	
	void queue_rpc(const StringName &p_method, const Array &p_args, 
				   int32_t p_target = 0, bool p_reliable = true) {
		pending_calls.push_back({ p_method, p_args, p_target, p_reliable });
	}
	
	void clear() {
		pending_calls.clear();
	}
};

} // namespace NetworkComponents

#endif // NETWORK_COMPONENTS_H