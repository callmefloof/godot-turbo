/**************************************************************************/
/*  turbo_entity_picker.h                                                 */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#ifndef TURBO_ENTITY_PICKER_H
#define TURBO_ENTITY_PICKER_H

#include "core/math/aabb.h"
#include "core/math/transform_3d.h"
#include "core/math/vector2.h"
#include "core/math/vector3.h"
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

class Camera3D;
class FlecsServer;
class Viewport;

// ============================================================================
// Pick Result - Information about a picked entity
// ============================================================================

struct TurboPickResult {
	RID entity_rid;
	RID world_rid;
	Vector3 hit_position;
	Vector3 hit_normal;
	float distance;
	int priority; // Higher priority entities are selected first (e.g., interactive objects)

	TurboPickResult() :
			distance(0.0f), priority(0) {}

	TurboPickResult(const RID &p_entity, const RID &p_world, const Vector3 &p_position, float p_distance) :
			entity_rid(p_entity), world_rid(p_world), hit_position(p_position), distance(p_distance), priority(0) {}

	// Comparison for sorting (closer = better, higher priority = better)
	bool operator<(const TurboPickResult &p_other) const {
		if (priority != p_other.priority) {
			return priority > p_other.priority; // Higher priority first
		}
		return distance < p_other.distance; // Closer first
	}

	Dictionary to_dictionary() const {
		Dictionary d;
		d["entity_rid"] = entity_rid;
		d["world_rid"] = world_rid;
		d["hit_position"] = hit_position;
		d["hit_normal"] = hit_normal;
		d["distance"] = distance;
		d["priority"] = priority;
		return d;
	}

	static TurboPickResult from_dictionary(const Dictionary &p_dict) {
		TurboPickResult result;
		result.entity_rid = p_dict.get("entity_rid", RID());
		result.world_rid = p_dict.get("world_rid", RID());
		result.hit_position = p_dict.get("hit_position", Vector3());
		result.hit_normal = p_dict.get("hit_normal", Vector3());
		result.distance = p_dict.get("distance", 0.0f);
		result.priority = p_dict.get("priority", 0);
		return result;
	}
};

// ============================================================================
// Pick Configuration
// ============================================================================

struct TurboPickConfig {
	// Ray settings
	float max_distance = 1000.0f;

	// Filtering
	uint32_t collision_mask = 0xFFFFFFFF;
	PackedStringArray required_components; // Only pick entities with these components
	PackedStringArray excluded_components; // Exclude entities with these components
	bool include_disabled = false;         // Include entities marked as disabled

	// Selection behavior
	bool select_closest = true;           // If false, returns all hits
	int max_results = 32;                 // Max entities to return when select_closest is false
	bool use_bounds_only = false;         // If true, only check AABB bounds (faster)
	bool prioritize_interactive = true;   // Interactive entities get higher priority

	// Layer filtering (for perception/simulation tiers)
	bool filter_by_perception_tier = false;
	int min_perception_tier = 0; // 0 = NONE, 1 = PARTIAL, 2 = FULL

	Dictionary to_dictionary() const {
		Dictionary d;
		d["max_distance"] = max_distance;
		d["collision_mask"] = collision_mask;
		d["required_components"] = required_components;
		d["excluded_components"] = excluded_components;
		d["include_disabled"] = include_disabled;
		d["select_closest"] = select_closest;
		d["max_results"] = max_results;
		d["use_bounds_only"] = use_bounds_only;
		d["prioritize_interactive"] = prioritize_interactive;
		d["filter_by_perception_tier"] = filter_by_perception_tier;
		d["min_perception_tier"] = min_perception_tier;
		return d;
	}

