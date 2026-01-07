#include "flecs_entity_inspector.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"
#include "scene/gui/tree.h"
#include "scene/gui/box_container.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/button.h"
#include "scene/gui/label.h"
#include "scene/gui/separator.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/check_box.h"
#include "scene/gui/color_picker.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"

void FlecsEntityInspector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_entity", "world_rid", "entity_id"), &FlecsEntityInspector::set_entity);
	ClassDB::bind_method(D_METHOD("set_entity_from_remote_data", "world_id", "entity_id", "components"), &FlecsEntityInspector::set_entity_from_remote_data);
	ClassDB::bind_method(D_METHOD("clear_inspector"), &FlecsEntityInspector::clear_inspector);
	ClassDB::bind_method(D_METHOD("refresh_entity"), &FlecsEntityInspector::refresh_entity);
}

void FlecsEntityInspector::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!flecs_server && Engine::get_singleton()->has_singleton("FlecsServer")) {
				flecs_server = Object::cast_to<FlecsServer>(
					Engine::get_singleton()->get_singleton_object("FlecsServer"));
			}
		} break;
	}
}

FlecsEntityInspector::FlecsEntityInspector() {
	set_name("FlecsEntityInspector");
	set_custom_minimum_size(Vector2(350, 0));

	// Try to get FlecsServer for local mode - it's OK if not available (remote mode)
	if (Engine::get_singleton()->has_singleton("FlecsServer")) {
		flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));
		if (!flecs_server) {
			WARN_PRINT("FlecsEntityInspector: Could not cast FlecsServer singleton - local mode unavailable");
		}
	} else {
		// FlecsServer not available - this is expected in remote debugging mode
		flecs_server = nullptr;
	}

	// Always create UI elements - they're needed for both local and remote modes
	// Main container to hold filter and scroll area
	main_container = memnew(VBoxContainer);
	main_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	add_child(main_container);

	// Component filter field
	component_filter = memnew(LineEdit);
	component_filter->set_placeholder("Filter components...");
	component_filter->set_clear_button_enabled(true);
	component_filter->set_custom_minimum_size(Vector2(0, 28));
	component_filter->connect("text_changed", callable_mp(this, &FlecsEntityInspector::_on_component_filter_changed));
	main_container->add_child(component_filter);

	// Scroll container for content
	scroll_container = memnew(ScrollContainer);
	scroll_container->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	scroll_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	main_container->add_child(scroll_container);

	// Content container
	content_container = memnew(VBoxContainer);
	content_container->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	content_container->add_theme_constant_override("separation", 8);
	scroll_container->add_child(content_container);

	// Initial empty state
	Label *empty = memnew(Label);
	empty->set_text("No entity selected");
	empty->add_theme_font_size_override("font_size", 11);
	content_container->add_child(empty);
}

FlecsEntityInspector::~FlecsEntityInspector() {
	component_data.clear();
	expanded_paths.clear();
	current_component_filter = "";
}

void FlecsEntityInspector::set_entity(RID p_world_rid, uint64_t p_entity_id) {
	print_line(vformat("FlecsEntityInspector::set_entity - world_rid=%s, entity_id=%s",
		String::num_int64(p_world_rid.get_id(), 16), String::num_int64(p_entity_id, 16)));
	
	current_world = p_world_rid;
	current_entity_id = p_entity_id;
	is_remote_mode = false;
	remote_components_data.clear();
	_rebuild_inspector();
	
	print_line("FlecsEntityInspector::set_entity - Inspector rebuilt");
}

