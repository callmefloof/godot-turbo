extends Node
# Example demonstrating FlecsQuery API usage
# This shows the high-performance manual iteration approach vs script systems

var server: FlecsServer
var world_rid: RID

# Queries
var movement_query: RID
var combat_query: RID
var player_query: RID

func _ready():
	server = FlecsServer
	setup_world()
	spawn_test_entities()
	demonstrate_query_features()

func setup_world():
	# Create and initialize world
	world_rid = server.create_world()
	server.init_world(world_rid)

	# Register components
	server.register_component_type(world_rid, "Position", {
		"x": TYPE_FLOAT,
		"y": TYPE_FLOAT
	})

	server.register_component_type(world_rid, "Velocity", {
		"x": TYPE_FLOAT,
		"y": TYPE_FLOAT
	})

	server.register_component_type(world_rid, "Health", {
		"current": TYPE_FLOAT,
		"max": TYPE_FLOAT
	})

	server.register_component_type(world_rid, "Damage", {
		"amount": TYPE_FLOAT
	})

	print("World setup complete")

func spawn_test_entities():
	# Spawn 1000 moving entities
	for i in range(1000):
		var entity = server.create_entity_with_name(world_rid, "Entity_%d" % i)

		server.set_component(entity, "Position", {
			"x": randf() * 1920.0,
			"y": randf() * 1080.0
		})

		server.set_component(entity, "Velocity", {
			"x": randf() * 200.0 - 100.0,
			"y": randf() * 200.0 - 100.0
		})

	# Spawn 100 combat entities
	for i in range(100):
		var entity = server.create_entity_with_name(world_rid, "Combatant_%d" % i)

		server.set_component(entity, "Position", {
			"x": randf() * 1920.0,
			"y": randf() * 1080.0
		})

		server.set_component(entity, "Health", {
			"current": 100.0,
			"max": 100.0
		})

		server.set_component(entity, "Damage", {
			"amount": 10.0
		})

	# Spawn 5 player entities
	for i in range(5):
		var entity = server.create_entity_with_name(world_rid, "Player_%d" % i)

		server.set_component(entity, "Position", {
			"x": 960.0 + i * 100.0,
			"y": 540.0
		})

		server.set_component(entity, "Velocity", {
			"x": 0.0,
			"y": 0.0
		})

		server.set_component(entity, "Health", {
			"current": 200.0,
			"max": 200.0
		})

	print("Spawned entities: 1000 moving, 100 combatants, 5 players")

func demonstrate_query_features():
	print("\n=== FlecsQuery Demonstration ===\n")

	# Example 1: Basic query creation
	print("1. Creating queries...")
	movement_query = server.create_query(world_rid, ["Position", "Velocity"])
	combat_query = server.create_query(world_rid, ["Health", "Damage"])
	player_query = server.create_query(world_rid, ["Position", "Health"])

	# Example 2: Get entity count
	print("\n2. Entity counts:")
	print("  Movement entities: ", server.query_get_entity_count(world_rid, movement_query))
	print("  Combat entities: ", server.query_get_entity_count(world_rid, combat_query))
	print("  Entities with health: ", server.query_get_entity_count(world_rid, player_query))

	# Example 3: Name filtering
	print("\n3. Name filtering:")
	server.query_set_filter_name_pattern(world_rid, player_query, "Player*")
	var players = server.query_get_entities(world_rid, player_query)
	print("  Players found: ", players.size())
	server.query_clear_filter(world_rid, player_query)

	# Example 4: Caching strategies
	print("\n4. Setting up caching:")
	# Movement entities are stable, use entity caching
	server.query_set_caching_strategy(world_rid, movement_query, 1)  # CACHE_ENTITIES
	print("  Movement query: CACHE_ENTITIES")

	# Example 5: Instrumentation
	print("\n5. Enabling instrumentation:")
	server.query_set_instrumentation_enabled(world_rid, movement_query, true)
	server.query_set_instrumentation_enabled(world_rid, combat_query, true)

	# Example 6: Fetch entities (RID-only)
	print("\n6. Fetching entities (RID-only mode):")
	var t0 = Time.get_ticks_usec()
	var entities = server.query_get_entities(world_rid, movement_query)
	var t1 = Time.get_ticks_usec()
	print("  Fetched %d entities in %d µs" % [entities.size(), t1 - t0])

	# Example 7: Fetch entities with components
	print("\n7. Fetching entities with components:")
	t0 = Time.get_ticks_usec()
	var entities_with_data = server.query_get_entities_with_components(world_rid, combat_query)
	t1 = Time.get_ticks_usec()
	print("  Fetched %d entities with data in %d µs" % [entities_with_data.size(), t1 - t0])

	if entities_with_data.size() > 0:
		var example = entities_with_data[0]
		print("  Example entry: rid=%s, components=%s" % [example["rid"], example["components"].keys()])

	# Example 8: Limited/paginated fetching
	print("\n8. Paginated fetching (100 entities per batch):")
	var batch_count = 0
	var offset = 0
	var batch_size = 100
	while true:
		var batch = server.query_get_entities_limited(world_rid, movement_query, batch_size, offset)
		if batch.is_empty():
			break
		batch_count += 1
		offset += batch_size
	print("  Processed %d batches" % batch_count)

	# Example 9: Entity matching
	print("\n9. Entity matching test:")
	if players.size() > 0:
		var player_rid = players[0]
		print("  Player matches movement query: ", server.query_matches_entity(world_rid, movement_query, player_rid))
		print("  Player matches combat query: ", server.query_matches_entity(world_rid, combat_query, player_rid))

	# Example 10: Instrumentation stats
	print("\n10. Instrumentation stats:")
	var stats = server.query_get_instrumentation_data(world_rid, movement_query)
	print("  Movement query:")
	print("    Total fetches: ", stats["total_fetches"])
	print("    Total entities: ", stats["total_entities_returned"])
	print("    Cache hits: ", stats["cache_hits"])
	print("    Cache misses: ", stats["cache_misses"])
	print("    Hit rate: %.1f%%" % (stats.get("cache_hit_rate", 0.0) * 100.0))

