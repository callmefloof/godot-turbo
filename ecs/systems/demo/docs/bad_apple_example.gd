extends Node

## Example usage of the optimized, multithreaded BadAppleSystem
## This demonstrates how to set up and configure the system for different scenarios

# References
var world_rid: RID
var bad_apple_system: BadAppleSystem
var video_player: VideoStreamPlayer
var mm_entity_rid: RID

# Performance tracking
var frame_times: Array[float] = []
var max_samples := 100

func _ready():
	setup_basic_example()
	# Uncomment to try different configurations:
	# setup_high_performance_example()
	# setup_low_end_hardware_example()
	# setup_benchmark_example()

## Basic setup - Auto-configured for most hardware
func setup_basic_example():
	# Create ECS world
	world_rid = FlecsServer.create_world()

	# Create video player
	video_player = VideoStreamPlayer.new()
	add_child(video_player)
	video_player.stream = load("res://videos/bad_apple.ogv")
	video_player.autoplay = true

	# Create multimesh entity (assuming you have a helper function)
	mm_entity_rid = create_multimesh_entity(480, 360)

	# Create and configure BadAppleSystem
	bad_apple_system = BadAppleSystem.new()
	bad_apple_system.set_world_id(world_rid)
	bad_apple_system.set_mm_entity(mm_entity_rid)
	bad_apple_system.set_video_player(video_player)

	# Default settings (optimal for most cases)
	bad_apple_system.use_multithreading = true
	bad_apple_system.threading_threshold = 10000
	bad_apple_system.max_threads = 8
	bad_apple_system.mode = 0  # REGULAR mode

	# Start the system
	bad_apple_system.start()

	print("BadAppleSystem started with default multithreading settings")

## High-performance setup - For powerful CPUs (8+ cores)
func setup_high_performance_example():
	world_rid = FlecsServer.create_world()

	video_player = VideoStreamPlayer.new()
	add_child(video_player)
	video_player.stream = load("res://videos/bad_apple_hd.ogv")  # 1280x720
	video_player.autoplay = true

	# Create larger multimesh for HD video
	mm_entity_rid = create_multimesh_entity(1280, 720)

	bad_apple_system = BadAppleSystem.new()
	bad_apple_system.set_world_id(world_rid)
	bad_apple_system.set_mm_entity(mm_entity_rid)
	bad_apple_system.set_video_player(video_player)

	# Aggressive threading for large workloads
	bad_apple_system.use_multithreading = true
	bad_apple_system.threading_threshold = 5000   # Lower threshold
	bad_apple_system.max_threads = 16            # More threads
	bad_apple_system.mode = 0

	bad_apple_system.start()

	print("BadAppleSystem started in high-performance mode")
	print("Video resolution: 1280x720 (921,600 pixels)")
	print("Max threads: 16")

## Low-end hardware setup - For 2-4 core CPUs
func setup_low_end_hardware_example():
	world_rid = FlecsServer.create_world()

	video_player = VideoStreamPlayer.new()
	add_child(video_player)
	video_player.stream = load("res://videos/bad_apple_low.ogv")  # 240x180
	video_player.autoplay = true

	# Create smaller multimesh
	mm_entity_rid = create_multimesh_entity(240, 180)

	bad_apple_system = BadAppleSystem.new()
	bad_apple_system.set_world_id(world_rid)
	bad_apple_system.set_mm_entity(mm_entity_rid)
	bad_apple_system.set_video_player(video_player)

	# Conservative threading for low-end hardware
	bad_apple_system.use_multithreading = true
	bad_apple_system.threading_threshold = 30000  # Higher threshold
	bad_apple_system.max_threads = 4              # Limit threads
	bad_apple_system.mode = 0

	bad_apple_system.start()

	print("BadAppleSystem started in low-end mode")
	print("Video resolution: 240x180 (43,200 pixels)")
	print("Max threads: 4")

## Benchmark example - Compare single vs multithreaded performance
func setup_benchmark_example():
	world_rid = FlecsServer.create_world()

	video_player = VideoStreamPlayer.new()
	add_child(video_player)
	video_player.stream = load("res://videos/bad_apple.ogv")
	video_player.autoplay = true

	mm_entity_rid = create_multimesh_entity(480, 360)

	bad_apple_system = BadAppleSystem.new()
	bad_apple_system.set_world_id(world_rid)
	bad_apple_system.set_mm_entity(mm_entity_rid)
	bad_apple_system.set_video_player(video_player)
	bad_apple_system.mode = 0

	# Start with single-threaded
	bad_apple_system.use_multithreading = false
	bad_apple_system.start()

	print("=== BENCHMARK MODE ===")
	print("Starting single-threaded benchmark...")

	# Benchmark single-threaded for 3 seconds
	await get_tree().create_timer(3.0).timeout
	var single_avg = calculate_average_frame_time()
	print("Single-threaded avg frame time: %.2f μs" % single_avg)

	frame_times.clear()

	# Switch to multithreaded
	bad_apple_system.use_multithreading = true
	bad_apple_system.max_threads = 8

	print("Starting multithreaded benchmark...")

	await get_tree().create_timer(3.0).timeout
	var multi_avg = calculate_average_frame_time()
	print("Multithreaded avg frame time: %.2f μs" % multi_avg)

	# Calculate speedup
	var speedup = single_avg / multi_avg
	print("=== RESULTS ===")
	print("Speedup: %.2fx" % speedup)
	print("Single-threaded: %.2f μs/frame" % single_avg)
	print("Multithreaded: %.2f μs/frame" % multi_avg)
	print("Frame budget @ 30fps: 33,333 μs")
	print("Processing overhead: %.1f%%" % ((multi_avg / 33333.0) * 100.0))

