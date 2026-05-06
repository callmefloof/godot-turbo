/**************************************************************************/
/*  turbo_debug_draw.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                          GODOT TURBO MODULE                            */
/**************************************************************************/

#include "turbo_debug_draw.h"

#include "core/config/engine.h"
#include "core/math/math_defs.h"
#include "core/math/math_funcs.h"
#include "scene/3d/camera_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/resources/material.h"

// ============================================================================
// TurboDebugDrawConfig
// ============================================================================

Dictionary TurboDebugDrawConfig::to_dictionary() const {
	Dictionary d;
	d["enabled"] = enabled;
	d["draw_entity_bounds"] = draw_entity_bounds;
	d["draw_entity_gizmos"] = draw_entity_gizmos;
	d["draw_selection_highlight"] = draw_selection_highlight;
	d["draw_custom_shapes"] = draw_custom_shapes;
	d["selection_color"] = selection_color;
	d["bounds_color"] = bounds_color;
	d["gizmo_x_color"] = gizmo_x_color;
	d["gizmo_y_color"] = gizmo_y_color;
	d["gizmo_z_color"] = gizmo_z_color;
	d["gizmo_axis_length"] = gizmo_axis_length;
	d["gizmo_line_width"] = gizmo_line_width;
	d["bounds_line_width"] = bounds_line_width;
	d["selection_pulse_speed"] = selection_pulse_speed;
	d["selection_pulse_amplitude"] = selection_pulse_amplitude;
	d["depth_test"] = depth_test;
	d["max_draw_distance"] = max_draw_distance;
	d["max_lines_per_frame"] = max_lines_per_frame;
	d["max_boxes_per_frame"] = max_boxes_per_frame;
	return d;
}

TurboDebugDrawConfig TurboDebugDrawConfig::from_dictionary(const Dictionary &p_dict) {
	TurboDebugDrawConfig cfg;
	cfg.enabled = p_dict.get("enabled", cfg.enabled);
	cfg.draw_entity_bounds = p_dict.get("draw_entity_bounds", cfg.draw_entity_bounds);
	cfg.draw_entity_gizmos = p_dict.get("draw_entity_gizmos", cfg.draw_entity_gizmos);
	cfg.draw_selection_highlight = p_dict.get("draw_selection_highlight", cfg.draw_selection_highlight);
	cfg.draw_custom_shapes = p_dict.get("draw_custom_shapes", cfg.draw_custom_shapes);
	cfg.selection_color = p_dict.get("selection_color", cfg.selection_color);
	cfg.bounds_color = p_dict.get("bounds_color", cfg.bounds_color);
	cfg.gizmo_x_color = p_dict.get("gizmo_x_color", cfg.gizmo_x_color);
	cfg.gizmo_y_color = p_dict.get("gizmo_y_color", cfg.gizmo_y_color);
	cfg.gizmo_z_color = p_dict.get("gizmo_z_color", cfg.gizmo_z_color);
	cfg.gizmo_axis_length = p_dict.get("gizmo_axis_length", cfg.gizmo_axis_length);
	cfg.gizmo_line_width = p_dict.get("gizmo_line_width", cfg.gizmo_line_width);
	cfg.bounds_line_width = p_dict.get("bounds_line_width", cfg.bounds_line_width);
	cfg.selection_pulse_speed = p_dict.get("selection_pulse_speed", cfg.selection_pulse_speed);
	cfg.selection_pulse_amplitude = p_dict.get("selection_pulse_amplitude", cfg.selection_pulse_amplitude);
	cfg.depth_test = p_dict.get("depth_test", cfg.depth_test);
	cfg.max_draw_distance = p_dict.get("max_draw_distance", cfg.max_draw_distance);
	cfg.max_lines_per_frame = p_dict.get("max_lines_per_frame", cfg.max_lines_per_frame);
	cfg.max_boxes_per_frame = p_dict.get("max_boxes_per_frame", cfg.max_boxes_per_frame);
	return cfg;
}

// ============================================================================
// TurboDebugDraw Implementation
// ============================================================================

