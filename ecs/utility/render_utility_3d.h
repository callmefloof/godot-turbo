#pragma once
#include "core/math/transform_3d.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/rid.h"
#include "core/variant/variant.h"

class VoxelGI;
class OccluderInstance3D;
class Viewport;
class SpotLight3D;
class OmniLight3D;
class DirectionalLight3D;
class Compositor;
class Camera3D;
class WorldEnvironment;
class Skeleton3D;
class ReflectionProbe;
class GPUParticles3D;
class MultiMeshInstance3D;
class MeshInstance3D;
class String;
struct Transform3D;
class RID;
struct Plane;
class Material;

/**
 * @file render_utility_3d.h
 * @brief Utility for creating 3D rendering entities in the Flecs ECS world.
 * 
 * This utility provides static methods to bridge Godot's 3D rendering system (RenderingServer
 * and Node3D hierarchy) with the Flecs ECS architecture. It creates ECS entities that 
 * represent rendering objects such as mesh instances, cameras, lights, particles, skeletons,
 * environments, and other visual elements.
 * 
 * @section render3d_thread_safety Thread Safety
 * 
 * The RenderUtility3D methods have mixed thread-safety characteristics:
 * - RenderingServer calls are generally thread-safe for resource creation
 * - FlecsServer entity creation is thread-safe (uses mutexes)
 * - NodeStorage operations are protected by mutexes
 * 
 * **Important Constraints:**
 * - Methods that accept Node3D or other Node pointers must access node properties,
 *   which should ideally be done from the main thread
 * - Scenario (3D world) visibility and hierarchy operations must be done on the main thread
 * - Creating rendering resources (meshes, textures, shaders, materials) is generally safe from any thread
 * - Modifying active scene rendering state should be synchronized with frame rendering
 * 
 * **Recommendation**: For maximum safety, create rendering entities during scene initialization
 * or from the main thread unless you're certain about the specific operation's thread-safety.
 * 
 * @section render3d_usage Usage Examples
 * 
 * @subsection render3d_example1 Creating a Mesh Instance
 * @code
 * // Create a mesh instance entity from an existing MeshInstance3D node
 * MeshInstance3D* mesh_node = get_node<MeshInstance3D>("Character");
 * RID world_id = get_world_id();
 * RID mesh_entity = RenderUtility3D::create_mesh_instance_with_object(world_id, mesh_node);
 * 
 * // The entity now tracks the mesh instance with all its rendering properties
 * @endcode
 * 
 * @subsection render3d_example2 Creating a Camera
 * @code
 * // Create a camera entity from a Camera3D node
 * Camera3D* camera = get_node<Camera3D>("MainCamera");
 * RID world_id = get_world_id();
 * RID camera_entity = RenderUtility3D::create_camera_with_object(world_id, camera);
 * @endcode
 * 
 * @subsection render3d_example3 Creating Lights
 * @code
 * // Create different types of lights
 * DirectionalLight3D* sun = get_node<DirectionalLight3D>("Sun");
 * RID world_id = get_world_id();
 * RID sun_entity = RenderUtility3D::create_directional_light_with_object(world_id, sun);
 * 
 * OmniLight3D* bulb = get_node<OmniLight3D>("RoomLight");
 * RID bulb_entity = RenderUtility3D::create_omni_light_with_object(world_id, bulb);
 * 
 * SpotLight3D* flashlight = get_node<SpotLight3D>("Flashlight");
 * RID flashlight_entity = RenderUtility3D::create_spot_light_with_object(world_id, flashlight);
 * @endcode
 * 
 * @subsection render3d_example4 Creating MultiMesh Instances
 * @code
 * // Create multiple instances efficiently with a MultiMesh
 * MultiMeshInstance3D* forest = get_node<MultiMeshInstance3D>("TreeField");
 * RID world_id = get_world_id();
 * 
 * // This creates the MultiMesh entity plus individual instance entities
 * TypedArray<RID> instance_entities = RenderUtility3D::create_multi_mesh_with_object(
 *     world_id, 
 *     forest
 * );
 * 
 * // instance_entities[0] is the parent MultiMesh entity
 * // instance_entities[1..N] are the individual instance entities
 * @endcode
 * 
 * @subsection render3d_example5 Creating Environment and Global Illumination
 * @code
 * // Create environment for sky, fog, ambient lighting, etc.
 * WorldEnvironment* env = get_node<WorldEnvironment>("Environment");
 * RID env_entity = RenderUtility3D::create_environment_with_object(world_id, env);
 * 
 * // Create voxel-based global illumination
 * VoxelGI* gi = get_node<VoxelGI>("GlobalIllumination");
 * RID gi_entity = RenderUtility3D::create_voxel_gi_with_object(world_id, gi);
 * 
 * // Create reflection probe for reflections
 * ReflectionProbe* probe = get_node<ReflectionProbe>("ReflectionProbe");
 * RID probe_entity = RenderUtility3D::create_reflection_probe_with_object(world_id, probe);
 * @endcode
 * 
 * @subsection render3d_example6 Creating Particles and Skeletons
 * @code
 * // Create a GPU particle system
 * GPUParticles3D* explosion = get_node<GPUParticles3D>("Explosion");
 * RID particle_entity = RenderUtility3D::create_particles_with_object(world_id, explosion);
 * 
 * // Create a skeleton for 3D character animation
 * Skeleton3D* skeleton = get_node<Skeleton3D>("CharacterSkeleton");
 * RID skeleton_entity = RenderUtility3D::create_skeleton_with_object(world_id, skeleton);
 * @endcode
 * 
 * @note This class should not be instantiated. All methods are static.
 * @note The created entities will have components from the all_components.h header.
 * @warning Do not delete the source Godot objects (MeshInstance3D, Camera3D, etc.) while
 *          the ECS entities reference them via ObjectInstanceComponent.
 * @warning Node3D transforms and visibility are managed by Godot's rendering system;
 *          modifying them directly via RenderingServer while nodes exist may cause inconsistencies.
 */
