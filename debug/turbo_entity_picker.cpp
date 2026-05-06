/**************************************************************************/
/*  turbo_entity_picker.cpp                                               */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#include "turbo_entity_picker.h"

#include "core/config/engine.h"
#include "core/os/os.h"
#include "scene/3d/camera_3d.h"
#include "scene/main/viewport.h"

#include "../ecs/flecs_types/flecs_server.h"

// ============================================================================
// TurboEntityPicker Implementation
// ============================================================================

void TurboEntityPicker::_bind_methods() {
	// Initialization
	ClassDB::bind_method(D_METHOD("initialize"), &TurboEntityPicker::initialize);
	ClassDB::bind_method(D_METHOD("shutdown"), &TurboEntityPicker::shutdown);

	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &TurboEntityPicker::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &TurboEntityPicker::get_camera);
	ClassDB::bind_method(D_METHOD("set_viewport", "viewport"), &TurboEntityPicker::set_viewport);
	ClassDB::bind_method(D_METHOD("get_viewport"), &TurboEntityPicker::get_viewport);

	// Configuration
	ClassDB::bind_method(D_METHOD("set_default_config_from_dict", "config"), &TurboEntityPicker::set_default_config_from_dict);
	ClassDB::bind_method(D_METHOD("get_default_config_dict"), &TurboEntityPicker::get_default_config_dict);
	ClassDB::bind_method(D_METHOD("set_preferred_strategy", "strategy"), &TurboEntityPicker::set_preferred_strategy);
	ClassDB::bind_method(D_METHOD("get_preferred_strategy"), &TurboEntityPicker::get_preferred_strategy);
	ClassDB::bind_method(D_METHOD("set_max_distance", "distance"), &TurboEntityPicker::set_max_distance);
	ClassDB::bind_method(D_METHOD("get_max_distance"), &TurboEntityPicker::get_max_distance);

	// Pickable Entity Registration
	ClassDB::bind_method(D_METHOD("register_pickable", "entity", "world", "bounds", "transform"), &TurboEntityPicker::register_pickable);
	ClassDB::bind_method(D_METHOD("update_pickable", "entity", "bounds", "transform"), &TurboEntityPicker::update_pickable);
	ClassDB::bind_method(D_METHOD("unregister_pickable", "entity"), &TurboEntityPicker::unregister_pickable);
	ClassDB::bind_method(D_METHOD("clear_pickables"), &TurboEntityPicker::clear_pickables);
	ClassDB::bind_method(D_METHOD("is_pickable_registered", "entity"), &TurboEntityPicker::is_pickable_registered);

	ClassDB::bind_method(D_METHOD("set_pickable_priority", "entity", "priority"), &TurboEntityPicker::set_pickable_priority);
	ClassDB::bind_method(D_METHOD("set_pickable_interactive", "entity", "interactive"), &TurboEntityPicker::set_pickable_interactive);
	ClassDB::bind_method(D_METHOD("set_pickable_enabled", "entity", "enabled"), &TurboEntityPicker::set_pickable_enabled);
	ClassDB::bind_method(D_METHOD("get_pickable_count"), &TurboEntityPicker::get_pickable_count);

	// Picking - Screen Space
	ClassDB::bind_method(D_METHOD("pick_at_screen_position", "screen_pos"), &TurboEntityPicker::pick_at_screen_position);
	ClassDB::bind_method(D_METHOD("pick_all_at_screen_position", "screen_pos", "max_results"), &TurboEntityPicker::pick_all_at_screen_position, DEFVAL(32));
	ClassDB::bind_method(D_METHOD("pick_all_at_screen_position_detailed", "screen_pos", "max_results"), &TurboEntityPicker::pick_all_at_screen_position_detailed, DEFVAL(32));

	// Picking - World Space (Ray)
	ClassDB::bind_method(D_METHOD("pick_ray", "from", "direction", "max_distance"), &TurboEntityPicker::pick_ray, DEFVAL(1000.0f));
	ClassDB::bind_method(D_METHOD("pick_all_ray", "from", "direction", "max_distance", "max_results"), &TurboEntityPicker::pick_all_ray, DEFVAL(1000.0f), DEFVAL(32));

	// Picking - Area
	ClassDB::bind_method(D_METHOD("pick_aabb", "bounds"), &TurboEntityPicker::pick_aabb);
	ClassDB::bind_method(D_METHOD("pick_sphere", "center", "radius"), &TurboEntityPicker::pick_sphere);
	ClassDB::bind_method(D_METHOD("pick_screen_rect", "screen_rect"), &TurboEntityPicker::pick_screen_rect);

	// Utility
	ClassDB::bind_method(D_METHOD("is_entity_at_screen_position", "entity", "screen_pos", "tolerance"), &TurboEntityPicker::is_entity_at_screen_position, DEFVAL(0.0f));

	// Debug
	ClassDB::bind_method(D_METHOD("get_last_pick_results"), &TurboEntityPicker::get_last_pick_results);
	ClassDB::bind_method(D_METHOD("get_last_pick_screen_pos"), &TurboEntityPicker::get_last_pick_screen_pos);
	ClassDB::bind_method(D_METHOD("get_last_pick_ray_origin"), &TurboEntityPicker::get_last_pick_ray_origin);
	ClassDB::bind_method(D_METHOD("get_last_pick_ray_direction"), &TurboEntityPicker::get_last_pick_ray_direction);
	ClassDB::bind_method(D_METHOD("get_statistics"), &TurboEntityPicker::get_statistics);
	ClassDB::bind_method(D_METHOD("get_debug_string"), &TurboEntityPicker::get_debug_string);
	ClassDB::bind_method(D_METHOD("reset_frame_statistics"), &TurboEntityPicker::reset_frame_statistics);

	// Update
	ClassDB::bind_method(D_METHOD("update", "delta"), &TurboEntityPicker::update);

	// Enums
	BIND_ENUM_CONSTANT(PICK_STRATEGY_AUTO);
	BIND_ENUM_CONSTANT(PICK_STRATEGY_OCTREE);
	BIND_ENUM_CONSTANT(PICK_STRATEGY_PHYSICS);
	BIND_ENUM_CONSTANT(PICK_STRATEGY_BOUNDS);
}

