extends Node

## VideoPlayer Diagnostic Helper
## Attach this to any scene with a VideoStreamPlayer to diagnose playback issues
## This will help identify why BadAppleSystem might not be working

@export var video_player_path: NodePath
@export var auto_diagnose: bool = true
@export var continuous_monitoring: bool = false

var video_player: VideoStreamPlayer
var diagnosis_complete: bool = false

func _ready():
	if auto_diagnose:
		call_deferred("diagnose")

func diagnose():
	print("\n" + "="*60)
	print("VIDEO PLAYER DIAGNOSTIC TOOL")
	print("="*60)

	# Step 1: Find the video player
	if video_player_path.is_empty():
		print("\n❌ ERROR: video_player_path not set!")
		print("   → Set the NodePath to your VideoStreamPlayer in the inspector")
		return

	video_player = get_node_or_null(video_player_path)
	if not video_player:
		print("\n❌ ERROR: Could not find VideoStreamPlayer at path: ", video_player_path)
		return

	if not video_player is VideoStreamPlayer:
		print("\n❌ ERROR: Node at path is not a VideoStreamPlayer!")
		print("   Found type: ", video_player.get_class())
		return

	print("\n✓ VideoStreamPlayer found: ", video_player.name)

	# Step 2: Check scene tree status
	print("\n--- Scene Tree Status ---")
	print("  Is in tree: ", video_player.is_inside_tree())
	print("  Is ready: ", video_player.is_node_ready())
	print("  Process mode: ", video_player.process_mode)

	if not video_player.is_inside_tree():
		print("\n⚠ WARNING: Video player is NOT in the scene tree!")
		print("   → Video cannot play unless it's added to the tree")
		print("   → Use add_child() before calling play()")

	# Step 3: Check stream
	print("\n--- Stream Status ---")
	var stream = video_player.get_stream()
	if not stream:
		print("  ❌ NO STREAM SET - This is likely your problem!")
		print("   → Set video_player.stream = load('res://path/to/video.ogv')")
		return

	print("  ✓ Stream is set")
	print("  Stream type: ", stream.get_class())
	print("  Stream resource path: ", stream.resource_path)

	# Step 4: Check playback settings
	print("\n--- Playback Settings ---")
	print("  Autoplay: ", video_player.has_autoplay())
	print("  Paused: ", video_player.is_paused())
	print("  Volume: ", video_player.get_volume())
	print("  Volume (dB): ", video_player.get_volume_db())
	print("  Speed scale: ", video_player.get_speed_scale())

	# Step 5: Check playback state
	print("\n--- Playback State ---")
	print("  Is playing: ", video_player.is_playing())

	if not video_player.is_playing() and video_player.has_autoplay():
		print("  ⚠ WARNING: Autoplay is enabled but video is NOT playing")
		print("   → This might be a timing issue")
		print("   → Trying to start playback manually...")

		video_player.play()
		await get_tree().process_frame

		if video_player.is_playing():
			print("  ✓ Manual play() succeeded!")
		else:
			print("  ❌ Manual play() failed - video still not playing")
			print("   → Check if video file is valid")
			print("   → Check console for codec errors")

	# Step 6: Check texture
	print("\n--- Texture Status ---")
	var texture = video_player.get_video_texture()
	if not texture:
		print("  ❌ Texture is NULL")
		if not video_player.is_playing():
			print("   → Reason: Video is not playing")
		else:
			print("   → Reason: Unknown (video playing but no texture)")
			print("   → This might indicate a codec issue")
	elif not texture.is_valid():
		print("  ❌ Texture is INVALID")
		print("   → The texture exists but cannot be used")
	else:
		print("  ✓ Texture is valid!")
		print("  Texture size: ", texture.get_size())
		print("  Texture class: ", texture.get_class())

		# Try to get image from texture
		var image = texture.get_image()
		if image and image.is_valid():
			print("  ✓ Image is valid!")
			print("  Image size: ", image.get_size())
			print("  Image format: ", image.get_format())
		else:
			print("  ❌ Cannot get valid image from texture")

	# Step 7: Final recommendation
	print("\n" + "="*60)
	print("DIAGNOSIS SUMMARY")
	print("="*60)

	if not video_player.is_inside_tree():
		print("❌ PROBLEM: Video player not in scene tree")
		print("   FIX: Add video player to scene with add_child()")
	elif not stream:
		print("❌ PROBLEM: No video stream set")
		print("   FIX: Set video_player.stream = load('res://your_video.ogv')")
	elif not video_player.is_playing():
		print("❌ PROBLEM: Video not playing")
		print("   FIX OPTIONS:")
		print("   1. Call video_player.play() explicitly")
		print("   2. Enable autoplay and wait one frame after add_child()")
		print("   3. Use: await get_tree().process_frame before starting BadAppleSystem")
	elif not texture or not texture.is_valid():
		print("❌ PROBLEM: Video playing but texture invalid")
		print("   POSSIBLE CAUSES:")
		print("   1. Codec not supported - try converting to .ogv (Theora)")
		print("   2. Video file corrupted")
		print("   3. Wait a few frames for texture to initialize")
	else:
		print("✅ ALL CHECKS PASSED!")
		print("   Video player appears to be working correctly")
		print("   If BadAppleSystem still doesn't work, check:")
		print("   1. MultiMesh entity is valid")
		print("   2. World RID is valid")
		print("   3. MultiMeshComponent is properly set")
		print("   4. use_colors was set BEFORE instance_count")

	print("="*60 + "\n")

	diagnosis_complete = true

	if continuous_monitoring:
		print("Continuous monitoring enabled - will update every second")

func _process(_delta):
	if continuous_monitoring and diagnosis_complete:
		pass  # Could add periodic status updates here

## Call this from code to get current status
func get_status() -> Dictionary:
	if not video_player:
		return {"error": "No video player set"}

	var texture = video_player.get_video_texture()

	return {
		"in_tree": video_player.is_inside_tree(),
		"has_stream": video_player.get_stream() != null,
		"is_playing": video_player.is_playing(),
		"is_paused": video_player.is_paused(),
		"has_autoplay": video_player.has_autoplay(),
		"texture_valid": texture != null and texture.is_valid(),
		"texture_size": texture.get_size() if texture else Vector2.ZERO
	}

## Print a quick status line (useful for continuous monitoring)
func print_status():
	var status = get_status()
	if status.has("error"):
		print("ERROR: ", status.error)
		return

	var playing_icon = "▶" if status.is_playing else "⏸"
	var texture_icon = "🖼" if status.texture_valid else "❌"

	print("%s Playing: %s | Texture: %s %s" % [
		playing_icon,
		status.is_playing,
		texture_icon,
		status.texture_size
	])