void FlecsEntityInspector::set_entity_from_remote_data(uint64_t p_world_id, uint64_t p_entity_id, const Array &p_components) {
	print_line(vformat("FlecsEntityInspector::set_entity_from_remote_data - world_id=%s, entity_id=%s, components=%d",
		String::num_int64(p_world_id, 16), String::num_int64(p_entity_id, 16), p_components.size()));
	
	// Validate inputs
	if (p_entity_id == 0) {
		print_line("FlecsEntityInspector::set_entity_from_remote_data - WARNING: entity_id is 0");
	}
	
	current_world = RID::from_uint64(p_world_id);
	current_entity_id = p_entity_id;
	is_remote_mode = true;
	remote_components_data = p_components.duplicate();  // Make a copy to avoid issues with array lifetime
	_rebuild_inspector();
	
	print_line("FlecsEntityInspector::set_entity_from_remote_data - Inspector rebuilt");
}

void FlecsEntityInspector::clear_inspector() {
	if (!content_container) {
		return;
	}
	
	// Properly free children when clearing
	for (int i = content_container->get_child_count() - 1; i >= 0; i--) {
		Node *child = content_container->get_child(i);
		if (child) {
			content_container->remove_child(child);
			memdelete(child);
		}
	}
	
	current_entity_id = 0;
	current_world = RID();
	is_remote_mode = false;
	remote_components_data.clear();
	component_data.clear();
	expanded_paths.clear();
	current_component_filter = "";
	if (component_filter) {
		component_filter->set_text("");
	}
	_rebuild_inspector();
}

void FlecsEntityInspector::refresh_entity() {
	if (current_entity_id != 0 && current_world.is_valid()) {
		_rebuild_inspector();
	}
}

void FlecsEntityInspector::_rebuild_inspector() {
	// Validate content_container exists
	if (!content_container) {
		ERR_PRINT("FlecsEntityInspector::_rebuild_inspector - content_container is null");
		return;
	}

	// Clear all content - properly free the nodes
	for (int i = content_container->get_child_count() - 1; i >= 0; i--) {
		Node *child = content_container->get_child(i);
		if (child) {
			content_container->remove_child(child);
			memdelete(child);
		}
	}

	component_data.clear();

	if (current_entity_id == 0 || !current_world.is_valid() || (!is_remote_mode && !flecs_server)) {
		Label *empty = memnew(Label);
		empty->set_text("No entity selected");
		empty->add_theme_font_size_override("font_size", 11);
		content_container->add_child(empty);
		return;
	}

	_build_entity_header();
	content_container->add_child(memnew(HSeparator));
	_build_components_section();

	// Add bottom spacer
	Control *spacer = memnew(Control);
	spacer->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	content_container->add_child(spacer);
}

void FlecsEntityInspector::_build_entity_header() {
	VBoxContainer *header = memnew(VBoxContainer);
	header->add_theme_constant_override("separation", 4);

	// Entity name
	entity_header = memnew(Label);
	String entity_name;
	int component_count = 0;
	
	if (is_remote_mode) {
		// Remote mode - use default name
		entity_name = "Entity_" + String::num_int64(current_entity_id, 16) + " [REMOTE]";
		component_count = remote_components_data.size();
	} else {
		// Local mode - query FlecsServer
		if (!flecs_server) {
			entity_name = "Entity_" + String::num_int64(current_entity_id, 16) + " [LOCAL - NO SERVER]";
			component_count = 0;
		} else {
			RID entity_rid = RID::from_uint64(current_entity_id);
			entity_name = flecs_server->get_entity_name(entity_rid);
			if (entity_name.is_empty()) {
				entity_name = "Entity_" + String::num_int64(current_entity_id, 16);
			}
			PackedStringArray components = flecs_server->get_component_types_as_name(entity_rid);
			component_count = components.size();
		}
	}
	
	entity_header->set_text(entity_name);
	entity_header->add_theme_font_size_override("font_size", 14);
	entity_header->add_theme_color_override("font_color", Color(0.9, 0.9, 1.0));
	header->add_child(entity_header);

	// Entity info
	entity_info = memnew(Label);
	entity_info->set_text("ID: " + String::num_int64(current_entity_id, 16) + 
		"\nComponents: " + itos(component_count) + 
		"\nWorld: " + String::num_int64(current_world.get_id(), 16));
	entity_info->add_theme_font_size_override("font_size", 10);
	entity_info->add_theme_color_override("font_color", Color(0.75, 0.75, 0.75));
	header->add_child(entity_info);

	content_container->add_child(header);
}