TurboEntityPicker::TurboEntityPicker() {
}

TurboEntityPicker::~TurboEntityPicker() {
	shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

void TurboEntityPicker::initialize() {
	flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));
}

void TurboEntityPicker::shutdown() {
	pickable_entities.clear();
	last_pick_results.clear();
	current_camera = nullptr;
	current_viewport = nullptr;
	flecs_server = nullptr;
}

void TurboEntityPicker::set_camera(Camera3D *p_camera) {
	current_camera = p_camera;
}

void TurboEntityPicker::set_viewport(Viewport *p_viewport) {
	current_viewport = p_viewport;
}

// ============================================================================
// Configuration
// ============================================================================

void TurboEntityPicker::set_default_config(const TurboPickConfig &p_config) {
	default_config = p_config;
}

void TurboEntityPicker::set_default_config_from_dict(const Dictionary &p_dict) {
	default_config = TurboPickConfig::from_dictionary(p_dict);
}

void TurboEntityPicker::set_preferred_strategy(PickStrategy p_strategy) {
	preferred_strategy = p_strategy;
}

void TurboEntityPicker::set_max_distance(float p_distance) {
	default_config.max_distance = p_distance;
}

// ============================================================================
// Pickable Entity Registration
// ============================================================================

void TurboEntityPicker::register_pickable(const RID &p_entity, const RID &p_world, const AABB &p_bounds, const Transform3D &p_transform) {
	TurboPickableEntity pickable;
	pickable.entity_rid = p_entity;
	pickable.world_rid = p_world;
	pickable.bounds = p_bounds;
	pickable.transform = p_transform;
	pickable.priority = 0;
	pickable.is_interactive = false;
	pickable.is_enabled = true;
	pickable.last_update_frame = current_frame;

	pickable_entities[p_entity] = pickable;
}

void TurboEntityPicker::update_pickable(const RID &p_entity, const AABB &p_bounds, const Transform3D &p_transform) {
	if (pickable_entities.has(p_entity)) {
		pickable_entities[p_entity].bounds = p_bounds;
		pickable_entities[p_entity].transform = p_transform;
		pickable_entities[p_entity].last_update_frame = current_frame;
	}
}

void TurboEntityPicker::unregister_pickable(const RID &p_entity) {
	pickable_entities.erase(p_entity);
}

void TurboEntityPicker::clear_pickables() {
	pickable_entities.clear();
}

bool TurboEntityPicker::is_pickable_registered(const RID &p_entity) const {
	return pickable_entities.has(p_entity);
}