class RenderUtility3D : Object {
	GDCLASS(RenderUtility3D, Object);

public:
	// This class is a utility for creating rendering entities in the ECS world.
	// It should not be instantiated directly, but rather used through its static methods.
	// It provides a way to create entities that represent rendering components in the ECS world,
	// ensuring that the necessary properties are set correctly.

	RenderUtility3D() = default;
	~RenderUtility3D();

	// ========================================================================
	// Mesh Instance Creation
	// ========================================================================

	/**
	 * @brief Create a mesh instance entity with specific rendering parameters.
	 * 
	 * Creates a new 3D instance via RenderingServer, configures it with the provided
	 * mesh and transform, and wraps it in a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param mesh_id The RID of the mesh to render
	 * @param transform The 3D transformation of the mesh instance
	 * @param name The name to assign to the entity
	 * @param scenario_id The scenario (3D world) RID to which this instance belongs
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note This method creates low-level rendering resources directly
	 * @note Configure additional properties (materials, shadows, etc.) via RenderingServer after creation
	 */
	static RID create_mesh_instance_with_id(const RID &world_id, const RID &mesh_id, const Transform3D &transform, const String &name, const RID &scenario_id);

	/**
	 * @brief Create a basic mesh instance entity with a transform.
	 * 
	 * Creates a minimal mesh instance entity with just a transform and scenario.
	 * The mesh itself must be assigned separately.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The 3D transformation of the mesh instance
	 * @param scenario_id The scenario RID
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The mesh must be set via RenderingServer or component modification after creation
	 */
	static RID create_mesh_instance(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name);

