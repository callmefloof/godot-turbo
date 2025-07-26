
#ifdef DEFINE_COMPONENT_PROXY
#pragma message("DEFINE_COMPONENT_PROXY is defined here")
#else
/*#error "DEFINE_COMPONENT_PROXY not defined yet!"*/
#endif

#include "flecs_world.h"

#include "../../../../core/object/ref_counted.h"
#include "../../../../core/string/string_name.h"
#include "../../../../core/templates/oa_hash_map.h"
#include "../../../../core/variant/callable.h"
#include "flecs_script_system.h"
#include "modules/godot_turbo/ecs/components/navigation/3d/3d_navigation_components.h"
#include "modules/godot_turbo/ecs/components/physics/2d/2d_physics_components.h"
#include "modules/godot_turbo/ecs/components/physics/3d/3d_physics_components.h"
#include "modules/godot_turbo/ecs/components/queryable_component.h"
#include "modules/godot_turbo/ecs/components/rendering/rendering_components.h"
#include "modules/godot_turbo/ecs/components/script_component_registry.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"
#include "modules/godot_turbo/ecs/components/transform_2d_component.h"
#include "modules/godot_turbo/ecs/components/transform_3d_component.h"
#include "modules/godot_turbo/ecs/components/worldcomponents.h"

void FlecsWorld::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("init_world"), &FlecsWorld::init_world);
	ClassDB::bind_method(D_METHOD("progress"),&FlecsWorld::progress);
	ClassDB::bind_method(D_METHOD("entity"),&FlecsWorld::entity);
	ClassDB::bind_method(D_METHOD("entity_n"),&FlecsWorld::entity_n);
	ClassDB::bind_method(D_METHOD("entity_nc"),&FlecsWorld::entity_nc);
	ClassDB::bind_method(D_METHOD("set", "comp_ref"),&FlecsWorld::set_component);
	ClassDB::bind_method(D_METHOD("remove", "comp_ref"),&FlecsWorld::remove);
	ClassDB::bind_method(D_METHOD("remove_t", "p_name"),&FlecsWorld::remove_t);
	ClassDB::bind_method(D_METHOD("remove_all_components"),&FlecsWorld::remove_all_components);
	ClassDB::bind_method(D_METHOD("add_script_system", "component_types","callable"),&FlecsWorld::add_script_system);
	ClassDB::bind_method(D_METHOD("register_component_type", "type_name","script_visible_component_ref"), &FlecsWorld::register_component_type);



}

HashMap<StringName, ComponentTypeInfo> FlecsWorld::component_registry;

FlecsWorld::FlecsWorld(/* args */)
{
	RenderingComponentModule::initialize(world);
	Physics2DComponentModule::initialize(world);
	Physics3DComponentModule::initialize(world);
	Navigation2DComponentModule::initialize(world);
	Navigation3DComponentModule::initialize(world);
	Transform2DComponentModule::initialize(world);
	Transform3DComponentModule::initialize(world);
	World3DComponentModule::initialize(world);
	World2DComponentModule::initialize(world);
	ScriptVisibleComponentModule::initialize(world);
}

FlecsWorld::~FlecsWorld()
{
	world.quit();
}

void FlecsWorld::init_world() {
	world.import<flecs::stats>();
	world.set<flecs::Rest>({});
}

bool FlecsWorld::progress() {
	for (auto& sys : script_systems) {
		if (sys.is_null()) {
			ERR_PRINT("FlecsWorld::progress: null system");
			continue;
		};
		if (!sys.is_valid()) {
			ERR_PRINT("FlecsWorld::progress: invalid system");
			continue;
		}
		sys->run();
	}
	return world.progress();
}



Ref<FlecsEntity> FlecsWorld::entity() const {
	Ref<FlecsEntity> flecs_entity = memnew(FlecsEntity);
	flecs_entity->set_entity(world.entity());
	return flecs_entity;
}
void FlecsWorld::set_component(const Ref<FlecsComponentBase> &comp_ref) {

	if (!comp_ref.is_valid()) {
		ERR_PRINT("add_component(): Component is null or invalid.");
		return;
	}

	// Handle dynamic script-visible components
	if (comp_ref->is_dynamic()) {
		const Ref<ScriptVisibleComponentRef> dyn = comp_ref;
		ScriptVisibleComponent* data = dyn->get_data();

		if (!data) {
			ERR_PRINT("add_component(): ScriptVisibleComponent has no data.");
			return;
		}

		const StringName type_name = data->name;
		const auto* schema = ScriptComponentRegistry::get_singleton()->get_schema(type_name);

		if (!schema) {
			ERR_PRINT("add_component(): Unknown script component type: " + type_name);
			return;
		}


		// Fill in missing defaults
		for (auto it = schema->begin(); it != schema->end(); ++it ) {
			StringName field_name = it->key;
			ScriptComponentRegistry::FieldDef def = it->value;

			if (!data->fields.has(field_name)) {
				data->fields.insert(field_name, def.default_value);
			} else {
				// Optional: Validate type
				if (data->fields.getptr(field_name)->get_type() != def.type) {
					WARN_PRINT("Field '" + String(field_name) + "' has wrong type — expected " + Variant::get_type_name(def.type));
				}
			}
		}

		// Set in ECS
		// ReSharper disable once CppExpressionWithoutSideEffects
		world.set<ScriptVisibleComponent>(*data);
		dyn->set_data(data); // ensures pointer is synced
		return;
	}

	// Static typed component path
	comp_ref->commit_to_entity(Ref<FlecsEntity>(this));
}


