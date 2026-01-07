# Flecs Profiler Diagnostic Script
# Attach this to a Node in your scene to verify profiler is working
extends Node

func _ready():
	print("\n=== Flecs Profiler Diagnostic ===")

	# Check if FlecsServer exists
	if not Engine.has_singleton("FlecsServer"):
		push_error("FlecsServer singleton NOT found!")
		return

	var flecs = Engine.get_singleton("FlecsServer")
	print("✓ FlecsServer singleton found")

	# Check worlds
	var worlds = flecs.get_world_list()
	print("✓ Worlds: ", worlds.size())

	if worlds.is_empty():
		print("⚠ No worlds created yet - create a world first")
		return

	var world_id = worlds[0]

	# Check systems
	var all_systems = flecs.get_all_systems(world_id)
	print("✓ Script systems: ", all_systems["script"].size())
	print("✓ C++ systems: ", all_systems["cpp"].size())

	if all_systems["script"].size() == 0 and all_systems["cpp"].size() == 0:
		print("⚠ No systems registered")
		return

	# Try to get metrics
	var metrics = flecs.get_system_metrics(world_id)
	print("✓ get_system_metrics() succeeded")
	print("  Frame count: ", metrics.get("frame_count", 0))
	print("  Systems in metrics: ", metrics.get("systems", []).size())

	# Print each system
	print("\n=== System Details ===")
	for system in metrics.get("systems", []):
		print("System: ", system.get("name", "Unknown"))
		print("  Type: ", system.get("type", "?"))
		print("  Time: ", system.get("time_usec", 0), " µs")
		print("  Calls: ", system.get("call_count", 0))
		print("  Entities: ", system.get("entity_count", 0))
		print("  Paused: ", system.get("paused", false))

	print("\n=== Profiler Check Complete ===")
	print("Now open the Flecs Profiler dock and click Start!")

func _process(_delta):
	# Optional: Print metrics every second
	if Engine.get_frames_drawn() % 60 == 0:  # Every 60 frames
		_print_quick_stats()

func _print_quick_stats():
	if not Engine.has_singleton("FlecsServer"):
		return

	var flecs = Engine.get_singleton("FlecsServer")
	var worlds = flecs.get_world_list()

	if worlds.is_empty():
		return

	var metrics = flecs.get_system_metrics(worlds[0])
	var active_systems = 0
	var total_time = 0

	for system in metrics.get("systems", []):
		if system.get("time_usec", 0) > 0:
			active_systems += 1
			total_time += system.get("time_usec", 0)

	if active_systems > 0:
		print("[Frame ", Engine.get_frames_drawn(), "] Active systems: ", active_systems, ", Total time: ", total_time, " µs")
