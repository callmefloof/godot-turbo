#include "flecs_editor_plugin.h"
#include "flecs_profiler.h"
#include "instance_manager.h"
#include "modules/godot_turbo/ecs/flecs_types/flecs_server.h"

#include "editor/editor_interface.h"
#include "editor/debugger/editor_debugger_node.h"
#include "editor/debugger/script_editor_debugger.h"
#include "editor/debugger/editor_debugger_plugin.h"
#include "scene/gui/label.h"
#include "scene/gui/button.h"
#include "scene/gui/box_container.h"
#include "scene/gui/tree.h"
#include "scene/gui/split_container.h"
#include "scene/gui/separator.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/line_edit.h"
#include "scene/main/timer.h"
#include "core/string/print_string.h"
#include "core/object/object_id.h"
#include "core/object/object.h"

// FlecsDebuggerBridge implementation (bridge registered via EditorDebuggerNode)
bool FlecsDebuggerBridge::_has_capture(const String &p_capture) const {
	return (p_capture == "flecs");
}

bool FlecsDebuggerBridge::_capture(const String &p_message, const Array &p_data, int p_session) {
	if (!editor_plugin) {
		return false;
	}
	return editor_plugin->capture_remote_message(p_message, p_data);
}

// FlecsWorldEditorPlugin implementation
FlecsWorldEditorPlugin *FlecsWorldEditorPlugin::singleton = nullptr;

FlecsWorldEditorPlugin *FlecsWorldEditorPlugin::get_singleton() {
	return singleton;
}

TypedArray<RID> FlecsWorldEditorPlugin::get_available_worlds() const {
	TypedArray<RID> result;
	
	// In remote mode, get worlds from world_dirty map (populated by _handle_remote_worlds)
	if (remote_mode) {
		Array keys = world_dirty.keys();
		for (int i = 0; i < keys.size(); i++) {
			result.append(keys[i]);
		}
	}
	
	// Also check world_cache
	if (result.is_empty()) {
		Array keys = world_cache.keys();
		for (int i = 0; i < keys.size(); i++) {
			result.append(keys[i]);
		}
	}
	
	// If still empty and we're in local mode, try FlecsServer directly
	if (result.is_empty() && !remote_mode && flecs_server) {
		result = flecs_server->get_world_list();
	}
	
	return result;
}

void FlecsWorldEditorPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_on_tree_item_expanded", "item"), &FlecsWorldEditorPlugin::_on_tree_item_expanded);
	ClassDB::bind_method(D_METHOD("_on_tree_item_selected"), &FlecsWorldEditorPlugin::_on_tree_item_selected);
	ClassDB::bind_method(D_METHOD("_on_refresh_pressed"), &FlecsWorldEditorPlugin::_on_refresh_pressed);
	ClassDB::bind_method(D_METHOD("_on_expand_all_pressed"), &FlecsWorldEditorPlugin::_on_expand_all_pressed);
	ClassDB::bind_method(D_METHOD("_on_collapse_all_pressed"), &FlecsWorldEditorPlugin::_on_collapse_all_pressed);
	ClassDB::bind_method(D_METHOD("_on_world_refresh_timer_timeout"), &FlecsWorldEditorPlugin::_on_world_refresh_timer_timeout);
	ClassDB::bind_method(D_METHOD("_on_search_text_changed", "text"), &FlecsWorldEditorPlugin::_on_search_text_changed);
	ClassDB::bind_method(D_METHOD("_on_debugger_session_stopped"), &FlecsWorldEditorPlugin::_on_debugger_session_stopped);
	ClassDB::bind_method(D_METHOD("_on_session_started"), &FlecsWorldEditorPlugin::_on_session_started);
}

void FlecsWorldEditorPlugin::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Initialize instance manager for multi-instance handling
			InstanceManager::get_singleton()->initialize();
			_on_enter_tree();
		} break;
		case NOTIFICATION_EXIT_TREE: {
			_on_exit_tree();
			// Shutdown instance manager
			InstanceManager::get_singleton()->shutdown();
		} break;
	}
}

FlecsWorldEditorPlugin::FlecsWorldEditorPlugin() {
	set_name("Flecs Worlds");
	singleton = this;
}

FlecsWorldEditorPlugin::~FlecsWorldEditorPlugin() {
	world_cache.clear();
	world_dirty.clear();
	tree_item_map.clear();
	singleton = nullptr;
}

String FlecsWorldEditorPlugin::get_plugin_name() const {
	return "Flecs Worlds";
}