void TurboEntityPicker::set_pickable_priority(const RID &p_entity, int p_priority) {
	if (pickable_entities.has(p_entity)) {
		pickable_entities[p_entity].priority = p_priority;
	}
}

void TurboEntityPicker::set_pickable_interactive(const RID &p_entity, bool p_interactive) {
	if (pickable_entities.has(p_entity)) {
		pickable_entities[p_entity].is_interactive = p_interactive;
	}
}

void TurboEntityPicker::set_pickable_enabled(const RID &p_entity, bool p_enabled) {
	if (pickable_entities.has(p_entity)) {
		pickable_entities[p_entity].is_enabled = p_enabled;
	}
}

// ============================================================================
// Strategy Selection
// ============================================================================

TurboEntityPicker::PickStrategy TurboEntityPicker::_determine_strategy() const {
	if (preferred_strategy != PICK_STRATEGY_AUTO) {
		return preferred_strategy;
	}

	// Auto-detect best strategy
	// For now, prefer bounds-based picking as it's the most reliable
	// without external dependencies
	if (!pickable_entities.is_empty()) {
		return PICK_STRATEGY_BOUNDS;
	}

	// Fall back to physics if available
	return PICK_STRATEGY_PHYSICS;
}

// ============================================================================
// Screen-Space Picking
// ============================================================================

RID TurboEntityPicker::pick_at_screen_position(const Vector2 &p_screen_pos) {
	return pick_at_screen_position_with_config(p_screen_pos, default_config);
}

RID TurboEntityPicker::pick_at_screen_position_with_config(const Vector2 &p_screen_pos, const TurboPickConfig &p_config) {
	if (!current_camera) {
		return RID();
	}

	Vector3 origin, direction;
	screen_to_ray(p_screen_pos, origin, direction);

	LocalVector<TurboPickResult> results;
	TurboPickConfig cfg = p_config;
	cfg.select_closest = true;
	cfg.max_results = 1;

	uint64_t start_time = OS::get_singleton()->get_ticks_usec();

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(origin, direction, cfg, results);
			break;
	}

	last_pick_time_usec = (double)(OS::get_singleton()->get_ticks_usec() - start_time);
	last_pick_screen_pos = p_screen_pos;
	last_pick_ray_origin = origin;
	last_pick_ray_direction = direction;
	picks_this_frame++;

	// Store results
	last_pick_results.clear();
	for (const TurboPickResult &result : results) {
		last_pick_results.push_back(result);
	}

	if (results.size() > 0) {
		return results[0].entity_rid;
	}

	return RID();
}

TypedArray<RID> TurboEntityPicker::pick_all_at_screen_position(const Vector2 &p_screen_pos, int p_max_results) {
	TypedArray<RID> result;

	if (!current_camera) {
		return result;
	}

	Vector3 origin, direction;
	screen_to_ray(p_screen_pos, origin, direction);

	LocalVector<TurboPickResult> results;
	TurboPickConfig cfg = default_config;
	cfg.select_closest = false;
	cfg.max_results = p_max_results;

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(origin, direction, cfg, results);
			break;
	}

	_sort_and_filter_results(results, cfg);

	for (const TurboPickResult &res : results) {
		result.push_back(res.entity_rid);
	}

	return result;
}

TypedArray<Dictionary> TurboEntityPicker::pick_all_at_screen_position_detailed(const Vector2 &p_screen_pos, int p_max_results) {
	TypedArray<Dictionary> result;

	if (!current_camera) {
		return result;
	}

	Vector3 origin, direction;
	screen_to_ray(p_screen_pos, origin, direction);

	LocalVector<TurboPickResult> results;
	TurboPickConfig cfg = default_config;
	cfg.select_closest = false;
	cfg.max_results = p_max_results;

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(origin, direction, cfg, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(origin, direction, cfg, results);
			break;
	}

	_sort_and_filter_results(results, cfg);

	for (const TurboPickResult &res : results) {
		result.push_back(res.to_dictionary());
	}

	return result;
}

// ============================================================================
// World-Space Picking (Ray)
// ============================================================================

RID TurboEntityPicker::pick_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance) {
	TurboPickConfig cfg = default_config;
	cfg.max_distance = p_max_distance;
	cfg.select_closest = true;
	cfg.max_results = 1;

	return pick_ray_with_config(p_from, p_direction, cfg);
}