void TurboDebugDraw::_bind_methods() {
	// Initialization
	ClassDB::bind_method(D_METHOD("initialize", "parent", "scenario"), &TurboDebugDraw::initialize);
	ClassDB::bind_method(D_METHOD("shutdown"), &TurboDebugDraw::shutdown);
	ClassDB::bind_method(D_METHOD("is_initialized"), &TurboDebugDraw::is_initialized);

	ClassDB::bind_method(D_METHOD("set_camera", "camera"), &TurboDebugDraw::set_camera);
	ClassDB::bind_method(D_METHOD("get_camera"), &TurboDebugDraw::get_camera);

	// Configuration
	ClassDB::bind_method(D_METHOD("set_config_from_dict", "config"), &TurboDebugDraw::set_config_from_dict);
	ClassDB::bind_method(D_METHOD("get_config_dict"), &TurboDebugDraw::get_config_dict);
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &TurboDebugDraw::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &TurboDebugDraw::is_enabled);
	ClassDB::bind_method(D_METHOD("set_depth_test", "depth_test"), &TurboDebugDraw::set_depth_test);
	ClassDB::bind_method(D_METHOD("is_depth_test_enabled"), &TurboDebugDraw::is_depth_test_enabled);

	// Selection
	ClassDB::bind_method(D_METHOD("set_selected_entity", "entity"), &TurboDebugDraw::set_selected_entity);
	ClassDB::bind_method(D_METHOD("get_selected_entity"), &TurboDebugDraw::get_selected_entity);
	ClassDB::bind_method(D_METHOD("clear_selection"), &TurboDebugDraw::clear_selection);

	// Entity Gizmo Management
	ClassDB::bind_method(D_METHOD("add_entity_gizmo", "entity", "transform", "bounds"), &TurboDebugDraw::add_entity_gizmo);
	ClassDB::bind_method(D_METHOD("update_entity_gizmo", "entity", "transform"), &TurboDebugDraw::update_entity_gizmo);
	ClassDB::bind_method(D_METHOD("update_entity_gizmo_bounds", "entity", "bounds"), &TurboDebugDraw::update_entity_gizmo_bounds);
	ClassDB::bind_method(D_METHOD("remove_entity_gizmo", "entity"), &TurboDebugDraw::remove_entity_gizmo);
	ClassDB::bind_method(D_METHOD("clear_entity_gizmos"), &TurboDebugDraw::clear_entity_gizmos);
	ClassDB::bind_method(D_METHOD("has_entity_gizmo", "entity"), &TurboDebugDraw::has_entity_gizmo);

	ClassDB::bind_method(D_METHOD("set_entity_gizmo_color", "entity", "color"), &TurboDebugDraw::set_entity_gizmo_color);
	ClassDB::bind_method(D_METHOD("clear_entity_gizmo_color", "entity"), &TurboDebugDraw::clear_entity_gizmo_color);
	ClassDB::bind_method(D_METHOD("set_entity_gizmo_show_bounds", "entity", "show"), &TurboDebugDraw::set_entity_gizmo_show_bounds);
	ClassDB::bind_method(D_METHOD("set_entity_gizmo_show_axes", "entity", "show"), &TurboDebugDraw::set_entity_gizmo_show_axes);

	// Immediate Draw Commands
	ClassDB::bind_method(D_METHOD("draw_line", "from", "to", "color", "width"), &TurboDebugDraw::draw_line, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("draw_box", "box", "color", "filled"), &TurboDebugDraw::draw_box, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("draw_box_transformed", "box", "transform", "color", "filled"), &TurboDebugDraw::draw_box_transformed, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("draw_sphere", "center", "radius", "color", "segments"), &TurboDebugDraw::draw_sphere, DEFVAL(16));
	ClassDB::bind_method(D_METHOD("draw_arrow", "from", "to", "color", "head_size"), &TurboDebugDraw::draw_arrow, DEFVAL(0.1f));
	ClassDB::bind_method(D_METHOD("draw_transform", "transform", "size"), &TurboDebugDraw::draw_transform, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("draw_ray", "from", "direction", "length", "color"), &TurboDebugDraw::draw_ray);
	ClassDB::bind_method(D_METHOD("draw_point", "position", "size", "color"), &TurboDebugDraw::draw_point);
	ClassDB::bind_method(D_METHOD("draw_lines", "points", "color"), &TurboDebugDraw::draw_lines);
	ClassDB::bind_method(D_METHOD("draw_polyline", "points", "color", "closed"), &TurboDebugDraw::draw_polyline, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("clear_draws"), &TurboDebugDraw::clear_draws);

	// Update
	ClassDB::bind_method(D_METHOD("update", "delta"), &TurboDebugDraw::update);
	ClassDB::bind_method(D_METHOD("force_rebuild"), &TurboDebugDraw::force_rebuild);

	// Statistics
	ClassDB::bind_method(D_METHOD("get_statistics"), &TurboDebugDraw::get_statistics);
	ClassDB::bind_method(D_METHOD("get_debug_string"), &TurboDebugDraw::get_debug_string);
	ClassDB::bind_method(D_METHOD("get_lines_drawn"), &TurboDebugDraw::get_lines_drawn);
	ClassDB::bind_method(D_METHOD("get_boxes_drawn"), &TurboDebugDraw::get_boxes_drawn);
	ClassDB::bind_method(D_METHOD("get_gizmos_drawn"), &TurboDebugDraw::get_gizmos_drawn);
}