void FlecsWorldEditorPlugin::_on_enter_tree() {
	// Get FlecsServer singleton
	if (!Engine::get_singleton()->has_singleton("FlecsServer")) {
		ERR_PRINT("FlecsWorldEditorPlugin: FlecsServer singleton not found");
		return;
	}
	
	flecs_server = Object::cast_to<FlecsServer>(Engine::get_singleton()->get_singleton_object("FlecsServer"));
	if (!flecs_server) {
		ERR_PRINT("FlecsWorldEditorPlugin: Could not cast FlecsServer singleton");
		return;
	}

	_build_dock_ui();
	_refresh_worlds_tree();

	// Register dock with editor
	add_control_to_dock(EditorPlugin::DOCK_SLOT_LEFT_BR, dock);

	// Start world refresh timer now that dock is in scene tree
	if (world_refresh_timer) {
		world_refresh_timer->start();
	}

	// Setup remote debugger
	_setup_remote_debugger();
}

void FlecsWorldEditorPlugin::_on_exit_tree() {
	_teardown_remote_debugger();

	if (dock) {
		remove_control_from_docks(dock);
		dock->queue_free();
		dock = nullptr;
	}
	world_cache.clear();
	world_dirty.clear();
	tree_item_map.clear();
}

void FlecsWorldEditorPlugin::_setup_remote_debugger() {
	EditorDebuggerNode *debugger_node = EditorDebuggerNode::get_singleton();
	if (!debugger_node) {
		return;
	}

	// Create and register debugger plugin (only once)
	if (!debugger_plugin.is_valid()) {
		debugger_plugin.instantiate();
		debugger_plugin->set_editor_plugin(this);
		debugger_node->add_debugger_plugin(debugger_plugin);
	}

	// Try to attach to existing debugger sessions
	if (!debugger_connected) {
		int session_count = debugger_node->get_child_count();
		for (int i = 0; i < session_count; i++) {
			Node *child = debugger_node->get_child(i);
			
			// Check if it's a ScriptEditorDebugger directly
			ScriptEditorDebugger *script_debugger = Object::cast_to<ScriptEditorDebugger>(child);
			if (script_debugger) {
				_attach_to_session(script_debugger);
				break;
			}
			
			// Otherwise search in its children (TabContainer case)
			int subchild_count = child->get_child_count();
			for (int j = 0; j < subchild_count; j++) {
				Node *subchild = child->get_child(j);
				script_debugger = Object::cast_to<ScriptEditorDebugger>(subchild);
				if (script_debugger) {
					_attach_to_session(script_debugger);
					break;
				}
			}
			if (debugger_connected) {
				break;
			}
		}
	}
}

void FlecsWorldEditorPlugin::_teardown_remote_debugger() {
	// Remove debugger plugin
	if (debugger_plugin.is_valid()) {
		EditorDebuggerNode *debugger_node = EditorDebuggerNode::get_singleton();
		if (debugger_node) {
			debugger_node->remove_debugger_plugin(debugger_plugin);
		}
		debugger_plugin.unref();
	}
	
	if (remote_session.is_valid()) {
		remote_session.unref();
		remote_mode = false;
	}
}

void FlecsWorldEditorPlugin::_attach_to_session(ScriptEditorDebugger *p_debugger) {
	if (!p_debugger) {
		return;
	}

	if (debugger_connected) {
		return;
	}

	remote_session = Ref<EditorDebuggerSession>(memnew(EditorDebuggerSession(p_debugger)));
	
	// Connect to debugger signals
	p_debugger->connect("started", callable_mp(this, &FlecsWorldEditorPlugin::_on_session_started));
	p_debugger->connect("stopped", callable_mp(this, &FlecsWorldEditorPlugin::_on_debugger_session_stopped));
	debugger_connected = true;
	
	// If already active, switch to remote mode immediately
	if (p_debugger->is_session_active()) {
		remote_mode = true;
		_request_remote_worlds();
	}
}

void FlecsWorldEditorPlugin::_on_session_started() {
	
	// Session started means we have a remote runtime - always enable remote mode
	active_session = remote_session;
	remote_mode = true;

	if (world_refresh_timer && world_refresh_timer->is_stopped() == false) {
		world_refresh_timer->stop();
	}

	_request_remote_worlds();

	// Clear any local-selected world/entity state when switching to remote
	selected_world = RID();
	selected_entity_id = 0;
	world_cache.clear();
	world_dirty.clear();
	tree_item_map.clear();
	pending_entity_requests.clear();
}

