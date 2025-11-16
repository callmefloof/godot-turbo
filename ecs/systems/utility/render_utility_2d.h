#pragma once
#include "core/variant/typed_array.h"
#include "scene/2d/camera_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/2d/multimesh_instance_2d.h"
#include "scene/2d/skeleton_2d.h"
#include "scene/main/canvas_item.h"
#include "scene/resources/mesh.h"
#include <scene/2d/light_2d.h>
#include <scene/2d/light_occluder_2d.h>
#include <scene/2d/mesh_instance_2d.h>

/**
 * @file render_utility_2d.h
 * @brief Utility for creating 2D rendering entities in the Flecs ECS world.
 * 
 * This utility provides static methods to bridge Godot's 2D rendering system (RenderingServer
 * and CanvasItem hierarchy) with the Flecs ECS architecture. It creates ECS entities that 
 * represent rendering objects such as mesh instances, cameras, lights, particles, skeletons,
 * and other visual elements.
 * 
 * @section render2d_thread_safety Thread Safety
 * 
 * The RenderUtility2D methods have mixed thread-safety characteristics:
 * - RenderingServer calls are generally thread-safe for resource creation
 * - FlecsServer entity creation is thread-safe (uses mutexes)
 * - NodeStorage operations are protected by mutexes
 * 
 * **Important Constraints:**
 * - Methods that accept CanvasItem or other Node pointers must access node properties,
 *   which should ideally be done from the main thread
 * - Canvas item visibility and hierarchy operations must be done on the main thread
 * - Creating rendering resources (meshes, textures, shaders) is generally safe from any thread
 * - Modifying active scene rendering state should be synchronized with frame rendering
 * 
 * **Recommendation**: For maximum safety, create rendering entities during scene initialization
 * or from the main thread unless you're certain about the specific operation's thread-safety.
 * 
 * @section render2d_usage Usage Examples
 * 
 * @subsection render2d_example1 Creating a Mesh Instance
 * @code
 * // Create a mesh instance entity from an existing MeshInstance2D node
 * MeshInstance2D* mesh_node = get_node<MeshInstance2D>("Sprite");
 * RID world_id = get_world_id();
 * RID mesh_entity = RenderUtility2D::create_mesh_instance_with_object(world_id, mesh_node);
 * 
 * // The entity now tracks the mesh instance with all its rendering properties
 * @endcode
 * 
 * @subsection render2d_example2 Creating a Camera
 * @code
 * // Create a camera entity from a Camera2D node
 * Camera2D* camera = get_node<Camera2D>("MainCamera");
 * RID world_id = get_world_id();
 * RID camera_entity = RenderUtility2D::create_camera_with_object(world_id, camera);
 * @endcode
 * 
 * @subsection render2d_example3 Creating Lights
 * @code
 * // Create a point light entity
 * PointLight2D* point_light = get_node<PointLight2D>("TorchLight");
 * RID world_id = get_world_id();
 * RID light_entity = RenderUtility2D::create_point_light_with_object(world_id, point_light);
 * 
 * // Create a directional light
 * DirectionalLight2D* sun = get_node<DirectionalLight2D>("Sun");
 * RID sun_entity = RenderUtility2D::create_directional_light_with_object(world_id, sun);
 * @endcode
 * 
 * @subsection render2d_example4 Creating MultiMesh Instances
 * @code
 * // Create multiple instances efficiently with a MultiMesh
 * MultiMeshInstance2D* grass = get_node<MultiMeshInstance2D>("GrassField");
 * RID world_id = get_world_id();
 * 
 * // This creates the MultiMesh entity plus individual instance entities
 * TypedArray<RID> instance_entities = RenderUtility2D::create_multi_mesh_with_object(
 *     world_id, 
 *     grass
 * );
 * 
 * // instance_entities[0] is the parent MultiMesh entity
 * // instance_entities[1..N] are the individual instance entities
 * @endcode
 * 
 * @subsection render2d_example5 Creating GPU Particles
 * @code
 * // Create a particle system entity
 * GPUParticles2D* particles = get_node<GPUParticles2D>("Explosion");
 * RID world_id = get_world_id();
 * RID particle_entity = RenderUtility2D::create_gpu_particles_with_object(
 *     world_id, 
 *     particles,
 *     1000  // particle count
 * );
 * @endcode
 * 
 * @subsection render2d_example6 Creating Skeletons and Light Occluders
 * @code
 * // Create a skeleton entity for 2D bone animation
 * Skeleton2D* skeleton = get_node<Skeleton2D>("CharacterSkeleton");
 * RID skeleton_entity = RenderUtility2D::create_skeleton_with_object(world_id, skeleton);
 * 
 * // Create a light occluder for 2D lighting shadows
 * LightOccluder2D* occluder = get_node<LightOccluder2D>("WallShadow");
 * RID occluder_entity = RenderUtility2D::create_light_occluder_with_object(world_id, occluder);
 * @endcode
 * 
 * @note This class should not be instantiated. All methods are static.
 * @note The created entities will have components from the all_components.h header.
 * @warning Do not delete the source Godot objects (MeshInstance2D, Camera2D, etc.) while
 *          the ECS entities reference them via ObjectInstanceComponent.
 * @warning Canvas item transforms and visibility are managed by Godot's rendering system;
 *          modifying them directly via RenderingServer while nodes exist may cause inconsistencies.
 */
