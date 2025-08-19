#pragma once
#include "core/object/object_id.h"
#include "core/templates/a_hash_map.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "core/object/object.h"
#include "core/os/thread.h"
#include "core/os/mutex.h"
#include "core/templates/rid.h"
#include "core/templates/rid_owner.h"
#include "core/templates/vset.h"
#include "core/variant/variant.h"
#include "core/object/ref_counted.h"
#include "flecs_script_system.h"
#include <cstdint>
#include "ecs/systems/commands/command.h"
#include "ecs/systems/pipeline_manager.h"
#include "ecs/systems/rendering/mulitmesh_render_system.h"
#include "ecs/systems/rendering/occlusion/occlusion_system.h"
#include "ecs/systems/rendering/mesh_render_system.h"
#include "core/variant/callable.h"
#include "ecs/flecs_types/flecs_variant.h"
#include "core/string/string_name.h"
#include "ecs/components/component_registry.h"
#include "ecs/utility/node_storage.h"
#include "ecs/utility/ref_storage.h"
#include <limits>
#include <optional>
#include <atomic>


#define CHECK_ENTITY_VALIDITY_V(entity_id, world_id, default_value, func_name) \
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id); \
	if (!entity_variant) { \
		ERR_PRINT("FlecsServer:: " #func_name ": entity_id is not a valid entity"); \
		return default_value; \
	} \
	flecs::entity entity = entity_variant->get_entity();

#define CHECK_WORLD_VALIDITY_V(world_id, default_value, func_name) \
	FlecsWorldVariant* world_variant = flecs_world_owners.get_or_null(world_id); \
	if (!world_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": world_id is not a valid world"); \
		return default_value; \
	} \
	flecs::world *world = &world_variant->get_world();

#define CHECK_ENTITY_VALIDITY(entity_id, world_id, func_name) \
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id); \
	if (!entity_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": entity_id is not a valid entity"); \
		return; \
	} \
	flecs::entity entity = entity_variant->get_entity();

#define CHECK_WORLD_VALIDITY(world_id, func_name) \
	FlecsWorldVariant* world_variant = flecs_world_owners.get_or_null(world_id); \
	if (!world_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": world_id is not a valid world"); \
		return; \
	} \
	flecs::world *world = &world_variant->get_world();

#define CHECK_SYSTEM_VALIDITY_V(system_id, world_id, default_value, func_name) \
	FlecsSystemVariant* system_variant = flecs_variant_owners.get(world_id).system_owner.get_or_null(system_id); \
	if (!system_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": system_id is not a valid system"); \
		return default_value; \
	} \
	flecs::system system = system_variant->get_system();

#define CHECK_SYSTEM_VALIDITY(system_id, world_id, func_name) \
	FlecsSystemVariant* system_variant = flecs_variant_owners.get(world_id).system_owner.get_or_null(system_id); \
	if (!system_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": system_id is not a valid system"); \
		return; \
	} \
	flecs::system system = system_variant->get_system();