void FlecsWorldEditorPlugin::_on_debugger_session_stopped() {
	active_session.unref();
	remote_mode = false;

	// Clear remote-selected state when leaving remote mode
	selected_world = RID();
	selected_entity_id = 0;
	world_cache.clear();
	world_dirty.clear();
	tree_item_map.clear();

	if (world_refresh_timer && world_refresh_timer->is_stopped()) {
		world_refresh_timer->start();
	}
	
	// Refresh to show local worlds again
	_refresh_worlds_tree();
}

bool FlecsWorldEditorPlugin::capture_remote_message(const String &p_message, const Array &p_data) {
	if (p_message == "flecs:worlds") {
		_handle_remote_worlds(p_data);
		if (profiler) {
			profiler->handle_remote_worlds(p_data);
		}
		return true;
	} else if (p_message == "flecs:components") {
		_handle_remote_components(p_data);
		return true;
	} else if (p_message == "flecs:entities") {
		_handle_remote_entities(p_data);
		return true;
	} else if (p_message == "flecs:profiler_metrics") {
		if (p_data.size() > 0) {
			Dictionary metrics = p_data[0];
			_handle_profiler_metrics(metrics);
		}
		return true;
	}
	return false;
}

void FlecsWorldEditorPlugin::_handle_profiler_metrics(const Dictionary &p_data) {
	if (profiler) {
		profiler->handle_remote_metrics(p_data);
	}
}

void FlecsWorldEditorPlugin::_handle_remote_worlds(const Array &p_data) {
	if (p_data.is_empty()) {
		return;
	}
	
	if (!worlds_tree) {
		return;
	}

	Dictionary response = p_data[0];
	Array worlds_array = response.get("worlds", Array());

	// Save selected world
	RID prev_selected = selected_world;

	// CRITICAL: Disable tooltips and block signals during tree manipulation to prevent crashes
	// The tooltip system can call get_tooltip() which accesses TreeItem pointers
	// If we're deleting/creating items, those pointers may be invalid
	bool prev_auto_tooltip = worlds_tree->is_auto_tooltip_enabled();
	worlds_tree->set_auto_tooltip(false);
	worlds_tree->set_block_signals(true);

	worlds_tree->clear();
	tree_item_map.clear();
	world_cache.clear();
	world_dirty.clear();
	pending_entity_requests.clear(); // Clear pending requests since tree items are now invalid

	if (worlds_array.is_empty()) {
		TreeItem *root = worlds_tree->create_item();
		root->set_text(0, "No Worlds Found [REMOTE]");
		worlds_tree->set_block_signals(false);
		worlds_tree->set_auto_tooltip(prev_auto_tooltip);
		return;
	}

	TreeItem *root = worlds_tree->create_item();
	root->set_text(0, "Flecs Worlds (" + itos(worlds_array.size()) + ") [REMOTE]");

	for (int i = 0; i < worlds_array.size(); i++) {
		Dictionary world_dict = worlds_array[i];
		uint64_t world_id = world_dict.get("id", 0);
		String world_name = world_dict.get("name", "Unknown");
		RID world_rid = RID::from_uint64(world_id);

		TreeItem *world_item = worlds_tree->create_item(root);
		world_item->set_text(0, world_name);
		world_item->set_metadata(0, world_rid);
		world_item->set_selectable(0, true);
		world_item->set_collapsed(true);

		// Restore selection
		if (world_rid == prev_selected) {
			world_item->select(0);
			selected_world = world_rid;
		}

		// Add placeholder for entities
		TreeItem *placeholder = worlds_tree->create_item(world_item);
		placeholder->set_text(0, "(click to load)");

		world_dirty[world_rid] = true;
	}
	
	// Force tree to update display
	worlds_tree->queue_redraw();
	worlds_tree->update_minimum_size();
	
	// Restore tooltip and signals after tree manipulation is complete
	worlds_tree->set_block_signals(false);
	worlds_tree->set_auto_tooltip(prev_auto_tooltip);

	// Ensure dock is visible
	if (dock && dock->is_inside_tree()) {
		dock->set_visible(true);
	}
}

void FlecsWorldEditorPlugin::_request_remote_worlds() {
	if (!remote_session.is_valid()) {
		return;
	}

	if (!remote_session->is_active()) {
		return;
	}

	Array args;
	remote_session->send_message("flecs:request_worlds", args);
}

void FlecsWorldEditorPlugin::_request_remote_entity_components(uint64_t p_world_id, uint64_t p_entity_id) {
	if (!active_session.is_valid() || !active_session->is_active()) {
		return;
	}

	Array args;
	args.push_back(p_world_id);
	args.push_back(p_entity_id);

	active_session->send_message("flecs:request_components", args);
}

