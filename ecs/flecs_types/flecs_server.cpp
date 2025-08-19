#include "flecs_server.h"

#include "component_registry.h"
#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/error/error_macros.h"
#include "core/templates/a_hash_map.h"
#include "core/templates/rid.h"
#include "core/templates/rid_owner.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "flecs.h"
#include "flecs_script_system.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/script_visible_component.h"
#include "node_storage.h"
#include "ref_storage.h"
#include "ecs/components/physics/2d/2d_physics_components.h"
#include "ecs/components/physics/3d/3d_physics_components.h"
#include "ecs/components/navigation/2d/2d_navigation_components.h"
#include "ecs/components/navigation/3d/3d_navigation_components.h"
#include "ecs/components/worldcomponents.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/visibility_component.h"
#include "ecs/components/object_instance_component.h"
#include "object_instance_component.h"
#include "ecs/components/dirty_transform.h"
#include "ecs/components/resource_component.h"
#include "ecs/components/scene_node_component.h"

#include "systems/rendering/mulitmesh_render_system.h"
#include "core/string/ustring.h"
#include "flecs_variant.h"
#include <cstdint>
#include <optional>
#include "ecs/components/rendering/rendering_components.h"

void FlecsServer::run_script_system(const RID& world_id, const RID& script_system_id) {
	FlecsScriptSystem* script_system = flecs_variant_owners.get(world_id).script_system_owner.get_or_null(script_system_id);
	if (!script_system) {
		ERR_PRINT("FlecsServer::run_script_system: script system not found");
		return;
	}
	script_system->run();
}


void FlecsServer::thread_func(void *p_udata) {

}

Error FlecsServer::init() {
	thread_exited = false;
	counter = 0;
	thread.start(FlecsServer::thread_func, this);
	return OK;
}

FlecsServer *FlecsServer::singleton = nullptr;

FlecsServer *FlecsServer::get_singleton() {
	if(!singleton){
		singleton = memnew(FlecsServer);
	}
	return singleton;
}


void FlecsServer::unlock() {
	mutex.unlock();
}

void FlecsServer::lock() {
	mutex.lock();
}

void FlecsServer::finish() {
	exit_thread = true;
	thread.wait_to_finish();
}

