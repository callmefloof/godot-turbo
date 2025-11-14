extends Node
## Runtime Component Creation Example
##
## This example demonstrates how to use create_runtime_component() to dynamically
## create component types at runtime with custom fields and types.

func _ready():
	# Create a world
	var world_id = FlecsServer.create_world()
	FlecsServer.init_world(world_id)

	print("=== Runtime Component Creation Example ===\n")

	# Example 1: Create a simple Health component with primitive types
	print("Example 1: Creating Health component with primitives")
	var health_fields = {
		"current": 100,      # int
		"max": 100,          # int
		"regen_rate": 5.0    # float
	}
	var health_comp_id = FlecsServer.create_runtime_component(world_id, "Health", health_fields)
	if health_comp_id.is_valid():
		print("✓ Health component created successfully: ", health_comp_id)
	else:
		print("✗ Failed to create Health component")

	# Example 2: Create a Player component with mixed Godot types
	print("\nExample 2: Creating Player component with Godot types")
	var player_fields = {
		"name": "Hero",                    # String
		"position": Vector3(0, 0, 0),      # Vector3
		"velocity": Vector3(0, 0, 0),      # Vector3
		"color": Color(1, 0, 0, 1),        # Color
		"level": 1,                        # int
		"experience": 0.0                  # float
	}
	var player_comp_id = FlecsServer.create_runtime_component(world_id, "Player", player_fields)
	if player_comp_id.is_valid():
		print("✓ Player component created successfully: ", player_comp_id)
	else:
		print("✗ Failed to create Player component")

	# Example 3: Create an Inventory component with complex types
	print("\nExample 3: Creating Inventory component with complex types")
	var inventory_fields = {
		"items": [],                       # Array
		"capacity": 20,                    # int
		"metadata": {},                    # Dictionary
		"equipped_weapon": RID()           # RID
	}
	var inventory_comp_id = FlecsServer.create_runtime_component(world_id, "Inventory", inventory_fields)
	if inventory_comp_id.is_valid():
		print("✓ Inventory component created successfully: ", inventory_comp_id)
	else:
		print("✗ Failed to create Inventory component")

	# Example 4: Test error handling - try to create duplicate component
	print("\nExample 4: Testing duplicate component error handling")
	var duplicate_comp_id = FlecsServer.create_runtime_component(world_id, "Health", {"hp": 50})
	if !duplicate_comp_id.is_valid():
		print("✓ Correctly prevented duplicate component creation")
	else:
		print("✗ Should have failed to create duplicate component")

	# Example 5: Use the runtime components on entities
	print("\nExample 5: Using runtime components on entities")
	var entity = FlecsServer.create_entity_with_name(world_id, "PlayerEntity")

	# Add the Player component
	FlecsServer.add_component(entity, player_comp_id)

	# Set the component data
	FlecsServer.set_component(entity, "Player", {
		"name": "Warrior",
		"position": Vector3(10, 0, 5),
		"velocity": Vector3(1, 0, 0),
		"color": Color(0, 1, 0, 1),
		"level": 5,
		"experience": 1250.5
	})

	# Retrieve and display the component data
	var player_data = FlecsServer.get_component_by_name(entity, "Player")
	print("✓ Player component data:")
	for key in player_data.keys():
		print("  - %s: %s" % [key, player_data[key]])

	# Example 6: Create a Physics component with 2D types
	print("\nExample 6: Creating Physics2D component")
	var physics_fields = {
		"position": Vector2(0, 0),         # Vector2
		"velocity": Vector2(0, 0),         # Vector2
		"acceleration": Vector2(0, 0),     # Vector2
		"mass": 1.0,                       # float
		"friction": 0.1,                   # float
		"is_static": false                 # bool
	}
	var physics_comp_id = FlecsServer.create_runtime_component(world_id, "Physics2D", physics_fields)
	if physics_comp_id.is_valid():
		print("✓ Physics2D component created successfully: ", physics_comp_id)
	else:
		print("✗ Failed to create Physics2D component")

	# Example 7: Create a Transform component with transform types
	print("\nExample 7: Creating TransformData component")
	var transform_fields = {
		"transform_2d": Transform2D(),     # Transform2D
		"transform_3d": Transform3D(),     # Transform3D
		"rotation": Quaternion(),          # Quaternion
		"scale": Vector3.ONE               # Vector3
	}
	var transform_comp_id = FlecsServer.create_runtime_component(world_id, "TransformData", transform_fields)
	if transform_comp_id.is_valid():
		print("✓ TransformData component created successfully: ", transform_comp_id)
	else:
		print("✗ Failed to create TransformData component")

	# Example 8: Create entities with multiple runtime components
	print("\nExample 8: Creating entity with multiple runtime components")
	var multi_entity = FlecsServer.create_entity_with_name(world_id, "ComplexEntity")

	FlecsServer.add_component(multi_entity, health_comp_id)
	FlecsServer.add_component(multi_entity, player_comp_id)
	FlecsServer.add_component(multi_entity, inventory_comp_id)

	FlecsServer.set_component(multi_entity, "Health", {
		"current": 75,
		"max": 100,
		"regen_rate": 2.5
	})

	FlecsServer.set_component(multi_entity, "Inventory", {
		"items": ["sword", "shield", "potion"],
		"capacity": 20,
		"metadata": {"gold": 500, "keys": 3},
		"equipped_weapon": RID()
	})

	var health_data = FlecsServer.get_component_by_name(multi_entity, "Health")
	var inventory_data = FlecsServer.get_component_by_name(multi_entity, "Inventory")

	print("✓ ComplexEntity components:")
	print("  Health: ", health_data)
	print("  Inventory: ", inventory_data)

	# Get all component types on the entity
	var component_names = FlecsServer.get_component_types_as_name(multi_entity)
	print("✓ All components on ComplexEntity: ", component_names)

	print("\n=== Runtime Component Creation Example Complete ===")