void FlecsWorldEditorPlugin::_request_remote_entities(uint64_t p_world_id, TreeItem *p_world_item) {
	if (!active_session.is_valid() || !active_session->is_active()) {
		return;
	}

	// Store the world item for when we get the response
	pending_entity_requests[p_world_id] = p_world_item->get_instance_id();

	int batch_size = int(batch_size_spinbox->get_value());
	
	Array args;
	args.push_back(p_world_id);
	args.push_back(0); // offset
	args.push_back(batch_size); // count

	active_session->send_message("flecs:request_entities", args);
}



void FlecsWorldEditorPlugin::_handle_remote_components(const Array &p_data) {
	if (p_data.is_empty()) {
		return;
	}
	
	if (!entity_inspector) {
		return;
	}

	Dictionary response = p_data[0];
	uint64_t world_id = response.get("world_id", 0);
	uint64_t entity_id = response.get("entity_id", 0);
	Array components = response.get("components", Array());

	// Check if this response is still relevant (user might have selected a different entity)
	if (entity_id != selected_entity_id || world_id != selected_world.get_id()) {
		return;
	}

	// Verify we're still in remote mode with an active session
	if (!remote_mode || !active_session.is_valid() || !active_session->is_active()) {
		return;
	}

	// Set the entity data in the inspector
	entity_inspector->set_entity_from_remote_data(world_id, entity_id, components);
}

void FlecsWorldEditorPlugin::_handle_remote_entities(const Array &p_data) {
	if (p_data.is_empty()) {
		return;
	}
	
	if (!worlds_tree) {
		return;
	}

	Dictionary response = p_data[0];
	uint64_t world_id = response.get("world_id", 0);
	Array entities = response.get("entities", Array());

	if (!remote_mode || !active_session.is_valid() || !active_session->is_active()) {
		return;
	}

	// Find the world item from pending requests
	if (!pending_entity_requests.has(world_id)) {
		return;
	}

	ObjectID world_item_id = pending_entity_requests[world_id];
	TreeItem *world_item = Object::cast_to<TreeItem>(ObjectDB::get_instance(world_item_id));

	if (!_is_pending_request_valid(world_id, world_item)) {
		pending_entity_requests.erase(world_id);
		return;
	}

	pending_entity_requests.erase(world_id);

	if (world_item->get_tree() != worlds_tree) {
		return;
	}

	// Set guard to prevent re-entrancy from signals triggered during tree manipulation
	// This is set after all early-return validations to ensure we only need to clear it
	// in the normal flow path.
	handling_entity_response = true;

	// CRITICAL: Disable tooltips and block signals during tree manipulation to prevent crashes
	// The tooltip system can call get_tooltip() which accesses TreeItem pointers
	// If we're deleting/creating items, those pointers may be invalid
	bool prev_auto_tooltip = worlds_tree->is_auto_tooltip_enabled();
	worlds_tree->set_auto_tooltip(false);
	worlds_tree->set_block_signals(true);

	// Clear placeholder children - must remove from tree_item_map first, then delete
	TreeItem *child = world_item->get_first_child();
	while (child) {
		TreeItem *next = child->get_next();
		// Remove from tree_item_map if it was an entity item
		if (tree_item_map.has(child)) {
			tree_item_map.erase(child);
		}
		memdelete(child);
		child = next;
	}

	RID world_rid = RID::from_uint64(world_id);

	// Add entities to tree
	if (entities.is_empty()) {
		// No entities - add a message
		TreeItem *no_entities = worlds_tree->create_item(world_item);
		no_entities->set_text(0, "No entities");
		no_entities->set_selectable(0, false);
	} else {
		for (int i = 0; i < entities.size(); i++) {
			Dictionary entity_dict = entities[i];
			uint64_t entity_id = entity_dict.get("id", 0);
			String entity_name = entity_dict.get("name", "Unknown");

			TreeItem *entity_item = worlds_tree->create_item(world_item);
			entity_item->set_text(0, entity_name);
			entity_item->set_selectable(0, true);

			// Map this item to world + entity
			Array pair;
			pair.push_back(world_rid);
			pair.push_back(entity_id);
			tree_item_map[entity_item] = pair;
		}
	}

	world_dirty[world_rid] = false;
	
	// Ensure the item stays expanded after we've added children
	world_item->set_collapsed(false);
	
	// Restore tooltip and signals after tree manipulation is complete
	worlds_tree->set_block_signals(false);
	worlds_tree->set_auto_tooltip(prev_auto_tooltip);
	
	// Clear guard
	handling_entity_response = false;
}