## Runtime mode switching example
func demonstrate_mode_switching():
	# Switch between different visual modes
	print("Switching to REGULAR mode")
	bad_apple_system.mode = 0  # Black & white

	await get_tree().create_timer(5.0).timeout

	print("Switching to INVERTED mode")
	bad_apple_system.mode = 1  # Inverted black & white

	await get_tree().create_timer(5.0).timeout

	print("Switching to RANDOM mode")
	bad_apple_system.mode = 2  # Random colors (uses fast hash)

	await get_tree().create_timer(5.0).timeout

	print("Back to REGULAR mode")
	bad_apple_system.mode = 0

## Dynamic threading adjustment based on load
func adaptive_threading_example():
	var target_frame_time_us := 16666  # 60fps budget
	var current_avg := 0.0

	while true:
		await get_tree().create_timer(1.0).timeout

		current_avg = calculate_average_frame_time()
		frame_times.clear()

		# If we're exceeding budget, try adding threads
		if current_avg > target_frame_time_us * 0.8:
			if bad_apple_system.max_threads < 16:
				bad_apple_system.max_threads += 2
				print("Increasing threads to: %d" % bad_apple_system.max_threads)

		# If we have headroom, reduce threads to save power
		elif current_avg < target_frame_time_us * 0.3:
			if bad_apple_system.max_threads > 2:
				bad_apple_system.max_threads -= 2
				print("Reducing threads to: %d" % bad_apple_system.max_threads)

		print("Avg frame time: %.2f μs (target: %d μs)" % [current_avg, target_frame_time_us])

## Helper: Create multimesh entity with appropriate instance count
func create_multimesh_entity(width: int, height: int) -> RID:
	var instance_count = width * height

	# Create multimesh
	var multi_mesh = MultiMesh.new()
	multi_mesh.transform_format = MultiMesh.TRANSFORM_3D
	multi_mesh.instance_count = instance_count
	multi_mesh.use_colors = true

	# Create a simple quad mesh
	var mesh = QuadMesh.new()
	mesh.size = Vector2(0.01, 0.01)
	multi_mesh.mesh = mesh

	# Position instances in a grid
	for y in height:
		for x in width:
			var idx = y * width + x
			var transform = Transform3D()
			transform.origin = Vector3(
				(x - width / 2.0) * 0.01,
				(y - height / 2.0) * 0.01,
				0
			)
			multi_mesh.set_instance_transform(idx, transform)
			multi_mesh.set_instance_color(idx, Color.WHITE)

	# Create entity with MultiMeshComponent
	var entity_rid = FlecsServer.create_entity(world_rid)

	# Set MultiMeshComponent using Dictionary (RID-based API)
	var mm_component_data = {
		"multi_mesh_id": multi_mesh.get_rid(),
		"instance_count": instance_count,
		"has_data": false,
		"has_color": true,
		"is_instanced": false,
		"transform_format": RenderingServer.MULTIMESH_TRANSFORM_3D
	}

	FlecsServer.set_component(entity_rid, "MultiMeshComponent", mm_component_data)

	return entity_rid

## Helper: Calculate average frame processing time
func calculate_average_frame_time() -> float:
	if frame_times.is_empty():
		return 0.0

	var sum := 0.0
	for time in frame_times:
		sum += time

	return sum / frame_times.size()

## Track frame times (call from _process or system callback)
func record_frame_time(time_us: float):
	frame_times.append(time_us)
	if frame_times.size() > max_samples:
		frame_times.pop_front()

## Display real-time performance stats
func _process(_delta):
	if frame_times.size() >= 10:
		var _avg = calculate_average_frame_time()
		# Update UI label or debug overlay
		# $DebugLabel.text = "Avg frame time: %.2f μs" % _avg

## Cleanup
func _exit_tree():
	if bad_apple_system:
		bad_apple_system.queue_free()

	if world_rid.is_valid():
		FlecsServer.free_world(world_rid)