#define CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, default_value, func_name) \
	FlecsScriptSystem* script_system = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(script_system_id); \
	if (!script_system) { \
		ERR_PRINT("FlecsServer::" #func_name ": script_system_id is not a valid script system"); \
		return default_value; \
	} \

#define CHECK_SCRIPT_SYSTEM_VALIDITY(script_system_id, world_id, func_name) \
	FlecsScriptSystem* script_system = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(script_system_id); \
	if (!script_system) { \
		ERR_PRINT("FlecsServer::" #func_name ": script_system_id is not a valid script system"); \
		return; \
	} \

#define CHECK_TYPE_ID_VALIDITY_V(type_rid, world_id, default_value, func_name) \
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(type_rid); \
	if (!type_id_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": type_id is not a valid type ID"); \
		return default_value; \
	} \
	flecs::entity_t type_id = type_id_variant->get_type();

#define CHECK_TYPE_ID_VALIDITY(type_id, world_id, func_name) \
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(type_id); \
	if (!type_id_variant) { \
		ERR_PRINT("FlecsServer::" #func_name ": type_id is not a valid type ID"); \
		return; \
	} \

constexpr static int MAX_WORLD_COUNT = std::numeric_limits<uint8_t>::max() + 1;
constexpr static int MAX_ENTITY_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_COMPONENT_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_SYSTEM_COUNT = std::numeric_limits<uint16_t>::max() + 1;
constexpr static int MAX_SCRIPT_SYSTEM_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_TYPE_ID_COUNT = std::numeric_limits<uint32_t>::max() + 1;
constexpr static int MAX_COMMAND_HANDLER_COUNT = std::numeric_limits<uint32_t>::max() + 1;


constexpr static int WORLD_OWNER_CHUNK_SIZE = 65536;
constexpr static int ENTITY_OWNER_CHUNK_SIZE = 65536;
constexpr static int COMPONENT_OWNER_CHUNK_SIZE = 65536;
constexpr static int SYSTEM_OWNER_CHUNK_SIZE = 65536;
constexpr static int SCRIPT_SYSTEM_OWNER_CHUNK_SIZE = 65536;
constexpr static int TYPE_ID_OWNER_CHUNK_SIZE = 65536;
constexpr static int COMMAND_HANDLER_OWNER_CHUNK_SIZE = 65536;



class FlecsServer : public Object {
	GDCLASS(FlecsServer, Object);
	constexpr static char render_command_handler_name[] = "Rendering";
	constexpr static char command_handler_name[] = "CommandHandler";
	constexpr static char script_system_name[] = "ScriptSystem";

	std::atomic_uint16_t counter = 0;
	static FlecsServer *singleton;
	static void thread_func(void *p_udata);
	bool thread_exited = false;
	mutable bool exit_thread;
	Thread thread;
	Mutex mutex;

	void run_script_system(const RID& world, const RID& script_system_id);

protected:
	static void _bind_methods();

public:
	static FlecsServer *get_singleton();
	Error init();
	void lock();
	void unlock();
	void finish();
	FlecsServer();
	~FlecsServer();
	RID create_world();
	int8_t get_world_count() const;
	void init_world(const RID& world_id);

	bool progress_world(const RID& world_id, const double delta);
	RID add_script_system(const RID& world_id, const Array &component_types, const Callable &callable);
	RID create_entity(const RID& world_id);
	RID create_entity_with_name(const RID& world_id, const String &name);
	RID create_entity_with_name_and_comps(const RID& world_id, const String &name, const TypedArray<RID> &components_type_ids);
	RID lookup(const RID& world_id, const String &entity_name);
	flecs::world *_get_world(const RID &world_id);
	RID get_world_of_entity(const RID &entity_id);
	void init_render_system(const RID &world_id);
	void set_log_level(const int level);
	RID register_component_type(const RID& world_id, const String &type_name, const Dictionary &script_visible_component_data);
	Ref<CommandHandler> get_render_system_command_handler(const RID &world_id);
	std::optional<PipelineManager> _get_pipeline_manager(const RID &world_id);
	void remove_all_components_from_entity(const RID &entity_id);
	bool has_component(const RID &entity_id,const String &component_type);
	PackedStringArray get_component_types_as_name(const RID &entity_id);
	TypedArray<RID> get_component_types_as_id(const RID &entity_id);

	String get_entity_name(const RID &entity_id);
	void set_entity_name(const RID& entity_id, const String &p_name);
	void set_component(const RID& entity_id, const String& component_name, const Dictionary &comp_data);
	void remove_component_from_entity_with_id(const RID &entity_id, const RID &component_type_id);
	void remove_component_from_entity_with_name(const RID &entity_id,const String &component_type);
	Dictionary get_component_by_name(const RID &entity_id, const String &component_type);
	Dictionary get_component_by_id(const RID& entity_id, const RID& component_type_id);
	RID get_component_type_by_name(const RID& entity_id, const String &component_type);
	RID get_parent(const RID &entity_id);
	void set_parent(const RID& entity_id, const RID& parent_id);
	void add_component(const RID &entity_id, const RID &comp_rid);
	void add_child(const RID &parent_id, const RID &child_id);
	void remove_child(const RID& parent_id, const RID &child_id);
	TypedArray<RID> get_children(const RID &parent_id);
	RID get_child(const RID &parent_id, int index);
	void set_children(const RID &parent_id, const TypedArray<RID> &children);
	RID get_child_by_name(const RID &parent_id,const String &name);
	void remove_child_by_name(const RID &parent_id, const String &name);
	void remove_child_by_index(const RID &parent_id, int index);
	void remove_all_children(const RID &parent_id);
	void add_relationship(const RID& entity_id, const RID &relationship);
	void remove_relationship(const RID& entity_id, const RID &relationship);
	RID get_relationship(const RID& entity_id, const String& first_entity, const String& second_entity);
	TypedArray<RID> get_relationships(const RID& entity_id);
	RID _create_rid_for_entity(const RID& world_id, const flecs::entity &entity);
	RID _create_rid_for_system(const RID& world_id, const flecs::system &system);
	RID _get_rid_for_world(const flecs::world *world);
	RID _create_rid_for_type_id(const RID& world_id, const flecs::entity_t &type_id);
	RID _create_rid_for_script_system(const RID& world_id, const FlecsScriptSystem &system);
	void free_world(const RID& world_id);
	void free_system(const RID& world_id, const RID& system_id);
	void free_script_system(const RID& world_id, const RID& script_system_id);
	void free_entity(const RID& world_id, const RID& entity_id);
	flecs::entity _get_entity(const RID& entity_id, const RID& world_id);
	void free_type_id(const RID& world_id, const RID& type_id);
	void add_to_ref_storage(const Ref<Resource> &resource, const RID &world_id);
	void remove_from_ref_storage(const RID &resource_rid, const RID &world_id);
	void add_to_node_storage(Node *node, const RID &world_id);
	void remove_from_node_storage(const  int64_t node_id, const RID &world_id);
	Ref<Resource> get_resource_from_ref_storage(const RID &resource_id, const RID &world_id);
	Node *get_node_from_node_storage(const int64_t node_id, const RID &world_id);
	RID _get_or_create_rid_for_entity(const RID &world_id, const flecs::entity &entity);
	flecs::system _get_system(const RID &system_id, const RID &world_id);
	flecs::entity_t _get_type_id(const RID &type_id, const RID &world_id);
	FlecsScriptSystem _get_script_system(const RID &script_system_id, const RID &world_id);
	void set_world_singleton_with_name(const RID &world_id, const String& comp_type, const Dictionary& comp_data);
	void set_world_singleton_with_id(const RID &world_id, const RID &comp_type_id, const Dictionary& comp_data);
	Dictionary get_world_singleton_with_name(const RID &world_id, const String& comp_type);
	Dictionary get_world_singleton_with_id(const RID &world_id, const RID &comp_type_id);



private:
	struct RID_Owner_Wrapper {
		RID world_id;
		RID_Owner<FlecsEntityVariant, true> entity_owner;
		RID_Owner<FlecsTypeIDVariant, true> type_id_owner;
		RID_Owner<FlecsSystemVariant, true> system_owner;
		RID_Owner<FlecsScriptSystem, true> script_system_owner;
		RID_Owner_Wrapper() = default;
		RID_Owner_Wrapper(RID world_id) : world_id(world_id),
			entity_owner(ENTITY_OWNER_CHUNK_SIZE, MAX_ENTITY_COUNT),
			type_id_owner(TYPE_ID_OWNER_CHUNK_SIZE, MAX_TYPE_ID_COUNT),
			system_owner(SYSTEM_OWNER_CHUNK_SIZE, MAX_SYSTEM_COUNT),
			script_system_owner(SCRIPT_SYSTEM_OWNER_CHUNK_SIZE, MAX_SCRIPT_SYSTEM_COUNT) {}
		//fun hack to get around the lack of move semantics
		RID_Owner_Wrapper(const RID_Owner_Wrapper& other) {
			List<RID> *p_owned = nullptr;
			other.entity_owner.get_owned_list(p_owned);
			if(p_owned){
				for (RID rid : *p_owned) {
					entity_owner.make_rid(FlecsEntityVariant(FlecsServer::get_singleton()->_get_entity(rid, world_id)));
				}
			}
			other.type_id_owner.get_owned_list(p_owned);
			if(p_owned) {
				for (RID rid : *p_owned) {
					type_id_owner.make_rid(FlecsTypeIDVariant(FlecsServer::get_singleton()->_get_type_id(rid, world_id)));
				}
			}
			other.system_owner.get_owned_list(p_owned);
			if(p_owned) {
				for (RID rid : *p_owned) {
					system_owner.make_rid(FlecsSystemVariant(FlecsServer::get_singleton()->_get_system(rid, world_id)));
				}
			}
			other.script_system_owner.get_owned_list(p_owned);
			if(p_owned) {
				for (RID rid : *p_owned) {
					script_system_owner.make_rid(FlecsScriptSystem(FlecsServer::get_singleton()->_get_script_system(rid, world_id)));
				}
			}
		}
		RID_Owner_Wrapper operator=(const RID_Owner_Wrapper& other) {
			if (this != &other) {
				world_id = other.world_id;

				List<RID> *p_owned = nullptr;

				type_id_owner.get_owned_list(p_owned);
				if(p_owned) {
					for(RID rid : *p_owned) {
						FlecsServer::get_singleton()->free_type_id(world_id, rid);
					}
				}
				system_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						FlecsServer::get_singleton()->free_system(world_id, rid);
					}
				}
				script_system_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						FlecsServer::get_singleton()->free_script_system(world_id, rid);
					}
				}

				other.entity_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						entity_owner.make_rid(FlecsEntityVariant(get_singleton()->_get_entity(rid, world_id)));
					}
				}
				other.type_id_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						type_id_owner.make_rid(FlecsTypeIDVariant(get_singleton()->_get_type_id(rid, world_id)));
					}
				}
				other.system_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						system_owner.make_rid(FlecsSystemVariant(get_singleton()->_get_system(rid, world_id)));
					}
				}
				other.script_system_owner.get_owned_list(p_owned);
				if(p_owned) {
					for (RID rid : *p_owned) {
						script_system_owner.make_rid(FlecsScriptSystem(get_singleton()->_get_script_system(rid, world_id)));
					}
				}
			}
			return *this;
		}
	};

	RID_Owner<FlecsWorldVariant, true> flecs_world_owners = RID_Owner<FlecsWorldVariant, true>(WORLD_OWNER_CHUNK_SIZE, MAX_WORLD_COUNT);
	Vector<RID> worlds;
	AHashMap<RID,RID_Owner_Wrapper> flecs_variant_owners = AHashMap<RID,RID_Owner_Wrapper>(MAX_WORLD_COUNT);
	Ref<CommandHandler> render_system_command_handler;
	AHashMap<RID, MultiMeshRenderSystem> multi_mesh_render_systems = AHashMap<RID, MultiMeshRenderSystem>(MAX_WORLD_COUNT);
	AHashMap<RID, MeshRenderSystem> mesh_render_systems = AHashMap<RID, MeshRenderSystem>(MAX_WORLD_COUNT);
	AHashMap<RID, OcclusionSystem> occlusion_systems = AHashMap<RID, OcclusionSystem>(MAX_WORLD_COUNT);
	AHashMap<RID, PipelineManager> pipeline_managers = AHashMap<RID, PipelineManager>(MAX_WORLD_COUNT);
	Callable command_handler_callback;
	AHashMap<RID, NodeStorage> node_storages = AHashMap<RID, NodeStorage>(MAX_WORLD_COUNT);
	AHashMap<RID, RefStorage> ref_storages = AHashMap<RID, RefStorage>(MAX_WORLD_COUNT);

};