void FlecsServer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_world"), &FlecsServer::create_world);
	ClassDB::bind_method(D_METHOD("init_world", "world_id"), &FlecsServer::init_world);
	ClassDB::bind_method(D_METHOD("progress_world", "world_id", "delta"), &FlecsServer::progress_world);
	ClassDB::bind_method(D_METHOD("create_entity", "world_id"), &FlecsServer::create_entity);
	ClassDB::bind_method(D_METHOD("create_entity_with_name", "world_id", "name"), &FlecsServer::create_entity_with_name);
	ClassDB::bind_method(D_METHOD("create_entity_with_name_and_comps", "world_id", "name", "components_type_ids"), &FlecsServer::create_entity_with_name_and_comps);
	ClassDB::bind_method(D_METHOD("lookup", "world_id", "entity_name"), &FlecsServer::lookup);
	ClassDB::bind_method(D_METHOD("get_world_of_entity", "entity_id"), &FlecsServer::get_world_of_entity);
	//all underscore types are not exposed and are only used internally
	ClassDB::bind_method(D_METHOD("init_render_system", "world_id"), &FlecsServer::init_render_system);
	ClassDB::bind_method(D_METHOD("register_component_type", "world_id", "type_name", "script_visible_component_data"), &FlecsServer::register_component_type);
	ClassDB::bind_method(D_METHOD("add_component", "entity_id", "component_type_id"), &FlecsServer::add_component);
	ClassDB::bind_method(D_METHOD("has_component", "entity_id", "component_type"), &FlecsServer::has_component);
	ClassDB::bind_method(D_METHOD("get_render_system_command_handler", "world_id"), &FlecsServer::get_render_system_command_handler);
	ClassDB::bind_method(D_METHOD("remove_all_components_from_entity", "entity_id"), &FlecsServer::remove_all_components_from_entity);
	ClassDB::bind_method(D_METHOD("get_component_types_as_name", "entity_id"), &FlecsServer::get_component_types_as_name);
	ClassDB::bind_method(D_METHOD("get_component_types_as_id", "entity_id"), &FlecsServer::get_component_types_as_id);
	ClassDB::bind_method(D_METHOD("get_entity_name", "entity_id"), &FlecsServer::get_entity_name);
	ClassDB::bind_method(D_METHOD("set_entity_name", "entity_id", "name"), &FlecsServer::set_entity_name);
	ClassDB::bind_method(D_METHOD("set_component", "entity_id", "component_name", "comp_data"), &FlecsServer::set_component);
	ClassDB::bind_method(D_METHOD("remove_component_from_entity_with_id", "entity_id", "component_type_id"), &FlecsServer::remove_component_from_entity_with_id);
	ClassDB::bind_method(D_METHOD("remove_component_from_entity_with_name", "entity_id", "component_type"), &FlecsServer::remove_component_from_entity_with_name);
	ClassDB::bind_method(D_METHOD("get_component_by_name", "entity_id", "component_type"), &FlecsServer::get_component_by_name);
	ClassDB::bind_method(D_METHOD("get_component_by_id", "entity_id", "component_type_id"), &FlecsServer::get_component_by_id);
	ClassDB::bind_method(D_METHOD("get_component_type_by_name", "entity_id", "component_type"), &FlecsServer::get_component_type_by_name);
	ClassDB::bind_method(D_METHOD("get_parent", "entity_id"), &FlecsServer::get_parent);
	ClassDB::bind_method(D_METHOD("set_parent", "entity_id", "parent_id"), &FlecsServer::set_parent);
	ClassDB::bind_method(D_METHOD("add_child", "parent_id", "child_id"), &FlecsServer::add_child);
	ClassDB::bind_method(D_METHOD("remove_child", "child"), &FlecsServer::remove_child);
	ClassDB::bind_method(D_METHOD("get_children", "parent_id"), &FlecsServer::get_children);
	ClassDB::bind_method(D_METHOD("get_child", "parent_id", "index"), &FlecsServer::get_child);
	ClassDB::bind_method(D_METHOD("add_script_system", "world_id", "component_types", "callable"), &FlecsServer::add_script_system);


	ClassDB::bind_method(D_METHOD("set_children", "parent_id", "children"), &FlecsServer::set_children);
	ClassDB::bind_method(D_METHOD("get_child_by_name", "parent_id", "name"), &FlecsServer::get_child_by_name);
	ClassDB::bind_method(D_METHOD("remove_child_by_name", "parent_id", "name"), &FlecsServer::remove_child_by_name);
	ClassDB::bind_method(D_METHOD("remove_child_by_index", "parent_id", "index"), &FlecsServer::remove_child_by_index);
	ClassDB::bind_method(D_METHOD("remove_all_children", "parent_id"), &FlecsServer::remove_all_children);
	ClassDB::bind_method(D_METHOD("add_relationship", "entity_id", "relationship"), &FlecsServer::add_relationship);
	ClassDB::bind_method(D_METHOD("remove_relationship", "entity_id", "relationship"), &FlecsServer::remove_relationship);
	ClassDB::bind_method(D_METHOD("get_relationships", "entity_id"), &FlecsServer::get_relationships);
	ClassDB::bind_method(D_METHOD("get_relationship", "entity_id", "relationship"), &FlecsServer::get_relationship);
	ClassDB::bind_method(D_METHOD("free_world", "world_id"), &FlecsServer::free_world);
	ClassDB::bind_method(D_METHOD("free_system", "world_id", "system_id"), &FlecsServer::free_system);
	ClassDB::bind_method(D_METHOD("free_script_system", "world_id", "script_system_id"), &FlecsServer::free_script_system);
	ClassDB::bind_method(D_METHOD("free_entity", "world_id", "entity_id"), &FlecsServer::free_entity);
	ClassDB::bind_method(D_METHOD("free_type_id", "world_id", "type_id"), &FlecsServer::free_type_id);
	ClassDB::bind_method(D_METHOD("add_to_ref_storage", "resource", "world_id"), &FlecsServer::add_to_ref_storage);
	ClassDB::bind_method(D_METHOD("remove_from_ref_storage", "resource_rid", "world_id"), &FlecsServer::remove_from_ref_storage);
	ClassDB::bind_method(D_METHOD("get_resource_from_ref_storage", "resource_id", "world_id"), &FlecsServer::get_resource_from_ref_storage);
	ClassDB::bind_method(D_METHOD("add_to_node_storage", "node", "world_id"), &FlecsServer::add_to_node_storage);
	ClassDB::bind_method(D_METHOD("remove_from_node_storage", "node_id", "world_id"), &FlecsServer::remove_from_node_storage);
	ClassDB::bind_method(D_METHOD("get_node_from_node_storage", "node_id", "world_id"), &FlecsServer::get_node_from_node_storage);
	ClassDB::bind_method(D_METHOD("set_world_singleton_with_name", "world_id", "name"), &FlecsServer::set_world_singleton_with_name);
	ClassDB::bind_method(D_METHOD("set_world_singleton_with_id", "world_id", "comp_type_id", "comp_data"), &FlecsServer::set_world_singleton_with_id);
	ClassDB::bind_method(D_METHOD("get_world_singleton_with_name", "world_id", "name"), &FlecsServer::get_world_singleton_with_name);
	ClassDB::bind_method(D_METHOD("get_world_singleton_with_id", "world_id", "comp_type_id"), &FlecsServer::get_world_singleton_with_id);

	//ClassDB::bind_static_method(get_class_static(), "get_singleton", &FlecsServer::get_singleton);
}

FlecsServer::FlecsServer() {
	if(!singleton) {
		singleton = this;
	}
	worlds = Vector<RID>();
	worlds.resize(MAX_WORLD_COUNT);
	render_system_command_handler = Ref<CommandHandler>(memnew(CommandHandler));
	command_handler_callback = Callable(render_system_command_handler.ptr(), "process_command");
	exit_thread = false;
}

FlecsServer::~FlecsServer() {
	singleton = nullptr;
}

