#include "world.h"

void World::_bind_methods()
{
    
}

World::World(/* args */)
{
	world.app()
	// Optional, gather statistics for explorer
	.enable_stats()
	.enable_rest()
	.run();
}

World::~World()
{
}

void World::_ready()
{
	
}

void World::_process(const double delta)
{
	world.progress(static_cast<float>(delta));
}