	static TurboPickConfig from_dictionary(const Dictionary &p_dict) {
		TurboPickConfig cfg;
		cfg.max_distance = p_dict.get("max_distance", cfg.max_distance);
		cfg.collision_mask = p_dict.get("collision_mask", cfg.collision_mask);
		cfg.required_components = p_dict.get("required_components", cfg.required_components);
		cfg.excluded_components = p_dict.get("excluded_components", cfg.excluded_components);
		cfg.include_disabled = p_dict.get("include_disabled", cfg.include_disabled);
		cfg.select_closest = p_dict.get("select_closest", cfg.select_closest);
		cfg.max_results = p_dict.get("max_results", cfg.max_results);
		cfg.use_bounds_only = p_dict.get("use_bounds_only", cfg.use_bounds_only);
		cfg.prioritize_interactive = p_dict.get("prioritize_interactive", cfg.prioritize_interactive);
		cfg.filter_by_perception_tier = p_dict.get("filter_by_perception_tier", cfg.filter_by_perception_tier);
		cfg.min_perception_tier = p_dict.get("min_perception_tier", cfg.min_perception_tier);
		return cfg;
	}
};

// ============================================================================
// Pickable Entity Data - Cached data for pickable entities
// ============================================================================

struct TurboPickableEntity {
	RID entity_rid;
	RID world_rid;
	AABB bounds;            // World-space bounds
	Transform3D transform;
	int priority;
	bool is_interactive;
	bool is_enabled;
	uint64_t last_update_frame;

	TurboPickableEntity() :
			priority(0), is_interactive(false), is_enabled(true), last_update_frame(0) {}
};

// ============================================================================
// TurboEntityPicker - Entity selection via ray casting
// ============================================================================
// Provides entity picking functionality for both editor and runtime use.
// Supports multiple picking strategies:
// 1. Octree-based picking (spatial acceleration structure)
// 2. Physics-based picking (using PhysicsServer3D)
// 3. Bounds-based picking (AABB checks against registered entities)
// ============================================================================

class TurboEntityPicker : public RefCounted {
	GDCLASS(TurboEntityPicker, RefCounted);

public:
	enum PickStrategy {
		PICK_STRATEGY_AUTO,      // Automatically choose best strategy
		PICK_STRATEGY_OCTREE,    // Use octree ray query
		PICK_STRATEGY_PHYSICS,   // Use PhysicsServer3D raycast
		PICK_STRATEGY_BOUNDS,    // Check AABB bounds only
	};

private:
	// Configuration
	TurboPickConfig default_config;
	PickStrategy preferred_strategy = PICK_STRATEGY_AUTO;

	// Camera reference
	Camera3D *current_camera = nullptr;
	Viewport *current_viewport = nullptr;

	// FlecsServer reference
	FlecsServer *flecs_server = nullptr;

	// Cached pickable entities (for bounds-based picking)
	HashMap<RID, TurboPickableEntity> pickable_entities;
	uint64_t current_frame = 0;

	// Last pick results (for debugging/inspection)
	LocalVector<TurboPickResult> last_pick_results;
	Vector2 last_pick_screen_pos;
	Vector3 last_pick_ray_origin;
	Vector3 last_pick_ray_direction;

	// Statistics
	int picks_this_frame = 0;
	int entities_tested = 0;
	double last_pick_time_usec = 0.0;

	// Internal methods
	PickStrategy _determine_strategy() const;
	void _pick_octree(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results);
	void _pick_physics(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results);
	void _pick_bounds(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results);

	bool _entity_matches_filter(const RID &p_entity, const TurboPickConfig &p_config) const;
	bool _ray_intersects_aabb(const Vector3 &p_from, const Vector3 &p_direction, const AABB &p_bounds, float &r_distance) const;
	int _get_entity_priority(const RID &p_entity) const;

	void _sort_and_filter_results(LocalVector<TurboPickResult> &r_results, const TurboPickConfig &p_config);

protected:
	static void _bind_methods();

public:
	// ========================================================================
	// Initialization
	// ========================================================================

	void initialize();
	void shutdown();

	void set_camera(Camera3D *p_camera);
	Camera3D *get_camera() const { return current_camera; }

	void set_viewport(Viewport *p_viewport);
	Viewport *get_viewport() const { return current_viewport; }

	// ========================================================================
	// Configuration
	// ========================================================================

	void set_default_config(const TurboPickConfig &p_config);
	TurboPickConfig get_default_config() const { return default_config; }
	Dictionary get_default_config_dict() const { return default_config.to_dictionary(); }
	void set_default_config_from_dict(const Dictionary &p_dict);

	void set_preferred_strategy(PickStrategy p_strategy);
	PickStrategy get_preferred_strategy() const { return preferred_strategy; }