RID FlecsServer::create_world() {
	if(counter >= std::numeric_limits<uint8_t>::max()) {
		ERR_PRINT("FlecsServer::create_world: Maximum number of worlds " + itos(std::numeric_limits<uint8_t>::max()) + " reached");
		return RID();
	}


	RID flecs_world = flecs_world_owners.make_rid(FlecsWorldVariant(flecs::world()));
	CHECK_WORLD_VALIDITY_V(flecs_world, RID(), create_world);
	world->import<RenderingBaseComponents>();
	world->import<Physics2DBaseComponents>();
	world->import<Physics3DBaseComponents>();
	world->import<Navigation2DBaseComponents>();
	world->import<Navigation3DBaseComponents>();
	world->component<Transform2DComponent>();
	ComponentRegistry::bind_to_world("Transform2DComponent", world->component<Transform2DComponent>().id());
	ComponentRegistry::bind_to_world("VisibilityComponent", world->component<VisibilityComponent>().id());
	ComponentRegistry::bind_to_world("ObjectInstanceComponent", world->component<ObjectInstanceComponent>().id());
	ComponentRegistry::bind_to_world("DirtyTransform", world->component<DirtyTransform>().id());
	ComponentRegistry::bind_to_world("ResourceComponent", world->component<ResourceComponent>().id());
	ComponentRegistry::bind_to_world("SceneNodeComponent", world->component<SceneNodeComponent>().id());
	ComponentRegistry::bind_to_world("World3DComponent", world->component<World3DComponent>().id());
	ComponentRegistry::bind_to_world("World2DComponent", world->component<World2DComponent>().id());

	flecs_variant_owners.insert(flecs_world, RID_Owner_Wrapper{
		flecs_world
	});

	multi_mesh_render_systems.insert(flecs_world, MultiMeshRenderSystem());
	occlusion_systems.insert(flecs_world, OcclusionSystem());
	mesh_render_systems.insert(flecs_world, MeshRenderSystem());
	
	node_storages.insert(flecs_world, NodeStorage());
	ref_storages.insert(flecs_world, RefStorage());
	pipeline_managers.insert(flecs_world, PipelineManager());
	
	worlds.insert(counter++, flecs_world);


	return flecs_world;
}

void FlecsServer::init_world(const RID& world_id) {
	CHECK_WORLD_VALIDITY(world_id, init_world);
	world->import<flecs::stats>();
	world->set<flecs::Rest>({});
    print_line("World initialized: " + itos((uint64_t)world->c_ptr()));
	auto threads = std::thread::hardware_concurrency();
	print_line("Detected hardware concurrency: " + itos(threads));
	world->set_threads(threads);


}

bool FlecsServer::progress_world(const RID& world_id, const double delta) {
	flecs::world *world = _get_world(world_id);
	if (!world) {
		ERR_PRINT("FlecsServer::progress_world: world not found");
		return false;
	}
	List<RID> *p_owned = nullptr;
	flecs_variant_owners.get(world_id).script_system_owner.get_owned_list(p_owned);
	if(p_owned){
		for (auto &sys_id :* p_owned) {
			run_script_system(world_id, sys_id);
		}
	}

	const bool progress = world->progress(delta);

	RS::get_singleton()->call_on_render_thread(command_handler_callback);

	return progress;
}



RID FlecsServer::create_entity(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity);
	flecs::entity entity = world->entity();
	return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
}

RID FlecsServer::create_entity_with_name(const RID &world_id, const String &p_name) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	RID flecs_entity = create_entity(world_id);
	FlecsEntityVariant *flecs_entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(flecs_entity);
	if (flecs_entity_variant) {
		flecs_entity_variant->get_entity().set_name(p_name.ascii().get_data());
	}
	return flecs_entity;
}

RID FlecsServer::create_entity_with_name_and_comps(const RID& world_id, const String &name, const TypedArray<RID> &components_type_ids) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	RID flecs_entity = create_entity_with_name(world_id,name);
	for(const RID comp_type_id : components_type_ids) {
		FlecsTypeIDVariant *type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
		if (type_id_variant) {
			add_component(flecs_entity, comp_type_id);
		} else {
			ERR_PRINT("FlecsServer::create_entity_with_name_and_comp: Component type ID not found");
		}
	}
	return flecs_entity;
}

RID FlecsServer::lookup(const RID &world_id, const String &entity_name) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), create_entity_with_name);
	if (world_variant) {
		flecs::entity entity = world->lookup(entity_name.ascii().get_data());
		if (!entity.is_valid()) {
			ERR_PRINT("FlecsServer::lookup: entity not found");
			return RID();
		}
		return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
	}
	ERR_FAIL_V_MSG(RID(), "FlecsServer::lookup: world_id is not a valid world");
}

flecs::world *FlecsServer::_get_world(const RID &world_id) {
	if (!worlds.has(world_id)) {
		ERR_PRINT("FlecsServer::_get_world: world_id is not a valid world");
		return nullptr;
	}
	FlecsWorldVariant *world_variant = flecs_world_owners.get_or_null(world_id);
	if (world_variant) {
		return &world_variant->get_world();
	}
	ERR_PRINT("FlecsServer::_get_world: world_id is not a valid world");
	ERR_FAIL_V(nullptr);
	
}

RID FlecsServer::get_world_of_entity(const RID &entity_id) {
	for (auto &pair : flecs_variant_owners) {
		FlecsEntityVariant *entity_variant = pair.value.entity_owner.get_or_null(entity_id);
		if (entity_variant) {
			return pair.key;
		}
	}
	ERR_FAIL_V_MSG(RID(), "FlecsServer::get_world_of_entity: entity_id is not a valid entity");
}