/// these are meant for entities
/// go to bed
Ref<FlecsEntity> FlecsWorld::entity_n(const StringName &p_name) const {
	Ref<FlecsEntity> flecs_entity = entity();
	flecs_entity->set_entity_name(p_name);
	return flecs_entity;
}

Ref<FlecsEntity> FlecsWorld::entity_nc(const StringName &p_name, const Ref<FlecsComponentBase> &p_comp) const {
	Ref<FlecsEntity> flecs_entity = entity();
	flecs_entity->set_name(p_name);
	flecs_entity->set_entity_name(p_name);
	flecs_entity->set_component(p_comp);
	return flecs_entity;
}

void FlecsWorld::remove(const Ref<FlecsComponentBase> &comp_ref) {
	if (comp_ref.is_null()) {
		ERR_PRINT("FlecsWorld::remove: null comp");
		return;
	}
	if (!comp_ref.is_valid()) {
		ERR_PRINT("FlecsWorld::remove: invalid comp");
	}
	const StringName& comp_type = comp_ref->get_type_name();
	return remove_t(comp_type);
}

void FlecsWorld::remove_t(const StringName &component_type) {
	const char* c_component_type = String(component_type).ascii().get_data();
	const flecs::entity component = FlecsEntity::entity->world().lookup(c_component_type);
	if (component.is_valid()) {
		ERR_PRINT("internal flecs component type is invalid. this likely means it wasn't added.");
		return;
	}
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		if (components[i]->get_type_name() != c_component_type) {
			continue;
		}
		// ReSharper disable once CppExpressionWithoutSideEffects
		world.entity().remove(component);
		components.remove_at(i);
		return;
	}
}

void FlecsWorld::remove_all_components() {
	FlecsEntity::remove_all_components();
}

flecs::world& FlecsWorld::get_world() {
	return world;
}

Ref<FlecsEntity> FlecsWorld::wrap_entity(const flecs::entity &e) {
	Ref<FlecsEntity> entity = memnew(FlecsEntity);
	entity->set_entity(e);
	return entity;
}

void FlecsWorld::register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const {
	if (!script_visible_component_ref.is_valid() || script_visible_component_ref.is_null()) {
		ERR_PRINT("component is not valid.");
		return;
	}
	const char *ctype_name = String(type_name).ascii().get_data();
	ecs_component_desc_t desc = { 0 };
	desc.entity = world.entity(ctype_name);
	desc.type.size = sizeof(ScriptVisibleComponent);
	desc.type.alignment = alignof(ScriptVisibleComponent);
	flecs::entity_t comp = ecs_component_init(world, &desc);

	component_registry[type_name] = {
		[]() -> Ref<FlecsComponentBase> {
			return memnew(ScriptVisibleComponentRef); // or component_ref->clone()
		},
		[type_name](const flecs::entity &e, const Ref<FlecsComponentBase> &comp) {
			if (!comp.is_valid() || comp.is_null()) {
				ERR_PRINT("Invalid component passed to set callback.");
				return;
			}
			if (comp->get_type_name() != type_name) {
				ERR_PRINT("Component type mismatch.");
				return;
			}
			// Cast and set data
			auto *data = comp->get_typed_data<ScriptVisibleComponent>();
			if (!data) {
				ERR_PRINT("Component data is null or type mismatch.");
				return;
			}
			// ReSharper disable once CppExpressionWithoutSideEffects
			e.set<ScriptVisibleComponent>(*data); // Actually sets component on entity
		}
	};
}
 void FlecsWorld::add_script_system(const Array &component_types, const Callable &callable) {
	FlecsScriptSystem* sys = memnew(FlecsScriptSystem);
	sys->set_world(&world);
	Vector<String> component_names;
	component_names.resize(component_types.size());
	int count = 0;
	for (auto it = component_types.begin(); it != component_types.end(); ++it) {
		component_names.set(count, *it);
		count++;
	}
	sys->set_required_components(component_names);
	sys->set_callback(callable);
	script_systems.append(sys);
}