RID TurboEntityPicker::pick_ray_with_config(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config) {
	LocalVector<TurboPickResult> results;

	uint64_t start_time = OS::get_singleton()->get_ticks_usec();

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(p_from, p_direction, p_config, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(p_from, p_direction, p_config, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(p_from, p_direction, p_config, results);
			break;
	}

	last_pick_time_usec = (double)(OS::get_singleton()->get_ticks_usec() - start_time);
	last_pick_ray_origin = p_from;
	last_pick_ray_direction = p_direction;
	picks_this_frame++;

	if (results.size() > 0) {
		return results[0].entity_rid;
	}

	return RID();
}

TypedArray<RID> TurboEntityPicker::pick_all_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance, int p_max_results) {
	TypedArray<RID> result;

	TurboPickConfig cfg = default_config;
	cfg.max_distance = p_max_distance;
	cfg.select_closest = false;
	cfg.max_results = p_max_results;

	LocalVector<TurboPickResult> results;

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(p_from, p_direction, cfg, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(p_from, p_direction, cfg, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(p_from, p_direction, cfg, results);
			break;
	}

	_sort_and_filter_results(results, cfg);

	for (const TurboPickResult &res : results) {
		result.push_back(res.entity_rid);
	}

	return result;
}

TypedArray<Dictionary> TurboEntityPicker::pick_all_ray_detailed(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config) {
	TypedArray<Dictionary> result;

	LocalVector<TurboPickResult> results;

	PickStrategy strategy = _determine_strategy();
	switch (strategy) {
		case PICK_STRATEGY_OCTREE:
			_pick_octree(p_from, p_direction, p_config, results);
			break;
		case PICK_STRATEGY_PHYSICS:
			_pick_physics(p_from, p_direction, p_config, results);
			break;
		case PICK_STRATEGY_BOUNDS:
		default:
			_pick_bounds(p_from, p_direction, p_config, results);
			break;
	}

	_sort_and_filter_results(results, p_config);

	for (const TurboPickResult &res : results) {
		result.push_back(res.to_dictionary());
	}

	return result;
}

// ============================================================================
// Area Picking
// ============================================================================

TypedArray<RID> TurboEntityPicker::pick_aabb(const AABB &p_bounds) {
	return pick_aabb_with_config(p_bounds, default_config);
}

TypedArray<RID> TurboEntityPicker::pick_aabb_with_config(const AABB &p_bounds, const TurboPickConfig &p_config) {
	TypedArray<RID> result;

	for (const KeyValue<RID, TurboPickableEntity> &kv : pickable_entities) {
		const TurboPickableEntity &pickable = kv.value;

		if (!pickable.is_enabled && !p_config.include_disabled) {
			continue;
		}

		// Transform bounds to world space
		AABB world_bounds = pickable.transform.xform(pickable.bounds);

		if (p_bounds.intersects(world_bounds)) {
			if (_entity_matches_filter(pickable.entity_rid, p_config)) {
				result.push_back(pickable.entity_rid);
			}
		}
	}

	return result;
}

TypedArray<RID> TurboEntityPicker::pick_sphere(const Vector3 &p_center, float p_radius) {
	return pick_sphere_with_config(p_center, p_radius, default_config);
}

TypedArray<RID> TurboEntityPicker::pick_sphere_with_config(const Vector3 &p_center, float p_radius, const TurboPickConfig &p_config) {
	TypedArray<RID> result;

	for (const KeyValue<RID, TurboPickableEntity> &kv : pickable_entities) {
		const TurboPickableEntity &pickable = kv.value;

		if (!pickable.is_enabled && !p_config.include_disabled) {
			continue;
		}

		// Check distance to entity center
		Vector3 entity_center = pickable.transform.origin;
		float dist_sq = p_center.distance_squared_to(entity_center);

		// Include bounds size in check
		float entity_radius = pickable.bounds.get_longest_axis_size() * 0.5f;
		float total_radius = p_radius + entity_radius;

		if (dist_sq <= total_radius * total_radius) {
			if (_entity_matches_filter(pickable.entity_rid, p_config)) {
				result.push_back(pickable.entity_rid);
			}
		}
	}

	return result;
}

TypedArray<RID> TurboEntityPicker::pick_screen_rect(const Rect2 &p_screen_rect) {
	TypedArray<RID> result;

	if (!current_camera) {
		return result;
	}

	// For each pickable entity, project its center to screen space
	// and check if it falls within the rectangle
	for (const KeyValue<RID, TurboPickableEntity> &kv : pickable_entities) {
		const TurboPickableEntity &pickable = kv.value;

		if (!pickable.is_enabled) {
			continue;
		}

		Vector3 world_pos = pickable.transform.origin;

		// Check if point is in front of camera
		if (!current_camera->is_position_in_frustum(world_pos)) {
			continue;
		}

		Vector2 screen_pos = current_camera->unproject_position(world_pos);

		if (p_screen_rect.has_point(screen_pos)) {
			result.push_back(pickable.entity_rid);
		}
	}

	return result;
}

// ============================================================================
// Picking Implementation - Bounds
// ============================================================================

void TurboEntityPicker::_pick_bounds(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results) {
	entities_tested = 0;

	Vector3 dir = p_direction.normalized();

	for (const KeyValue<RID, TurboPickableEntity> &kv : pickable_entities) {
		const TurboPickableEntity &pickable = kv.value;
		entities_tested++;

		if (!pickable.is_enabled && !p_config.include_disabled) {
			continue;
		}

		// Transform bounds to world space
		AABB world_bounds = pickable.transform.xform(pickable.bounds);

		float distance = 0.0f;
		if (_ray_intersects_aabb(p_from, dir, world_bounds, distance)) {
			if (distance <= p_config.max_distance) {
				if (_entity_matches_filter(pickable.entity_rid, p_config)) {
					TurboPickResult result;
					result.entity_rid = pickable.entity_rid;
					result.world_rid = pickable.world_rid;
					result.hit_position = p_from + dir * distance;
					result.distance = distance;
					result.priority = _get_entity_priority(pickable.entity_rid);
					r_results.push_back(result);
				}
			}
		}
	}

	_sort_and_filter_results(r_results, p_config);
}

// ============================================================================
// Picking Implementation - Octree
// ============================================================================

void TurboEntityPicker::_pick_octree(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results) {
	// TODO: Integrate with spatial octree if available
	// For now, fall back to bounds-based picking
	_pick_bounds(p_from, p_direction, p_config, r_results);
}

// ============================================================================
// Picking Implementation - Physics
// ============================================================================

void TurboEntityPicker::_pick_physics(const Vector3 &p_from, const Vector3 &p_direction, const TurboPickConfig &p_config, LocalVector<TurboPickResult> &r_results) {
	// TODO: Integrate with PhysicsServer3D
	// For now, fall back to bounds-based picking
	_pick_bounds(p_from, p_direction, p_config, r_results);
}

// ============================================================================
// Utility
// ============================================================================

void TurboEntityPicker::screen_to_ray(const Vector2 &p_screen_pos, Vector3 &r_origin, Vector3 &r_direction) const {
	if (!current_camera) {
		r_origin = Vector3();
		r_direction = Vector3(0, 0, -1);
		return;
	}

	r_origin = current_camera->project_ray_origin(p_screen_pos);
	r_direction = current_camera->project_ray_normal(p_screen_pos);
}

bool TurboEntityPicker::is_entity_at_screen_position(const RID &p_entity, const Vector2 &p_screen_pos, float p_tolerance) {
	if (!pickable_entities.has(p_entity) || !current_camera) {
		return false;
	}

	const TurboPickableEntity &pickable = pickable_entities[p_entity];

	// Project entity center to screen
	Vector3 world_pos = pickable.transform.origin;

	if (!current_camera->is_position_in_frustum(world_pos)) {
		return false;
	}

	Vector2 entity_screen_pos = current_camera->unproject_position(world_pos);
	float distance = entity_screen_pos.distance_to(p_screen_pos);

	if (p_tolerance > 0) {
		return distance <= p_tolerance;
	}

	// Use bounds projected size as tolerance
	float bounds_size = pickable.bounds.get_longest_axis_size();
	float camera_distance = current_camera->get_global_position().distance_to(world_pos);
	float projected_size = (bounds_size / camera_distance) * current_camera->get_viewport()->get_visible_rect().size.x * 0.5f;

	return distance <= projected_size;
}

bool TurboEntityPicker::get_entity_surface_position(const RID &p_entity, const Vector2 &p_screen_pos, Vector3 &r_position, Vector3 &r_normal) {
	if (!pickable_entities.has(p_entity) || !current_camera) {
		return false;
	}

	Vector3 origin, direction;
	screen_to_ray(p_screen_pos, origin, direction);

	const TurboPickableEntity &pickable = pickable_entities[p_entity];
	AABB world_bounds = pickable.transform.xform(pickable.bounds);

	float distance = 0.0f;
	if (_ray_intersects_aabb(origin, direction, world_bounds, distance)) {
		r_position = origin + direction * distance;
		// Approximate normal based on which face was hit
		r_normal = (r_position - world_bounds.get_center()).normalized();
		return true;
	}

	return false;
}

bool TurboEntityPicker::_ray_intersects_aabb(const Vector3 &p_from, const Vector3 &p_direction, const AABB &p_bounds, float &r_distance) const {
	Vector3 inv_dir = Vector3(
			p_direction.x != 0.0f ? 1.0f / p_direction.x : 1e10f,
			p_direction.y != 0.0f ? 1.0f / p_direction.y : 1e10f,
			p_direction.z != 0.0f ? 1.0f / p_direction.z : 1e10f);

	Vector3 t1 = (p_bounds.position - p_from) * inv_dir;
	Vector3 t2 = (p_bounds.position + p_bounds.size - p_from) * inv_dir;

	Vector3 t_min = Vector3(MIN(t1.x, t2.x), MIN(t1.y, t2.y), MIN(t1.z, t2.z));
	Vector3 t_max = Vector3(MAX(t1.x, t2.x), MAX(t1.y, t2.y), MAX(t1.z, t2.z));

	float tmin = MAX(MAX(t_min.x, t_min.y), t_min.z);
	float tmax = MIN(MIN(t_max.x, t_max.y), t_max.z);

	if (tmax < 0 || tmin > tmax) {
		return false;
	}

	r_distance = tmin >= 0 ? tmin : tmax;
	return true;
}

bool TurboEntityPicker::_entity_matches_filter(const RID &p_entity, const TurboPickConfig &p_config) const {
	if (!flecs_server) {
		return true; // No filter if no server
	}

	// Check required components
	for (int i = 0; i < p_config.required_components.size(); i++) {
		// TODO: Check if entity has component via FlecsServer
		// For now, pass all entities
	}

	// Check excluded components
	for (int i = 0; i < p_config.excluded_components.size(); i++) {
		// TODO: Check if entity has component via FlecsServer
	}

	return true;
}

int TurboEntityPicker::_get_entity_priority(const RID &p_entity) const {
	if (pickable_entities.has(p_entity)) {
		const TurboPickableEntity &pickable = pickable_entities[p_entity];
		int priority = pickable.priority;

		// Boost interactive entities
		if (pickable.is_interactive && default_config.prioritize_interactive) {
			priority += 100;
		}

		return priority;
	}
	return 0;
}

void TurboEntityPicker::_sort_and_filter_results(LocalVector<TurboPickResult> &r_results, const TurboPickConfig &p_config) {
	if (r_results.size() == 0) {
		return;
	}

	// Sort by priority and distance
	r_results.sort();

	// Limit results
	if (p_config.max_results > 0 && (int)r_results.size() > p_config.max_results) {
		r_results.resize(p_config.max_results);
	}

	// If select_closest, keep only the first result
	if (p_config.select_closest && r_results.size() > 1) {
		r_results.resize(1);
	}
}

// ============================================================================
// Debug / Inspection
// ============================================================================

TypedArray<Dictionary> TurboEntityPicker::get_last_pick_results() const {
	TypedArray<Dictionary> result;
	for (const TurboPickResult &res : last_pick_results) {
		result.push_back(res.to_dictionary());
	}
	return result;
}

Dictionary TurboEntityPicker::get_statistics() const {
	Dictionary stats;
	stats["pickable_count"] = (int)pickable_entities.size();
	stats["picks_this_frame"] = picks_this_frame;
	stats["entities_tested"] = entities_tested;
	stats["last_pick_time_usec"] = last_pick_time_usec;
	stats["current_frame"] = current_frame;
	stats["preferred_strategy"] = (int)preferred_strategy;
	return stats;
}

String TurboEntityPicker::get_debug_string() const {
	return vformat("TurboEntityPicker: pickables=%d picks=%d tested=%d time=%.2fus",
			(int)pickable_entities.size(), picks_this_frame, entities_tested, last_pick_time_usec);
}

void TurboEntityPicker::reset_frame_statistics() {
	picks_this_frame = 0;
	entities_tested = 0;
}

// ============================================================================
// Update
// ============================================================================

void TurboEntityPicker::update(double p_delta) {
	current_frame++;
	reset_frame_statistics();
}