	/**
	 * @brief Create a mesh instance entity from an existing Godot MeshInstance3D node.
	 * 
	 * Converts an existing MeshInstance3D node into a Flecs entity, preserving all
	 * its rendering properties (mesh, materials, transform, skeleton, etc.).
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param mesh_instance_3d Pointer to the MeshInstance3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note The entity will have MeshInstance3DComponent and ObjectInstanceComponent
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the MeshInstance3D node remains valid for the lifetime of the entity
	 */
	static RID create_mesh_instance_with_object(const RID &world_id, MeshInstance3D *mesh_instance_3d);

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
	 * @param mesh_id The mesh RID to instance
	 * @param material_ids Array of material RIDs to apply
	 * @param scenario_id The scenario RID
	 * @param name The name to assign to the entity
	 * @param use_colors Whether instances have per-instance colors (default: false)
	 * @param use_custom_data Whether instances have per-instance custom data (default: false)
	 * @param use_indirect Whether to use indirect rendering (default: false)
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Individual instance transforms must be set separately via RenderingServer
	 * @note Useful for rendering forests, grass, debris, or large numbers of identical objects
	 */
	static RID create_multi_mesh(const RID &world_id,
			const Transform3D &transform,
			uint32_t size,
			const RID &mesh_id,
			const TypedArray<RID> &material_ids,
			const RID &scenario_id,
			const String &name,
			bool use_colors = false,
			bool use_custom_data = false, bool use_indirect = false);

	/**
	 * @brief Create a MultiMesh entity from an existing Godot MultiMeshInstance3D node.
	 * 
	 * Converts an existing MultiMeshInstance3D node into a Flecs entity hierarchy.
	 * Creates a parent entity for the MultiMesh plus individual entities for each instance.
	 * 
	 * @param world_id The RID of the Flecs world where the entities will be created
	 * @param multi_mesh_instance Pointer to the MultiMeshInstance3D node (must not be null)
	 * @return TypedArray<RID> Array of entity RIDs: [0] = parent MultiMesh entity, [1..N] = instance entities
	 * 
	 * @note The first element is the parent MultiMesh entity
	 * @note Subsequent elements are individual instance entities
	 * @note All instances share the same mesh but can have individual transforms/colors
	 * @warning Ensure the MultiMeshInstance3D node remains valid for the lifetime of the entities
	 */
	static TypedArray<RID> create_multi_mesh_with_object(const RID &world_id, MultiMeshInstance3D *multi_mesh_instance);

	/**
	 * @brief Create a single MultiMesh instance entity.
	 * 
	 * Creates an entity representing one instance within a MultiMesh, with its own transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The transform for this specific instance
	 * @param index The index of this instance within the MultiMesh
	 * @param multi_mesh_id The RID of the parent MultiMesh
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note The instance must be part of an existing MultiMesh
	 * @note Use this when you need individual entity control over MultiMesh instances
	 */
	static RID create_multi_mesh_instance(const RID &world_id, const Transform3D &transform, const uint32_t index, const RID &multi_mesh_id, const String &name);

	/**
	 * @brief Create multiple MultiMesh instance entities at once.
	 * 
	 * Creates a batch of instance entities for a MultiMesh, each with its own transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entities will be created
	 * @param transform Array of transforms, one per instance
	 * @param multi_mesh_id The RID of the parent MultiMesh
	 * @return TypedArray<RID> Array of entity RIDs for the created instances
	 * 
	 * @note The number of transforms should match the MultiMesh instance count
	 * @note Efficient for creating many instances in one call
	 */
	static TypedArray<RID> create_multi_mesh_instances(const RID &world_id, const TypedArray<Transform3D> &transform, const RID &multi_mesh_id);

	// ========================================================================
	// Particles Creation
	// ========================================================================

	/**
	 * @brief Create a GPU particles entity with specific parameters.
	 * 
	 * Creates a GPU-based particle system entity with detailed configuration.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The particle system's transform
	 * @param particles_id The RID of the particles resource
	 * @param particle_count The number of particles
	 * @param scenario_id The scenario RID
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note GPU particles are efficient for large numbers of particles
	 */
	static RID create_particles(const RID &world_id, const Transform3D &transform, const RID &particles_id, const int particle_count, const RID &scenario_id, const String &name);

