/**************************************************************************/
/*  turbo_debug_draw.h                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#ifndef TURBO_DEBUG_DRAW_H
#define TURBO_DEBUG_DRAW_H

#include "core/math/aabb.h"
#include "core/math/color.h"
#include "core/math/transform_3d.h"
#include "core/math/vector3.h"
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "scene/resources/immediate_mesh.h"

class MeshInstance3D;
class Node3D;
class StandardMaterial3D;
class Camera3D;

// ============================================================================
// Debug Draw Configuration
// ============================================================================

struct TurboDebugDrawConfig {
	// Global toggle
	bool enabled = false;

	// Draw type toggles
	bool draw_entity_bounds = true;
	bool draw_entity_gizmos = true;
	bool draw_selection_highlight = true;
	bool draw_custom_shapes = true;

	// Colors
	Color selection_color = Color(1.0f, 0.8f, 0.0f, 1.0f);      // Gold
	Color bounds_color = Color(0.3f, 0.7f, 1.0f, 0.5f);         // Light blue
	Color gizmo_x_color = Color(1.0f, 0.2f, 0.2f, 1.0f);        // Red
	Color gizmo_y_color = Color(0.2f, 1.0f, 0.2f, 1.0f);        // Green
	Color gizmo_z_color = Color(0.2f, 0.2f, 1.0f, 1.0f);        // Blue

	// Gizmo settings
	float gizmo_axis_length = 1.0f;
	float gizmo_line_width = 2.0f;
	float bounds_line_width = 1.0f;
	float selection_pulse_speed = 3.0f;
	float selection_pulse_amplitude = 0.2f;

	// Visibility
	bool depth_test = true;
	float max_draw_distance = 500.0f;

	// Frame budget (max primitives per frame)
	int max_lines_per_frame = 10000;
	int max_boxes_per_frame = 500;

	Dictionary to_dictionary() const;
	static TurboDebugDrawConfig from_dictionary(const Dictionary &p_dict);
};

// ============================================================================
// Debug Line Data
// ============================================================================

struct TurboDebugLine {
	Vector3 from;
	Vector3 to;
	Color color;
	float width;

	TurboDebugLine() :
			width(1.0f) {}
	TurboDebugLine(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_width = 1.0f) :
			from(p_from), to(p_to), color(p_color), width(p_width) {}
};

// ============================================================================
// Debug Box Data (wireframe or solid)
// ============================================================================

struct TurboDebugBox {
	AABB bounds;
	Color color;
	bool filled;
	Transform3D transform; // Optional transform for oriented boxes

	TurboDebugBox() :
			filled(false) {}
	TurboDebugBox(const AABB &p_bounds, const Color &p_color, bool p_filled = false) :
			bounds(p_bounds), color(p_color), filled(p_filled) {}
	TurboDebugBox(const AABB &p_bounds, const Color &p_color, const Transform3D &p_transform, bool p_filled = false) :
			bounds(p_bounds), color(p_color), filled(p_filled), transform(p_transform) {}
};

// ============================================================================
// Debug Sphere Data
// ============================================================================

struct TurboDebugSphere {
	Vector3 center;
	float radius;
	Color color;
	int segments;

	TurboDebugSphere() :
			radius(1.0f), segments(16) {}
	TurboDebugSphere(const Vector3 &p_center, float p_radius, const Color &p_color, int p_segments = 16) :
			center(p_center), radius(p_radius), color(p_color), segments(p_segments) {}
};

// ============================================================================
// Debug Arrow Data
// ============================================================================

struct TurboDebugArrow {
	Vector3 from;
	Vector3 to;
	Color color;
	float head_size;

	TurboDebugArrow() :
			head_size(0.1f) {}
	TurboDebugArrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_head_size = 0.1f) :
			from(p_from), to(p_to), color(p_color), head_size(p_head_size) {}
};

// ============================================================================
// Entity Gizmo Data
// ============================================================================

struct TurboEntityGizmo {
	RID entity_rid;
	Transform3D transform;
	AABB bounds;
	Color override_color;
	bool use_override_color;
	bool show_bounds;
	bool show_axes;
	bool is_selected;
	float pulse_phase;

	TurboEntityGizmo() :
			use_override_color(false),
			show_bounds(true),
			show_axes(true),
			is_selected(false),
			pulse_phase(0.0f) {}
};

// ============================================================================
// TurboDebugDraw - Main debug drawing system
// ============================================================================
// Provides runtime 3D debug visualization for entities, gizmos, and custom shapes.
// Can be used both in-editor and at runtime for game debugging.
// ============================================================================

class TurboDebugDraw : public RefCounted {
	GDCLASS(TurboDebugDraw, RefCounted);

private:
	// Configuration
	TurboDebugDrawConfig config;

	// Rendering resources
	Ref<ImmediateMesh> debug_mesh;
	Ref<StandardMaterial3D> line_material;
	Ref<StandardMaterial3D> line_material_no_depth;
	Ref<StandardMaterial3D> solid_material;

	// Scene node for rendering
	MeshInstance3D *mesh_instance = nullptr;
	Node3D *parent_node = nullptr;
	RID scenario_rid;

	// Camera for distance culling
	Camera3D *current_camera = nullptr;

	// Draw queues (cleared each frame)
	LocalVector<TurboDebugLine> lines;
	LocalVector<TurboDebugBox> boxes;
	LocalVector<TurboDebugSphere> spheres;
	LocalVector<TurboDebugArrow> arrows;

	// Entity gizmos (persistent until removed)
	HashMap<RID, TurboEntityGizmo> entity_gizmos;

	// Selected entity (special highlighting)
	RID selected_entity;

	// Timing
	double current_time = 0.0;
	uint64_t current_frame = 0;

	// Statistics
	int lines_drawn = 0;
	int boxes_drawn = 0;
	int spheres_drawn = 0;
	int gizmos_drawn = 0;

	// Internal methods
	void _create_materials();
	void _rebuild_mesh();
	void _clear_mesh();

	// Primitive building
	void _add_line_to_mesh(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color);
	void _add_box_wireframe(const AABB &p_box, const Color &p_color);
	void _add_box_wireframe_transformed(const AABB &p_box, const Transform3D &p_transform, const Color &p_color);
	void _add_box_solid(const AABB &p_box, const Color &p_color);
	void _add_sphere_wireframe(const Vector3 &p_center, float p_radius, const Color &p_color, int p_segments);
	void _add_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_head_size);
	void _add_transform_axes(const Transform3D &p_transform, float p_size);

	// Entity gizmo drawing
	void _draw_entity_gizmos();
	void _draw_entity_gizmo(TurboEntityGizmo &p_gizmo, double p_delta);

	// Culling
	bool _is_point_visible(const Vector3 &p_point) const;
	float _get_distance_to_camera(const Vector3 &p_point) const;

	// Animation
	Color _apply_pulse_effect(const Color &p_color, float p_phase) const;

protected:
	static void _bind_methods();

public:
	// ========================================================================
	// Initialization
	// ========================================================================

	void initialize(Node3D *p_parent, const RID &p_scenario);
	void shutdown();
	bool is_initialized() const { return debug_mesh.is_valid() && mesh_instance != nullptr; }

	void set_camera(Camera3D *p_camera);
	Camera3D *get_camera() const { return current_camera; }

	// ========================================================================
	// Configuration
	// ========================================================================

	void set_config(const TurboDebugDrawConfig &p_config);
	TurboDebugDrawConfig get_config() const { return config; }
	Dictionary get_config_dict() const { return config.to_dictionary(); }
	void set_config_from_dict(const Dictionary &p_dict);

	void set_enabled(bool p_enabled);
	bool is_enabled() const { return config.enabled; }

	void set_depth_test(bool p_depth_test);
	bool is_depth_test_enabled() const { return config.depth_test; }

	// ========================================================================
	// Selection
	// ========================================================================

	void set_selected_entity(const RID &p_entity);
	RID get_selected_entity() const { return selected_entity; }
	void clear_selection();

	// ========================================================================
	// Entity Gizmo Management
	// ========================================================================

	void add_entity_gizmo(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds);
	void update_entity_gizmo(const RID &p_entity, const Transform3D &p_transform);
	void update_entity_gizmo_bounds(const RID &p_entity, const AABB &p_bounds);
	void remove_entity_gizmo(const RID &p_entity);
	void clear_entity_gizmos();
	bool has_entity_gizmo(const RID &p_entity) const;

	void set_entity_gizmo_color(const RID &p_entity, const Color &p_color);
	void clear_entity_gizmo_color(const RID &p_entity);

	void set_entity_gizmo_show_bounds(const RID &p_entity, bool p_show);
	void set_entity_gizmo_show_axes(const RID &p_entity, bool p_show);

	// ========================================================================
	// Immediate Draw Commands (single frame)
	// ========================================================================

	void draw_line(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_width = 1.0f);
	void draw_box(const AABB &p_box, const Color &p_color, bool p_filled = false);
	void draw_box_transformed(const AABB &p_box, const Transform3D &p_transform, const Color &p_color, bool p_filled = false);
	void draw_sphere(const Vector3 &p_center, float p_radius, const Color &p_color, int p_segments = 16);
	void draw_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_head_size = 0.1f);
	void draw_transform(const Transform3D &p_transform, float p_size = 1.0f);
	void draw_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_length, const Color &p_color);
	void draw_point(const Vector3 &p_position, float p_size, const Color &p_color);

	// Draw multiple primitives at once
	void draw_lines(const PackedVector3Array &p_points, const Color &p_color);
	void draw_polyline(const PackedVector3Array &p_points, const Color &p_color, bool p_closed = false);

	// Clear immediate draw commands
	void clear_draws();

	// ========================================================================
	// Update Loop
	// ========================================================================

	void update(double p_delta);

	// Force immediate rebuild
	void force_rebuild();

	// ========================================================================
	// Statistics
	// ========================================================================

	Dictionary get_statistics() const;
	String get_debug_string() const;

	int get_lines_drawn() const { return lines_drawn; }
	int get_boxes_drawn() const { return boxes_drawn; }
	int get_gizmos_drawn() const { return gizmos_drawn; }

	// ========================================================================
	// Constructors
	// ========================================================================

	TurboDebugDraw();
	~TurboDebugDraw();
};

#endif // TURBO_DEBUG_DRAW_H