void FlecsServer::init_render_system(const RID &world_id) {
	CHECK_WORLD_VALIDITY(world_id, create_entity_with_name);

	flecs::entity main_camera;

	world->each<CameraComponent>([&](flecs::entity e, const CameraComponent& cam) {
		if (e.has<MainCamera>()) {
			main_camera = e;
			return; // early out
		}
	});
	
	if(!main_camera.is_alive()){
		ERR_PRINT("Main camera not found! Cancelling init.");
		return;
	}
	multi_mesh_render_systems.get(world_id).set_world(world);
	occlusion_systems.get(world_id).set_world(world);
	mesh_render_systems.get(world_id).set_world(world);

	multi_mesh_render_systems.get(world_id).set_main_camera(main_camera);
	occlusion_systems.get(world_id).set_main_camera(main_camera);
	mesh_render_systems.get(world_id).set_main_camera(main_camera);

	auto render_command_handler = get_render_system_command_handler(world_id);
	auto pipeline_manager = _get_pipeline_manager(world_id);
	if(!pipeline_manager.has_value()) {
		ERR_PRINT("FlecsServer::init_render_system: PipelineManager not found for world_id: " + itos(world_id.get_id()));
		return;
	}
	multi_mesh_render_systems.get(world_id).create_frustum_culling(render_command_handler, pipeline_manager.value());
	occlusion_systems.get(world_id).create_occlusion_culling(render_command_handler, pipeline_manager.value());
	multi_mesh_render_systems.get(world_id).create_rendering(render_command_handler, pipeline_manager.value());
	mesh_render_systems.get(world_id).create_mesh_render_system(render_command_handler, pipeline_manager.value());
}

void FlecsServer::set_log_level(const int level) {
	flecs::log::set_level(level);
}


RID FlecsServer::register_component_type(const RID& world_id, const String &type_name, const Dictionary &script_visible_component_data) {
	CHECK_WORLD_VALIDITY_V(world_id,RID(), register_component_type);
	const char *ctype_name = String(type_name).ascii().get_data();
	ecs_component_desc_t desc = { 0 };
	desc.entity = world->entity(ctype_name);
	desc.type.size = sizeof(ScriptVisibleComponent);
	desc.type.alignment = alignof(ScriptVisibleComponent);
	flecs::entity_t comp = ecs_component_init(world->c_ptr(), &desc);
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp));
}
 RID FlecsServer::add_script_system(const RID& world_id, const Array &component_types, const Callable &callable) {
	CHECK_WORLD_VALIDITY_V(world_id, RID(), add_script_system);
	FlecsScriptSystem flecs_script_system;
	flecs_script_system.set_world(world_id);
	PackedStringArray component_names;
	component_names.resize(component_types.size());
	int count = 0;
	for (auto it = component_types.begin(); it != component_types.end(); ++it) {
		component_names.set(count, *it);
		count++;
	}
	flecs_script_system.init(world_id,component_names,callable);
	return flecs_variant_owners.get(world_id).script_system_owner.make_rid(flecs_script_system);
}



std::optional<PipelineManager> FlecsServer::_get_pipeline_manager(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, std::nullopt, _get_pipeline_manager);
	if (worlds.has(world_id)) {
		auto it = pipeline_managers.find(world_id);
		if (it != pipeline_managers.end()) {
			return it->value;
		}
	}
	ERR_FAIL_V_MSG(std::nullopt, "PipelineManager not found for world_id: " + itos(world_id.get_id()));
}

Ref<CommandHandler> FlecsServer::get_render_system_command_handler(const RID &world_id) {
	CHECK_WORLD_VALIDITY_V(world_id, nullptr, get_render_system_command_handler);
	return render_system_command_handler;
}


void FlecsServer::remove_all_components_from_entity(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_all_components_from_entity: world_id is not valid");
		return;
	}
	CHECK_ENTITY_VALIDITY(entity_id, world_id, remove_all_components_from_entity);
	entity.clear(); // Clear all components from the entity
}


Dictionary FlecsServer::get_component_by_name(const RID &entity_id, const String &component_type)  {
	Dictionary component_data;
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_by_name: world_id is not valid");
		return component_data;
	}
	FlecsEntityVariant *entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		return ComponentRegistry::to_dict(entity, StringName(component_type));
	} 
	ERR_PRINT("FlecsServer::get_component_by_name: entity_id is not a valid entity");
	return component_data;
}
bool FlecsServer::has_component(const RID& entity_id, const String &component_type) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::has_component: world_id is not valid");
		return false;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity comp_type = entity.world().component(String(component_type).ascii().get_data());
		return entity.has(comp_type);
	}
	ERR_PRINT("FlecsServer::has_component: entity_id is not a valid entity");
	return false;
}


PackedStringArray FlecsServer::get_component_types_as_name(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_types_as_name: world_id is not valid");
		return PackedStringArray();
	}
	flecs::world *world = _get_world(world_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		PackedStringArray component_types;
		entity.each([&](flecs::id type) {
			component_types.push_back(String(world->component(type).name().c_str()));
		});
		return component_types;
	}
	ERR_PRINT("FlecsServer::get_component_types_as_name: entity_id is not a valid entity");
	return PackedStringArray();
}

TypedArray<RID> FlecsServer::get_component_types_as_id(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_types_as_id: world_id is not valid");
		return TypedArray<RID>();
	}
	TypedArray<RID> component_ids;
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		entity.each([&](flecs::id type) {
			component_ids.push_back(flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(type)));
		});
		return component_ids;
	}
	ERR_PRINT("FlecsServer::get_component_types_as_id: entity_id is not a valid entity");
	return TypedArray<RID>();
}

String FlecsServer::get_entity_name(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_entity_name: world_id is not valid");
		return String();
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		return String(entity.name().c_str());
	}
	ERR_PRINT("FlecsServer::get_entity_name: entity_id is not a valid entity");
	return "ERROR";
}
 void FlecsServer::set_entity_name(const RID& entity_id, const String &p_name) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::set_component: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		entity.set_name(p_name.ascii().get_data());
	} else {
		ERR_PRINT("FlecsServer::set_entity_name: entity_id is not a valid entity");
	}
}