TurboDebugDraw::TurboDebugDraw() {
}

TurboDebugDraw::~TurboDebugDraw() {
	shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

void TurboDebugDraw::initialize(Node3D *p_parent, const RID &p_scenario) {
	if (is_initialized()) {
		return;
	}

	parent_node = p_parent;
	scenario_rid = p_scenario;

	// Create immediate mesh
	debug_mesh.instantiate();

	// Create materials
	_create_materials();

	// Create mesh instance
	mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_mesh(debug_mesh);
	mesh_instance->set_material_override(line_material);
	mesh_instance->set_cast_shadows_setting(GeometryInstance3D::SHADOW_CASTING_SETTING_OFF);

	if (parent_node) {
		parent_node->add_child(mesh_instance);
	}
}

void TurboDebugDraw::shutdown() {
	if (mesh_instance) {
		if (mesh_instance->get_parent()) {
			mesh_instance->get_parent()->remove_child(mesh_instance);
		}
		memdelete(mesh_instance);
		mesh_instance = nullptr;
	}

	debug_mesh.unref();
	line_material.unref();
	line_material_no_depth.unref();
	solid_material.unref();

	parent_node = nullptr;
	current_camera = nullptr;

	clear_draws();
	clear_entity_gizmos();
}

void TurboDebugDraw::set_camera(Camera3D *p_camera) {
	current_camera = p_camera;
}

// ============================================================================
// Materials
// ============================================================================

void TurboDebugDraw::_create_materials() {
	// Line material with depth test
	line_material.instantiate();
	line_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	line_material->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	line_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	line_material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	line_material->set_depth_draw_mode(BaseMaterial3D::DEPTH_DRAW_ALWAYS);

	// Line material without depth test (draws on top)
	line_material_no_depth.instantiate();
	line_material_no_depth->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	line_material_no_depth->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	line_material_no_depth->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	line_material_no_depth->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
	line_material_no_depth->set_flag(BaseMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	line_material_no_depth->set_render_priority(100);

	// Solid material for filled shapes
	solid_material.instantiate();
	solid_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
	solid_material->set_flag(BaseMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	solid_material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
	solid_material->set_cull_mode(BaseMaterial3D::CULL_DISABLED);
}

// ============================================================================
// Configuration
// ============================================================================

void TurboDebugDraw::set_config(const TurboDebugDrawConfig &p_config) {
	config = p_config;

	// Update material based on depth test setting
	if (mesh_instance) {
		if (config.depth_test) {
			mesh_instance->set_material_override(line_material);
		} else {
			mesh_instance->set_material_override(line_material_no_depth);
		}
	}
}

void TurboDebugDraw::set_config_from_dict(const Dictionary &p_dict) {
	set_config(TurboDebugDrawConfig::from_dictionary(p_dict));
}

void TurboDebugDraw::set_enabled(bool p_enabled) {
	config.enabled = p_enabled;
	if (mesh_instance) {
		mesh_instance->set_visible(p_enabled);
	}
}

void TurboDebugDraw::set_depth_test(bool p_depth_test) {
	config.depth_test = p_depth_test;
	if (mesh_instance) {
		if (p_depth_test) {
			mesh_instance->set_material_override(line_material);
		} else {
			mesh_instance->set_material_override(line_material_no_depth);
		}
	}
}

// ============================================================================
// Selection
// ============================================================================

void TurboDebugDraw::set_selected_entity(const RID &p_entity) {
	if (selected_entity != p_entity) {
		// Update old selection
		if (selected_entity.is_valid() && entity_gizmos.has(selected_entity)) {
			entity_gizmos[selected_entity].is_selected = false;
		}

		selected_entity = p_entity;

		// Update new selection
		if (selected_entity.is_valid() && entity_gizmos.has(selected_entity)) {
			entity_gizmos[selected_entity].is_selected = true;
		}
	}
}

void TurboDebugDraw::clear_selection() {
	if (selected_entity.is_valid() && entity_gizmos.has(selected_entity)) {
		entity_gizmos[selected_entity].is_selected = false;
	}
	selected_entity = RID();
}

// ============================================================================
// Entity Gizmo Management
// ============================================================================

void TurboDebugDraw::add_entity_gizmo(const RID &p_entity, const Transform3D &p_transform, const AABB &p_bounds) {
	TurboEntityGizmo gizmo;
	gizmo.entity_rid = p_entity;
	gizmo.transform = p_transform;
	gizmo.bounds = p_bounds;
	gizmo.show_bounds = config.draw_entity_bounds;
	gizmo.show_axes = config.draw_entity_gizmos;
	gizmo.is_selected = (p_entity == selected_entity);

	entity_gizmos[p_entity] = gizmo;
}

void TurboDebugDraw::update_entity_gizmo(const RID &p_entity, const Transform3D &p_transform) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].transform = p_transform;
	}
}