void FlecsWorldEditorPlugin::_build_dock_ui() {
	// Main container
	dock = memnew(VBoxContainer);
	dock->set_name("Flecs Worlds");

	// Toolbar
	HBoxContainer *toolbar = memnew(HBoxContainer);
	toolbar->set_custom_minimum_size(Vector2(0, 36));
	dock->add_child(toolbar);

	Button *refresh_btn = memnew(Button);
	refresh_btn->set_text("Refresh");
	refresh_btn->set_custom_minimum_size(Vector2(80, 0));
	refresh_btn->connect(SceneStringName(pressed), callable_mp(this, &FlecsWorldEditorPlugin::_on_refresh_pressed));
	toolbar->add_child(refresh_btn);

	Button *expand_btn = memnew(Button);
	expand_btn->set_text("Expand All");
	expand_btn->set_custom_minimum_size(Vector2(100, 0));
	expand_btn->connect(SceneStringName(pressed), callable_mp(this, &FlecsWorldEditorPlugin::_on_expand_all_pressed));
	toolbar->add_child(expand_btn);

	Button *collapse_btn = memnew(Button);
	collapse_btn->set_text("Collapse All");
	collapse_btn->set_custom_minimum_size(Vector2(100, 0));
	collapse_btn->connect(SceneStringName(pressed), callable_mp(this, &FlecsWorldEditorPlugin::_on_collapse_all_pressed));
	toolbar->add_child(collapse_btn);

	toolbar->add_child(memnew(VSeparator));

	Label *batch_label = memnew(Label);
	batch_label->set_text("Batch:");
	toolbar->add_child(batch_label);

	batch_size_spinbox = memnew(SpinBox);
	batch_size_spinbox->set_min(10);
	batch_size_spinbox->set_max(1000);
	batch_size_spinbox->set_value(ENTITIES_PER_PAGE);
	batch_size_spinbox->set_custom_minimum_size(Vector2(80, 0));
	toolbar->add_child(batch_size_spinbox);

	toolbar->add_spacer(false);

	// Main split
	HSplitContainer *split = memnew(HSplitContainer);
	split->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	split->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	dock->add_child(split);

	// Left panel: Tree
	VBoxContainer *left_panel = memnew(VBoxContainer);
	left_panel->set_custom_minimum_size(Vector2(300, 0));
	split->add_child(left_panel);

	Label *worlds_label = memnew(Label);
	worlds_label->set_text("Worlds");
	worlds_label->add_theme_font_size_override("font_size", 12);
	left_panel->add_child(worlds_label);

	// Search field for filtering entities
	search_field = memnew(LineEdit);
	search_field->set_placeholder("Filter entities...");
	search_field->set_clear_button_enabled(true);
	search_field->set_custom_minimum_size(Vector2(0, 28));
	search_field->connect("text_changed", callable_mp(this, &FlecsWorldEditorPlugin::_on_search_text_changed));
	left_panel->add_child(search_field);

	worlds_tree = memnew(Tree);
	worlds_tree->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	worlds_tree->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	// Disable auto tooltips by default - they can cause crashes when tree items are being
	// modified while the tooltip system tries to access them (especially on Wayland)
	worlds_tree->set_auto_tooltip(false);
	worlds_tree->connect("item_selected", callable_mp(this, &FlecsWorldEditorPlugin::_on_tree_item_selected));
	worlds_tree->connect("item_collapsed", callable_mp(this, &FlecsWorldEditorPlugin::_on_tree_item_expanded));
	left_panel->add_child(worlds_tree);

	// Right panel: Entity inspector
	entity_inspector = memnew(FlecsEntityInspector);
	entity_inspector->set_name("Inspector");
	entity_inspector->set_custom_minimum_size(Vector2(350, 0));
	entity_inspector->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	entity_inspector->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	split->add_child(entity_inspector);

	split->set_split_offset(350);

	// Auto-refresh timer for world list
	world_refresh_timer = memnew(Timer);
	world_refresh_timer->set_wait_time(1.0);
	world_refresh_timer->set_one_shot(false);
	world_refresh_timer->connect("timeout", callable_mp(this, &FlecsWorldEditorPlugin::_on_world_refresh_timer_timeout));
	dock->add_child(world_refresh_timer);
	// Timer will be started after dock is added to editor scene tree
}



