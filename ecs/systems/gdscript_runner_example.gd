# gdscript_runner_example.gd
# Example demonstrating how to use GDScriptRunnerSystem with converted scene nodes

extends Node3D

# This script shows how to implement _flecs_process and _flecs_physics_process
# methods that will be called by the GDScriptRunnerSystem when this node
# is converted to an ECS entity.

## Called every frame by GDScriptRunnerSystem (similar to _process)
## @param entity_rid: RID - The RID of this entity in the ECS world
## @param delta: float - Time elapsed since last frame
func _flecs_process(entity_rid: RID, delta: float) -> void:
	# Access the FlecsServer to query/modify components
	var flecs = FlecsServer.get_singleton()
	if not flecs:
		return

	# Get the world RID (you need to store this when converting the scene)
	var world_rid = get_meta("flecs_world_rid", RID())
	if not world_rid.is_valid():
		return

	# Example: Read Transform3DComponent
	var transform_comp = flecs.get_component_by_name(world_rid, entity_rid, "Transform3DComponent")
	if transform_comp:
		# Modify position
		var pos = transform_comp.get("position", Vector3.ZERO)
		pos.x += delta * 1.0  # Move right
		transform_comp["position"] = pos

		# Write back to ECS
		flecs.set_component(world_rid, entity_rid, "Transform3DComponent", transform_comp)

	# Example: Read custom components
	var velocity_comp = flecs.get_component_by_name(world_rid, entity_rid, "VelocityComponent")
	if velocity_comp:
		var velocity = velocity_comp.get("velocity", Vector3.ZERO)
		# Process velocity...
		print("Entity velocity: ", velocity)


## Called at physics rate by GDScriptRunnerSystem (similar to _physics_process)
## @param entity_rid: RID - The RID of this entity in the ECS world
## @param delta: float - Physics time step (typically fixed at 0.016 for 60Hz)
func _flecs_physics_process(entity_rid: RID, delta: float) -> void:
	var flecs = FlecsServer.get_singleton()
	if not flecs:
		return

	var world_rid = get_meta("flecs_world_rid", RID())
	if not world_rid.is_valid():
		return

	# Example: Apply physics forces
	var physics_comp = flecs.get_component_by_name(world_rid, entity_rid, "PhysicsBody3DComponent")
	if physics_comp:
		# Apply gravity or other physics
		var velocity = physics_comp.get("linear_velocity", Vector3.ZERO)
		velocity.y -= 9.8 * delta  # Gravity
		physics_comp["linear_velocity"] = velocity

		flecs.set_component(world_rid, entity_rid, "PhysicsBody3DComponent", physics_comp)

	# Example: Check collision
	var collision_comp = flecs.get_component_by_name(world_rid, entity_rid, "CollisionComponent")
	if collision_comp:
		var is_colliding = collision_comp.get("is_colliding", false)
		if is_colliding:
			print("Entity is colliding!")


## Example: More complex entity logic
func _flecs_process_advanced_example(entity_rid: RID, delta: float) -> void:
	var flecs = FlecsServer.get_singleton()
	var world_rid = get_meta("flecs_world_rid", RID())

	# Get multiple components at once
	var transform = flecs.get_component_by_name(world_rid, entity_rid, "Transform3DComponent")
	var health = flecs.get_component_by_name(world_rid, entity_rid, "HealthComponent")
	var ai_state = flecs.get_component_by_name(world_rid, entity_rid, "AIStateComponent")

	# Example: AI behavior based on health
	if health and ai_state:
		var current_health = health.get("current", 100)
		var max_health = health.get("max", 100)

		if current_health < max_health * 0.3:
			# Low health - flee behavior
			ai_state["state"] = "fleeing"
			flecs.set_component(world_rid, entity_rid, "AIStateComponent", ai_state)
		elif current_health > max_health * 0.7:
			# High health - aggressive behavior
			ai_state["state"] = "aggressive"
			flecs.set_component(world_rid, entity_rid, "AIStateComponent", ai_state)

	# Example: Query nearby entities
	var nearby_query = flecs.create_query(world_rid, PackedStringArray(["Transform3DComponent", "EnemyComponent"]))
	var enemies = flecs.query_get_entities(world_rid, nearby_query)

	if transform:
		var my_pos = transform.get("position", Vector3.ZERO)

		# Find closest enemy
		var closest_distance = INF
		var closest_enemy = RID()

		for enemy_rid in enemies:
			if enemy_rid == entity_rid:
				continue  # Skip self

			var enemy_transform = flecs.get_component_by_name(world_rid, enemy_rid, "Transform3DComponent")
			if enemy_transform:
				var enemy_pos = enemy_transform.get("position", Vector3.ZERO)
				var distance = my_pos.distance_to(enemy_pos)

				if distance < closest_distance:
					closest_distance = distance
					closest_enemy = enemy_rid

		# React to closest enemy
		if closest_enemy.is_valid() and closest_distance < 10.0:
			print("Enemy nearby at distance: ", closest_distance)
			# Update AI to target this enemy

	flecs.free_query(world_rid, nearby_query)