void TurboDebugDraw::update_entity_gizmo_bounds(const RID &p_entity, const AABB &p_bounds) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].bounds = p_bounds;
	}
}

void TurboDebugDraw::remove_entity_gizmo(const RID &p_entity) {
	entity_gizmos.erase(p_entity);
}

void TurboDebugDraw::clear_entity_gizmos() {
	entity_gizmos.clear();
}

bool TurboDebugDraw::has_entity_gizmo(const RID &p_entity) const {
	return entity_gizmos.has(p_entity);
}

void TurboDebugDraw::set_entity_gizmo_color(const RID &p_entity, const Color &p_color) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].override_color = p_color;
		entity_gizmos[p_entity].use_override_color = true;
	}
}

void TurboDebugDraw::clear_entity_gizmo_color(const RID &p_entity) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].use_override_color = false;
	}
}

void TurboDebugDraw::set_entity_gizmo_show_bounds(const RID &p_entity, bool p_show) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].show_bounds = p_show;
	}
}

void TurboDebugDraw::set_entity_gizmo_show_axes(const RID &p_entity, bool p_show) {
	if (entity_gizmos.has(p_entity)) {
		entity_gizmos[p_entity].show_axes = p_show;
	}
}

// ============================================================================
// Immediate Draw Commands
// ============================================================================

void TurboDebugDraw::draw_line(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_width) {
	if (!config.draw_custom_shapes) {
		return;
	}
	if ((int)lines.size() >= config.max_lines_per_frame) {
		return;
	}
	lines.push_back(TurboDebugLine(p_from, p_to, p_color, p_width));
}