void FlecsEntityInspector::_build_components_section() {
	print_line("FlecsEntityInspector::_build_components_section - START");
	
	if (!content_container) {
		ERR_PRINT("FlecsEntityInspector::_build_components_section - content_container is null");
		return;
	}
	
	if (is_remote_mode) {
		print_line(vformat("FlecsEntityInspector::_build_components_section - Remote mode, %d components", remote_components_data.size()));
		
		// Remote mode - use data from remote_components_data
		if (remote_components_data.is_empty()) {
			Label *no_comps = memnew(Label);
			no_comps->set_text("No components");
			no_comps->add_theme_font_size_override("font_size", 10);
			content_container->add_child(no_comps);
			return;
		}

		for (int i = 0; i < remote_components_data.size(); i++) {
			Variant comp_var = remote_components_data[i];
			if (comp_var.get_type() != Variant::DICTIONARY) {
				print_line(vformat("FlecsEntityInspector::_build_components_section - Skipping non-dict component at index %d", i));
				continue;
			}
			
			Dictionary component_dict = comp_var;
			String comp_name = component_dict.get("name", "Unknown");
			
			// Skip empty component names
			if (comp_name.is_empty() || comp_name == "Unknown") {
				print_line(vformat("FlecsEntityInspector::_build_components_section - Skipping component with empty/unknown name at index %d", i));
				continue;
			}
			
			Variant data_var = component_dict.get("data", Dictionary());
			Dictionary comp_data_dict;
			if (data_var.get_type() == Variant::DICTIONARY) {
				comp_data_dict = data_var;
			}

			print_line(vformat("FlecsEntityInspector::_build_components_section - Processing component '%s' with %d data fields", 
				comp_name, comp_data_dict.size()));

			// Check if component matches filter
			if (!current_component_filter.is_empty()) {
				String comp_name_lower = comp_name.to_lower();
				if (!comp_name_lower.contains(current_component_filter)) {
					continue;  // Skip components that don't match filter
				}
			}

			// Always create a component widget, even for empty data
			Control *comp_widget = _build_component_widget(comp_name, comp_data_dict);
			if (comp_widget) {
				content_container->add_child(comp_widget);
				if (!comp_data_dict.is_empty()) {
					component_data[comp_name] = comp_data_dict.duplicate();
				}
			}
		}
	} else {
		print_line("FlecsEntityInspector::_build_components_section - Local mode");
		
		// Local mode - query FlecsServer
		if (!flecs_server) {
			print_line("FlecsEntityInspector::_build_components_section - flecs_server is null in local mode, cannot display components");
			Label *no_server = memnew(Label);
			no_server->set_text("FlecsServer not available");
			no_server->add_theme_font_size_override("font_size", 10);
			content_container->add_child(no_server);
			return;
		}
		
		RID entity_rid = RID::from_uint64(current_entity_id);
		if (!entity_rid.is_valid()) {
			print_line("FlecsEntityInspector::_build_components_section - entity_rid is invalid");
			return;
		}
		
		PackedStringArray component_names = flecs_server->get_component_types_as_name(entity_rid);
		print_line(vformat("FlecsEntityInspector::_build_components_section - Found %d components", component_names.size()));

		if (component_names.is_empty()) {
			Label *no_comps = memnew(Label);
			no_comps->set_text("No components");
			no_comps->add_theme_font_size_override("font_size", 10);
			content_container->add_child(no_comps);
			return;
		}

		for (int i = 0; i < component_names.size(); i++) {
			String comp_name = component_names[i];
			
			// Skip empty component names
			if (comp_name.is_empty()) {
				continue;
			}
			
			Dictionary comp_data_dict = flecs_server->get_component_by_name(entity_rid, comp_name);

			// Check if component matches filter
			if (!current_component_filter.is_empty()) {
				String comp_name_lower = comp_name.to_lower();
				if (!comp_name_lower.contains(current_component_filter)) {
					continue;  // Skip components that don't match filter
				}
			}

			// Always create a component widget, even for empty data
			Control *comp_widget = _build_component_widget(comp_name, comp_data_dict);
			if (comp_widget) {
				content_container->add_child(comp_widget);
				if (!comp_data_dict.is_empty()) {
					component_data[comp_name] = comp_data_dict.duplicate();
				}
			}
		}
	}
	
	print_line("FlecsEntityInspector::_build_components_section - COMPLETE");
}

