#include "flecs_world.h"
#ifdef DEFINE_COMPONENT_PROXY
#pragma message("DEFINE_COMPONENT_PROXY is defined here")
#else
#error "DEFINE_COMPONENT_PROXY not defined yet!"
#endif
void FlecsWorld::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("init_world"), &FlecsWorld::init_world);
	ClassDB::bind_method(D_METHOD("progress"),&FlecsWorld::progress);
}

FlecsWorld::FlecsWorld(/* args */)
{
	
}

FlecsWorld::~FlecsWorld()
{
	world.quit();
}

void FlecsWorld::init_world() {
	world.import<flecs::stats>();
	world.set<flecs::Rest>({});
}

bool FlecsWorld::progress() const {
	return world.progress();
}



Ref<FlecsEntity> FlecsWorld::entity() const {
	Ref<FlecsEntity> flecs_entity = memnew(FlecsEntity);
	flecs_entity->set_entity(world.entity());
	return flecs_entity;
}
void FlecsWorld::set(const StringName &component_type, const Ref<FlecsComponentBase> &data) {

}


/// these are meant for entities
/// go to bed
Ref<FlecsEntity> FlecsWorld::entity(const StringName &name) {
	Ref<FlecsEntity> flecs_entity = entity();

	flecs_entity->set_name(name);
	return flecs_entity;
}
Ref<FlecsEntity> FlecsWorld::entity(const StringName &name, const StringName &component_type) {
	Ref<FlecsEntity> flecs_entity = entity(name);
	flecs_entity->set(component_type);
	return flecs_entity;
}
Ref<FlecsEntity> FlecsWorld::entity(const StringName &name, const StringName &component_type, const Dictionary &data) {
	Ref<FlecsEntity> flecs_entity = entity(name);
	flecs_entity->set(component_type, data);
	return flecs_entity;
}
Ref<FlecsEntity> FlecsWorld::entity(const StringName &name, const StringName &component_type, const Ref<FlecsComponentBase> &data) {
	Ref<FlecsEntity> flecs_entity = entity(name);
	flecs_entity->set(component_type, data);
	return flecs_entity;
}

void FlecsWorld::remove(const String &component_type) {
	//
}
void FlecsWorld::remove_all_components() {
	FlecsEntity::remove_all_components();
}
Ref<FlecsComponentBase> FlecsWorld::get(const String &component_type) {
	return FlecsEntity::get(component_type);
}
PackedStringArray FlecsWorld::get_component_types() {
	return FlecsEntity::get_component_types();
}

void FlecsWorld::set(const StringName &component_type) {
	FlecsEntity::set(component_type);
}


flecs::world *FlecsWorld::get_world() {
	return &world;
}