void TurboDebugDraw::draw_box(const AABB &p_box, const Color &p_color, bool p_filled) {
	if (!config.draw_custom_shapes) {
		return;
	}
	if ((int)boxes.size() >= config.max_boxes_per_frame) {
		return;
	}
	boxes.push_back(TurboDebugBox(p_box, p_color, p_filled));
}

void TurboDebugDraw::draw_box_transformed(const AABB &p_box, const Transform3D &p_transform, const Color &p_color, bool p_filled) {
	if (!config.draw_custom_shapes) {
		return;
	}
	if ((int)boxes.size() >= config.max_boxes_per_frame) {
		return;
	}
	boxes.push_back(TurboDebugBox(p_box, p_color, p_transform, p_filled));
}

void TurboDebugDraw::draw_sphere(const Vector3 &p_center, float p_radius, const Color &p_color, int p_segments) {
	if (!config.draw_custom_shapes) {
		return;
	}
	spheres.push_back(TurboDebugSphere(p_center, p_radius, p_color, p_segments));
}

void TurboDebugDraw::draw_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_head_size) {
	if (!config.draw_custom_shapes) {
		return;
	}
	arrows.push_back(TurboDebugArrow(p_from, p_to, p_color, p_head_size));
}

void TurboDebugDraw::draw_transform(const Transform3D &p_transform, float p_size) {
	if (!config.draw_custom_shapes) {
		return;
	}

	Vector3 origin = p_transform.origin;
	draw_line(origin, origin + p_transform.basis.get_column(0) * p_size, config.gizmo_x_color);
	draw_line(origin, origin + p_transform.basis.get_column(1) * p_size, config.gizmo_y_color);
	draw_line(origin, origin + p_transform.basis.get_column(2) * p_size, config.gizmo_z_color);
}

void TurboDebugDraw::draw_ray(const Vector3 &p_from, const Vector3 &p_direction, float p_length, const Color &p_color) {
	draw_arrow(p_from, p_from + p_direction.normalized() * p_length, p_color, 0.05f * p_length);
}

void TurboDebugDraw::draw_point(const Vector3 &p_position, float p_size, const Color &p_color) {
	// Draw a small cross at the point
	float half = p_size * 0.5f;
	draw_line(p_position - Vector3(half, 0, 0), p_position + Vector3(half, 0, 0), p_color);
	draw_line(p_position - Vector3(0, half, 0), p_position + Vector3(0, half, 0), p_color);
	draw_line(p_position - Vector3(0, 0, half), p_position + Vector3(0, 0, half), p_color);
}

void TurboDebugDraw::draw_lines(const PackedVector3Array &p_points, const Color &p_color) {
	for (int i = 0; i + 1 < p_points.size(); i += 2) {
		draw_line(p_points[i], p_points[i + 1], p_color);
	}
}

void TurboDebugDraw::draw_polyline(const PackedVector3Array &p_points, const Color &p_color, bool p_closed) {
	if (p_points.size() < 2) {
		return;
	}

	for (int i = 0; i < p_points.size() - 1; i++) {
		draw_line(p_points[i], p_points[i + 1], p_color);
	}

	if (p_closed && p_points.size() > 2) {
		draw_line(p_points[p_points.size() - 1], p_points[0], p_color);
	}
}

void TurboDebugDraw::clear_draws() {
	lines.clear();
	boxes.clear();
	spheres.clear();
	arrows.clear();
}

// ============================================================================
// Mesh Building
// ============================================================================

void TurboDebugDraw::_clear_mesh() {
	if (debug_mesh.is_valid()) {
		debug_mesh->clear_surfaces();
	}
}