	void set_max_distance(float p_distance);
	float get_max_distance() const { return default_config.max_distance; }

	// ========================================================================
	// Pickable Entity Registration
	// ========================================================================
	// For bounds-based picking, entities must be registered.
	// For octree/physics picking, this is optional but can improve priority handling.

	void register_pickable(const RID &p_entity, const RID &p_world, const AABB &p_bounds, const Transform3D &p_transform);
	void update_pickable(const RID &p_entity, const AABB &p_bounds, const Transform3D &p_transform);
	void unregister_pickable(const RID &p_entity);
	void clear_pickables();
	bool is_pickable_registered(const RID &p_entity) const;

	void set_pickable_priority(const RID &p_entity, int p_priority);
	void set_pickable_interactive(const RID &p_entity, bool p_interactive);
	void set_pickable_enabled(const RID &p_entity, bool p_enabled);

	int get_pickable_count() const { return pickable_entities.size(); }

	// ========================================================================
	// Picking - Screen Space
	// ========================================================================

	// Pick entity at screen position using default camera
	RID pick_at_screen_position(const Vector2 &p_screen_pos);
	RID pick_at_screen_position_with_config(const Vector2 &p_screen_pos, const TurboPickConfig &p_config);

	// Pick all entities at screen position
	TypedArray<RID> pick_all_at_screen_position(const Vector2 &p_screen_pos, int p_max_results = 32);
	TypedArray<Dictionary> pick_all_at_screen_position_detailed(const Vector2 &p_screen_pos, int p_max_results = 32);

	// ========================================================================
	// Picking - World Space (Ray)
	// ========================================================================

	// Pick entity along a ray
	RID pick_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance = 1000.0f);
	RID pick_ray_with_config(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config);

	// Pick all entities along a ray
	TypedArray<RID> pick_all_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance = 1000.0f, int p_max_results = 32);
	TypedArray<Dictionary> pick_all_ray_detailed(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config);

	// ========================================================================
	// Picking - Area
	// ========================================================================

	// Pick entities within an AABB region
	TypedArray<RID> pick_aabb(const AABB &p_bounds);
	TypedArray<RID> pick_aabb_with_config(const AABB &p_bounds, const TurboPickConfig &p_config);

	// Pick entities within a sphere
	TypedArray<RID> pick_sphere(const Vector3 &p_center, float p_radius);
	TypedArray<RID> pick_sphere_with_config(const Vector3 &p_center, float p_radius, const TurboPickConfig &p_config);

	// Pick entities within a screen rectangle (marquee selection)
	TypedArray<RID> pick_screen_rect(const Rect2 &p_screen_rect);

	// ========================================================================
	// Utility
	// ========================================================================

	// Convert screen position to world ray
	void screen_to_ray(const Vector2 &p_screen_pos, Vector3 &r_origin, Vector3 &r_direction) const;

	// Check if an entity is under the cursor
	bool is_entity_at_screen_position(const RID &p_entity, const Vector2 &p_screen_pos, float p_tolerance = 0.0f);

	// Get the 3D position on an entity surface at screen position
	bool get_entity_surface_position(const RID &p_entity, const Vector2 &p_screen_pos, Vector3 &r_position, Vector3 &r_normal);

	// ========================================================================
	// Debug / Inspection
	// ========================================================================

	// Get last pick information (for debugging)
	TypedArray<Dictionary> get_last_pick_results() const;
	Vector2 get_last_pick_screen_pos() const { return last_pick_screen_pos; }
	Vector3 get_last_pick_ray_origin() const { return last_pick_ray_origin; }
	Vector3 get_last_pick_ray_direction() const { return last_pick_ray_direction; }

	// Statistics
	Dictionary get_statistics() const;
	String get_debug_string() const;
	void reset_frame_statistics();

	// ========================================================================
	// Frame Update
	// ========================================================================

	void update(double p_delta);

	// ========================================================================
	// Constructors
	// ========================================================================

	TurboEntityPicker();
	~TurboEntityPicker();
};

VARIANT_ENUM_CAST(TurboEntityPicker::PickStrategy);

#endif // TURBO_ENTITY_PICKER_H