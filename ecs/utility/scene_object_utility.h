#pragma once

#include "core/object/object.h"
#include "scene/main/scene_tree.h"
#include "core/variant/typed_array.h"
#include <cassert>

/**
 * @class SceneObjectUtility
 * @brief Converts Godot scene graph nodes into Flecs ECS entities.
 * 
 * SceneObjectUtility acts as a bridge between Godot's traditional scene tree architecture 
 * and the Flecs Entity Component System. It traverses the scene graph and creates 
 * corresponding ECS entities with appropriate components based on node types.
 * 
 * The utility handles 30+ different node types including:
 * - 3D Rendering: MeshInstance3D, MultiMeshInstance3D, Camera3D, Light3D variants, Skeleton3D, etc.
 * - 2D Rendering: MeshInstance2D, MultiMeshInstance2D, Camera2D, Light2D variants, CanvasItem, etc.
 * - 3D Physics: RigidBody3D, Area3D, PhysicsBody3D, Joint3D, SoftBody3D
 * - 2D Physics: RigidBody2D, Area2D, PhysicsBody2D, Joint2D
 * - 3D Navigation: NavigationAgent3D, NavigationLink3D, NavigationObstacle3D, NavigationRegion3D
 * - 2D Navigation: NavigationAgent2D, NavigationLink2D, NavigationObstacle2D, NavigationRegion2D
 * - Environment: WorldEnvironment, Viewport, ReflectionProbe, VoxelGI
 * 
 * For each node type, the utility delegates to specialized creation utilities 
 * (e.g., RenderUtility3D, Physics3DUtility, Navigation3DUtility) which set up 
 * the appropriate components for that entity.
 * 
 * @note This class uses a singleton pattern and is not thread-safe.
 * @note Entity creation should be done on the main thread due to Godot API constraints.
 * 
 * @example
 * ```
 * RID world_id = FlecsServer.create_world()
 * var scene_util = SceneObjectUtility.get_singleton()
 * var entities = scene_util.create_entities_from_scene(world_id, get_tree())
 * print("Created %d entities from scene" % entities.size())
 * ```
 */
class SceneObjectUtility : public Object {
	GDCLASS(SceneObjectUtility, Object)
private:
	static inline SceneObjectUtility* instance = nullptr;
public:
	SceneObjectUtility() = default;
	~SceneObjectUtility() = default;
	
	/**
	 * @brief Creates ECS entities from all root nodes in a SceneTree.
	 * 
	 * This is the primary entry point for converting an entire scene into ECS entities.
	 * It starts from the root of the scene tree and recursively processes all child nodes.
	 * 
	 * @param world_id The RID of the Flecs world to create entities in
	 * @param tree The SceneTree to traverse
	 * @return TypedArray<RID> Array of created entity RIDs (empty on error)
	 * 
	 * @note This will process ALL nodes in the scene tree, potentially creating many entities.
	 * @warning Returns empty array if tree is null.
	 */
	TypedArray<RID> create_entities_from_scene(const RID &world_id, SceneTree* tree );
	
	/**
	 * @brief Recursively creates ECS entities from a node and all its descendants.
	 * 
	 * Traverses the scene graph depth-first, creating entities for each node encountered.
	 * The recursion can be limited by the max_depth parameter to prevent stack overflow
	 * on deeply nested scene hierarchies.
	 * 
	 * @param world_id The RID of the Flecs world to create entities in
	 * @param base_node The root node to start traversal from
	 * @param entities Accumulated array of entity RIDs (pass empty array initially)
	 * @param current_depth Current recursion depth (default: 0)
	 * @param max_depth Maximum allowed recursion depth (default: 10000)
	 * @return TypedArray<RID> Array containing all created entity RIDs
	 * 
	 * @warning Returns the input entities array unchanged if base_node is null or max_depth exceeded.
	 * @note Each node may create one or more entities (e.g., MultiMeshInstance creates multiple).
	 */
	TypedArray<RID> create_entities(const RID &world_id, const Node *base_node, const TypedArray<RID> &entities,
			int current_depth = 0, const int max_depth = 10000);
	
	/**
	 * @brief Creates an ECS entity from a single Godot node.
	 * 
	 * This is the core conversion method that performs type checking to determine
	 * the node's actual type and delegates to the appropriate specialized utility.
	 * The method uses a type-dispatch pattern, checking node types from most specific
	 * to most general.
	 * 
	 * Type checking order:
	 * 1. 3D Navigation nodes
	 * 2. 2D Navigation nodes
	 * 3. 3D Physics nodes
	 * 4. 2D Physics nodes
	 * 5. 3D Rendering nodes
	 * 6. 2D Rendering nodes (CanvasItem checked last as it's the most generic)
	 * 7. Fallback: Generic entity with SceneNodeComponent
	 * 
	 * @param world_id The RID of the Flecs world to create the entity in
	 * @param node The Godot node to convert
	 * @return TypedArray<RID> Array of created entity RIDs. Usually contains 1-2 entities:
	 *                          - The main entity for the node
	 *                          - Optional script entity (if node has a script attached)
	 *                          - MultiMesh nodes may create many entities (one per instance)
	 * 
	 * @warning Returns empty array if node is null.
	 * @note Automatically attaches any scripts found on the node as child entities.
	 */
	TypedArray<RID> create_entity(const RID &world_id, Node* node);
	
	/**
	 * @brief Creates a resource entity for a node's attached script.
	 * 
	 * If the node has a script attached, this method creates a separate resource entity
	 * for that script and establishes a parent-child relationship in the ECS hierarchy.
	 * The script entity becomes a child of the main node entity.
	 * 
	 * @param world_id The RID of the Flecs world
	 * @param node The node to check for an attached script
	 * @param node_entity The RID of the main entity for this node (becomes parent)
	 * @return RID The created script resource entity, or invalid RID if no script found
	 * 
	 * @note Uses ResourceObjectUtility to create the script resource entity.
	 * @note Establishes a Flecs ChildOf relationship between script and node entity.
	 */
	RID get_node_script(const RID &world_id, const Node *node, const RID &node_entity);
	
	/** @brief Binds methods for GDScript/engine reflection. */
	static void _bind_methods();
	
	/**
	 * @brief Returns the singleton instance of SceneObjectUtility.
	 * 
	 * Creates the instance on first call using memnew().
	 * 
	 * @return SceneObjectUtility* Pointer to the singleton instance
	 * @note Not thread-safe - should only be called from main thread.
	 */
	static SceneObjectUtility* get_singleton();
};

