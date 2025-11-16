extends Node

## Minimal test script to verify BadAppleSystem video playback fix
## This script tests that the race condition has been resolved and video plays correctly

var world_rid: RID
var bad_apple_system: BadAppleSystem
var video_player: VideoStreamPlayer
var mm_entity_rid: RID
var test_passed := false
var frames_checked := 0
var frames_with_valid_texture := 0

func _ready():
	print("=== BadAppleSystem Fix Verification Test ===")
	print("Testing video playback initialization...")
	call_deferred("run_test")

func run_test():
	# Create ECS world
	world_rid = FlecsServer.create_world()
	print("✓ World created: ", world_rid)

	# Create video player
	video_player = VideoStreamPlayer.new()
	add_child(video_player)

	# Use a simple test pattern - replace with your actual video path
	var test_video_path = "res://videos/bad_apple.ogv"
	if ResourceLoader.exists(test_video_path):
		video_player.stream = load(test_video_path)
		print("✓ Video stream loaded: ", test_video_path)
	else:
		print("⚠ Video not found at: ", test_video_path)
		print("  Please update the path or create a test video")
		print("  Test will continue but may not show video")

	video_player.autoplay = true
	print("✓ Video player created with autoplay enabled")

	# Wait one frame to ensure video player has processed NOTIFICATION_ENTER_TREE
	await get_tree().process_frame
	print("✓ Waited one frame for video player initialization")

	# Create multimesh entity (small resolution for testing)
	var width = 160
	var height = 120
	mm_entity_rid = create_multimesh_entity(width, height)
	print("✓ MultiMesh entity created: %dx%d = %d instances" % [width, height, width * height])

	# Create and configure BadAppleSystem
	bad_apple_system = BadAppleSystem.new()
	bad_apple_system.set_world_id(world_rid)
	bad_apple_system.set_mm_entity(mm_entity_rid)
	bad_apple_system.set_video_player(video_player)
	bad_apple_system.set_mode(0)  # REGULAR mode
	print("✓ BadAppleSystem configured")

	# Start the system
	bad_apple_system.start()
	print("✓ BadAppleSystem started")

	print("\n--- Initial Video Player Status ---")
	print_video_player_debug_info()

	print("\n--- Monitoring video playback ---")
	print("Checking if video texture becomes valid within 5 seconds...")

	# Monitor for 5 seconds
	for i in range(150):  # 150 frames at 30fps = 5 seconds
		await get_tree().process_frame
		frames_checked += 1

		if video_player.is_playing():
			var texture = video_player.get_video_texture()
			if texture and texture.is_valid():
				frames_with_valid_texture += 1
				if frames_with_valid_texture >= 5:
					test_passed = true
					break

	print("\n=== TEST RESULTS ===")
	print("Frames checked: ", frames_checked)
	print("Frames with valid texture: ", frames_with_valid_texture)
	print("Video is playing: ", video_player.is_playing())

	if test_passed:
		print("\n✅ TEST PASSED!")
		print("   Video playback started successfully")
		print("   The race condition fix is working correctly")
	else:
		print("\n❌ TEST FAILED!")
		print("   Video did not start playing or texture remained invalid")
		print("   Possible issues:")
		if not video_player.is_playing():
			print("   - Video player is not playing")
			print("   - Check if video file exists and is valid")
		else:
			print("   - Video is playing but texture is invalid")
			print("   - This may indicate a codec or format issue")

	print("\n--- Final Video Player Debug Info ---")
	print_video_player_debug_info()

func print_video_player_debug_info():
	print("  Has autoplay: ", video_player.has_autoplay())
	print("  Is playing: ", video_player.is_playing())
	print("  Is paused: ", video_player.is_paused())
	print("  Is in tree: ", video_player.is_inside_tree())

	if video_player.stream:
		print("  Stream: VALID")
		print("  Stream type: ", video_player.stream.get_class())
	else:
		print("  Stream: NULL - THIS IS THE PROBLEM!")

	var tex = video_player.get_video_texture()
	if tex and tex.is_valid():
		print("  Texture: VALID")
		print("  Texture size: ", tex.get_size())
	else:
		print("  Texture: INVALID or NULL")
		if not video_player.stream:
			print("    → Reason: No stream set")
		elif not video_player.is_playing():
			print("    → Reason: Video not playing")
		else:
			print("    → Reason: Unknown (video playing but no texture)")

func create_multimesh_entity(width: int, height: int) -> RID:
	var instance_count = width * height

	# Create multimesh
	var multi_mesh = MultiMesh.new()
	multi_mesh.transform_format = MultiMesh.TRANSFORM_3D
	multi_mesh.use_colors = true  # MUST be set BEFORE instance_count
	multi_mesh.instance_count = instance_count

	# Create a simple quad mesh
	var mesh = QuadMesh.new()
	mesh.size = Vector2(0.01, 0.01)
	multi_mesh.mesh = mesh

	# Position instances in a grid (simplified - only set a few for testing)
	for y in range(min(height, 10)):
		for x in range(min(width, 10)):
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

func _exit_tree():
	if bad_apple_system:
		bad_apple_system.queue_free()

	if world_rid.is_valid():
		FlecsServer.free_world(world_rid)

	print("\n=== Test Cleanup Complete ===")