void FlecsWorldEditorPlugin::_refresh_worlds_tree() {
	if (!worlds_tree) {
		return;
	}

	// Use remote if remote_mode is enabled (session started signal received)
	if (remote_mode && remote_session.is_valid()) {
		_request_remote_worlds();
		return;
	}

	if (!flecs_server) {
		return;
	}

	TypedArray<RID> world_list = flecs_server->get_world_list();

	// Check if world list changed
	TreeItem *root = worlds_tree->get_root();
	if (root) {
		int current_count = root->get_child_count();
		if (current_count == world_list.size()) {
			// Same count, check if they're the same worlds
			bool same = true;
			for (int i = 0; i < world_list.size(); i++) {
				TreeItem *child = root->get_child(i);
				if (child && child->get_metadata(0) != world_list[i]) {
					same = false;
					break;
				}
			}
			if (same) {
				return; // No changes, skip refresh
			}
		}
	}

	// Save selected world
	RID prev_selected = selected_world;

	// CRITICAL: Disable tooltips and block signals during tree manipulation to prevent crashes
	// The tooltip system can call get_tooltip() which accesses TreeItem pointers
	// If we're deleting/creating items, those pointers may be invalid
	bool prev_auto_tooltip = worlds_tree->is_auto_tooltip_enabled();
	worlds_tree->set_auto_tooltip(false);
	worlds_tree->set_block_signals(true);

	worlds_tree->clear();
	tree_item_map.clear();
	world_cache.clear();
	world_dirty.clear();
	pending_entity_requests.clear(); // Clear pending requests since tree items are now invalid

	if (world_list.is_empty()) {
		// Restore tooltip and signals before returning
		worlds_tree->set_block_signals(false);
		worlds_tree->set_auto_tooltip(prev_auto_tooltip);
		return;
	}

	root = worlds_tree->create_item();
	root->set_text(0, "Flecs Worlds (" + itos(world_list.size()) + ")");

	for (int i = 0; i < world_list.size(); i++) {
		RID world_rid = world_list[i];
		String world_name = _format_world_name(world_rid);

		TreeItem *world_item = worlds_tree->create_item(root);
		world_item->set_text(0, world_name);
		world_item->set_metadata(0, world_rid);
		world_item->set_selectable(0, true);

		// Restore selection
		if (world_rid == prev_selected) {
			world_item->select(0);
			selected_world = world_rid;
		}

		// Add placeholder for entities
		TreeItem *placeholder = worlds_tree->create_item(world_item);
		placeholder->set_text(0, "(click to load)");

		world_dirty[world_rid] = true;
	}

	// Restore tooltip and signals after tree manipulation is complete
	worlds_tree->set_block_signals(false);
	worlds_tree->set_auto_tooltip(prev_auto_tooltip);
}

void FlecsWorldEditorPlugin::_on_tree_item_expanded(TreeItem *item) {
	// Prevent re-entrancy during entity response handling
	if (handling_entity_response) {
		return;
	}
	
	if (!item) {
		return;
	}

	// Only load when expanding (not when collapsing)
	if (item->is_collapsed()) {
		return;
	}

	Variant meta = item->get_metadata(0);
	if (meta.get_type() != Variant::RID) {
		return;
	}

	RID world_rid = meta;
	selected_world = world_rid;

	// Check if already loaded
	if (world_dirty.has(world_rid) && !world_dirty[world_rid]) {
		return;
	}

	// Check if we're in remote mode
	if (active_session.is_valid() && active_session->is_active()) {
		_request_remote_entities(world_rid.get_id(), item);
	} else {
		_load_entities_batch(world_rid, item);
	}
}