class RenderUtility2D : public Object{
	GDCLASS(RenderUtility2D, Object)
public:
	RenderUtility2D() = default;
	~RenderUtility2D() = default;

	// ========================================================================
	// Mesh Instance Creation
	// ========================================================================

	/**
	 * @brief Create a mesh instance entity with specific rendering parameters.
	 * 
	 * Creates a new canvas item via RenderingServer, configures it with the provided
	 * mesh and transform, and wraps it in a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param mesh_id The RID of the mesh to render
	 * @param transform The 2D transformation of the mesh instance
	 * @param name The name to assign to the entity
	 * @param canvas_id The canvas RID to which this instance belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note This method creates low-level rendering resources directly
	 * @note Configure additional properties via RenderingServer after creation
	 */
    static RID create_mesh_instance_with_id(const RID &world_id, const RID &mesh_id, const Transform2D &transform, const String &name, const RID &canvas_id);

	/**
	 * @brief Create a basic mesh instance entity with a transform.
	 * 
	 * Creates a minimal mesh instance entity with just a transform. The mesh itself
	 * must be assigned separately.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The 2D transformation of the mesh instance
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The mesh must be set via RenderingServer or component modification after creation
	 */
    static RID create_mesh_instance(const RID &world_id, const Transform2D &transform,  const String &name);

	/**
	 * @brief Create a mesh instance entity from an existing Godot MeshInstance2D node.
	 * 
	 * Converts an existing MeshInstance2D node into a Flecs entity, preserving all
	 * its rendering properties (mesh, texture, material, transform, etc.).
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param mesh_instance_2d Pointer to the MeshInstance2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note The entity will have MeshInstance2DComponent and ObjectInstanceComponent
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the MeshInstance2D node remains valid for the lifetime of the entity
	 */
    static RID create_mesh_instance_with_object(const RID &world_id, MeshInstance2D *mesh_instance_2d);

	// ========================================================================
	// MultiMesh Creation
	// ========================================================================

	/**
	 * @brief Create a MultiMesh entity for efficient batch rendering.
	 * 
	 * Creates a MultiMesh resource and entity for rendering many instances of the same
	 * mesh efficiently. Supports optional per-instance colors and custom data.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The base transform for the MultiMesh
	 * @param size The number of instances in the MultiMesh
	 * @param mesh The mesh resource to instance
	 * @param name The name to assign to the entity
	 * @param texture_id The texture RID to use (can be invalid if not needed)
	 * @param use_colors Whether instances have per-instance colors (default: false)
	 * @param use_custom_data Whether instances have per-instance custom data (default: false)
	 * @param use_indirect Whether to use indirect rendering (default: false)
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Individual instance transforms must be set separately via RenderingServer
	 * @note Useful for rendering grass, debris, or large numbers of identical objects
	 */
    static RID create_multi_mesh(const RID &world_id,
	    const Transform2D &transform,
	    const uint32_t size,
	    const Ref<Mesh> mesh,
	    const String &name,
	    const RID& texture_id,
	    const bool use_colors = false,
	    const bool use_custom_data = false,
	    const bool use_indirect = false);

	/**
	 * @brief Create a MultiMesh entity from an existing Godot MultiMeshInstance2D node.
	 * 
	 * Converts an existing MultiMeshInstance2D node into a Flecs entity hierarchy.
	 * Creates a parent entity for the MultiMesh plus individual entities for each instance.
	 * 
	 * @param world_id The RID of the Flecs world where the entities will be created
	 * @param multi_mesh_instance Pointer to the MultiMeshInstance2D node (must not be null)
	 * @return TypedArray<RID> Array of entity RIDs: [0] = parent MultiMesh entity, [1..N] = instance entities
	 * 
	 * @note The first element is the parent MultiMesh entity
	 * @note Subsequent elements are individual instance entities
	 * @note All instances share the same mesh but can have individual transforms/colors
	 * @warning Ensure the MultiMeshInstance2D node remains valid for the lifetime of the entities
	 */
    static TypedArray<RID> create_multi_mesh_with_object(const RID &world_id,
	    MultiMeshInstance2D *multi_mesh_instance);