	/**
	 * @brief Create a GPU particles entity from an existing Godot GPUParticles3D node.
	 * 
	 * Converts an existing GPUParticles3D node into a Flecs entity. GPU particles
	 * are ideal for effects like fire, smoke, sparks, magic, rain, explosions, etc.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param gpu_particles_3d Pointer to the GPUParticles3D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note GPUParticles3D uses the GPU for simulation, allowing thousands of particles
	 * @note Requires a ParticleProcessMaterial or custom shader for particle behavior
	 * @warning Ensure the GPUParticles3D node remains valid for the lifetime of the entity
	 */
	static RID create_particles_with_object(const RID &world_id, GPUParticles3D *gpu_particles_3d);

	// ========================================================================
	// Reflection Probe Creation
	// ========================================================================

	/**
	 * @brief Create a reflection probe entity with specific parameters.
	 * 
	 * Creates a reflection probe entity for capturing and reflecting the environment.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param probe_id The RID of the reflection probe
	 * @param transform The probe's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_reflection_probe(const RID &world_id, const RID &probe_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a reflection probe entity from an existing Godot ReflectionProbe node.
	 * 
	 * Converts an existing ReflectionProbe node into a Flecs entity. Reflection probes
	 * capture the surrounding environment to provide realistic reflections on objects.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param reflection_probe Pointer to the ReflectionProbe node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note ReflectionProbe captures a cubemap of the environment for PBR reflections
	 * @note Can be updated in real-time or baked for performance
	 * @warning Ensure the ReflectionProbe node remains valid for the lifetime of the entity
	 */
	static RID create_reflection_probe_with_object(const RID &world_id, ReflectionProbe *reflection_probe);

	// ========================================================================
	// Skeleton Creation
	// ========================================================================

	/**
	 * @brief Create a skeleton entity with a skeleton RID.
	 * 
	 * Creates a 3D skeleton entity for bone-based animation and skinning.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param skeleton_id The RID of the skeleton (from RenderingServer)
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_skeleton(const RID &world_id, const RID &skeleton_id, const String &name);

	/**
	 * @brief Create a skeleton entity from an existing Godot Skeleton3D node.
	 * 
	 * Converts an existing Skeleton3D node into a Flecs entity. Skeleton3D is used
	 * for 3D skeletal animation, skinning, and inverse kinematics.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param skeleton_3d Pointer to the Skeleton3D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Skeleton3D controls bone transforms for character animation
	 * @note Works with MeshInstance3D for skinned mesh rendering
	 * @warning Ensure the Skeleton3D node remains valid for the lifetime of the entity
	 */
	static RID create_skeleton_with_object(const RID &world_id, Skeleton3D *skeleton_3d);

	// ========================================================================
	// Environment Creation
	// ========================================================================

