# Example: Using FlecsServer Profiling API
# This script demonstrates how to collect and display Flecs system metrics

extends Node

# Reference to FlecsServer singleton
var flecs_server: Object = null

# Store metrics for comparison
var previous_metrics: Dictionary = {}

func _ready():
	# Get FlecsServer singleton
	if Engine.has_singleton("FlecsServer"):
		flecs_server = Engine.get_singleton("FlecsServer")
		print("FlecsServer profiler example initialized")
	else:
		push_error("FlecsServer singleton not found!")
		return

func _process(_delta):
	if flecs_server == null:
		return

	# Example 1: Basic system metrics collection
	example_basic_metrics()

	# Example 2: Find slowest system
	example_find_slowest()

	# Example 3: Track system performance over time
	example_track_performance()

func example_basic_metrics():
	"""Example 1: Collect and display basic system metrics"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	print("\n=== Frame %d System Metrics ===" % metrics.get("frame_count", 0))
	for system in systems:
		var name = system.get("name", "Unknown")
		var time_usec = system.get("time_usec", 0)
		var entities = system.get("entity_count", 0)
		var calls = system.get("call_count", 0)

		# Skip inactive systems
		if time_usec == 0 and calls == 0:
			continue

		print("  %s: %d µs, %d entities, %d calls" % [name, time_usec, entities, calls])

func example_find_slowest():
	"""Example 2: Find and report the slowest system this frame"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	var slowest_system = null
	var max_time = 0

	for system in systems:
		var time_usec = system.get("time_usec", 0)
		if time_usec > max_time:
			max_time = time_usec
			slowest_system = system

	if slowest_system:
		print("\n🐌 Slowest system: %s (%d µs)" % [
			slowest_system.get("name", "Unknown"),
			max_time
		])

		# Show additional details if available
		if slowest_system.has("avg_time_usec"):
			print("   Average: %d µs" % slowest_system["avg_time_usec"])
		if slowest_system.has("max_time_usec"):
			print("   Max: %d µs" % slowest_system["max_time_usec"])

func example_track_performance():
	"""Example 3: Track system performance changes"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	for system in systems:
		var name = system.get("name", "Unknown")
		var time_usec = system.get("time_usec", 0)

		if time_usec == 0:
			continue

		# Compare with previous frame
		if previous_metrics.has(name):
			var prev_time = previous_metrics[name]
			var delta_time = time_usec - prev_time
			var percent_change = 0.0

			if prev_time > 0:
				percent_change = (float(delta_time) / float(prev_time)) * 100.0

			# Alert on significant performance changes
			if abs(percent_change) > 50.0:  # 50% change
				var arrow = "📈" if delta_time > 0 else "📉"
				print("%s %s: %.1f%% change (%d -> %d µs)" % [
					arrow, name, percent_change, prev_time, time_usec
				])

		# Store for next frame
		previous_metrics[name] = time_usec

# Advanced Examples

func example_filter_by_type():
	"""Filter systems by type (script vs cpp)"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	var script_systems = []
	var cpp_systems = []

	for system in systems:
		if system.get("type") == "script":
			script_systems.append(system)
		elif system.get("type") == "cpp":
			cpp_systems.append(system)

	print("\nScript systems: %d" % script_systems.size())
	print("C++ systems: %d" % cpp_systems.size())

func example_calculate_total_ecs_time():
	"""Calculate total time spent in ECS systems"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return 0

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	var total_time_usec = 0
	for system in systems:
		total_time_usec += system.get("time_usec", 0)

	var total_time_ms = total_time_usec / 1000.0
	print("\nTotal ECS time: %.2f ms" % total_time_ms)
	return total_time_ms

func example_monitor_entity_throughput():
	"""Monitor how many entities are being processed"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	var total_entities = 0
	for system in systems:
		total_entities += system.get("entity_count", 0)

	print("\nTotal entities processed: %d" % total_entities)

func example_check_system_health():
	"""Check for potential performance issues"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])
	var systems = metrics.get("systems", [])

	var warnings = []

	for system in systems:
		var name = system.get("name", "Unknown")
		var time_usec = system.get("time_usec", 0)
		var entities = system.get("entity_count", 0)

		# Warn about slow systems
		if time_usec > 16666:  # More than 16ms (one frame at 60 FPS)
			warnings.append("⚠️ %s is taking %d µs (> 16ms)" % [name, time_usec])

		# Warn about systems processing many entities slowly
		if entities > 1000 and time_usec > 1000:
			var usec_per_entity = float(time_usec) / float(entities)
			if usec_per_entity > 10.0:  # More than 10µs per entity
				warnings.append("⚠️ %s: %.1f µs per entity (inefficient?)" % [
					name, usec_per_entity
				])

		# Warn about paused systems
		if system.get("paused", false):
			warnings.append("⏸️ %s is paused" % name)

	if warnings.size() > 0:
		print("\n=== Performance Warnings ===")
		for warning in warnings:
			print(warning)

func example_export_metrics_to_file():
	"""Export metrics to JSON for later analysis"""
	var worlds = flecs_server.get_world_list()
	if worlds.is_empty():
		return

	var metrics = flecs_server.get_system_metrics(worlds[0])

	# Convert to JSON
	var json = JSON.stringify(metrics, "\t")

	# Save to file
	var file = FileAccess.open("user://flecs_metrics.json", FileAccess.WRITE)
	if file:
		file.store_string(json)
		file.close()
		print("\nMetrics exported to user://flecs_metrics.json")

# Utility function to format time nicely
func format_time_usec(usec: int) -> String:
	if usec < 1000:
		return "%d µs" % usec
	elif usec < 1000000:
		return "%.2f ms" % (usec / 1000.0)
	else:
		return "%.2f s" % (usec / 1000000.0)