	/**
	 * @brief Create a single MultiMesh instance entity.
	 * 
	 * Creates an entity representing one instance within a MultiMesh, with its own transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The transform for this specific instance
	 * @param index The index of this instance within the MultiMesh
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The instance must be part of an existing MultiMesh
	 * @note Use this when you need individual entity control over MultiMesh instances
	 */
    static RID create_multi_mesh_instance(
	    const RID &world_id,
	    const Transform2D &transform,
	    const uint32_t index,
	    const String &name);

	/**
	 * @brief Create multiple MultiMesh instance entities at once.
	 * 
	 * Creates a batch of instance entities for a MultiMesh, each with its own transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entities will be created
	 * @param transform Array of transforms, one per instance
	 * @param multi_mesh The RID of the parent MultiMesh entity
	 * @return TypedArray<RID> Array of entity RIDs for the created instances
	 * 
	 * @note The number of transforms should match the MultiMesh instance count
	 * @note Efficient for creating many instances in one call
	 */
    static TypedArray<RID> create_multi_mesh_instances(
	    const RID &world_id,
	    const TypedArray<Transform2D>& transform,
	    const RID &multi_mesh);

	// ========================================================================
	// Camera Creation
	// ========================================================================

	/**
	 * @brief Create a camera entity with specific parameters.
	 * 
	 * Creates a camera entity with a given camera RID and transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param camera_id The RID of the camera (from RenderingServer)
	 * @param transform The camera's 2D transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note You are responsible for creating and managing the camera RID
	 */
    static RID create_camera_with_id(const RID &world_id, const RID &camera_id, const Transform2D &transform, const String &name);

	/**
	 * @brief Create a camera entity from an existing Godot Camera2D node.
	 * 
	 * Converts an existing Camera2D node into a Flecs entity, preserving all camera
	 * properties (zoom, offset, limits, drag margins, etc.).
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param camera_2d Pointer to the Camera2D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note The entity will have Camera2DComponent and ObjectInstanceComponent
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the Camera2D node remains valid for the lifetime of the entity
	 */
    static RID create_camera_with_object(const RID &world_id, Camera2D *camera_2d);

	// ========================================================================
	// Light Creation
	// ========================================================================

	/**
	 * @brief Create a directional light entity with a light RID.
	 * 
	 * Creates a directional light entity with specified light RID and transform.
	 * Directional lights illuminate the entire canvas uniformly from a direction.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_id The RID of the light (from RenderingServer)
	 * @param transform The light's transform (rotation determines direction)
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
    static RID create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform2D &transform,  const String &name);

	/**
	 * @brief Create a basic directional light entity.
	 * 
	 * Creates a directional light entity with just a transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The light's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Light properties must be configured separately
	 */
    static RID create_directional_light(const RID &world_id, const Transform2D &transform, const String &name);

	/**
	 * @brief Create a directional light entity from an existing Godot DirectionalLight2D node.
	 * 
	 * Converts an existing DirectionalLight2D node into a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param directional_light Pointer to the DirectionalLight2D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note DirectionalLight2D provides global illumination for 2D scenes
	 * @warning Ensure the DirectionalLight2D node remains valid for the lifetime of the entity
	 */
    static RID create_directional_light_with_object(const RID &world_id, DirectionalLight2D *directional_light);

	/**
	 * @brief Create a basic point light entity.
	 * 
	 * Creates a point light entity with a transform. Point lights radiate from a position.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The light's position and rotation
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure light properties (color, energy, range) separately
	 */
    static RID create_point_light(const RID &world_id,  const Transform2D &transform, const String &name);

	/**
	 * @brief Create a point light entity with a light RID.
	 * 
	 * Creates a point light entity with specified light RID and transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_id The RID of the light (from RenderingServer)
	 * @param transform The light's position
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
    static RID create_point_light_with_id(const RID &world_id,  const RID &light_id, const Transform2D &transform, const String &name);

	/**
	 * @brief Create a point light entity from an existing Godot PointLight2D node.
	 * 
	 * Converts an existing PointLight2D node into a Flecs entity, preserving all
	 * light properties (color, energy, range, shadows, texture, etc.).
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param point_light Pointer to the PointLight2D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note PointLight2D is useful for torches, lamps, explosions, etc.
	 * @warning Ensure the PointLight2D node remains valid for the lifetime of the entity
	 */
    static RID create_point_light_with_object(const RID &world_id, PointLight2D *point_light);

	// ========================================================================
	// Canvas Item Creation
	// ========================================================================

	/**
	 * @brief Create a generic canvas item entity from any CanvasItem node.
	 * 
	 * Converts any CanvasItem-derived node (Node2D, Control, and their subclasses)
	 * into a Flecs entity. This is the most generic 2D rendering entity creator.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param canvas_item Pointer to the CanvasItem node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Works with any CanvasItem: Sprite2D, Polygon2D, Line2D, Control widgets, etc.
	 * @note The entity will have CanvasItemComponent and ObjectInstanceComponent
	 * @warning Ensure the CanvasItem node remains valid for the lifetime of the entity
	 */
    static RID create_canvas_item_with_object(const RID &world_id, CanvasItem *canvas_item);