void FlecsServer::set_component(const RID& entity_id, const String& component_type, const Dictionary &comp_data) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::set_component: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity comp_type = entity.world().component(component_type.ascii().get_data());
		if (comp_type.is_valid()) {
			ComponentRegistry::from_dict(entity, comp_data, StringName(component_type));
		}
	} else {
		ERR_PRINT("FlecsServer::set_component: entity_id is not a valid entity");
	}
}

void FlecsServer::remove_component_from_entity_with_id(const RID &entity_id, const RID &component_id) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_id: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity_t comp_id = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_id)->get_type();
		if (comp_id) {
			entity.remove(comp_id);
		}
	} else {
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_id: entity_id is not a valid entity");
	}
}

void FlecsServer::remove_component_from_entity_with_name(const RID &entity_id, const String &component_type) {
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_name: world_id is not valid");
		return;
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity_t comp_type = entity.world().component(component_type.ascii().get_data()).id();
		if (comp_type) {
			entity.remove(comp_type);
		}
	} else {
		ERR_PRINT("FlecsServer::remove_component_from_entity_with_name: entity_id is not a valid entity");
	}
}

Dictionary FlecsServer::get_component_by_id(const RID& entity_id, const RID& component_type_id) {
	
	RID world_id = get_world_of_entity(entity_id);
	if(!world_id.is_valid()){
		ERR_PRINT("FlecsServer::get_component_by_id: world_id is not valid");
		return Dictionary();
	}
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (entity_variant) {
		flecs::entity entity = entity_variant->get_entity();
		FlecsTypeIDVariant* comp_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_type_id);
		if(comp_variant){
			flecs::entity_t comp_id = comp_variant->get_type();
			if (comp_id) {
				return ComponentRegistry::to_dict(entity, comp_id);
			}
		}
	}
	ERR_PRINT("FlecsServer::get_component_by_id: entity_id or component_type_id is not valid");
	return Dictionary();
}

RID FlecsServer::get_component_type_by_name(const RID& entity_id, const String &component_type) {
	bool is_world = flecs_world_owners.owns(entity_id);
	RID world_id = is_world ? entity_id : get_world_of_entity(entity_id);
	bool is_entity = false;
	if(!is_world){
		is_entity = flecs_variant_owners.get(world_id).entity_owner.owns(entity_id);
	}
	if(is_entity){
		CHECK_ENTITY_VALIDITY_V(entity_id, world_id, RID(), get_component_type_by_name)
		flecs::entity comp_type;
		comp_type = entity.world().component(String(component_type).ascii().get_data());
		if (comp_type.is_valid()) {
			return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp_type.id()));
		}
		ERR_FAIL_V_MSG(RID(), "Component type not found: " + component_type);
	}else if(is_world){
		CHECK_WORLD_VALIDITY_V(world_id, RID(), get_component_type_by_name)
		flecs::entity comp_type = world->component(String(component_type).ascii().get_data());
		if (comp_type.is_valid()) {
			return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(comp_type.id()));
		}
		ERR_FAIL_V_MSG(RID(), "Component type not found: " + component_type);
	}else if (!is_world && !is_entity){
		ERR_PRINT("FlecsServer::get_component_type_by_name: id is not valid");
		return RID();
	}
	else{
		CRASH_COND_MSG(is_world == true && is_entity == true, "is_world == true && is_entity == true, this should not happen.");
		return RID();
	} 
	
}

RID FlecsServer::get_parent(const RID& entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (flecs_entity) {
		flecs::entity entity = flecs_entity->get_entity();
		flecs::entity parent = entity.parent();
		if (parent.is_valid()) {
			return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(parent));
		}
	}
	ERR_FAIL_V_MSG(RID(), "Parent not found for entity_id: " + itos(entity_id.get_id()));
}

void FlecsServer::set_parent(const RID& entity_id, const RID& parent_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (flecs_entity && parent_variant) {
		flecs::entity entity = flecs_entity->get_entity();
		flecs::entity parent = parent_variant->get_entity();
		entity.add(flecs::ChildOf, parent);
	} else {
		ERR_PRINT("FlecsServer::set_parent: entity_id or parent_id is not a valid entity");
	}
}



RID FlecsServer::get_child(const RID& entity_id, int index) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* flecs_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (flecs_entity) {
		flecs::entity entity = flecs_entity->get_entity();
		int i = 0;
		flecs::entity child;
		entity.children([&](flecs::entity c) {
			if (i == index) {
				child = c;
			}
			i++;
		});
		if (child.is_valid()) {
			return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child));
		}
	}
	ERR_FAIL_V_MSG(RID(), "Child not found for entity_id: " + itos(entity_id.get_id()) + " at index: " + itos(index));
}

void FlecsServer::set_children(const RID &parent_id, const TypedArray<RID> &p_children) {
	// Clear existing children.
	remove_all_children(parent_id);
	//Add new children.
	for (int i = 0; i < p_children.size(); i++) {
		RID child_id_value = p_children[i];
		add_child(parent_id, child_id_value);
	}
}

RID FlecsServer::get_child_by_name(const RID &parent_id,const String &name){
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		RID child_rid;
		parent.children([&](flecs::entity child) {
			if (child.name() == name.ascii().get_data()) {
				child_rid = flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child));
			}
		});
		return child_rid;
	}
	ERR_FAIL_V_MSG(RID(), "Child not found for parent_id: " + itos(parent_id.get_id()) + " with name: " + name);
}