	/**
	 * @brief Create an environment entity with an environment RID.
	 * 
	 * Creates an environment entity for controlling sky, ambient light, fog, and other
	 * global rendering settings.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param environment_id The RID of the environment
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_environment(const RID &world_id, const RID &environment_id, const String &name);

	/**
	 * @brief Create an environment entity from an existing Godot WorldEnvironment node.
	 * 
	 * Converts an existing WorldEnvironment node into a Flecs entity. WorldEnvironment
	 * controls the global visual environment of the 3D world.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param world_environment Pointer to the WorldEnvironment node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note WorldEnvironment uses an Environment resource to define sky, fog, tonemap, etc.
	 * @note Only one WorldEnvironment should be active per scene/viewport
	 * @warning Ensure the WorldEnvironment node remains valid for the lifetime of the entity
	 */
	static RID create_environment_with_object(const RID &world_id, WorldEnvironment *world_environment);

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
	 * @param transform The camera's 3D transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note You are responsible for creating and managing the camera RID
	 */
	static RID create_camera_with_id(const RID &world_id, const RID &camera_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a basic camera entity with a transform.
	 * 
	 * Creates a minimal camera entity with just a transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The camera's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Camera parameters (FOV, near/far planes, projection) must be configured separately
	 */
	static RID create_camera(const RID &world_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a camera entity from an existing Godot Camera3D node.
	 * 
	 * Converts an existing Camera3D node into a Flecs entity, preserving all camera
	 * properties (FOV, near/far planes, projection type, etc.).
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param camera_3d Pointer to the Camera3D node to convert (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note The entity will have Camera3DComponent and ObjectInstanceComponent
	 * @note The node is added to NodeStorage for lifecycle tracking
	 * @warning Ensure the Camera3D node remains valid for the lifetime of the entity
	 */
	static RID create_camera_with_object(const RID &world_id, Camera3D *camera_3d);

	// ========================================================================
	// Compositor Creation
	// ========================================================================

	/**
	 * @brief Create a compositor entity with a compositor RID.
	 * 
	 * Creates a compositor entity for advanced post-processing and rendering pipelines.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param compositor_id The RID of the compositor
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_compositor(const RID &world_id, const RID &compositor_id, const String &name);

	/**
	 * @brief Create a compositor entity from a Compositor resource.
	 * 
	 * Creates a compositor entity from a Compositor resource reference.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param compositor Reference to the Compositor resource
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Compositor allows custom rendering pipelines and effects
	 */
	static RID create_compositor_with_object(const RID &world_id, const Ref<Compositor> &compositor);

	// ========================================================================
	// Light Creation
	// ========================================================================

	/**
	 * @brief Create a directional light entity with a light RID.
	 * 
	 * Creates a directional light entity with specified light RID and transform.
	 * Directional lights simulate sunlight with parallel rays.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_id The RID of the light (from RenderingServer)
	 * @param transform The light's transform (rotation determines direction)
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_directional_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const String &name);

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
	 * @note Light properties (color, energy, shadows) must be configured separately
	 */
	static RID create_directional_light(const RID &world_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a directional light entity from an existing Godot DirectionalLight3D node.
	 * 
	 * Converts an existing DirectionalLight3D node into a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param directional_light Pointer to the DirectionalLight3D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note DirectionalLight3D is typically used for sun/moon lighting
	 * @note Supports cascaded shadow maps for large-scale shadows
	 * @warning Ensure the DirectionalLight3D node remains valid for the lifetime of the entity
	 */
	static RID create_directional_light_with_object(const RID &world_id, DirectionalLight3D *directional_light);

	/**
	 * @brief Create an omni light entity with a light RID.
	 * 
	 * Creates an omni (point) light entity with specified light RID and transform.
	 * Omni lights radiate equally in all directions from a point.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_id The RID of the light (from RenderingServer)
	 * @param transform The light's position
	 * @param scenario_id The scenario RID
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_omni_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const RID &scenario_id, const String &name);

	/**
	 * @brief Create a basic omni light entity.
	 * 
	 * Creates an omni light entity with a transform and scenario.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The light's position
	 * @param scenario_id The scenario RID
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure light properties (color, energy, range, attenuation) separately
	 */
	static RID create_omni_light(const RID &world_id, const Transform3D &transform, const RID &scenario_id, const String &name);

	/**
	 * @brief Create an omni light entity from an existing Godot OmniLight3D node.
	 * 
	 * Converts an existing OmniLight3D node into a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param omni_light Pointer to the OmniLight3D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note OmniLight3D is useful for lamps, torches, fire, magical effects, etc.
	 * @note Supports omni-directional shadows (cubemap shadows)
	 * @warning Ensure the OmniLight3D node remains valid for the lifetime of the entity
	 */
	static RID create_omni_light_with_object(const RID &world_id, OmniLight3D *omni_light);

	/**
	 * @brief Create a spot light entity with a light RID.
	 * 
	 * Creates a spot light entity with specified light RID and transform.
	 * Spot lights emit a cone of light, like a flashlight or stage light.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param light_id The RID of the light (from RenderingServer)
	 * @param transform The light's position and direction
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_spot_light_with_id(const RID &world_id, const RID &light_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a basic spot light entity.
	 * 
	 * Creates a spot light entity with just a transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The light's position and direction
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Configure light properties (color, energy, range, angle) separately
	 */
	static RID create_spot_light(const RID &world_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a spot light entity from an existing Godot SpotLight3D node.
	 * 
	 * Converts an existing SpotLight3D node into a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param spot_light Pointer to the SpotLight3D node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note SpotLight3D is useful for flashlights, car headlights, stage lights, etc.
	 * @note Supports cone-shaped shadows
	 * @warning Ensure the SpotLight3D node remains valid for the lifetime of the entity
	 */
	static RID create_spot_light_with_object(const RID &world_id, SpotLight3D *spot_light);

	// ========================================================================
	// Viewport Creation
	// ========================================================================

	/**
	 * @brief Create a viewport entity with a viewport RID.
	 * 
	 * Creates a viewport entity for rendering to textures or sub-windows.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param viewport_id The RID of the viewport
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_viewport_with_id(const RID &world_id, const RID &viewport_id, const String &name);

	/**
	 * @brief Create a viewport entity from an existing Godot Viewport node.
	 * 
	 * Converts an existing Viewport node into a Flecs entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param viewport Pointer to the Viewport node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note Viewport renders a scene to a texture for render targets, minimaps, mirrors, etc.
	 * @warning Ensure the Viewport node remains valid for the lifetime of the entity
	 */
	static RID create_viewport_with_object(const RID &world_id, Viewport *viewport);

	// ========================================================================
	// VoxelGI Creation
	// ========================================================================

	/**
	 * @brief Create a VoxelGI entity with specific parameters.
	 * 
	 * Creates a VoxelGI (voxel-based global illumination) entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param voxel_gi_id The RID of the VoxelGI
	 * @param transform The VoxelGI's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_voxel_gi_with_id(const RID &world_id, const RID &voxel_gi_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a basic VoxelGI entity.
	 * 
	 * Creates a VoxelGI entity with just a transform.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param transform The VoxelGI's transform
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note VoxelGI parameters must be configured separately
	 */
	static RID create_voxel_gi(const RID &world_id, const Transform3D &transform, const String &name);

	/**
	 * @brief Create a VoxelGI entity from an existing Godot VoxelGI node.
	 * 
	 * Converts an existing VoxelGI node into a Flecs entity. VoxelGI provides
	 * real-time global illumination using voxel cone tracing.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param voxel_gi Pointer to the VoxelGI node (must not be null)
	 * @return RID The RID of the created Flecs entity, or invalid RID on failure
	 * 
	 * @note VoxelGI captures light bounces for realistic indirect lighting
	 * @note More expensive than lightmaps but allows dynamic lighting
	 * @warning Ensure the VoxelGI node remains valid for the lifetime of the entity
	 */
	static RID create_voxel_gi_with_object(const RID &world_id, VoxelGI *voxel_gi);

	// ========================================================================
	// Scenario Creation
	// ========================================================================

	/**
	 * @brief Create a scenario entity with a scenario RID.
	 * 
	 * Creates a scenario (3D world) entity. Scenarios contain all 3D visual elements.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param scenario_id The RID of the scenario
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 */
	static RID create_scenario_with_id(const RID &world_id, const RID &scenario_id, const String &name);

	/**
	 * @brief Create a basic scenario entity.
	 * 
	 * Creates a new scenario (3D world) entity.
	 * 
	 * @param world_id The RID of the Flecs world where the entity will be created
	 * @param name The name to assign to the entity
	 * @return RID The RID of the created Flecs entity
	 * 
	 * @note Scenarios are the root container for 3D visual elements in Godot
	 * @note Each scenario has its own spatial partitioning and culling
	 */
	static RID create_scenario(const RID &world_id, const String &name);

	// ========================================================================
	// GDScript Bindings
	// ========================================================================

	/**
	 * @brief Binds methods to GDScript/C# for use in scripts.
	 * @internal This method is called automatically during class registration.
	 */
	static void _bind_methods();
};