Control *FlecsEntityInspector::_build_component_widget(const String &p_component_name, 
	                                                     const Dictionary &p_component_data) {
	VBoxContainer *component_box = memnew(VBoxContainer);
	component_box->add_theme_constant_override("separation", 4);

	// Header panel
	PanelContainer *header_panel = memnew(PanelContainer);
	HBoxContainer *header_box = memnew(HBoxContainer);
	header_box->add_theme_constant_override("separation", 8);

	Label *comp_label = memnew(Label);
	comp_label->set_text("📦 " + p_component_name);
	comp_label->add_theme_font_size_override("font_size", 11);
	comp_label->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	header_box->add_child(comp_label);

	// Only show apply/revert buttons if component has data
	if (!p_component_data.is_empty()) {
		Button *apply_btn = memnew(Button);
		apply_btn->set_text("✓");
		apply_btn->set_custom_minimum_size(Vector2(30, 0));
		apply_btn->set_tooltip_text("Apply changes");
		apply_btn->connect(SceneStringName(pressed), 
			callable_mp(this, &FlecsEntityInspector::_apply_component_changes).bind(p_component_name));
		header_box->add_child(apply_btn);

		Button *revert_btn = memnew(Button);
		revert_btn->set_text("↺");
		revert_btn->set_custom_minimum_size(Vector2(30, 0));
		revert_btn->set_tooltip_text("Revert changes");
		revert_btn->connect(SceneStringName(pressed), 
			callable_mp(this, &FlecsEntityInspector::_revert_component_changes).bind(p_component_name));
		header_box->add_child(revert_btn);
	}

	header_panel->add_child(header_box);
	component_box->add_child(header_panel);

	// Content area - either property tree or "no data" message
	if (!p_component_data.is_empty()) {
		// Property tree with read-only fields
		Tree *tree = _build_property_tree(p_component_name, p_component_data);
		if (tree) {
			tree->set_custom_minimum_size(Vector2(0, 150));
			tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
			component_box->add_child(tree);
		}
	} else {
		// No data - show a box with generic message
		PanelContainer *no_data_panel = memnew(PanelContainer);
		no_data_panel->set_custom_minimum_size(Vector2(0, 40));
		
		VBoxContainer *no_data_box = memnew(VBoxContainer);
		no_data_box->set_alignment(BoxContainer::ALIGNMENT_CENTER);
		
		Label *no_data_label = memnew(Label);
		no_data_label->set_text("Tag component (no data)");
		no_data_label->add_theme_font_size_override("font_size", 10);
		no_data_label->add_theme_color_override("font_color", Color(0.6, 0.6, 0.6));
		no_data_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		no_data_box->add_child(no_data_label);
		
		no_data_panel->add_child(no_data_box);
		component_box->add_child(no_data_panel);
	}

	component_box->add_child(memnew(HSeparator));
	return component_box;
}

Tree *FlecsEntityInspector::_build_property_tree(const String &p_component_name, 
	                                               const Dictionary &p_data) {
	Tree *tree = memnew(Tree);
	tree->set_column_expand(0, true);
	tree->set_column_expand(1, true);
	tree->set_column_custom_minimum_width(0, 100);
	tree->set_column_custom_minimum_width(1, 150);
	tree->set_hide_root(true);
	tree->set_v_scroll_enabled(true);

	TreeItem *root = tree->create_item();

	Array keys = p_data.keys();
	for (int i = 0; i < keys.size(); i++) {
		String key = keys[i];
		Variant value = p_data[key];
		_add_property_item(root, "", key, value, p_component_name, 0);
	}

	return tree;
}

