/**************************************************************************/
/*  turbo_debug_system.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#ifndef TURBO_DEBUG_SYSTEM_H
#define TURBO_DEBUG_SYSTEM_H

#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/templates/hash_map.h"
#include "core/templates/local_vector.h"
#include "core/templates/rid.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

#include "turbo_debug_draw.h"
#include "turbo_debug_panel.h"
#include "turbo_entity_picker.h"

class Camera3D;
class FlecsServer;
class Node3D;
class SceneTree;
class Viewport;
class Window;

// ============================================================================
// Debug System Configuration
// ============================================================================

struct TurboDebugSystemConfig {
	// Global enable
	bool enabled = true;

	// Feature toggles
	bool enable_picking = true;
	bool enable_gizmos = true;
	bool enable_panel = true;
	bool enable_hotkeys = true;

	// Auto-initialization
	bool auto_initialize = true;        // Initialize when added to scene
	bool auto_find_camera = true;       // Automatically find and use main camera
	bool auto_register_entities = true; // Auto-register entities for picking

	// Input settings
	String pick_action = "turbo_debug_pick";      // Input action for picking
	String toggle_panel_action = "turbo_debug_toggle_panel";
	String toggle_gizmos_action = "turbo_debug_toggle_gizmos";
	int pick_mouse_button = 1; // MOUSE_BUTTON_LEFT

	// Visualization defaults
	bool default_show_bounds = true;
	bool default_show_axes = true;
	bool default_depth_test = true;

	// Performance
	float pick_cooldown = 0.05f;        // Minimum time between picks
	int max_gizmos = 1000;              // Maximum simultaneous gizmos
	int max_pickables = 10000;          // Maximum registered pickables

	Dictionary to_dictionary() const {
		Dictionary d;
		d["enabled"] = enabled;
		d["enable_picking"] = enable_picking;
		d["enable_gizmos"] = enable_gizmos;
		d["enable_panel"] = enable_panel;
		d["enable_hotkeys"] = enable_hotkeys;
		d["auto_initialize"] = auto_initialize;
		d["auto_find_camera"] = auto_find_camera;
		d["auto_register_entities"] = auto_register_entities;
		d["pick_action"] = pick_action;
		d["toggle_panel_action"] = toggle_panel_action;
		d["toggle_gizmos_action"] = toggle_gizmos_action;
		d["pick_mouse_button"] = pick_mouse_button;
		d["default_show_bounds"] = default_show_bounds;
		d["default_show_axes"] = default_show_axes;
		d["default_depth_test"] = default_depth_test;
		d["pick_cooldown"] = pick_cooldown;
		d["max_gizmos"] = max_gizmos;
		d["max_pickables"] = max_pickables;
		return d;
	}

	static TurboDebugSystemConfig from_dictionary(const Dictionary &p_dict) {
		TurboDebugSystemConfig cfg;
		cfg.enabled = p_dict.get("enabled", cfg.enabled);
		cfg.enable_picking = p_dict.get("enable_picking", cfg.enable_picking);
		cfg.enable_gizmos = p_dict.get("enable_gizmos", cfg.enable_gizmos);
		cfg.enable_panel = p_dict.get("enable_panel", cfg.enable_panel);
		cfg.enable_hotkeys = p_dict.get("enable_hotkeys", cfg.enable_hotkeys);
		cfg.auto_initialize = p_dict.get("auto_initialize", cfg.auto_initialize);
		cfg.auto_find_camera = p_dict.get("auto_find_camera", cfg.auto_find_camera);
		cfg.auto_register_entities = p_dict.get("auto_register_entities", cfg.auto_register_entities);
		cfg.pick_action = p_dict.get("pick_action", cfg.pick_action);
		cfg.toggle_panel_action = p_dict.get("toggle_panel_action", cfg.toggle_panel_action);
		cfg.toggle_gizmos_action = p_dict.get("toggle_gizmos_action", cfg.toggle_gizmos_action);
		cfg.pick_mouse_button = p_dict.get("pick_mouse_button", cfg.pick_mouse_button);
		cfg.default_show_bounds = p_dict.get("default_show_bounds", cfg.default_show_bounds);
		cfg.default_show_axes = p_dict.get("default_show_axes", cfg.default_show_axes);
		cfg.default_depth_test = p_dict.get("default_depth_test", cfg.default_depth_test);
		cfg.pick_cooldown = p_dict.get("pick_cooldown", cfg.pick_cooldown);
		cfg.max_gizmos = p_dict.get("max_gizmos", cfg.max_gizmos);
		cfg.max_pickables = p_dict.get("max_pickables", cfg.max_pickables);
		return cfg;
	}
};

// ============================================================================
// Selection Changed Event Data
// ============================================================================

struct TurboSelectionEvent {
	RID previous_entity;
	RID previous_world;
	RID new_entity;
	RID new_world;
	Vector3 pick_position;
	bool was_user_initiated;

	Dictionary to_dictionary() const {
		Dictionary d;
		d["previous_entity"] = previous_entity;
		d["previous_world"] = previous_world;
		d["new_entity"] = new_entity;
		d["new_world"] = new_world;
		d["pick_position"] = pick_position;
		d["was_user_initiated"] = was_user_initiated;
		return d;
	}
};

// ============================================================================
// TurboDebugSystem - Main debug infrastructure coordinator
// ============================================================================
// Singleton that coordinates all debug functionality:
// - Entity picking and selection
// - 3D debug visualization (gizmos, bounds, etc.)
// - In-game debug panel
// - Hotkey handling
// - Integration with AxiomScript (optional)
//
// Usage:
//   var debug = TurboDebugSystem.get_singleton()
//   debug.set_enabled(true)
//   debug.pick_entity_at_mouse()
//   debug.show_panel()
//
// Per Axiom Philosophy:
// - All state is explicit and queryable
// - No hidden per-frame magic
// - Degradation is graceful
// ============================================================================

class TurboDebugSystem : public Object {
	GDCLASS(TurboDebugSystem, Object);

private:
	static TurboDebugSystem *singleton;

	// Configuration
	TurboDebugSystemConfig config;

	// Subsystems (owned)
	Ref<TurboDebugDraw> debug_draw;
	Ref<TurboEntityPicker> entity_picker;
	TurboDebugPanel *debug_panel = nullptr;

	// External references
	FlecsServer *flecs_server = nullptr;
	Camera3D *current_camera = nullptr;
	Viewport *current_viewport = nullptr;
	Node3D *visualization_root = nullptr;
	SceneTree *scene_tree = nullptr;

	// Selection state
	RID selected_entity;
	RID selected_world;
	Vector3 selection_position;

	// Tracked entities (for auto-gizmo management)
	HashMap<RID, RID> tracked_entities; // entity_rid -> world_rid

	// State
	bool initialized = false;
	bool processing_enabled = true;

	// Timing
	double current_time = 0.0;
	double last_pick_time = 0.0;
	uint64_t current_frame = 0;

	// Statistics
	int entities_tracked = 0;
	int picks_this_session = 0;
	int selection_changes = 0;

	// ========================================================================
	// Internal Methods
	// ========================================================================

	void _setup_input_actions();
	void _cleanup_input_actions();

	void _auto_find_camera();
	void _on_camera_changed();

	void _create_visualization_root();
	void _destroy_visualization_root();

	void _register_default_toggles();

	void _handle_pick_input(const Vector2 &p_position);
	void _perform_selection(const RID &p_entity, const RID &p_world, const Vector3 &p_position, bool p_user_initiated);
	void _update_selection_gizmo();
	void _clear_selection_gizmo();

	void _on_entity_created(const RID &p_entity, const RID &p_world);
	void _on_entity_destroyed(const RID &p_entity, const RID &p_world);
	void _on_entity_transform_changed(const RID &p_entity, const Transform3D &p_transform);
	void _on_entity_bounds_changed(const RID &p_entity, const AABB &p_bounds);

	// Visualization toggle callbacks
	void _on_toggle_bounds(bool p_enabled);
	void _on_toggle_axes(bool p_enabled);
	void _on_toggle_selection(bool p_enabled);

protected:
	static void _bind_methods();

public:
	static TurboDebugSystem *get_singleton() { return singleton; }

	// ========================================================================
	// Initialization
	// ========================================================================

	void initialize(SceneTree *p_scene_tree);
	void initialize_with_viewport(Viewport *p_viewport);
	void shutdown();
	bool is_initialized() const { return initialized; }

	void set_flecs_server(FlecsServer *p_server);
	FlecsServer *get_flecs_server() const { return flecs_server; }

	// ========================================================================
	// Configuration
	// ========================================================================

	void set_config(const TurboDebugSystemConfig &p_config);
	TurboDebugSystemConfig get_config() const { return config; }
	Dictionary get_config_dict() const { return config.to_dictionary(); }
	void set_config_from_dict(const Dictionary &p_dict);

	void set_enabled(bool p_enabled);
	bool is_enabled() const { return config.enabled; }

	void set_picking_enabled(bool p_enabled);
	bool is_picking_enabled() const { return config.enable_picking; }

	void set_gizmos_enabled(bool p_enabled);
	bool is_gizmos_enabled() const { return config.enable_gizmos; }

	void set_panel_enabled(bool p_enabled);
	bool is_panel_enabled() const { return config.enable_panel; }

	// ========================================================================
	// Camera & Viewport
	// ========================================================================

	void set_camera(Camera3D *p_camera);
	Camera3D *get_camera() const { return current_camera; }

	void set_viewport(Viewport *p_viewport);
	Viewport *get_viewport() const { return current_viewport; }

	void set_visualization_root(Node3D *p_root);
	Node3D *get_visualization_root() const { return visualization_root; }

	// ========================================================================
	// Selection
	// ========================================================================

	void select_entity(const RID &p_entity, const RID &p_world);
	void clear_selection();
	bool has_selection() const { return selected_entity.is_valid(); }

	RID get_selected_entity() const { return selected_entity; }
	RID get_selected_world() const { return selected_world; }
	Vector3 get_selection_position() const { return selection_position; }

	// Get detailed info about selected entity
	Dictionary get_selected_entity_info() const;
	TypedArray<Dictionary> get_selected_entity_components() const;

	// ========================================================================
	// Picking (User-Facing API)
	// ========================================================================

	// Pick at current mouse position
	RID pick_at_mouse();
	RID pick_and_select_at_mouse();

	// Pick at specific screen position
	RID pick_at_position(const Vector2 &p_screen_pos);
	RID pick_and_select_at_position(const Vector2 &p_screen_pos);

	// Pick along a ray
	RID pick_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance = 1000.0f);

	// Pick with full configuration
	RID pick_with_config(const Vector2 &p_screen_pos, const Dictionary &p_config);
	TypedArray<RID> pick_all_with_config(const Vector2 &p_screen_pos, const Dictionary &p_config);

	// ========================================================================
	// Entity Tracking (for auto-gizmos)
	// ========================================================================

	void track_entity(const RID &p_entity, const RID &p_world);
	void untrack_entity(const RID &p_entity);
	void clear_tracked_entities();
	bool is_entity_tracked(const RID &p_entity) const;
	TypedArray<RID> get_tracked_entities() const;

	void update_tracked_entity(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds);

	// ========================================================================
	// Debug Panel Access
	// ========================================================================

	void show_panel();
	void hide_panel();
	void toggle_panel();
	bool is_panel_visible() const;

	TurboDebugPanel *get_panel() const { return debug_panel; }

	// Panel toggles (convenience wrappers)
	void register_visualization_toggle(const String &p_name, const String &p_tooltip, bool p_default, const String &p_category = "General");
	void set_visualization_toggle(const String &p_name, bool p_enabled);
	bool get_visualization_toggle(const String &p_name) const;

	// Panel logging
	void log(const String &p_message);
	void log_warning(const String &p_message);
	void log_error(const String &p_message);

	// ========================================================================
	// Debug Draw Access
	// ========================================================================

	Ref<TurboDebugDraw> get_debug_draw() const { return debug_draw; }
	Ref<TurboEntityPicker> get_entity_picker() const { return entity_picker; }

	// Convenience wrappers for debug draw
	void draw_line(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color);
	void draw_box(const AABB &p_box, const Color &p_color, bool p_filled = false);
	void draw_sphere(const Vector3 &p_center, float p_radius, const Color &p_color);
	void draw_transform(const Transform3D &p_transform, float p_size = 1.0f);
	void draw_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color);
	void clear_debug_draws();

	// ========================================================================
	// Gizmo Management
	// ========================================================================

	void add_gizmo(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds);
	void update_gizmo(const RID &p_entity, const Transform3D &p_transform);
	void update_gizmo_bounds(const RID &p_entity, const AABB &p_bounds);
	void remove_gizmo(const RID &p_entity);
	void clear_gizmos();
	bool has_gizmo(const RID &p_entity) const;

	void set_gizmo_color(const RID &p_entity, const Color &p_color);
	void set_gizmo_show_bounds(const RID &p_entity, bool p_show);
	void set_gizmo_show_axes(const RID &p_entity, bool p_show);

	// ========================================================================
	// Update Loop
	// ========================================================================

	void process(double p_delta);
	void set_processing_enabled(bool p_enabled);
	bool is_processing_enabled() const { return processing_enabled; }

	// ========================================================================
	// Input Handling
	// ========================================================================

	void handle_input(const Ref<InputEvent> &p_event);

	// ========================================================================
	// Statistics
	// ========================================================================

	Dictionary get_statistics() const;
	String get_debug_string() const;

	int get_tracked_entity_count() const { return entities_tracked; }
	int get_picks_this_session() const { return picks_this_session; }
	int get_selection_changes() const { return selection_changes; }

	// ========================================================================
	// Signals
	// ========================================================================
	// entity_selected(entity_rid, world_rid)
	// entity_deselected(entity_rid, world_rid)
	// selection_changed(event_dict)
	// entity_picked(entity_rid, world_rid, position)
	// panel_visibility_changed(visible)

	// ========================================================================
	// Constructors
	// ========================================================================

	TurboDebugSystem();
	~TurboDebugSystem();
};

#endif // TURBO_DEBUG_SYSTEM_H