	/**
	 * @brief Create a canvas item entity with specific parameters.
	 * 
	 * Creates a canvas item entity with detailed configuration.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param canvas_item_id The RID of the canvas item (from RenderingServer)
	 * @param transform The canvas item's transform
	 * @param name The name to assign to the entity
	 * @param class_name The class name for identification (e.g., "Sprite2D")
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Useful for low-level control over canvas item creation
	 */
    static RID create_canvas_item_with_id(const RID &world_id, const RID &canvas_item_id,const Transform2D& transform, const String &name, const String &class_name);

	// ========================================================================
	// Skeleton Creation
	// ========================================================================

	/**
	 * @brief Create a skeleton entity with a skeleton RID.
	 * 
	 * Creates a 2D skeleton entity for bone-based animation.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param skeleton_id The RID of the skeleton (from RenderingServer)
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
    static RID create_skeleton_with_id(const RID &world_id, const RID &skeleton_id, const String &name);

	/**
	 * @brief Create a skeleton entity from an existing Godot Skeleton2D node.
	 * 
	 * Converts an existing Skeleton2D node into a Flecs entity. Skeleton2D is used
	 * for 2D skeletal animation and deformation.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param skeleton_2d Pointer to the Skeleton2D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Skeleton2D controls Bone2D children for animation
	 * @note Commonly used with MeshInstance2D for character animation
	 * @warning Ensure the Skeleton2D node remains valid for the lifetime of the entity
	 */
    static RID create_skeleton_with_object(const RID &world_id, Skeleton2D *skeleton_2d);

	// ========================================================================
	// Light Occluder Creation
	// ========================================================================

	/**
	 * @brief Create a light occluder entity from an existing Godot LightOccluder2D node.
	 * 
	 * Converts an existing LightOccluder2D node into a Flecs entity. Light occluders
	 * cast shadows when light from Light2D nodes hits them.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_occluder Pointer to the LightOccluder2D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note LightOccluder2D uses an OccluderPolygon2D resource to define shadow shape
	 * @note Useful for walls, obstacles, and any object that should cast 2D shadows
	 * @warning Ensure the LightOccluder2D node remains valid for the lifetime of the entity
	 */
    static RID create_light_occluder_with_object(const RID &world_id, LightOccluder2D *light_occluder);

	/**
	 * @brief Create a light occluder entity with specific parameters.
	 * 
	 * Creates a light occluder entity with detailed configuration.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_occluder_id The RID of the light occluder (from RenderingServer)
	 * @param transform The occluder's transform
	 * @param canvas_id The canvas RID to which this occluder belongs
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
    static RID create_light_occluder_with_id(const RID &world_id, const RID& light_occluder_id, const Transform2D& transform, const RID& canvas_id, const String& name );

	/**
	 * @brief Create a basic light occluder entity.
	 * 
	 * Creates a light occluder entity with a transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The occluder's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The occluder polygon must be configured separately
	 */
    static RID create_light_occluder(const RID &world_id, const Transform2D &transform, const String &name);

	// ========================================================================
	// GPU Particles Creation
	// ========================================================================

	/**
	 * @brief Create a GPU particles entity with specific parameters.
	 * 
	 * Creates a GPU-based particle system entity with detailed configuration.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param canvas_item_id The RID of the canvas item
	 * @param particles_id The RID of the particles resource
	 * @param texture_id The texture RID for particles
	 * @param transform The particle system's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note GPU particles are efficient for large numbers of particles
	 */
    static RID create_gpu_particles_with_id(const RID &world_id, const RID canvas_item_id, const RID particles_id, const RID texture_id, const Transform2D &transform,  const String& name);

	/**
	 * @brief Create a GPU particles entity from an existing Godot GPUParticles2D node.
	 * 
	 * Converts an existing GPUParticles2D node into a Flecs entity. GPU particles
	 * are ideal for effects like fire, smoke, sparks, magic, rain, etc.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param gpu_particles Pointer to the GPUParticles2D node (must not be null)
	 * @param count The number of particles (default: 0 uses the node's amount)
	 * @param max_depth Maximum hierarchy depth to traverse (default: 10000)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note GPUParticles2D uses the GPU for simulation, allowing thousands of particles
	 * @note Requires a ParticleProcessMaterial or custom shader for particle behavior
	 * @warning Ensure the GPUParticles2D node remains valid for the lifetime of the entity
	 */
    static RID create_gpu_particles_with_object(const RID &world_id, GPUParticles2D* gpu_particles, uint32_t count = 0, const uint32_t max_depth = 10000);
};