void FlecsServer::remove_child_by_name(const RID &parent_id, const String &name){
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			if (child.name() == name.ascii().get_data()) {
				child.remove(flecs::ChildOf);
			}
		});
	} else {
		ERR_PRINT("FlecsServer::remove_child_by_name: parent_id is not a valid entity");
	}
	
}
void FlecsServer::remove_child_by_index(const RID &parent_id, int index) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		int i = 0;
		parent.children([&](flecs::entity child) {
			if (i == index) {
				child.remove(flecs::ChildOf);
			}
			i++;
		});
	} else {
		ERR_PRINT("FlecsServer::remove_child_by_index: parent_id is not a valid entity");
	}
}

void FlecsServer::remove_all_children(const RID &parent_id) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			child.remove(flecs::ChildOf);
		});
	} else {
		ERR_PRINT("FlecsServer::remove_all_children: parent_id is not a valid entity");
	}
}

void FlecsServer::add_child(const RID &parent_id, const RID& child_id) {
	// Implementation for adding a child entity
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	FlecsEntityVariant* child_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(child_id);
	if (parent_variant && child_variant) {
		flecs::entity parent = parent_variant->get_entity();
		flecs::entity child = child_variant->get_entity();
		child.add(flecs::ChildOf, parent);
		return;
	}
	ERR_PRINT("FlecsServer::add_child: parent or child entity not found");
}

void FlecsServer::remove_child(const RID& parent_id, const RID &child_id) {
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant* parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	FlecsEntityVariant* child_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(child_id);
	if (parent_variant && child_variant) {
		flecs::entity parent = parent_variant->get_entity();
		flecs::entity child = child_variant->get_entity();
		if(child.parent() != parent) {
			ERR_PRINT("FlecsServer::remove_child: child is not a child of the specified parent");
			return;
		}
		child.remove(flecs::ChildOf);
		return;
	}
	ERR_PRINT("FlecsServer::remove_child: parent or child entity not found");
}

TypedArray<RID> FlecsServer::get_children(const RID &parent_id) {
	TypedArray<RID> child_array;
	RID world_id = get_world_of_entity(parent_id);
	FlecsEntityVariant *parent_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(parent_id);
	if (parent_variant) {
		flecs::entity parent = parent_variant->get_entity();
		parent.children([&](flecs::entity child) {
			child_array.push_back(flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(child)).get_id());
		});
	}
	return child_array;
}

void FlecsServer::add_component(const RID& entity_id, const RID& component_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsTypeIDVariant* type_id_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(component_id);
	if (entity_variant && type_id_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity component_type = entity.world().component(type_id_variant->get_type());
		if (component_type.is_valid()) {
			entity.add(component_type);
		} else {
			ERR_PRINT("FlecsServer::add_component: component_type is not valid");
		}
	} else {
		ERR_PRINT("FlecsServer::add_component: entity_id or component_id is not valid");
	}

}

void FlecsServer::add_relationship(const RID& entity_id, const RID &relationship) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* relationship_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(relationship);
	if (entity_variant && relationship_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity rel_entity = relationship_variant->get_entity();
		entity.add(rel_entity);
	} else {
		ERR_PRINT("FlecsServer::add_relationship: entity_id or relationship is not valid");
	}
}

void FlecsServer::remove_relationship(const RID& entity_id, const RID &relationship) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	FlecsEntityVariant* relationship_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(relationship);
	if (entity_variant && relationship_variant) {
		flecs::entity entity = entity_variant->get_entity();
		flecs::entity rel_entity = relationship_variant->get_entity();
		entity.remove(rel_entity);
	} else {
		ERR_PRINT("FlecsServer::remove_relationship: entity_id or relationship is not valid");
	}

}

RID FlecsServer::get_relationship(const RID &entity_id, const String& first_entity, const String& second_entity) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (!entity_variant) {
		ERR_PRINT("FlecsServer::get_relationship: entity_id is not valid");
		return RID();
	}
	flecs::entity entity = entity_variant->get_entity();
	flecs::entity first = entity.world().component(first_entity.ascii().get_data());
	flecs::entity second = entity.world().component(second_entity.ascii().get_data());
	if (!first.is_valid() || !second.is_valid()) {
		ERR_PRINT("FlecsServer::get_relationship: first or second entity is not valid");
		return RID();
	}
	if(!entity.has(first,second)){
		ERR_PRINT("FlecsServer::get_relationship: entity does not have the relationship between " + first_entity + " and " + second_entity);
		return RID();
	}
	const flecs::entity_t* rel_entity = static_cast<const flecs::entity_t*>(entity.get(first, second));
	if (!rel_entity) {
		ERR_PRINT("FlecsServer::get_relationship: relationship is not valid");
		return RID();
	}
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(*rel_entity));
}

