#include "world.h"

void World::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("init_world"), &World::init_world);
	ClassDB::bind_method(D_METHOD("progress"),&World::progress);
}

World::World(/* args */)
{
	
}

World::~World()
{
	world.quit();
}

void World::init_world() {
	world.import<flecs::stats>();
	world.set<flecs::Rest>({});
}

void World::progress() {
	world.progress();
}

flecs::world *World::get_world() {
	return &world;
}