## Example: Scene conversion workflow
static func example_convert_scene_to_ecs() -> void:
	var flecs = FlecsServer.get_singleton()
	var scene_util = SceneObjectUtility.get_singleton()

	# Create ECS world
	var world_rid = flecs.create_world()

	# Get the scene tree
	var tree = Engine.get_main_loop() as SceneTree
	if not tree:
		return

	# Convert all nodes in the scene to entities
	var entities = scene_util.create_entities_from_scene(world_rid, tree)

	print("Converted %d nodes to ECS entities" % entities.size())

	# Store world RID in each node's metadata so scripts can access it
	for child in tree.root.get_children():
		if child.has_meta("flecs_entity_rid"):
			child.set_meta("flecs_world_rid", world_rid)


## Example: Component-only approach (no scene tree)
func _flecs_process_pure_ecs_example(entity_rid: RID, delta: float) -> void:
	# This example shows how to work with pure ECS without relying on the scene tree
	var flecs = FlecsServer.get_singleton()
	var world_rid = get_meta("flecs_world_rid", RID())

	# Get all component types on this entity
	var component_types = flecs.get_component_types_as_name(world_rid, entity_rid)

	print("Entity has components: ", component_types)

	# Dynamically process based on available components
	if "Transform3DComponent" in component_types:
		var transform = flecs.get_component_by_name(world_rid, entity_rid, "Transform3DComponent")
		# Process transform...

	if "VelocityComponent" in component_types:
		var velocity = flecs.get_component_by_name(world_rid, entity_rid, "VelocityComponent")
		# Apply velocity...

	if "HealthComponent" in component_types:
		var health = flecs.get_component_by_name(world_rid, entity_rid, "HealthComponent")
		# Update health...


## Note: C# alternative naming convention
## If you're using C#, use PascalCase instead:
##
## void _FlecsProcess(Rid entityRid, float delta) { }
## void _FlecsPhysicsProcess(Rid entityRid, float delta) { }
##
## The GDScriptRunnerSystem will automatically detect which convention to use.


## Performance tip: Cache world_rid and frequently accessed components
var cached_world_rid: RID
var cached_entity_rid: RID

func _ready() -> void:
	# Cache RIDs when node is ready
	cached_world_rid = get_meta("flecs_world_rid", RID())
	cached_entity_rid = get_meta("flecs_entity_rid", RID())


func _flecs_process_optimized(entity_rid: RID, delta: float) -> void:
	# Use cached RIDs for better performance
	if not cached_world_rid.is_valid():
		return

	var flecs = FlecsServer.get_singleton()

	# Direct component access with caching
	var transform = flecs.get_component_by_name(cached_world_rid, entity_rid, "Transform3DComponent")
	if transform:
		# Fast path - component exists
		var pos = transform["position"]
		pos.x += delta
		transform["position"] = pos
		flecs.set_component(cached_world_rid, entity_rid, "Transform3DComponent", transform)