void TurboDebugDraw::_rebuild_mesh() {
	if (!debug_mesh.is_valid()) {
		return;
	}

	_clear_mesh();

	lines_drawn = 0;
	boxes_drawn = 0;
	spheres_drawn = 0;
	gizmos_drawn = 0;

	if (!config.enabled) {
		return;
	}

	debug_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

	// Draw immediate lines
	for (const TurboDebugLine &line : lines) {
		_add_line_to_mesh(line.from, line.to, line.color);
	}

	// Draw boxes
	for (const TurboDebugBox &box : boxes) {
		if (box.filled) {
			// Filled boxes need separate handling
			// For now, just draw wireframe
			_add_box_wireframe(box.bounds, box.color);
		} else {
			if (box.transform != Transform3D()) {
				_add_box_wireframe_transformed(box.bounds, box.transform, box.color);
			} else {
				_add_box_wireframe(box.bounds, box.color);
			}
		}
		boxes_drawn++;
	}

	// Draw spheres
	for (const TurboDebugSphere &sphere : spheres) {
		_add_sphere_wireframe(sphere.center, sphere.radius, sphere.color, sphere.segments);
		spheres_drawn++;
	}

	// Draw arrows
	for (const TurboDebugArrow &arrow : arrows) {
		_add_arrow(arrow.from, arrow.to, arrow.color, arrow.head_size);
	}

	// Draw entity gizmos
	_draw_entity_gizmos();

	debug_mesh->surface_end();
}

void TurboDebugDraw::_add_line_to_mesh(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color) {
	debug_mesh->surface_set_color(p_color);
	debug_mesh->surface_add_vertex(p_from);
	debug_mesh->surface_set_color(p_color);
	debug_mesh->surface_add_vertex(p_to);
	lines_drawn++;
}

void TurboDebugDraw::_add_box_wireframe(const AABB &p_box, const Color &p_color) {
	Vector3 pos = p_box.position;
	Vector3 size = p_box.size;

	// Bottom face
	_add_line_to_mesh(pos, pos + Vector3(size.x, 0, 0), p_color);
	_add_line_to_mesh(pos + Vector3(size.x, 0, 0), pos + Vector3(size.x, 0, size.z), p_color);
	_add_line_to_mesh(pos + Vector3(size.x, 0, size.z), pos + Vector3(0, 0, size.z), p_color);
	_add_line_to_mesh(pos + Vector3(0, 0, size.z), pos, p_color);

	// Top face
	Vector3 top = pos + Vector3(0, size.y, 0);
	_add_line_to_mesh(top, top + Vector3(size.x, 0, 0), p_color);
	_add_line_to_mesh(top + Vector3(size.x, 0, 0), top + Vector3(size.x, 0, size.z), p_color);
	_add_line_to_mesh(top + Vector3(size.x, 0, size.z), top + Vector3(0, 0, size.z), p_color);
	_add_line_to_mesh(top + Vector3(0, 0, size.z), top, p_color);

	// Vertical edges
	_add_line_to_mesh(pos, top, p_color);
	_add_line_to_mesh(pos + Vector3(size.x, 0, 0), top + Vector3(size.x, 0, 0), p_color);
	_add_line_to_mesh(pos + Vector3(size.x, 0, size.z), top + Vector3(size.x, 0, size.z), p_color);
	_add_line_to_mesh(pos + Vector3(0, 0, size.z), top + Vector3(0, 0, size.z), p_color);
}

void TurboDebugDraw::_add_box_wireframe_transformed(const AABB &p_box, const Transform3D &p_transform, const Color &p_color) {
	Vector3 pos = p_box.position;
	Vector3 size = p_box.size;

	// 8 corners of the box
	Vector3 corners[8] = {
		pos,
		pos + Vector3(size.x, 0, 0),
		pos + Vector3(size.x, 0, size.z),
		pos + Vector3(0, 0, size.z),
		pos + Vector3(0, size.y, 0),
		pos + Vector3(size.x, size.y, 0),
		pos + Vector3(size.x, size.y, size.z),
		pos + Vector3(0, size.y, size.z)
	};

	// Transform corners
	for (int i = 0; i < 8; i++) {
		corners[i] = p_transform.xform(corners[i]);
	}

	// Bottom face
	_add_line_to_mesh(corners[0], corners[1], p_color);
	_add_line_to_mesh(corners[1], corners[2], p_color);
	_add_line_to_mesh(corners[2], corners[3], p_color);
	_add_line_to_mesh(corners[3], corners[0], p_color);

	// Top face
	_add_line_to_mesh(corners[4], corners[5], p_color);
	_add_line_to_mesh(corners[5], corners[6], p_color);
	_add_line_to_mesh(corners[6], corners[7], p_color);
	_add_line_to_mesh(corners[7], corners[4], p_color);

	// Vertical edges
	_add_line_to_mesh(corners[0], corners[4], p_color);
	_add_line_to_mesh(corners[1], corners[5], p_color);
	_add_line_to_mesh(corners[2], corners[6], p_color);
	_add_line_to_mesh(corners[3], corners[7], p_color);
}

