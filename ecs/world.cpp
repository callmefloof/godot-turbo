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
	auto on_ready = world.lookup("OnReady");
	if (on_ready) {
		ecs_run(world.c_ptr(), on_ready.id(), 0.0f, nullptr);
	}
	on_ready.destruct(); // Prevent reuse
}

void World::_process(const double delta)
{
	world.progress(static_cast<float>(delta));
}