TreeItem *FlecsEntityInspector::_add_property_item(TreeItem *p_parent, const String &p_path, 
	                                                 const String &p_key, const Variant &p_value, 
	                                                 const String &p_component_name, int p_depth) {
	if (p_depth > MAX_NESTING_DEPTH) {
		return nullptr;
	}

	TreeItem *item = p_parent->create_child(-1);
	
	String new_path = p_path.is_empty() ? p_key : p_path + "." + p_key;
	item->set_text(0, p_key);

	// Check if expandable
	bool is_expandable = _is_expandable(p_value);

	if (is_expandable) {
		item->set_selectable(0, true);
		item->set_collapsed(true);
		item->set_metadata(0, new_path);

		// For complex types, add children
		if (p_value.get_type() == Variant::DICTIONARY) {
			Dictionary dict = p_value;
			Array dict_keys = dict.keys();

			for (int i = 0; i < dict_keys.size(); i++) {
				String dict_key = dict_keys[i];
				Variant dict_value = dict[dict_key];
				_add_property_item(item, new_path, dict_key, dict_value, p_component_name, p_depth + 1);
			}
		} else if (p_value.get_type() == Variant::ARRAY) {
			Array arr = p_value;
			int count = MIN(arr.size(), MAX_ARRAY_ITEMS);

			for (int i = 0; i < count; i++) {
				_add_property_item(item, new_path, "[" + itos(i) + "]", arr[i], p_component_name, p_depth + 1);
			}
		}

		// Show summary for complex types
		String summary = _format_value(p_value, p_depth);
		item->set_text(1, summary);
	} else {
		// Create editor widget for primitives
		Control *editor = _create_property_editor(new_path, p_value, p_component_name);
		if (editor) {
			item->set_cell_mode(1, TreeItem::CELL_MODE_CUSTOM);
		}
	}

	// Type info
	String type_str = _get_type_string(p_value);
	item->set_tooltip_text(0, type_str);

	return item;
}

Control *FlecsEntityInspector::_create_property_editor(const String &p_path, const Variant &p_value, 
	                                                     const String &p_component_name) {
	Control *editor = nullptr;

	switch (p_value.get_type()) {
		case Variant::BOOL: {
			CheckBox *checkbox = memnew(CheckBox);
			checkbox->set_pressed(p_value);
			checkbox->connect(SceneStringName(toggled), 
				callable_mp(this, &FlecsEntityInspector::_on_bool_toggled)
				.bind(p_component_name, p_path));
			editor = checkbox;
		} break;

		case Variant::INT: {
			SpinBox *spinbox = memnew(SpinBox);
			spinbox->set_value(p_value);
			spinbox->set_min(-1e9);
			spinbox->set_max(1e9);
			spinbox->set_step(1);
			spinbox->connect("value_changed", 
				callable_mp(this, &FlecsEntityInspector::_on_int_changed)
				.bind(p_component_name, p_path));
			editor = spinbox;
		} break;

		case Variant::FLOAT: {
			SpinBox *spinbox = memnew(SpinBox);
			spinbox->set_value(p_value);
			spinbox->set_min(-1e9);
			spinbox->set_max(1e9);
			spinbox->set_step(0.01);
			spinbox->connect("value_changed", 
				callable_mp(this, &FlecsEntityInspector::_on_float_changed)
				.bind(p_component_name, p_path));
			editor = spinbox;
		} break;

		case Variant::STRING: {
			LineEdit *line = memnew(LineEdit);
			line->set_text(p_value);
			line->set_custom_minimum_size(Vector2(150, 0));
			line->connect("text_changed", 
				callable_mp(this, &FlecsEntityInspector::_on_string_changed)
				.bind(p_component_name, p_path));
			editor = line;
		} break;

		case Variant::COLOR: {
			ColorPickerButton *color_btn = memnew(ColorPickerButton);
			color_btn->set_pick_color(p_value);
			color_btn->set_custom_minimum_size(Vector2(50, 24));
			color_btn->connect("color_changed", 
				callable_mp(this, &FlecsEntityInspector::_on_color_changed)
				.bind(p_component_name, p_path));
			editor = color_btn;
		} break;

		default: {
			Label *label = memnew(Label);
			label->set_text(_format_value(p_value));
			label->add_theme_font_size_override("font_size", 9);
			editor = label;
		} break;
	}

	return editor;
}