void TurboDebugDraw::_add_sphere_wireframe(const Vector3 &p_center, float p_radius, const Color &p_color, int p_segments) {
	// Draw 3 circles (XY, XZ, YZ planes)
	for (int plane = 0; plane < 3; plane++) {
		for (int i = 0; i < p_segments; i++) {
			float angle1 = (float)i / p_segments * Math::TAU;
			float angle2 = (float)(i + 1) / p_segments * Math::TAU;

			Vector3 p1, p2;
			switch (plane) {
				case 0: // XY plane
					p1 = p_center + Vector3(Math::cos(angle1), Math::sin(angle1), 0) * p_radius;
					p2 = p_center + Vector3(Math::cos(angle2), Math::sin(angle2), 0) * p_radius;
					break;
				case 1: // XZ plane
					p1 = p_center + Vector3(Math::cos(angle1), 0, Math::sin(angle1)) * p_radius;
					p2 = p_center + Vector3(Math::cos(angle2), 0, Math::sin(angle2)) * p_radius;
					break;
				case 2: // YZ plane
					p1 = p_center + Vector3(0, Math::cos(angle1), Math::sin(angle1)) * p_radius;
					p2 = p_center + Vector3(0, Math::cos(angle2), Math::sin(angle2)) * p_radius;
					break;
			}

			_add_line_to_mesh(p1, p2, p_color);
		}
	}
}

void TurboDebugDraw::_add_arrow(const Vector3 &p_from, const Vector3 &p_to, const Color &p_color, float p_head_size) {
	// Main line
	_add_line_to_mesh(p_from, p_to, p_color);

	// Arrow head
	Vector3 dir = (p_to - p_from).normalized();
	float length = (p_to - p_from).length();
	float head_length = MIN(p_head_size, length * 0.3f);

	// Find perpendicular vectors
	Vector3 perp1, perp2;
	if (Math::abs(dir.y) < 0.9f) {
		perp1 = dir.cross(Vector3(0, 1, 0)).normalized();
	} else {
		perp1 = dir.cross(Vector3(1, 0, 0)).normalized();
	}
	perp2 = dir.cross(perp1).normalized();

	Vector3 head_base = p_to - dir * head_length;
	float head_radius = head_length * 0.4f;

	_add_line_to_mesh(p_to, head_base + perp1 * head_radius, p_color);
	_add_line_to_mesh(p_to, head_base - perp1 * head_radius, p_color);
	_add_line_to_mesh(p_to, head_base + perp2 * head_radius, p_color);
	_add_line_to_mesh(p_to, head_base - perp2 * head_radius, p_color);
}

void TurboDebugDraw::_add_transform_axes(const Transform3D &p_transform, float p_size) {
	Vector3 origin = p_transform.origin;
	_add_line_to_mesh(origin, origin + p_transform.basis.get_column(0) * p_size, config.gizmo_x_color);
	_add_line_to_mesh(origin, origin + p_transform.basis.get_column(1) * p_size, config.gizmo_y_color);
	_add_line_to_mesh(origin, origin + p_transform.basis.get_column(2) * p_size, config.gizmo_z_color);
}

// ============================================================================
// Entity Gizmo Drawing
// ============================================================================

void TurboDebugDraw::_draw_entity_gizmos() {
	if (!config.draw_entity_gizmos && !config.draw_entity_bounds) {
		return;
	}

	for (KeyValue<RID, TurboEntityGizmo> &kv : entity_gizmos) {
		TurboEntityGizmo &gizmo = kv.value;

		// Distance culling
		if (!_is_point_visible(gizmo.transform.origin)) {
			continue;
		}

		_draw_entity_gizmo(gizmo, 0.0);
		gizmos_drawn++;
	}
}