void FlecsWorldEditorPlugin::_load_entities_batch(RID world_rid, TreeItem *world_item, int64_t batch_start) {
	if (!flecs_server || !worlds_tree) {
		return;
	}

	// CRITICAL: Disable tooltips and block signals during tree manipulation to prevent crashes
	// The tooltip system can call get_tooltip() which accesses TreeItem pointers
	// If we're deleting/creating items, those pointers may be invalid
	bool prev_auto_tooltip = worlds_tree->is_auto_tooltip_enabled();
	worlds_tree->set_auto_tooltip(false);
	worlds_tree->set_block_signals(true);

	// Clear children - must remove from tree_item_map first, then delete
	TreeItem *child = world_item->get_first_child();
	while (child) {
		TreeItem *next = child->get_next();
		// Remove from tree_item_map if it was an entity item
		if (tree_item_map.has(child)) {
			tree_item_map.erase(child);
		}
		memdelete(child);
		child = next;
	}

	// Get or create cache
	if (!world_cache.has(world_rid)) {
		world_cache[world_rid] = Dictionary();
	}

	int batch_size = int(batch_size_spinbox->get_value());

	// Load entities - placeholder implementation
	// In production, would use WorldInfo::dump_all_entities()

	// For now, add a few sample entities to show structure
	for (int i = 0; i < 5; i++) {
		int entity_id = batch_start + i;
		String entity_name = vformat("Entity_%d", entity_id);

		TreeItem *entity_item = worlds_tree->create_item(world_item);
		entity_item->set_text(0, entity_name);
		entity_item->set_selectable(0, true);

		Dictionary entity_data;
		entity_data["name"] = entity_name;
		entity_data["id"] = entity_id;
		Dictionary world_entities = world_cache[world_rid];
		world_entities[entity_id] = entity_data;
		world_cache[world_rid] = world_entities;

		Array pair;
		pair.push_back(world_rid);
		pair.push_back(entity_id);
		tree_item_map[entity_item] = pair;
	}

	world_dirty[world_rid] = false;

	// Restore tooltip and signals after tree manipulation is complete
	worlds_tree->set_block_signals(false);
	worlds_tree->set_auto_tooltip(prev_auto_tooltip);
}

void FlecsWorldEditorPlugin::_on_tree_item_selected() {
	if (!worlds_tree || !entity_inspector) {
		return;
	}

	TreeItem *selected = worlds_tree->get_selected();
	if (!selected) {
		return;
	}

	if (tree_item_map.has(selected)) {
		Array pair = tree_item_map[selected];
		selected_world = pair[0];
		selected_entity_id = pair[1];
		
		// Check if we're in remote mode
		if (active_session.is_valid() && active_session->is_active()) {
			// Defer the request to avoid race conditions with UI popup/tooltip handling
			callable_mp(this, &FlecsWorldEditorPlugin::_request_remote_entity_components)
				.bind(selected_world.get_id(), selected_entity_id)
				.call_deferred();
		} else {
			// Local mode - use FlecsServer directly
			entity_inspector->set_entity(selected_world, selected_entity_id);
		}
	} else {
		Variant meta = selected->get_metadata(0);
		if (meta.get_type() == Variant::RID) {
			selected_world = meta;
			selected_entity_id = 0; // Clear entity selection when world is selected
		}
		entity_inspector->clear_inspector();
	}

	_update_inspector();
}

void FlecsWorldEditorPlugin::_update_inspector() {
	if (!entity_inspector) {
		return;
	}

	if (selected_entity_id == 0 || !selected_world.is_valid()) {
		entity_inspector->clear_inspector();
		return;
	}

	// In remote mode, the inspector is updated via _handle_remote_components
	// In local mode, set the entity directly
	if (!remote_mode && flecs_server) {
		entity_inspector->set_entity(selected_world, selected_entity_id);
	}
}

void FlecsWorldEditorPlugin::_on_world_refresh_timer_timeout() {
	// Try to setup debugger connection if not already done
	if (!debugger_connected) {
		_setup_remote_debugger();
	}
	
	_refresh_worlds_tree();
}



void FlecsWorldEditorPlugin::_on_refresh_pressed() {
	_refresh_worlds_tree();
}

void FlecsWorldEditorPlugin::_on_expand_all_pressed() {
	if (!worlds_tree) {
		return;
	}

	TreeItem *root = worlds_tree->get_root();
	if (!root) {
		return;
	}

	Vector<TreeItem *> queue;
	queue.push_back(root);

	while (!queue.is_empty()) {
		TreeItem *item = queue[queue.size() - 1];
		queue.remove_at(queue.size() - 1);
		item->set_collapsed(false);

		TreeItem *child = item->get_first_child();
		while (child) {
			queue.push_back(child);
			child = child->get_next();
		}
	}
}

void FlecsWorldEditorPlugin::_on_collapse_all_pressed() {
	if (!worlds_tree) {
		return;
	}

	TreeItem *root = worlds_tree->get_root();
	if (!root) {
		return;
	}

	Vector<TreeItem *> queue;
	queue.push_back(root);

	while (!queue.is_empty()) {
		TreeItem *item = queue[queue.size() - 1];
		queue.remove_at(queue.size() - 1);
		item->set_collapsed(true);

		TreeItem *child = item->get_first_child();
		while (child) {
			queue.push_back(child);
			child = child->get_next();
		}
	}
}