TypedArray<RID> FlecsServer::get_relationships(const RID &entity_id) {
	RID world_id = get_world_of_entity(entity_id);
	FlecsEntityVariant* entity_variant = flecs_variant_owners.get(world_id).entity_owner.get_or_null(entity_id);
	if (!entity_variant) {
		ERR_PRINT("FlecsServer::get_relationships: entity_id is not valid");
		return TypedArray<RID>();
	}
	flecs::entity entity = entity_variant->get_entity();
	TypedArray<RID> relationships;
	Vector<flecs::entity_t> relationship_ids;
	List<RID> *p_owned = nullptr;
	flecs_variant_owners.get(world_id).type_id_owner.get_owned_list(p_owned);
	if(p_owned) {
		for (const RID& rid : *p_owned) {
			FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(rid);
			if(!type_variant) {
				continue;
			}
			flecs::entity_t type_id = type_variant->get_type();
			if(type_id == 0) {
				continue;
			}
			if(entity.has(type_id)) {
				relationships.append(rid);
				relationship_ids.push_back(type_id);
			}
		}
	}

	entity.children([&](flecs::entity child) {
		if(!child.is_pair()){
			return;
		}
		if(!relationship_ids.has(child.id())) {
			return;
		}
		relationships.append(_create_rid_for_type_id(world_id, child.id()));
	});

	return relationships;
}

RID FlecsServer::_create_rid_for_entity(const RID& world_id, const flecs::entity &entity) {
	return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
}

RID FlecsServer::_create_rid_for_system(const RID& world_id, const flecs::system &system) {
	return flecs_variant_owners.get(world_id).system_owner.make_rid(FlecsSystemVariant(system));
}

RID FlecsServer::_get_rid_for_world(const flecs::world *world) {
	if (!world) {
		ERR_PRINT("FlecsServer::_get_rid_for_world: world is null");
		return RID();
	}
	for(auto it = worlds.begin(); it != worlds.end(); ++it) {
		if (_get_world(*it)->c_ptr() == world->c_ptr()) {
			return *it;
		}
	}
	ERR_PRINT("FlecsServer::_get_rid_for_world: world not found");
	return RID();
}

RID FlecsServer::_create_rid_for_type_id(const RID& world_id, const flecs::entity_t &type_id) {
	return flecs_variant_owners.get(world_id).type_id_owner.make_rid(FlecsTypeIDVariant(type_id));
}

RID FlecsServer::_create_rid_for_script_system(const RID& world_id, const FlecsScriptSystem &system) {
	return flecs_variant_owners.get(world_id).script_system_owner.make_rid(system);
}

void FlecsServer::free_world(const RID& rid) {
	if (flecs_world_owners.owns(rid)) {
		List<RID> *p_owned = nullptr;
		flecs_variant_owners.get(rid).entity_owner.get_owned_list(p_owned);
		if (p_owned) {
			for (const RID& owned : *p_owned) {
				flecs_variant_owners.get(rid).entity_owner.free(owned);
			}
		}

		flecs_variant_owners.get(rid).type_id_owner.get_owned_list(p_owned);
		if (p_owned) {
			for (const RID& owned : *p_owned) {
				flecs_variant_owners.get(rid).type_id_owner.free(owned);
			}
		}

		flecs_variant_owners.get(rid).system_owner.get_owned_list(p_owned);
		if (p_owned) {
			for (const RID& owned : *p_owned) {
				flecs_variant_owners.get(rid).system_owner.free(owned);
			}
		}

		flecs_variant_owners.get(rid).script_system_owner.get_owned_list(p_owned);
		if (p_owned) {
			for (const RID& owned : *p_owned) {
				flecs_variant_owners.get(rid).script_system_owner.free(owned);
			}
		}
		flecs_variant_owners.erase(rid);

		multi_mesh_render_systems.erase(rid);
		occlusion_systems.erase(rid);
		mesh_render_systems.erase(rid);
		

		worlds.erase(rid);
		flecs_world_owners.free(rid);

		pipeline_managers.erase(rid);

		node_storages.erase(rid);
		ref_storages.erase(rid);
		return;
	}

}

void FlecsServer::free_system(const RID& world_id, const RID& system_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).system_owner.free(system_id);
	} else {
		ERR_PRINT("FlecsServer::free_system: world_id is not a valid world");
	}
}

void FlecsServer::free_script_system(const RID& world_id, const RID& script_system_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).script_system_owner.free(script_system_id);
	} else {
		ERR_PRINT("FlecsServer::free_script_system: world_id is not a valid world");
	}
}

void FlecsServer::free_entity(const RID& world_id, const RID& entity_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).entity_owner.free(entity_id);
	} else {
		ERR_PRINT("FlecsServer::free_entity: world_id is not a valid world");
	}
}

flecs::entity FlecsServer::_get_entity(const RID& entity_id, const RID& world_id) {
	CHECK_ENTITY_VALIDITY_V(entity_id, world_id, flecs::entity(), _get_entity);
	return entity;
}

void FlecsServer::free_type_id(const RID& world_id, const RID& type_id) {
	if (flecs_variant_owners.has(world_id)) {
		flecs_variant_owners.get(world_id).type_id_owner.free(type_id);
	} else {
		ERR_PRINT("FlecsServer::free_type_id: world_id is not a valid world");
	}
}

void FlecsServer::add_to_ref_storage(const Ref<Resource> &resource, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		ref_storages.get(world_id).add(resource, resource->get_rid());
	} else {
		ERR_PRINT("FlecsServer::add_to_ref_storage: world_id is not a valid world");
	}
}

void FlecsServer::remove_from_ref_storage(const RID &resource_rid, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		ref_storages.get(world_id).release(resource_rid);
	} else {
		ERR_PRINT("FlecsServer::remove_from_ref_storage: world_id is not a valid world");
	}
}

void FlecsServer::add_to_node_storage(Node *node, const RID &world_id) {
	if (node_storages.has(world_id)) {
		node_storages.get(world_id).add(node, node->get_instance_id());
	} else {
		ERR_PRINT("FlecsServer::add_to_node_storage: world_id is not a valid world");
	}
}