void TurboDebugDraw::_draw_entity_gizmo(TurboEntityGizmo &p_gizmo, double p_delta) {
	Color bounds_color = p_gizmo.use_override_color ? p_gizmo.override_color : config.bounds_color;

	// Apply selection highlighting
	if (p_gizmo.is_selected && config.draw_selection_highlight) {
		p_gizmo.pulse_phase += p_delta * config.selection_pulse_speed;
		if (p_gizmo.pulse_phase > Math::TAU) {
			p_gizmo.pulse_phase -= Math::TAU;
		}
		bounds_color = _apply_pulse_effect(config.selection_color, p_gizmo.pulse_phase);
	}

	// Draw bounds
	if (p_gizmo.show_bounds && config.draw_entity_bounds) {
		// Transform bounds to world space
		AABB world_bounds = p_gizmo.transform.xform(p_gizmo.bounds);
		_add_box_wireframe(world_bounds, bounds_color);
	}

	// Draw axes
	if (p_gizmo.show_axes && config.draw_entity_gizmos) {
		_add_transform_axes(p_gizmo.transform, config.gizmo_axis_length);
	}
}

// ============================================================================
// Culling & Utility
// ============================================================================

bool TurboDebugDraw::_is_point_visible(const Vector3 &p_point) const {
	if (!current_camera) {
		return true; // No camera = always visible
	}

	float distance = _get_distance_to_camera(p_point);
	return distance <= config.max_draw_distance;
}

float TurboDebugDraw::_get_distance_to_camera(const Vector3 &p_point) const {
	if (!current_camera) {
		return 0.0f;
	}
	return current_camera->get_global_position().distance_to(p_point);
}

Color TurboDebugDraw::_apply_pulse_effect(const Color &p_color, float p_phase) const {
	float pulse = (Math::sin(p_phase) + 1.0f) * 0.5f; // 0 to 1
	float brightness = 1.0f + pulse * config.selection_pulse_amplitude;

	Color result = p_color;
	result.r = MIN(result.r * brightness, 1.0f);
	result.g = MIN(result.g * brightness, 1.0f);
	result.b = MIN(result.b * brightness, 1.0f);
	return result;
}

// ============================================================================
// Update
// ============================================================================

void TurboDebugDraw::update(double p_delta) {
	current_time += p_delta;
	current_frame++;

	// Update gizmo animations
	for (KeyValue<RID, TurboEntityGizmo> &kv : entity_gizmos) {
		TurboEntityGizmo &gizmo = kv.value;
		if (gizmo.is_selected) {
			gizmo.pulse_phase += p_delta * config.selection_pulse_speed;
			if (gizmo.pulse_phase > Math::TAU) {
				gizmo.pulse_phase -= Math::TAU;
			}
		}
	}

	// Rebuild mesh
	_rebuild_mesh();

	// Clear immediate draws for next frame
	clear_draws();
}

void TurboDebugDraw::force_rebuild() {
	_rebuild_mesh();
}

// ============================================================================
// Statistics
// ============================================================================

Dictionary TurboDebugDraw::get_statistics() const {
	Dictionary stats;
	stats["lines_drawn"] = lines_drawn;
	stats["boxes_drawn"] = boxes_drawn;
	stats["spheres_drawn"] = spheres_drawn;
	stats["gizmos_drawn"] = gizmos_drawn;
	stats["entity_gizmos_registered"] = (int)entity_gizmos.size();
	stats["current_frame"] = current_frame;
	stats["enabled"] = config.enabled;
	return stats;
}

String TurboDebugDraw::get_debug_string() const {
	return vformat("TurboDebugDraw: lines=%d boxes=%d spheres=%d gizmos=%d",
			lines_drawn, boxes_drawn, spheres_drawn, gizmos_drawn);
}