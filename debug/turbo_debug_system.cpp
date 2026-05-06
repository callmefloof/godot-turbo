/**************************************************************************/
/*  turbo_debug_system.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#include "turbo_debug_system.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "core/input/input_event.h"
#include "core/input/input_map.h"
#include "core/os/memory.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"

#include "../ecs/flecs_types/flecs_server.h"

TurboDebugSystem *TurboDebugSystem::singleton = nullptr;

// ============================================================================
// TurboDebugSystem Implementation
// ============================================================================

void TurboDebugSystem::_bind_methods() {
	// Initialization
	ClassDB::bind_method(D_METHOD("initialize", "scene_tree"), &TurboDebugSystem::initialize);
	ClassDB::bind_method(D_METHOD("initialize_with_viewport", "viewport"), &TurboDebugSystem::initialize_with_viewport);
	ClassDB::bind_method(D_METHOD("shutdown"), &TurboDebugSystem::shutdown);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TurboDebugSystem::is_initialized);

	// Configuration
	ClassDB::bind_method(D_METHOD("set_config_from_dict", "config"), &TurboDebugSystem::set_config_from_dict);
	ClassDB::bind_method(D_METHOD("get_config_dict"), &TurboDebugSystem::get_config_dict);
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &TurboDebugSystem::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &TurboDebugSystem::is_enabled);
	ClassDB::bind_method(D_METHOD("set_picking_enabled", "enabled"), &TurboDebugSystem::set_picking_enabled);
	ClassDB::bind_method(D_METHOD("is_picking_enabled"), &TurboDebugSystem::is_picking_enabled);
	ClassDB::bind_method(D_METHOD("set_gizmos_enabled", "enabled"), &TurboDebugSystem::set_gizmos_enabled);
	ClassDB::bind_method(D_METHOD("is_gizmos_enabled"), &TurboDebugSystem::is_gizmos_enabled);
	ClassDB::bind_method(D_METHOD("set_panel_enabled", "enabled"), &TurboDebugSystem::set_panel_enabled);
	ClassDB::bind_method(D_METHOD("is_panel_enabled"), &TurboDebugSystem::is_panel_enabled);

	// Camera & Viewport
	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &TurboDebugSystem::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &TurboDebugSystem::get_camera);
	ClassDB::bind_method(D_METHOD("set_viewport", "viewport"), &TurboDebugSystem::set_viewport);
	ClassDB::bind_method(D_METHOD("get_viewport"), &TurboDebugSystem::get_viewport);

	// Selection
	ClassDB::bind_method(D_METHOD("select_entity", "entity", "world"), &TurboDebugSystem::select_entity);
	ClassDB::bind_method(D_METHOD("clear_selection"), &TurboDebugSystem::clear_selection);
	ClassDB::bind_method(D_METHOD("has_selection"), &TurboDebugSystem::has_selection);
	ClassDB::bind_method(D_METHOD("get_selected_entity"), &TurboDebugSystem::get_selected_entity);
	ClassDB::bind_method(D_METHOD("get_selected_world"), &TurboDebugSystem::get_selected_world);
	ClassDB::bind_method(D_METHOD("get_selection_position"), &TurboDebugSystem::get_selection_position);
	ClassDB::bind_method(D_METHOD("get_selected_entity_info"), &TurboDebugSystem::get_selected_entity_info);
	ClassDB::bind_method(D_METHOD("get_selected_entity_components"), &TurboDebugSystem::get_selected_entity_components);

	// Picking
	ClassDB::bind_method(D_METHOD("pick_at_mouse"), &TurboDebugSystem::pick_at_mouse);
	ClassDB::bind_method(D_METHOD("pick_and_select_at_mouse"), &TurboDebugSystem::pick_and_select_at_mouse);
	ClassDB::bind_method(D_METHOD("pick_at_position", "screen_pos"), &TurboDebugSystem::pick_at_position);
	ClassDB::bind_method(D_METHOD("pick_and_select_at_position", "screen_pos"), &TurboDebugSystem::pick_and_select_at_position);
	ClassDB::bind_method(D_METHOD("pick_ray", "from", "direction", "max_distance"), &TurboDebugSystem::pick_ray, DEFVAL(1000.0f));

	// Entity Tracking
	ClassDB::bind_method(D_METHOD("track_entity", "entity", "world"), &TurboDebugSystem::track_entity);
	ClassDB::bind_method(D_METHOD("untrack_entity", "entity"), &TurboDebugSystem::untrack_entity);
	ClassDB::bind_method(D_METHOD("clear_tracked_entities"), &TurboDebugSystem::clear_tracked_entities);
	ClassDB::bind_method(D_METHOD("is_entity_tracked", "entity"), &TurboDebugSystem::is_entity_tracked);
	ClassDB::bind_method(D_METHOD("get_tracked_entities"), &TurboDebugSystem::get_tracked_entities);
	ClassDB::bind_method(D_METHOD("update_tracked_entity", "entity", "transform", "bounds"), &TurboDebugSystem::update_tracked_entity);

	// Panel
	ClassDB::bind_method(D_METHOD("show_panel"), &TurboDebugSystem::show_panel);
	ClassDB::bind_method(D_METHOD("hide_panel"), &TurboDebugSystem::hide_panel);
	ClassDB::bind_method(D_METHOD("toggle_panel"), &TurboDebugSystem::toggle_panel);
	ClassDB::bind_method(D_METHOD("is_panel_visible"), &TurboDebugSystem::is_panel_visible);
	ClassDB::bind_method(D_METHOD("get_panel"), &TurboDebugSystem::get_panel);

	ClassDB::bind_method(D_METHOD("register_visualization_toggle", "name", "tooltip", "default_value", "category"), &TurboDebugSystem::register_visualization_toggle, DEFVAL("General"));
	ClassDB::bind_method(D_METHOD("set_visualization_toggle", "name", "enabled"), &TurboDebugSystem::set_visualization_toggle);
	ClassDB::bind_method(D_METHOD("get_visualization_toggle", "name"), &TurboDebugSystem::get_visualization_toggle);

	ClassDB::bind_method(D_METHOD("log", "message"), &TurboDebugSystem::log);
	ClassDB::bind_method(D_METHOD("log_warning", "message"), &TurboDebugSystem::log_warning);
	ClassDB::bind_method(D_METHOD("log_error", "message"), &TurboDebugSystem::log_error);

	// Debug Draw
	ClassDB::bind_method(D_METHOD("get_debug_draw"), &TurboDebugSystem::get_debug_draw);
	ClassDB::bind_method(D_METHOD("get_entity_picker"), &TurboDebugSystem::get_entity_picker);

	ClassDB::bind_method(D_METHOD("draw_line", "from", "to", "color"), &TurboDebugSystem::draw_line);
	ClassDB::bind_method(D_METHOD("draw_box", "box", "color", "filled"), &TurboDebugSystem::draw_box, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("draw_sphere", "center", "radius", "color"), &TurboDebugSystem::draw_sphere);
	ClassDB::bind_method(D_METHOD("draw_transform", "transform", "size"), &TurboDebugSystem::draw_transform, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("draw_arrow", "from", "to", "color"), &TurboDebugSystem::draw_arrow);
	ClassDB::bind_method(D_METHOD("clear_debug_draws"), &TurboDebugSystem::clear_debug_draws);

	// Gizmos
	ClassDB::bind_method(D_METHOD("add_gizmo", "entity", "transform", "bounds"), &TurboDebugSystem::add_gizmo);
	ClassDB::bind_method(D_METHOD("update_gizmo", "entity", "transform"), &TurboDebugSystem::update_gizmo);
	ClassDB::bind_method(D_METHOD("update_gizmo_bounds", "entity", "bounds"), &TurboDebugSystem::update_gizmo_bounds);
	ClassDB::bind_method(D_METHOD("remove_gizmo", "entity"), &TurboDebugSystem::remove_gizmo);
	ClassDB::bind_method(D_METHOD("clear_gizmos"), &TurboDebugSystem::clear_gizmos);
	ClassDB::bind_method(D_METHOD("has_gizmo", "entity"), &TurboDebugSystem::has_gizmo);
	ClassDB::bind_method(D_METHOD("set_gizmo_color", "entity", "color"), &TurboDebugSystem::set_gizmo_color);
	ClassDB::bind_method(D_METHOD("set_gizmo_show_bounds", "entity", "show"), &TurboDebugSystem::set_gizmo_show_bounds);
	ClassDB::bind_method(D_METHOD("set_gizmo_show_axes", "entity", "show"), &TurboDebugSystem::set_gizmo_show_axes);

	// Update
	ClassDB::bind_method(D_METHOD("process", "delta"), &TurboDebugSystem::process);
	ClassDB::bind_method(D_METHOD("set_processing_enabled", "enabled"), &TurboDebugSystem::set_processing_enabled);
	ClassDB::bind_method(D_METHOD("is_processing_enabled"), &TurboDebugSystem::is_processing_enabled);

	// Statistics
	ClassDB::bind_method(D_METHOD("get_statistics"), &TurboDebugSystem::get_statistics);
	ClassDB::bind_method(D_METHOD("get_debug_string"), &TurboDebugSystem::get_debug_string);
	ClassDB::bind_method(D_METHOD("get_tracked_entity_count"), &TurboDebugSystem::get_tracked_entity_count);

	// Signals
	ADD_SIGNAL(MethodInfo("entity_selected", PropertyInfo(Variant::RID, "entity_rid"), PropertyInfo(Variant::RID, "world_rid")));
	ADD_SIGNAL(MethodInfo("entity_deselected", PropertyInfo(Variant::RID, "entity_rid"), PropertyInfo(Variant::RID, "world_rid")));
	ADD_SIGNAL(MethodInfo("selection_changed", PropertyInfo(Variant::DICTIONARY, "event")));
	ADD_SIGNAL(MethodInfo("entity_picked", PropertyInfo(Variant::RID, "entity_rid"), PropertyInfo(Variant::RID, "world_rid"), PropertyInfo(Variant::VECTOR3, "position")));
	ADD_SIGNAL(MethodInfo("panel_visibility_changed", PropertyInfo(Variant::BOOL, "visible")));
}

TurboDebugSystem::TurboDebugSystem() {
	singleton = this;
}

TurboDebugSystem::~TurboDebugSystem() {
	shutdown();
	if (singleton == this) {
		singleton = nullptr;
	}
}

// ============================================================================
// Initialization
// ============================================================================

void TurboDebugSystem::initialize(SceneTree *p_scene_tree) {
	if (initialized) {
		return;
	}

	scene_tree = p_scene_tree;

	if (scene_tree) {
		Window *root = scene_tree->get_root();
		if (root) {
			current_viewport = root;
		}
	}

	// Get FlecsServer
	flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));

	// Create subsystems
	debug_draw.instantiate();
	entity_picker.instantiate();
	debug_panel = memnew(TurboDebugPanel);

	// Initialize subsystems
	entity_picker->initialize();
	entity_picker->set_viewport(current_viewport);

	// Create visualization root
	if (config.auto_initialize) {
		_create_visualization_root();
	}

	// Setup input actions
	if (config.enable_hotkeys) {
		_setup_input_actions();
	}

	// Auto-find camera
	if (config.auto_find_camera) {
		_auto_find_camera();
	}

	// Register default toggles
	_register_default_toggles();

	initialized = true;
}

void TurboDebugSystem::initialize_with_viewport(Viewport *p_viewport) {
	if (initialized) {
		return;
	}

	current_viewport = p_viewport;

	// Get scene tree from viewport if possible
	if (p_viewport) {
		scene_tree = p_viewport->get_tree();
	}

	// Get FlecsServer
	flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));

	// Create subsystems
	debug_draw.instantiate();
	entity_picker.instantiate();
	debug_panel = memnew(TurboDebugPanel);

	// Initialize subsystems
	entity_picker->initialize();
	entity_picker->set_viewport(current_viewport);

	// Create visualization root
	if (config.auto_initialize) {
		_create_visualization_root();
	}

	// Setup input actions
	if (config.enable_hotkeys) {
		_setup_input_actions();
	}

	// Auto-find camera
	if (config.auto_find_camera) {
		_auto_find_camera();
	}

	// Register default toggles
	_register_default_toggles();

	initialized = true;
}

void TurboDebugSystem::shutdown() {
	if (!initialized) {
		return;
	}

	// Cleanup input actions
	_cleanup_input_actions();

	// Shutdown subsystems
	if (debug_draw.is_valid()) {
		debug_draw->shutdown();
		debug_draw.unref();
	}

	if (entity_picker.is_valid()) {
		entity_picker->shutdown();
		entity_picker.unref();
	}

	if (debug_panel) {
		debug_panel->shutdown();
		if (debug_panel->get_parent()) {
			debug_panel->get_parent()->remove_child(debug_panel);
		}
		memdelete(debug_panel);
		debug_panel = nullptr;
	}

	// Cleanup visualization root
	_destroy_visualization_root();

	tracked_entities.clear();
	current_camera = nullptr;
	current_viewport = nullptr;
	scene_tree = nullptr;
	flecs_server = nullptr;

	initialized = false;
}

void TurboDebugSystem::set_flecs_server(FlecsServer *p_server) {
	flecs_server = p_server;
}

// ============================================================================
// Configuration
// ============================================================================

void TurboDebugSystem::set_config(const TurboDebugSystemConfig &p_config) {
	config = p_config;

	// Apply config to subsystems
	if (debug_draw.is_valid()) {
		debug_draw->set_enabled(config.enabled && config.enable_gizmos);
		debug_draw->set_depth_test(config.default_depth_test);
	}
}

void TurboDebugSystem::set_config_from_dict(const Dictionary &p_dict) {
	set_config(TurboDebugSystemConfig::from_dictionary(p_dict));
}

void TurboDebugSystem::set_enabled(bool p_enabled) {
	config.enabled = p_enabled;

	if (debug_draw.is_valid()) {
		debug_draw->set_enabled(p_enabled && config.enable_gizmos);
	}
}

void TurboDebugSystem::set_picking_enabled(bool p_enabled) {
	config.enable_picking = p_enabled;
}

void TurboDebugSystem::set_gizmos_enabled(bool p_enabled) {
	config.enable_gizmos = p_enabled;

	if (debug_draw.is_valid()) {
		debug_draw->set_enabled(config.enabled && p_enabled);
	}
}

void TurboDebugSystem::set_panel_enabled(bool p_enabled) {
	config.enable_panel = p_enabled;

	if (debug_panel) {
		if (p_enabled) {
			debug_panel->show_panel();
		} else {
			debug_panel->hide_panel();
		}
	}
}

// ============================================================================
// Camera & Viewport
// ============================================================================

void TurboDebugSystem::set_camera(Camera3D *p_camera) {
	current_camera = p_camera;

	if (debug_draw.is_valid()) {
		debug_draw->set_camera(p_camera);
	}

	if (entity_picker.is_valid()) {
		entity_picker->set_camera(p_camera);
	}

	_on_camera_changed();
}

void TurboDebugSystem::set_viewport(Viewport *p_viewport) {
	current_viewport = p_viewport;

	if (entity_picker.is_valid()) {
		entity_picker->set_viewport(p_viewport);
	}
}

void TurboDebugSystem::set_visualization_root(Node3D *p_root) {
	visualization_root = p_root;

	if (debug_draw.is_valid() && visualization_root) {
		RID scenario;
		if (current_viewport) {
			scenario = current_viewport->find_world_3d()->get_scenario();
		}
		debug_draw->initialize(visualization_root, scenario);
	}
}

void TurboDebugSystem::_auto_find_camera() {
	if (!current_viewport) {
		return;
	}

	// Try to find current camera from viewport
	Camera3D *camera = current_viewport->get_camera_3d();
	if (camera) {
		set_camera(camera);
	}
}

void TurboDebugSystem::_on_camera_changed() {
	// Update subsystems with new camera
}

void TurboDebugSystem::_create_visualization_root() {
	if (visualization_root) {
		return;
	}

	visualization_root = memnew(Node3D);
	visualization_root->set_name("TurboDebugVisualization");

	// Add to scene tree if available
	if (scene_tree) {
		Window *root = scene_tree->get_root();
		if (root) {
			root->call_deferred("add_child", visualization_root);
		}
	}

	// Initialize debug draw with visualization root
	if (debug_draw.is_valid()) {
		RID scenario;
		if (current_viewport) {
			Ref<World3D> world = current_viewport->find_world_3d();
			if (world.is_valid()) {
				scenario = world->get_scenario();
			}
		}
		debug_draw->initialize(visualization_root, scenario);
	}

	// Add debug panel to tree
	if (debug_panel && scene_tree) {
		Window *root = scene_tree->get_root();
		if (root) {
			root->call_deferred("add_child", debug_panel);
			debug_panel->initialize(this);
		}
	}
}

void TurboDebugSystem::_destroy_visualization_root() {
	if (visualization_root) {
		if (visualization_root->get_parent()) {
			visualization_root->get_parent()->remove_child(visualization_root);
		}
		memdelete(visualization_root);
		visualization_root = nullptr;
	}
}

// ============================================================================
// Selection
// ============================================================================

void TurboDebugSystem::select_entity(const RID &p_entity, const RID &p_world) {
	_perform_selection(p_entity, p_world, Vector3(), false);
}

void TurboDebugSystem::clear_selection() {
	if (!selected_entity.is_valid()) {
		return;
	}

	RID prev_entity = selected_entity;
	RID prev_world = selected_world;

	_clear_selection_gizmo();

	selected_entity = RID();
	selected_world = RID();
	selection_position = Vector3();

	emit_signal("entity_deselected", prev_entity, prev_world);

	TurboSelectionEvent event;
	event.previous_entity = prev_entity;
	event.previous_world = prev_world;
	event.new_entity = RID();
	event.new_world = RID();
	event.was_user_initiated = false;
	emit_signal("selection_changed", event.to_dictionary());

	if (debug_panel) {
		debug_panel->clear_selected_entity();
	}

	selection_changes++;
}

void TurboDebugSystem::_perform_selection(const RID &p_entity, const RID &p_world, const Vector3 &p_position, bool p_user_initiated) {
	RID prev_entity = selected_entity;
	RID prev_world = selected_world;

	// Clear previous selection gizmo
	if (prev_entity.is_valid()) {
		_clear_selection_gizmo();
	}

	selected_entity = p_entity;
	selected_world = p_world;
	selection_position = p_position;

	// Update selection gizmo
	_update_selection_gizmo();

	// Emit signals
	if (prev_entity.is_valid()) {
		emit_signal("entity_deselected", prev_entity, prev_world);
	}

	emit_signal("entity_selected", selected_entity, selected_world);

	TurboSelectionEvent event;
	event.previous_entity = prev_entity;
	event.previous_world = prev_world;
	event.new_entity = selected_entity;
	event.new_world = selected_world;
	event.pick_position = p_position;
	event.was_user_initiated = p_user_initiated;
	emit_signal("selection_changed", event.to_dictionary());

	// Update debug panel
	if (debug_panel) {
		debug_panel->set_selected_entity(selected_entity, selected_world);
	}

	selection_changes++;
}

void TurboDebugSystem::_update_selection_gizmo() {
	if (!debug_draw.is_valid() || !selected_entity.is_valid()) {
		return;
	}

	debug_draw->set_selected_entity(selected_entity);
}

void TurboDebugSystem::_clear_selection_gizmo() {
	if (debug_draw.is_valid()) {
		debug_draw->clear_selection();
	}
}

Dictionary TurboDebugSystem::get_selected_entity_info() const {
	Dictionary result;

	if (!flecs_server || !selected_entity.is_valid()) {
		return result;
	}

	result["rid"] = selected_entity;
	result["world"] = selected_world;
	result["name"] = flecs_server->get_entity_name(selected_entity);
	result["position"] = selection_position;

	return result;
}

TypedArray<Dictionary> TurboDebugSystem::get_selected_entity_components() const {
	TypedArray<Dictionary> result;

	if (!flecs_server || !selected_entity.is_valid()) {
		return result;
	}

	PackedStringArray comp_names = flecs_server->get_component_types_as_name(selected_entity);
	for (int i = 0; i < comp_names.size(); i++) {
		Dictionary comp_info;
		comp_info["name"] = comp_names[i];
		comp_info["data"] = flecs_server->get_component_by_name(selected_entity, comp_names[i]);
		result.push_back(comp_info);
	}

	return result;
}

// ============================================================================
// Picking
// ============================================================================

RID TurboDebugSystem::pick_at_mouse() {
	if (!entity_picker.is_valid() || !config.enable_picking) {
		return RID();
	}

	Vector2 mouse_pos = Input::get_singleton()->get_mouse_position();
	return entity_picker->pick_at_screen_position(mouse_pos);
}

RID TurboDebugSystem::pick_and_select_at_mouse() {
	RID picked = pick_at_mouse();

	if (picked.is_valid()) {
		// Get world RID from tracker if available
		RID world = tracked_entities.has(picked) ? tracked_entities[picked] : RID();

		// Get hit position from picker
		TypedArray<Dictionary> results = entity_picker->get_last_pick_results();
		Vector3 hit_pos;
		if (results.size() > 0) {
			Dictionary first = results[0];
			hit_pos = first.get("hit_position", Vector3());
		}

		_perform_selection(picked, world, hit_pos, true);
		emit_signal("entity_picked", picked, world, hit_pos);
		picks_this_session++;
	}

	return picked;
}

RID TurboDebugSystem::pick_at_position(const Vector2 &p_screen_pos) {
	if (!entity_picker.is_valid() || !config.enable_picking) {
		return RID();
	}

	return entity_picker->pick_at_screen_position(p_screen_pos);
}

RID TurboDebugSystem::pick_and_select_at_position(const Vector2 &p_screen_pos) {
	RID picked = pick_at_position(p_screen_pos);

	if (picked.is_valid()) {
		RID world = tracked_entities.has(picked) ? tracked_entities[picked] : RID();

		TypedArray<Dictionary> results = entity_picker->get_last_pick_results();
		Vector3 hit_pos;
		if (results.size() > 0) {
			Dictionary first = results[0];
			hit_pos = first.get("hit_position", Vector3());
		}

		_perform_selection(picked, world, hit_pos, true);
		emit_signal("entity_picked", picked, world, hit_pos);
		picks_this_session++;
	}

	return picked;
}

RID TurboDebugSystem::pick_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_max_distance) {
	if (!entity_picker.is_valid() || !config.enable_picking) {
		return RID();
	}

	return entity_picker->pick_ray(p_from, p_direction, p_max_distance);
}

RID TurboDebugSystem::pick_with_config(const Vector2 &p_screen_pos, const Dictionary &p_config) {
	if (!entity_picker.is_valid() || !config.enable_picking) {
		return RID();
	}

	TurboPickConfig pick_config = TurboPickConfig::from_dictionary(p_config);
	return entity_picker->pick_at_screen_position_with_config(p_screen_pos, pick_config);
}

TypedArray<RID> TurboDebugSystem::pick_all_with_config(const Vector2 &p_screen_pos, const Dictionary &p_config) {
	if (!entity_picker.is_valid() || !config.enable_picking) {
		return TypedArray<RID>();
	}

	TurboPickConfig pick_config = TurboPickConfig::from_dictionary(p_config);
	// This would need additional implementation in entity_picker
	return entity_picker->pick_all_at_screen_position(p_screen_pos, pick_config.max_results);
}

// ============================================================================
// Entity Tracking
// ============================================================================

void TurboDebugSystem::track_entity(const RID &p_entity, const RID &p_world) {
	tracked_entities[p_entity] = p_world;
	entities_tracked = tracked_entities.size();

	// Register for picking
	if (entity_picker.is_valid()) {
		// Default bounds - should be updated via update_tracked_entity
		entity_picker->register_pickable(p_entity, p_world, AABB(Vector3(-0.5f, -0.5f, -0.5f), Vector3(1, 1, 1)), Transform3D());
	}
}

void TurboDebugSystem::untrack_entity(const RID &p_entity) {
	tracked_entities.erase(p_entity);
	entities_tracked = tracked_entities.size();

	if (entity_picker.is_valid()) {
		entity_picker->unregister_pickable(p_entity);
	}

	if (debug_draw.is_valid()) {
		debug_draw->remove_entity_gizmo(p_entity);
	}

	// Clear selection if this was the selected entity
	if (selected_entity == p_entity) {
		clear_selection();
	}
}

void TurboDebugSystem::clear_tracked_entities() {
	tracked_entities.clear();
	entities_tracked = 0;

	if (entity_picker.is_valid()) {
		entity_picker->clear_pickables();
	}

	if (debug_draw.is_valid()) {
		debug_draw->clear_entity_gizmos();
	}
}

bool TurboDebugSystem::is_entity_tracked(const RID &p_entity) const {
	return tracked_entities.has(p_entity);
}

TypedArray<RID> TurboDebugSystem::get_tracked_entities() const {
	TypedArray<RID> result;
	for (const KeyValue<RID, RID> &kv : tracked_entities) {
		result.push_back(kv.key);
	}
	return result;
}

void TurboDebugSystem::update_tracked_entity(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds) {
	if (!tracked_entities.has(p_entity)) {
		return;
	}

	if (entity_picker.is_valid()) {
		entity_picker->update_pickable(p_entity, p_bounds, p_transform);
	}

	if (debug_draw.is_valid()) {
		if (debug_draw->has_entity_gizmo(p_entity)) {
			debug_draw->update_entity_gizmo(p_entity, p_transform);
			debug_draw->update_entity_gizmo_bounds(p_entity, p_bounds);
		} else if (config.enable_gizmos) {
			debug_draw->add_entity_gizmo(p_entity, p_transform, p_bounds);
		}
	}
}

// ============================================================================
// Debug Panel Access
// ============================================================================

void TurboDebugSystem::show_panel() {
	if (debug_panel && config.enable_panel) {
		debug_panel->show_panel();
		emit_signal("panel_visibility_changed", true);
	}
}

void TurboDebugSystem::hide_panel() {
	if (debug_panel) {
		debug_panel->hide_panel();
		emit_signal("panel_visibility_changed", false);
	}
}

void TurboDebugSystem::toggle_panel() {
	if (debug_panel) {
		debug_panel->toggle_panel();
		emit_signal("panel_visibility_changed", debug_panel->is_panel_visible());
	}
}

bool TurboDebugSystem::is_panel_visible() const {
	return debug_panel && debug_panel->is_panel_visible();
}

void TurboDebugSystem::register_visualization_toggle(const String &p_name, const String &p_tooltip, bool p_default, const String &p_category) {
	if (debug_panel) {
		debug_panel->register_toggle(p_name, p_tooltip, p_default, p_category);
	}
}

void TurboDebugSystem::set_visualization_toggle(const String &p_name, bool p_enabled) {
	if (debug_panel) {
		debug_panel->set_toggle_enabled(p_name, p_enabled);
	}
}

bool TurboDebugSystem::get_visualization_toggle(const String &p_name) const {
	if (debug_panel) {
		return debug_panel->is_toggle_enabled(p_name);
	}
	return false;
}

void TurboDebugSystem::log(const String &p_message) {
	if (debug_panel) {
		debug_panel->log_message(p_message);
	}
}

void TurboDebugSystem::log_warning(const String &p_message) {
	if (debug_panel) {
		debug_panel->log_warning(p_message);
	}
}

void TurboDebugSystem::log_error(const String &p_message) {
	if (debug_panel) {
		debug_panel->log_error(p_message);
	}
}

// ============================================================================
// Debug Draw Wrappers
// ============================================================================

void TurboDebugSystem::draw_line(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	if (debug_draw.is_valid()) {
		debug_draw->draw_line(p_from, p_to, p_color);
	}
}

void TurboDebugSystem::draw_box(const AABB &p_box, const Color &p_color, bool p_filled) {
	if (debug_draw.is_valid()) {
		debug_draw->draw_box(p_box, p_color, p_filled);
	}
}

void TurboDebugSystem::draw_sphere(const Vector3 &p_center, float p_radius, const Color &p_color) {
	if (debug_draw.is_valid()) {
		debug_draw->draw_sphere(p_center, p_radius, p_color);
	}
}

void TurboDebugSystem::draw_transform(const Transform3D &p_transform, float p_size) {
	if (debug_draw.is_valid()) {
		debug_draw->draw_transform(p_transform, p_size);
	}
}

void TurboDebugSystem::draw_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	if (debug_draw.is_valid()) {
		debug_draw->draw_arrow(p_from, p_to, p_color);
	}
}

void TurboDebugSystem::clear_debug_draws() {
	if (debug_draw.is_valid()) {
		debug_draw->clear_draws();
	}
}

// ============================================================================
// Gizmo Management
// ============================================================================

void TurboDebugSystem::add_gizmo(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds) {
	if (debug_draw.is_valid() && config.enable_gizmos) {
		debug_draw->add_entity_gizmo(p_entity, p_transform, p_bounds);
	}
}

void TurboDebugSystem::update_gizmo(const RID &p_entity, const Transform3D &p_transform) {
	if (debug_draw.is_valid()) {
		debug_draw->update_entity_gizmo(p_entity, p_transform);
	}
}

void TurboDebugSystem::update_gizmo_bounds(const RID &p_entity, const AABB &p_bounds) {
	if (debug_draw.is_valid()) {
		debug_draw->update_entity_gizmo_bounds(p_entity, p_bounds);
	}
}

void TurboDebugSystem::remove_gizmo(const RID &p_entity) {
	if (debug_draw.is_valid()) {
		debug_draw->remove_entity_gizmo(p_entity);
	}
}

void TurboDebugSystem::clear_gizmos() {
	if (debug_draw.is_valid()) {
		debug_draw->clear_entity_gizmos();
	}
}

bool TurboDebugSystem::has_gizmo(const RID &p_entity) const {
	if (debug_draw.is_valid()) {
		return debug_draw->has_entity_gizmo(p_entity);
	}
	return false;
}

void TurboDebugSystem::set_gizmo_color(const RID &p_entity, const Color &p_color) {
	if (debug_draw.is_valid()) {
		debug_draw->set_entity_gizmo_color(p_entity, p_color);
	}
}

void TurboDebugSystem::set_gizmo_show_bounds(const RID &p_entity, bool p_show) {
	if (debug_draw.is_valid()) {
		debug_draw->set_entity_gizmo_show_bounds(p_entity, p_show);
	}
}

void TurboDebugSystem::set_gizmo_show_axes(const RID &p_entity, bool p_show) {
	if (debug_draw.is_valid()) {
		debug_draw->set_entity_gizmo_show_axes(p_entity, p_show);
	}
}

// ============================================================================
// Input Handling
// ============================================================================

void TurboDebugSystem::_setup_input_actions() {
	InputMap *input_map = InputMap::get_singleton();
	if (!input_map) {
		return;
	}

	// Add debug pick action if not exists
	if (!input_map->has_action(config.pick_action)) {
		input_map->add_action(config.pick_action);

		// Default: Ctrl+Click
		Ref<InputEventMouseButton> mb;
		mb.instantiate();
		mb->set_button_index(MouseButton::LEFT);
		mb->set_ctrl_pressed(true);
		input_map->action_add_event(config.pick_action, mb);
	}

	// Add toggle panel action
	if (!input_map->has_action(config.toggle_panel_action)) {
		input_map->add_action(config.toggle_panel_action);

		// Default: F3
		Ref<InputEventKey> key;
		key.instantiate();
		key->set_keycode(Key::F3);
		input_map->action_add_event(config.toggle_panel_action, key);
	}

	// Add toggle gizmos action
	if (!input_map->has_action(config.toggle_gizmos_action)) {
		input_map->add_action(config.toggle_gizmos_action);

		// Default: F4
		Ref<InputEventKey> key;
		key.instantiate();
		key->set_keycode(Key::F4);
		input_map->action_add_event(config.toggle_gizmos_action, key);
	}
}

void TurboDebugSystem::_cleanup_input_actions() {
	InputMap *input_map = InputMap::get_singleton();
	if (!input_map) {
		return;
	}

	// Remove our custom actions
	if (input_map->has_action(config.pick_action)) {
		input_map->erase_action(config.pick_action);
	}
	if (input_map->has_action(config.toggle_panel_action)) {
		input_map->erase_action(config.toggle_panel_action);
	}
	if (input_map->has_action(config.toggle_gizmos_action)) {
		input_map->erase_action(config.toggle_gizmos_action);
	}
}

void TurboDebugSystem::handle_input(const Ref<InputEvent> &p_event) {
	if (!config.enabled || !config.enable_hotkeys) {
		return;
	}

	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}

	// Check for pick action
	if (config.enable_picking && p_event->is_action_pressed(config.pick_action)) {
		pick_and_select_at_mouse();
	}

	// Check for toggle panel action
	if (config.enable_panel && p_event->is_action_pressed(config.toggle_panel_action)) {
		toggle_panel();
	}

	// Check for toggle gizmos action
	if (p_event->is_action_pressed(config.toggle_gizmos_action)) {
		set_gizmos_enabled(!config.enable_gizmos);
	}

	// Handle mouse button for picking
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == (MouseButton)config.pick_mouse_button) {
		if (mb->is_ctrl_pressed() && config.enable_picking) {
			pick_and_select_at_mouse();
		}
	}
}

void TurboDebugSystem::_handle_pick_input(const Vector2 &p_position) {
	if (!config.enable_picking) {
		return;
	}

	// Check cooldown
	if (current_time - last_pick_time < config.pick_cooldown) {
		return;
	}
	last_pick_time = current_time;

	pick_and_select_at_position(p_position);
}

// ============================================================================
// Default Toggles
// ============================================================================

void TurboDebugSystem::_register_default_toggles() {
	if (!debug_panel) {
		return;
	}

	// Gizmo toggles
	debug_panel->register_toggle_with_callback("Entity Bounds", "Show AABB bounds for entities", config.default_show_bounds,
			callable_mp(this, &TurboDebugSystem::_on_toggle_bounds), "Gizmos");

	debug_panel->register_toggle_with_callback("Entity Axes", "Show transform axes for entities", config.default_show_axes,
			callable_mp(this, &TurboDebugSystem::_on_toggle_axes), "Gizmos");

	debug_panel->register_toggle_with_callback("Selection Highlight", "Highlight selected entity", true,
			callable_mp(this, &TurboDebugSystem::_on_toggle_selection), "Gizmos");
}

void TurboDebugSystem::_on_toggle_bounds(bool p_enabled) {
	if (debug_draw.is_valid()) {
		TurboDebugDrawConfig draw_config = debug_draw->get_config();
		draw_config.draw_entity_bounds = p_enabled;
		debug_draw->set_config(draw_config);
	}
}

void TurboDebugSystem::_on_toggle_axes(bool p_enabled) {
	if (debug_draw.is_valid()) {
		TurboDebugDrawConfig draw_config = debug_draw->get_config();
		draw_config.draw_entity_gizmos = p_enabled;
		debug_draw->set_config(draw_config);
	}
}

void TurboDebugSystem::_on_toggle_selection(bool p_enabled) {
	if (debug_draw.is_valid()) {
		TurboDebugDrawConfig draw_config = debug_draw->get_config();
		draw_config.draw_selection_highlight = p_enabled;
		debug_draw->set_config(draw_config);
	}
}

// ============================================================================
// Update Loop
// ============================================================================

void TurboDebugSystem::process(double p_delta) {
	if (!initialized || !processing_enabled || !config.enabled) {
		return;
	}

	current_time += p_delta;
	current_frame++;

	// Update subsystems
	if (debug_draw.is_valid()) {
		debug_draw->update(p_delta);
	}

	if (entity_picker.is_valid()) {
		entity_picker->update(p_delta);
	}

	if (debug_panel) {
		debug_panel->update(p_delta);
	}

	// Auto-update camera if configured
	if (config.auto_find_camera && !current_camera) {
		_auto_find_camera();
	}
}

void TurboDebugSystem::set_processing_enabled(bool p_enabled) {
	processing_enabled = p_enabled;
}

// ============================================================================
// Statistics
// ============================================================================

Dictionary TurboDebugSystem::get_statistics() const {
	Dictionary stats;

	stats["initialized"] = initialized;
	stats["enabled"] = config.enabled;
	stats["tracked_entities"] = entities_tracked;
	stats["picks_this_session"] = picks_this_session;
	stats["selection_changes"] = selection_changes;
	stats["current_frame"] = current_frame;

	if (selected_entity.is_valid()) {
		stats["selected_entity"] = selected_entity;
		stats["selected_world"] = selected_world;
	}

	if (debug_draw.is_valid()) {
		stats["debug_draw"] = debug_draw->get_statistics();
	}

	if (entity_picker.is_valid()) {
		stats["entity_picker"] = entity_picker->get_statistics();
	}

	return stats;
}

String TurboDebugSystem::get_debug_string() const {
	return vformat("TurboDebugSystem: tracked=%d picks=%d selection_changes=%d enabled=%s",
			entities_tracked, picks_this_session, selection_changes, config.enabled ? "true" : "false");
}