void FlecsEntityInspector::_on_bool_toggled(bool p_value, const String &p_component_name, const String &p_path) {
	if (!component_data.has(p_component_name)) {
		return;
	}
	Variant comp_var = component_data[p_component_name];
	if (comp_var.get_type() == Variant::DICTIONARY) {
		Dictionary comp = comp_var;
		_set_value_at_path(comp, p_path.split("."), p_value);
		component_data[p_component_name] = comp;
	}
}

void FlecsEntityInspector::_on_int_changed(double p_value, const String &p_component_name, const String &p_path) {
	if (!component_data.has(p_component_name)) {
		return;
	}
	Variant comp_var = component_data[p_component_name];
	if (comp_var.get_type() == Variant::DICTIONARY) {
		Dictionary comp = comp_var;
		_set_value_at_path(comp, p_path.split("."), (int64_t)p_value);
		component_data[p_component_name] = comp;
	}
}

void FlecsEntityInspector::_on_float_changed(double p_value, const String &p_component_name, const String &p_path) {
	if (!component_data.has(p_component_name)) {
		return;
	}
	Variant comp_var = component_data[p_component_name];
	if (comp_var.get_type() == Variant::DICTIONARY) {
		Dictionary comp = comp_var;
		_set_value_at_path(comp, p_path.split("."), p_value);
		component_data[p_component_name] = comp;
	}
}

void FlecsEntityInspector::_on_string_changed(const String &p_value, const String &p_component_name, const String &p_path) {
	if (!component_data.has(p_component_name)) {
		return;
	}
	Variant comp_var = component_data[p_component_name];
	if (comp_var.get_type() == Variant::DICTIONARY) {
		Dictionary comp = comp_var;
		_set_value_at_path(comp, p_path.split("."), p_value);
		component_data[p_component_name] = comp;
	}
}

void FlecsEntityInspector::_on_color_changed(const Color &p_value, const String &p_component_name, const String &p_path) {
	if (!component_data.has(p_component_name)) {
		return;
	}
	Variant comp_var = component_data[p_component_name];
	if (comp_var.get_type() == Variant::DICTIONARY) {
		Dictionary comp = comp_var;
		_set_value_at_path(comp, p_path.split("."), p_value);
		component_data[p_component_name] = comp;
	}
}

void FlecsEntityInspector::_on_tree_item_expanded(TreeItem *p_item) {
	if (!p_item) {
		return;
	}
	String path = p_item->get_metadata(0);
	if (!path.is_empty()) {
		expanded_paths.insert(path);
	}
}

void FlecsEntityInspector::_on_tree_item_collapsed(TreeItem *p_item) {
	if (!p_item) {
		return;
	}
	String path = p_item->get_metadata(0);
	if (!path.is_empty()) {
		expanded_paths.erase(path);
	}
}

void FlecsEntityInspector::_on_component_filter_changed(const String &p_text) {
	current_component_filter = p_text.strip_edges().to_lower();
	_rebuild_inspector();
}

void FlecsEntityInspector::_apply_component_changes(const String &p_component_name) {
	if (!flecs_server || !component_data.has(p_component_name)) {
		return;
	}

	RID entity_rid = RID::from_uint64(current_entity_id);
	Dictionary new_values = component_data[p_component_name];

	flecs_server->set_component(entity_rid, p_component_name, new_values);
	print_line(vformat("✓ Applied changes to %s", p_component_name));
}

