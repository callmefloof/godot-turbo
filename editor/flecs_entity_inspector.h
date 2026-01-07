#ifndef FLECS_ENTITY_INSPECTOR_H
#define FLECS_ENTITY_INSPECTOR_H

#include "scene/gui/panel_container.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/box_container.h"
#include "scene/gui/tree.h"
#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/check_box.h"
#include "scene/gui/color_picker.h"
#include "core/variant/variant.h"
#include "core/templates/hash_set.h"

class FlecsServer;

/**
 * @class FlecsEntityInspector
 * @brief Live inspector panel for viewing and editing entity components
 *
 * Displays entity information and components with expandable trees similar to Godot's
 * node inspector. Supports nested dictionaries, arrays, and inline editing.
 */
class FlecsEntityInspector : public PanelContainer {
	GDCLASS(FlecsEntityInspector, PanelContainer);

protected:
	static void _bind_methods();
	virtual void _notification(int p_what);

public:
	FlecsEntityInspector();
	~FlecsEntityInspector();

	/** Set the entity to inspect */
	void set_entity(RID p_world_rid, uint64_t p_entity_id);

	/** Set the entity to inspect from remote data (for remote debugging) */
	void set_entity_from_remote_data(uint64_t p_world_id, uint64_t p_entity_id, const Array &p_components);

	/** Clear the inspector */
	void clear_inspector();

	/** Refresh the current entity's data */
	void refresh_entity();

	/** Get currently inspected entity ID */
	uint64_t get_entity_id() const { return current_entity_id; }

	/** Get currently inspected world RID */
	RID get_world_rid() const { return current_world; }

private:
	FlecsServer *flecs_server = nullptr;
	RID current_world;
	uint64_t current_entity_id = 0;
	bool is_remote_mode = false;
	Array remote_components_data;

	// UI Components
	VBoxContainer *main_container = nullptr;
	LineEdit *component_filter = nullptr;
	String current_component_filter;
	ScrollContainer *scroll_container = nullptr;
	VBoxContainer *content_container = nullptr;
	Label *entity_header = nullptr;
	Label *entity_info = nullptr;

	// Component data tracking
	Dictionary component_data;  // component_name -> current_values
	HashSet<String> expanded_paths;

	// Rebuild the inspector UI
	void _rebuild_inspector();
	void _build_entity_header();
	void _build_components_section();

	// Build component widget
	Control *_build_component_widget(const String &p_component_name, const Dictionary &p_component_data);

	// Build property tree structure
	Tree *_build_property_tree(const String &p_component_name, const Dictionary &p_data);

	// Recursively build tree items
	TreeItem *_add_property_item(TreeItem *p_parent, const String &p_path, const String &p_key,
	                              const Variant &p_value, const String &p_component_name, int p_depth = 0);

	// Build inline editor for property
	Control *_create_property_editor(const String &p_path, const Variant &p_value,
	                                  const String &p_component_name);

	// Field change callbacks
	void _on_bool_toggled(bool p_value, const String &p_component_name, const String &p_path);
	void _on_int_changed(double p_value, const String &p_component_name, const String &p_path);
	void _on_float_changed(double p_value, const String &p_component_name, const String &p_path);
	void _on_string_changed(const String &p_value, const String &p_component_name, const String &p_path);
	void _on_color_changed(const Color &p_value, const String &p_component_name, const String &p_path);

	// Tree callbacks
	void _on_tree_item_expanded(TreeItem *p_item);
	void _on_tree_item_collapsed(TreeItem *p_item);

	// Filter callback
	void _on_component_filter_changed(const String &p_text);

	// Apply/Revert changes
	void _apply_component_changes(const String &p_component_name);
	void _revert_component_changes(const String &p_component_name);

	// Utility methods
	String _get_type_string(const Variant &p_value) const;
	String _format_value(const Variant &p_value, int p_depth = 0) const;
	bool _is_expandable(const Variant &p_value) const;
	
	// Navigate nested dictionaries using path array
	Variant _get_value_at_path(const Dictionary &p_dict, const PackedStringArray &p_path) const;
	void _set_value_at_path(Dictionary &p_dict, const PackedStringArray &p_path, const Variant &p_value);
	void _collect_tree_values(TreeItem *p_item, Dictionary &p_out, const PackedStringArray &p_path);

	static constexpr int MAX_NESTING_DEPTH = 4;
	static constexpr int MAX_ARRAY_ITEMS = 100;
};

#endif // FLECS_ENTITY_INSPECTOR_H