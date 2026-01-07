# Godot Turbo Network Module

A comprehensive networking layer for Godot Turbo ECS, providing multiplayer synchronization, entity replication, and authority management.

## 🚀 Features

- **Entity Replication** - Automatic synchronization of ECS entities and components across peers
- **Authority Management** - Server, client, and transferable authority modes
- **Input Handling** - Client-side prediction with server reconciliation
- **Interpolation** - Smooth networked movement with configurable delay
- **RPC System** - Remote procedure calls on networked entities
- **Relevancy** - Interest management to optimize bandwidth
- **Delta Compression** - Efficient state synchronization
- **Debug Logging** - Comprehensive logging for troubleshooting

## 📋 Table of Contents

- [Quick Start](#quick-start)
- [Architecture](#architecture)
- [API Reference](#api-reference)
- [Components](#components)
- [Configuration](#configuration)
- [Examples](#examples)
- [Best Practices](#best-practices)

## ⚡ Quick Start

### Hosting a Game (Server)

```gdscript
extends Node

var world: RID

func _ready():
    # Create ECS world
    world = FlecsServer.create_world()
    
    # Register world for networking
    NetworkServer.register_world(world)
    
    # Start hosting
    var err = NetworkServer.host_game(7777, 16)  # Port 7777, max 16 players
    if err == OK:
        print("Server started!")
    
    # Connect signals
    NetworkServer.peer_connected.connect(_on_peer_connected)
    NetworkServer.peer_disconnected.connect(_on_peer_disconnected)

func _on_peer_connected(peer_id: int):
    print("Player ", peer_id, " connected!")
    # Spawn player entity for this peer
    spawn_player_for_peer(peer_id)

func _on_peer_disconnected(peer_id: int, reason: int):
    print("Player ", peer_id, " disconnected")

func spawn_player_for_peer(peer_id: int):
    # Create entity
    var entity = FlecsServer.create_entity_with_name(world, "Player_" + str(peer_id))
    
    # Register for networking
    var network_id = NetworkServer.register_networked_entity(
        world, entity, 
        "res://scenes/player.tscn",  # Scene for remote spawning
        {"peer_id": peer_id}         # Custom spawn data
    )
    
    # Configure replication
    NetworkServer.set_entity_replicated_components(world, entity, 
        ["Transform3DComponent", "HealthComponent", "PlayerStateComponent"])
    
    # Give authority to owning client
    NetworkServer.set_entity_authority(world, entity, 
        NetworkServer.AUTHORITY_CLIENT, peer_id)
    NetworkServer.set_entity_owner(world, entity, peer_id)

func _physics_process(delta):
    # Process network operations
    NetworkServer.network_process(delta)
    
    # Progress ECS world
    FlecsServer.progress_world(world, delta)
```

### Joining a Game (Client)

```gdscript
extends Node

var world: RID

func _ready():
    # Create local ECS world
    world = FlecsServer.create_world()
    NetworkServer.register_world(world)
    
    # Connect signals before joining
    NetworkServer.connection_succeeded.connect(_on_connected)
    NetworkServer.connection_failed.connect(_on_connection_failed)
    NetworkServer.entity_spawned_remote.connect(_on_entity_spawned)
    
    # Join server
    NetworkServer.join_game("127.0.0.1", 7777)

func _on_connected():
    print("Connected to server!")

func _on_connection_failed(reason: String):
    print("Connection failed: ", reason)

func _on_entity_spawned(network_id: int, entity: RID):
    print("Remote entity spawned: ", network_id)
    
    # Check if this is our player
    var owner = NetworkServer.get_entity_owner(world, entity)
    if owner == NetworkServer.get_local_peer_id():
        setup_local_player(entity)

func setup_local_player(entity: RID):
    # Enable prediction for our player
    NetworkServer.configure_component_replication(
        world, entity, "Transform3DComponent",
        NetworkServer.REPLICATION_CONTINUOUS, true, 255  # interpolate, high priority
    )

func _physics_process(delta):
    NetworkServer.network_process(delta)
    FlecsServer.progress_world(world, delta)
    
    # Send input for local player
    if has_authority_over_player():
        var input = collect_player_input()
        NetworkServer.send_input(world, local_player_entity, input)
```

## 🏗️ Architecture

### Module Structure

```
network/
├── components/
│   └── network_components.h    # ECS components for networking
├── systems/
│   ├── network_replication_system.h/cpp    # (Future) Automatic replication
│   └── network_interpolation_system.h/cpp  # (Future) Smooth interpolation
├── network_types.h             # Protocol definitions, packet structures
├── network_server.h/cpp        # Main singleton server
└── README.md                   # This file
```

### Data Flow

```
┌─────────────────┐     ┌─────────────────┐
│   Host/Server   │     │     Client      │
├─────────────────┤     ├─────────────────┤
│  ECS World      │     │  ECS World      │
│  (Authoritative)│     │  (Replicated)   │
└────────┬────────┘     └────────┬────────┘
         │                       │
         │ Entity Updates        │ Input Commands
         ▼                       ▼
┌─────────────────────────────────────────┐
│           NetworkServer                  │
│  - Packet serialization                 │
│  - Entity ID mapping                    │
│  - Authority management                 │
│  - Replication                          │
└─────────────────────────────────────────┘
         │                       │
         │ Godot MultiplayerPeer │
         ▼                       ▼
┌─────────────────────────────────────────┐
│           Network Transport             │
│         (ENet by default)               │
└─────────────────────────────────────────┘
```

## 📚 API Reference

### NetworkServer Singleton

#### Connection Management

| Method | Description |
|--------|-------------|
| `host_game(port, max_clients, bind_address)` | Start hosting a game server |
| `join_game(address, port)` | Connect to a hosted game |
| `disconnect_game(reason)` | Disconnect from current game |
| `is_host()` | Check if this peer is the host |
| `is_connected_to_game()` | Check if connected |
| `get_local_peer_id()` | Get local peer's ID |
| `get_network_role()` | Get current role (NONE/HOST/CLIENT) |

#### Entity Networking

| Method | Description |
|--------|-------------|
| `register_networked_entity(world, entity, scene, data)` | Register entity for replication |
| `unregister_networked_entity(world, entity)` | Stop replicating an entity |
| `get_entity_network_id(world, entity)` | Get network ID for entity |
| `get_entity_by_network_id(world, network_id)` | Find entity by network ID |
| `is_entity_networked(world, entity)` | Check if entity is networked |

#### Replication Configuration

| Method | Description |
|--------|-------------|
| `set_entity_replicated_components(world, entity, components)` | Set which components to replicate |
| `get_entity_replicated_components(world, entity)` | Get replicated component list |
| `configure_component_replication(world, entity, component, mode, interpolate, priority)` | Fine-tune replication |

#### Authority Management

| Method | Description |
|--------|-------------|
| `set_entity_authority(world, entity, mode, peer)` | Set authority mode and peer |
| `get_entity_authority_mode(world, entity)` | Get authority mode |
| `get_entity_authority_peer(world, entity)` | Get authoritative peer ID |
| `has_authority(world, entity)` | Check if local peer has authority |
| `request_authority(world, entity)` | Request authority transfer |
| `release_authority(world, entity)` | Release authority to server |

#### Input & Prediction

| Method | Description |
|--------|-------------|
| `send_input(world, entity, input)` | Send input for client-auth entity |
| `get_input_tick()` | Get current input tick |
| `set_interpolation_delay(delay_ms)` | Set interpolation delay |
| `get_interpolation_delay()` | Get interpolation delay |

#### RPC

| Method | Description |
|--------|-------------|
| `entity_rpc(world, entity, method, args, target, reliable)` | Call method on entity across network |
| `set_rpc_callback(callback)` | Set handler for incoming RPCs |

### Signals

| Signal | Arguments | Description |
|--------|-----------|-------------|
| `peer_connected` | `peer_id: int` | A peer connected |
| `peer_disconnected` | `peer_id: int, reason: int` | A peer disconnected |
| `connection_succeeded` | - | Successfully connected to server |
| `connection_failed` | `reason: String` | Failed to connect |
| `server_started` | - | Server started hosting |
| `server_stopped` | - | Server stopped |
| `entity_spawned_remote` | `network_id: int, entity: RID` | Remote entity spawned locally |
| `entity_despawned_remote` | `network_id: int` | Remote entity despawned |
| `authority_changed` | `network_id: int, new_authority: int` | Entity authority changed |

## 🧩 Components

### NetworkIdentity

Every networked entity has this component.

```cpp
struct NetworkIdentity {
    uint64_t network_id;           // Unique across network
    bool is_network_spawned;       // Was spawned by network system
    uint64_t spawn_tick;           // When spawned
    String spawn_scene_path;       // Scene for remote spawning
    Dictionary spawn_data;         // Custom spawn data
};
```

### NetworkAuthority

Defines who controls the entity.

```cpp
enum AuthorityMode {
    SERVER,       // Server has authority (default)
    CLIENT,       // Specific client has authority
    TRANSFERABLE, // Authority can be requested
    SHARED        // Last write wins (use sparingly)
};

struct NetworkAuthority {
    AuthorityMode mode;
    int32_t authority_peer_id;     // Who has authority
    int32_t owner_peer_id;         // Who owns/spawned entity
    bool is_local_authority;       // Convenience flag
};
```

### NetworkReplicated

Configures what gets replicated.

```cpp
enum ReplicationMode {
    CONTINUOUS,   // Every tick
    ON_CHANGE,    // When modified (default)
    RELIABLE,     // On change, reliable delivery
    ONCE,         // Only on spawn
    NONE          // Don't replicate
};

struct NetworkReplicated {
    Vector<ComponentReplicationConfig> replicated_components;
    bool is_active;
    float relevancy_radius;  // -1 = always relevant
};
```

### NetworkTransformInterpolation3D

Handles smooth movement interpolation.

```cpp
struct NetworkTransformInterpolation3D {
    TransformState buffer[32];     // Recent states
    int interpolation_delay_ticks; // Delay for smoothness
    Transform3D interpolated_transform;
};
```

## ⚙️ Configuration

### Tick Rate

```gdscript
# Set before hosting/joining
NetworkServer.set_tick_rate(60)  # 60 ticks per second (default)
```

### Interpolation

```gdscript
# Interpolation delay (higher = smoother, more latency)
NetworkServer.set_interpolation_delay(100.0)  # 100ms delay
```

### Debug Logging

```gdscript
NetworkServer.set_debug_logging(true)  # Enable verbose logging
```

### Auto Spawning

```gdscript
# Disable if you want manual control over spawning
NetworkServer.set_auto_spawn_enabled(false)

# Set custom spawn callback
NetworkServer.set_spawn_callback(func(world, spawn_data):
    var entity = FlecsServer.create_entity(world)
    # Custom setup...
    return entity
)
```

## 📖 Examples

### Player Movement with Prediction

```gdscript
# Client-side prediction example
extends Node

var world: RID
var local_player: RID
var predicted_position: Vector3

func _physics_process(delta):
    if not NetworkServer.has_authority(world, local_player):
        return
    
    # Collect input
    var input = {
        "direction": Input.get_vector("left", "right", "up", "down"),
        "jump": Input.is_action_just_pressed("jump")
    }
    
    # Send to server
    NetworkServer.send_input(world, local_player, input)
    
    # Apply locally (prediction)
    apply_movement(local_player, input, delta)
    
    # Server will send back authoritative state
    # If misprediction detected, reconcile

func _on_prediction_correction(entity, server_state):
    # Compare predicted vs server state
    var error = (predicted_position - server_state.position).length()
    if error > 0.1:
        # Snap or smooth correction
        apply_correction(entity, server_state)
```

### Entity RPC

```gdscript
# Calling an RPC
func deal_damage(target_entity: RID, damage: int):
    NetworkServer.entity_rpc(
        world, target_entity,
        "receive_damage",        # Method name
        [damage],                # Arguments
        0,                       # Target: 0 = all peers
        true                     # Reliable
    )

# Handling incoming RPC
func _ready():
    NetworkServer.set_rpc_callback(_on_rpc)

func _on_rpc(world: RID, entity: RID, method: StringName, args: Array, sender: int):
    match method:
        "receive_damage":
            var damage = args[0]
            apply_damage_to_entity(entity, damage)
        "play_effect":
            spawn_effect_at_entity(entity, args[0])
```

### Custom Spawn Handler

```gdscript
func _ready():
    NetworkServer.set_spawn_callback(_on_spawn_request)

func _on_spawn_request(world: RID, spawn_data: Dictionary) -> RID:
    var scene_path = spawn_data.get("scene", "")
    
    if scene_path.is_empty():
        # Create bare entity
        return FlecsServer.create_entity(world)
    
    # Load and instantiate scene
    var scene = load(scene_path)
    var instance = scene.instantiate()
    add_child(instance)
    
    # Create linked ECS entity
    var entity = FlecsServer.create_entity(world)
    FlecsServer.set_component(entity, "SceneNodeComponent", {
        "node_id": instance.get_instance_id()
    })
    
    return entity
```

## ✅ Best Practices

### 1. Authority Assignment

```gdscript
# Player entities: Client authority
NetworkServer.set_entity_authority(world, player, 
    NetworkServer.AUTHORITY_CLIENT, owning_peer_id)

# NPCs/AI: Server authority (default)
NetworkServer.set_entity_authority(world, npc, 
    NetworkServer.AUTHORITY_SERVER, 1)

# Interactables: Transferable
NetworkServer.set_entity_authority(world, pickup, 
    NetworkServer.AUTHORITY_TRANSFERABLE, 1)
```

### 2. Component Priorities

```gdscript
# Critical gameplay state: High priority, reliable
NetworkServer.configure_component_replication(
    world, entity, "HealthComponent",
    NetworkServer.REPLICATION_RELIABLE, false, 255)

# Position: High priority, continuous
NetworkServer.configure_component_replication(
    world, entity, "Transform3DComponent",
    NetworkServer.REPLICATION_CONTINUOUS, true, 200)

# Cosmetic state: Low priority
NetworkServer.configure_component_replication(
    world, entity, "AnimationComponent",
    NetworkServer.REPLICATION_ON_CHANGE, false, 50)
```

### 3. Bandwidth Optimization

- Use `ON_CHANGE` replication mode when possible
- Set appropriate `relevancy_radius` for entities
- Batch entity operations when spawning multiple entities
- Use unreliable delivery for frequently updated, non-critical data

### 4. Error Handling

```gdscript
NetworkServer.connection_failed.connect(func(reason):
    show_error_dialog("Connection failed: " + reason)
    return_to_main_menu()
)

NetworkServer.peer_disconnected.connect(func(peer_id, reason):
    if peer_id == 1:  # Server disconnected
        show_error_dialog("Lost connection to server")
        cleanup_and_return_to_menu()
)
```

## 🔧 Troubleshooting

### Connection Issues

1. **"Failed to create server"** - Port may be in use
2. **"Connection timeout"** - Check firewall, server address
3. **"Version mismatch"** - Ensure all peers use same build

### Replication Issues

1. **Entity not syncing** - Check `is_entity_networked()`, verify components are registered
2. **Jittery movement** - Increase `interpolation_delay`
3. **Input lag** - Decrease `interpolation_delay`, implement client prediction

### Debug Commands

```gdscript
# Print network stats
print(NetworkServer.get_network_stats())

# Check specific entity
print(NetworkServer.get_entity_network_stats(world, entity))

# Enable verbose logging
NetworkServer.set_debug_logging(true)
```

## 📄 License

MIT License - See main Godot Turbo LICENSE file.

## 🙏 Acknowledgments

- Godot Engine team for MultiplayerPeer API
- Flecs for the excellent ECS framework
- Overwatch GDC talks for networking architecture inspiration