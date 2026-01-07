#ifndef FLECS_EDITOR_PLUGIN_H
#define FLECS_EDITOR_PLUGIN_H

#include "editor/plugins/editor_plugin.h"
#include "editor/debugger/editor_debugger_plugin.h"
#include "scene/gui/box_container.h"
#include "scene/gui/tree.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/line_edit.h"
#include "flecs_entity_inspector.h"

class FlecsServer;
class FlecsProfiler;
class Timer;
class EditorDebuggerSession;
class ScriptEditorDebugger;
class FlecsWorldEditorPlugin;

/**
 * @class FlecsDebuggerBridge
 * @brief EditorDebuggerPlugin for capturing Flecs messages from runtime
 */
class FlecsDebuggerBridge : public EditorDebuggerPlugin {
	GDCLASS(FlecsDebuggerBridge, EditorDebuggerPlugin);

private:
	FlecsWorldEditorPlugin *editor_plugin = nullptr;

protected:
	static void _bind_methods() {}

public:
	void set_editor_plugin(FlecsWorldEditorPlugin *p_plugin) { editor_plugin = p_plugin; }
	virtual bool has_capture(const String &p_capture) const override { return _has_capture(p_capture); }
	virtual bool capture(const String &p_message, const Array &p_data, int p_session) override { return _capture(p_message, p_data, p_session); }

	virtual bool _has_capture(const String &p_capture) const;
	virtual bool _capture(const String &p_message, const Array &p_data, int p_session);

	FlecsDebuggerBridge() {}
	~FlecsDebuggerBridge() {}
};

/**
 * @class FlecsWorldEditorPlugin
 * @brief EditorPlugin that provides a Flecs world inspector dock
 *
 * Registers the Flecs World Editor dock with the Godot editor.
 * The actual UI is built dynamically at editor initialization time.
 * 
 * Features:
 * - World/entity tree browser
 * - Entity search/filter functionality
 * - Entity component inspector
 * - Remote debugging support for inspecting running games
 */
class FlecsWorldEditorPlugin : public EditorPlugin {
	GDCLASS(FlecsWorldEditorPlugin, EditorPlugin);

protected:
	static void _bind_methods();
	virtual void _notification(int p_what);

	void _on_enter_tree();
	void _on_exit_tree();

	// Remote debugging
	void _setup_remote_debugger();
	void _teardown_remote_debugger();
	void _attach_to_session(ScriptEditorDebugger *p_debugger);
	void _on_debugger_session_stopped();
	void _on_session_started();

public:
	// Called by FlecsDebuggerBridge
	bool capture_remote_message(const String &p_message, const Array &p_data);

private:
	void _request_remote_worlds();
	void _handle_remote_worlds(const Array &p_data);
	void _request_remote_entities(uint64_t p_world_id, TreeItem *p_world_item);
	void _handle_remote_entities(const Array &p_data);
	void _request_remote_entity_components(uint64_t p_world_id, uint64_t p_entity_id);
	void _handle_remote_components(const Array &p_data);
	void _handle_profiler_metrics(const Dictionary &p_data);

public:
	FlecsWorldEditorPlugin();
	~FlecsWorldEditorPlugin();

	virtual String get_plugin_name() const override;
	virtual bool has_main_screen() const override { return false; }

private:
	FlecsServer *flecs_server = nullptr;
	VBoxContainer *dock = nullptr;
	Tree *worlds_tree = nullptr;
	LineEdit *search_field = nullptr;
	String current_search_filter;
	FlecsEntityInspector *entity_inspector = nullptr;
	Timer *world_refresh_timer = nullptr;
	SpinBox *batch_size_spinbox = nullptr;

	// Remote debugging
	Ref<EditorDebuggerSession> remote_session;
	Ref<EditorDebuggerSession> active_session;
	Ref<FlecsDebuggerBridge> debugger_plugin;
	bool remote_mode = false;
	bool debugger_connected = false;

	// Cache structures
	Dictionary world_cache;      // RID -> {entity_id -> entity_data}
	Dictionary world_dirty;      // RID -> bool
	Dictionary tree_item_map;    // TreeItem -> (world_rid, entity_id)
	Dictionary pending_entity_requests; // world_id -> ObjectID (TreeItem in tree)

	// Guard flag to prevent re-entrancy during entity response handling
	bool handling_entity_response = false;

	RID selected_world;
	uint64_t selected_entity_id = 0;

	static constexpr int ENTITIES_PER_PAGE = 200;

	// UI Building
	void _build_dock_ui();
	void _refresh_worlds_tree();
	void _on_tree_item_expanded(TreeItem *item);
	void _on_tree_item_selected();
	void _on_refresh_pressed();
	void _on_expand_all_pressed();
	void _on_collapse_all_pressed();
	void _on_world_refresh_timer_timeout();
	void _on_search_text_changed(const String &p_text);
	void _apply_search_filter();
	void _filter_tree_item(TreeItem *p_item, const String &p_filter);
	bool _item_matches_filter(TreeItem *p_item, const String &p_filter) const;
	void _load_entities_batch(RID world_rid, TreeItem *world_item, int64_t batch_start = 0);
	void _update_inspector();

	// Utility
	void _clear_pending_requests_for_tree(Tree *p_tree);
	bool _is_pending_request_valid(uint64_t p_world_id, TreeItem *p_world_item);
	String _format_world_name(RID world_rid) const;
	String _format_entity_name(const String &name, uint64_t entity_id) const;

public:
	// Expose world data for profiler integration
	TypedArray<RID> get_available_worlds() const;
	bool is_remote_mode() const { return remote_mode; }
	Ref<EditorDebuggerSession> get_active_session() const { return active_session; }
	
	// Singleton access for other plugins
	static FlecsWorldEditorPlugin *get_singleton();
	
	// Profiler integration
	void set_profiler(FlecsProfiler *p_profiler) { profiler = p_profiler; }

private:
	static FlecsWorldEditorPlugin *singleton;
	FlecsProfiler *profiler = nullptr;
};

#endif // FLECS_EDITOR_PLUGIN_H