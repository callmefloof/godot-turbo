#include "pipeline_manager.h"
#include "core/string/ustring.h"

PipelineManager::PipelineManager(flecs::world &p_world) {
	world = p_world;
	pipeline = world.get_pipeline();
}

PipelineManager::PipelineManager(const PipelineManager &rhs) :
		world(rhs.world) {
	this->pipeline = rhs.pipeline;
	this->systems = rhs.systems;
}

PipelineManager &PipelineManager::operator=(const PipelineManager &rhs) {
	if (this == &rhs) {
		return *this;
	}
	this->world = rhs.world;
	this->pipeline = rhs.pipeline;
	this->systems = rhs.systems;
	return *this;
}

PipelineManager::PipelineManager(PipelineManager &&rhs) noexcept : world(std::move(rhs.world)) {
	this->pipeline = std::move(rhs.pipeline);
	this->systems = std::move(rhs.systems);
}

PipelineManager &PipelineManager::operator=(PipelineManager &&rhs) noexcept {
	if (this == &rhs) {
		return *this;
	}
	this->world = std::move(rhs.world);
	this->pipeline = std::move(rhs.pipeline);
	this->systems = std::move(rhs.systems);
	return *this;
}

flecs::system *PipelineManager::try_get_system(const String &name) {
	for (auto& render_system : systems){
		if(render_system.system.name() == name.ascii().get_data()){
			return  &render_system.system;
		}
	}
	return nullptr;
}

void PipelineManager::add_to_pipeline(const flecs::system& system, flecs::entity_t phase, String dependency) {
    // Assign the system to the specified phase
    system.add(flecs::DependsOn, phase);

    // If a dependency is provided, ensure the system runs after it
    if (!dependency.is_empty()) {
        flecs::entity dep_entity = world.lookup(dependency.ascii().get_data());
        if (dep_entity.is_alive()) {
            system.add(flecs::DependsOn, dep_entity);
        }
    }

    // Add the system to the pipeline
    pipeline.add(system);

    // Store the system in the systems vector for future reference
    systems.push_back({system, phase});
}