void FlecsServer::remove_from_node_storage(const int64_t node_id, const RID &world_id) {
	if (node_storages.has(world_id)) {
		node_storages.get(world_id).release(ObjectID(node_id));
	} else {
		ERR_PRINT("FlecsServer::remove_from_node_storage: world_id is not a valid world");
	}
}

Ref<Resource> FlecsServer::get_resource_from_ref_storage(const RID &resource_rid, const RID &world_id) {
	if (ref_storages.has(world_id)) {
		RefContainer* ref_storage = ref_storages.get(world_id).get(resource_rid);
		if(ref_storage){
			return ref_storage->resource;
		}
	} 
	ERR_PRINT("FlecsServer::get_resource_from_ref_storage: world_id is not a valid world");
	return Ref<Resource>();
	
}

Node* FlecsServer::get_node_from_node_storage(const int64_t node_id, const RID &world_id) {
	if (node_storages.has(world_id)) {
		NodeContainer* node_storage = node_storages.get(world_id).try_get(ObjectID(node_id));
		if (node_storage) {
			return node_storage->node;
		}
		ERR_PRINT("FlecsServer::get_node_from_node_storage: Node not found in storage for node_id: " + itos(node_id));
		return nullptr;
	}
	ERR_PRINT("FlecsServer::get_node_from_node_storage: world_id is not a valid world");
	return nullptr;
}

RID FlecsServer::_get_or_create_rid_for_entity(const RID &world_id, const flecs::entity &entity) {
	if (flecs_variant_owners.has(world_id)) {
		List<RID> *p_owned = nullptr;
		flecs_variant_owners.get(world_id).entity_owner.get_owned_list(p_owned);
		if (p_owned) {
			for (const RID& owned : *p_owned) {
				FlecsEntityVariant* owned_entity = flecs_variant_owners.get(world_id).entity_owner.get_or_null(owned);
				if (owned_entity && owned_entity->get_entity().id() == entity.id()) {
					return owned;
				}
			}
		}
		if(entity.is_valid()) {
			return flecs_variant_owners.get(world_id).entity_owner.make_rid(FlecsEntityVariant(entity));
		}
		ERR_PRINT("FlecsServer::_get_or_create_rid_for_entity: entity is not valid");
		return RID();

	} else {
		ERR_PRINT("FlecsServer::_get_or_create_rid_for_entity: world_id is not a valid world");
		return RID();
	}
}

flecs::system FlecsServer::_get_system(const RID &system_id, const RID &world_id) {
	CHECK_SYSTEM_VALIDITY_V(system_id, world_id, flecs::system(), _get_system);
	return system;
}

FlecsScriptSystem FlecsServer::_get_script_system(const RID &script_system_id, const RID &world_id) {
	CHECK_SCRIPT_SYSTEM_VALIDITY_V(script_system_id, world_id, FlecsScriptSystem(), _get_script_system);
	return *script_system;
}

flecs::entity_t FlecsServer::_get_type_id(const RID &entity_id, const RID &world_id) {
	CHECK_TYPE_ID_VALIDITY_V(entity_id, world_id, flecs::entity_t(), _get_type_id);
	return type_id;
}

void FlecsServer::set_world_singleton_with_name(const RID &world_id, const String& comp_type, const Dictionary& comp_data){
	RID comp_type_id = get_component_type_by_name(world_id, comp_type);
	if (!comp_type_id.is_valid()) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_name: Component type not found: " + comp_type);
		return;
	}
	set_world_singleton_with_id(world_id, comp_type_id, comp_data);
}	
void FlecsServer::set_world_singleton_with_id(const RID &world_id, const RID &comp_type_id, const Dictionary& comp_data){
	CHECK_WORLD_VALIDITY(world_id, set_world_singleton_with_id);
	FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
	if (!type_variant) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_id: Component type ID not found: " + itos(comp_type_id.get_id()));
		return;
	}
	flecs::entity_t comp_type = type_variant->get_type();
	if (!comp_type) {
		ERR_PRINT("FlecsServer::set_world_singleton_with_id: Component type is not valid");
		return;
	}
	ComponentRegistry::from_dict(world, comp_data, comp_type);
}
Dictionary FlecsServer::get_world_singleton_with_name(const RID &world_id, const String& comp_type){
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_singleton_with_name);
	RID comp_type_id = get_component_type_by_name(world_id, comp_type);
	if (!comp_type_id.is_valid()) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_name: Component type not found: " + comp_type);
		return Dictionary();
	}
	return get_world_singleton_with_id(world_id, comp_type_id);
}
Dictionary FlecsServer::get_world_singleton_with_id(const RID &world_id, const RID &comp_type_id){
	CHECK_WORLD_VALIDITY_V(world_id, Dictionary(), get_world_singleton_with_id);
	FlecsTypeIDVariant* type_variant = flecs_variant_owners.get(world_id).type_id_owner.get_or_null(comp_type_id);
	if (!type_variant) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_id: Component type ID not found: " + itos(comp_type_id.get_id()));
		return Dictionary();
	}
	flecs::entity comp_type = world->component(type_variant->get_type());
	if (!comp_type.is_valid()) {
		ERR_PRINT("FlecsServer::get_world_singleton_with_id: Component type is not valid");
		return Dictionary();
	}
	return ComponentRegistry::to_dict(world, comp_type);
}