void FlecsWorldEditorPlugin::_clear_pending_requests_for_tree(Tree *p_tree) {
	if (!p_tree || pending_entity_requests.is_empty()) {
		return;
	}
	Array keys = pending_entity_requests.keys();
	for (int i = 0; i < keys.size(); i++) {
		uint64_t world_id = keys[i];
		ObjectID oid = pending_entity_requests[world_id];
		TreeItem *item = Object::cast_to<TreeItem>(ObjectDB::get_instance(oid));
		if (!item || item->get_tree() != p_tree) {
			pending_entity_requests.erase(world_id);
		}
	}
}

bool FlecsWorldEditorPlugin::_is_pending_request_valid(uint64_t p_world_id, TreeItem *p_world_item) {
	if (!p_world_item || !p_world_item->get_tree()) {
		return false;
	}
	if (!pending_entity_requests.has(p_world_id)) {
		return false;
	}
	ObjectID oid = pending_entity_requests[p_world_id];
	TreeItem *stored_item = Object::cast_to<TreeItem>(ObjectDB::get_instance(oid));
	if (!stored_item) {
		pending_entity_requests.erase(p_world_id);
		return false;
	}
	if (stored_item != p_world_item || stored_item->get_tree() != p_world_item->get_tree()) {
		pending_entity_requests.erase(p_world_id);
		return false;
	}
	return true;
}

String FlecsWorldEditorPlugin::_format_world_name(RID world_rid) const {
	return "World [" + String::num_int64(world_rid.get_id(), 16).to_upper() + "]";
}

String FlecsWorldEditorPlugin::_format_entity_name(const String &name, uint64_t entity_id) const {
	if (name.is_empty()) {
		return "Entity#" + itos(entity_id);
	}
	return name + " (#" + itos(entity_id) + ")";
}

void FlecsWorldEditorPlugin::_on_search_text_changed(const String &p_text) {
	current_search_filter = p_text.strip_edges().to_lower();
	_apply_search_filter();
}

void FlecsWorldEditorPlugin::_apply_search_filter() {
	if (!worlds_tree) {
		return;
	}

	TreeItem *root = worlds_tree->get_root();
	if (!root) {
		return;
	}

	// Apply filter to the entire tree
	_filter_tree_item(root, current_search_filter);
}

void FlecsWorldEditorPlugin::_filter_tree_item(TreeItem *p_item, const String &p_filter) {
	if (!p_item) {
		return;
	}

	// Process all children first (bottom-up approach)
	TreeItem *child = p_item->get_first_child();
	while (child) {
		_filter_tree_item(child, p_filter);
		child = child->get_next();
	}

	// Root and world items should always be visible
	TreeItem *root = worlds_tree->get_root();
	if (p_item == root) {
		p_item->set_visible(true);
		return;
	}

	// Check if this is a world item (direct child of root)
	TreeItem *parent = p_item->get_parent();
	bool is_world_item = (parent == root);

	if (is_world_item) {
		// World items: always visible, but expand if filter is active and has matching children
		p_item->set_visible(true);
		
		if (!p_filter.is_empty()) {
			// Check if any child matches
			bool has_visible_child = false;
			TreeItem *world_child = p_item->get_first_child();
			while (world_child) {
				if (world_child->is_visible()) {
					has_visible_child = true;
					break;
				}
				world_child = world_child->get_next();
			}
			
			// Auto-expand worlds with matching entities
			if (has_visible_child) {
				p_item->set_collapsed(false);
			}
		}
	} else {
		// Entity items: filter by name
		if (p_filter.is_empty()) {
			// No filter - show all
			p_item->set_visible(true);
		} else {
			// Check if this item matches the filter
			bool matches = _item_matches_filter(p_item, p_filter);
			p_item->set_visible(matches);
		}
	}
}

bool FlecsWorldEditorPlugin::_item_matches_filter(TreeItem *p_item, const String &p_filter) const {
	if (!p_item || p_filter.is_empty()) {
		return true;
	}

	// Get the item's text and check if it contains the filter string (case-insensitive)
	String item_text = p_item->get_text(0).to_lower();
	
	// Simple contains check
	if (item_text.contains(p_filter)) {
		return true;
	}

	// Also check metadata if it's an entity (might have ID info)
	if (tree_item_map.has(p_item)) {
		Array pair = tree_item_map[p_item];
		if (pair.size() >= 2) {
			uint64_t entity_id = pair[1];
			String id_str = String::num_int64(entity_id);
			String id_hex = String::num_int64(entity_id, 16).to_lower();
			
			if (id_str.contains(p_filter) || id_hex.contains(p_filter)) {
				return true;
			}
		}
	}

	return false;
}