func _process(delta):
	# Progress the ECS world
	server.progress_world(world_rid, delta)

	# High-performance manual iteration using queries
	update_movement_system(delta)

	# Print stats every 60 frames
	if Engine.get_frames_drawn() % 60 == 0:
		print_performance_stats()

func update_movement_system(delta: float):
	# This is a high-performance manual iteration
	# Fetches RIDs only, then gets components individually
	var entities = server.query_get_entities(world_rid, movement_query)

	for entity_rid in entities:
		var pos = server.get_component_by_name(entity_rid, "Position")
		var vel = server.get_component_by_name(entity_rid, "Velocity")

		# Update position
		pos["x"] += vel["x"] * delta
		pos["y"] += vel["y"] * delta

		# Wrap around screen
		if pos["x"] < 0: pos["x"] += 1920.0
		if pos["x"] > 1920.0: pos["x"] -= 1920.0
		if pos["y"] < 0: pos["y"] += 1080.0
		if pos["y"] > 1080.0: pos["y"] -= 1080.0

		server.set_component(entity_rid, "Position", pos)

func print_performance_stats():
	print("\n=== Performance Stats ===")

	# Movement query stats
	var movement_stats = server.query_get_instrumentation_data(world_rid, movement_query)
	print("Movement Query:")
	print("  Entities: ", movement_stats["last_fetch_entity_count"])
	print("  Last fetch: ", movement_stats["last_fetch_usec"], " µs")
	print("  Cache hits: ", movement_stats["cache_hits"])
	print("  Hit rate: %.1f%%" % (movement_stats.get("cache_hit_rate", 0.0) * 100.0))

	# Combat query stats
	var combat_stats = server.query_get_instrumentation_data(world_rid, combat_query)
	print("Combat Query:")
	print("  Total fetches: ", combat_stats["total_fetches"])
	print("  Total entities returned: ", combat_stats["total_entities_returned"])

func example_alternative_with_components():
	# Alternative approach: fetch entities with components pre-loaded
	# Faster if you need all components, but uses more memory
	var entities = server.query_get_entities_with_components(world_rid, movement_query)

	for entity_data in entities:
		var rid = entity_data["rid"]
		var pos = entity_data["components"]["Position"]
		var vel = entity_data["components"]["Velocity"]

		# Process...
		pos["x"] += vel["x"] * 0.016
		pos["y"] += vel["y"] * 0.016

		# Write back
		server.set_component(rid, "Position", pos)

func example_chunked_processing():
	# Process entities in chunks across multiple frames
	# Useful for very large entity counts
	var chunk_size = 100
	var offset = 0

	while true:
		var batch = server.query_get_entities_limited(world_rid, movement_query, chunk_size, offset)

		if batch.is_empty():
			break

		# Process this batch
		for entity_rid in batch:
			# ... process entity ...
			pass

		offset += chunk_size

		# Optional: spread across frames
		# await get_tree().process_frame

func _exit_tree():
	# Cleanup
	if movement_query.is_valid():
		server.free_query(world_rid, movement_query)
	if combat_query.is_valid():
		server.free_query(world_rid, combat_query)
	if player_query.is_valid():
		server.free_query(world_rid, player_query)
	if world_rid.is_valid():
		server.free_world(world_rid)

	print("Cleanup complete")