void FlecsEntityInspector::_revert_component_changes(const String &p_component_name) {
	if (!component_data.has(p_component_name)) {
		return;
	}

	print_line(vformat("↺ Reverted component '%s'", p_component_name));
	refresh_entity();
}

String FlecsEntityInspector::_get_type_string(const Variant &p_value) const {
	return Variant::get_type_name(p_value.get_type());
}

String FlecsEntityInspector::_format_value(const Variant &p_value, int p_depth) const {
	switch (p_value.get_type()) {
		case Variant::BOOL:
			return p_value ? "true" : "false";
		case Variant::INT:
			return itos(p_value);
		case Variant::FLOAT:
			return rtos(p_value);
		case Variant::STRING:
			return "\"" + String(p_value) + "\"";
		case Variant::COLOR: {
			Color c = p_value;
			return vformat("#%02X%02X%02X%02X",
				uint8_t(c.r * 255), uint8_t(c.g * 255),
				uint8_t(c.b * 255), uint8_t(c.a * 255));
		}
		case Variant::VECTOR2: {
			Vector2 v = p_value;
			return vformat("(%.2f, %.2f)", v.x, v.y);
		}
		case Variant::VECTOR3: {
			Vector3 v = p_value;
			return vformat("(%.2f, %.2f, %.2f)", v.x, v.y, v.z);
		}
		case Variant::DICTIONARY: {
			Dictionary d = p_value;
			return vformat("{%d items}", d.size());
		}
		case Variant::ARRAY: {
			Array a = p_value;
			return vformat("[%d items]", a.size());
		}
		default:
			return "[" + _get_type_string(p_value) + "]";
	}
}

bool FlecsEntityInspector::_is_expandable(const Variant &p_value) const {
	return p_value.get_type() == Variant::DICTIONARY || p_value.get_type() == Variant::ARRAY;
}

void FlecsEntityInspector::_set_value_at_path(Dictionary &p_dict, const PackedStringArray &p_path, const Variant &p_value) {
	if (p_path.is_empty()) {
		return;
	}

	Dictionary *current = &p_dict;
	for (int i = 0; i < (int)p_path.size() - 1; i++) {
		String key = p_path[i];
		if (!current->has(key)) {
			(*current)[key] = Dictionary();
		}
		Variant next_var = (*current)[key];
		if (next_var.get_type() != Variant::DICTIONARY) {
			(*current)[key] = Dictionary();
		}
		// Cannot use pointer to variant's internal dictionary, must work differently
		// For now, just set at current level if it's the last key
		if (i == (int)p_path.size() - 2) {
			(*current)[p_path[i + 1]] = p_value;
			return;
		}
	}
}

Variant FlecsEntityInspector::_get_value_at_path(const Dictionary &p_dict, const PackedStringArray &p_path) const {
	if (p_path.is_empty()) {
		return Variant();
	}

	Variant current_var = p_dict;

	for (int i = 0; i < (int)p_path.size(); i++) {
		String key = p_path[i];
		if (current_var.get_type() != Variant::DICTIONARY) {
			return Variant();
		}
		Dictionary current_dict = current_var;
		if (!current_dict.has(key)) {
			return Variant();
		}
		current_var = current_dict[key];
		if (i == (int)p_path.size() - 1) {
			return current_var;
		}
	}

	return Variant();
}

void FlecsEntityInspector::_collect_tree_values(TreeItem *p_item, Dictionary &p_out, const PackedStringArray &p_path) {
	if (!p_item) {
		return;
	}

	// Recursively collect from tree structure
	TreeItem *child = p_item->get_first_child();
	while (child) {
		String key = child->get_text(0);
		PackedStringArray new_path = p_path;
		new_path.push_back(key);

		child = child->get_next();
	}
}