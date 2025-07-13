#include "../flecs_types/flecs_world.h"

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

void FlecsWorld::progress() const {
	world.progress();
}

flecs::world *FlecsWorld::get_world() {
	return &world;
}
