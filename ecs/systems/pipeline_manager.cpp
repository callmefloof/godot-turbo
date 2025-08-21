#include "pipeline_manager.h"
#include "core/string/ustring.h"
#include "core/string/print_string.h"

PipelineManager::PipelineManager(flecs::world *p_world) {
	world = p_world;
	pipeline = world->get_pipeline();
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
// function added to avoid cyclic dependencies if the system implicitly is on phase flecs::OnUpdate
void PipelineManager::add_to_pipeline(flecs::system system) {
    // Store the system in the systems vector for future reference
    systems.push_back({system, flecs::OnUpdate});
    // Debug: Confirm system added to pipeline
    print_line("System added to pipeline with phase: " + String::num_int64(flecs::OnUpdate));
}

void PipelineManager::add_to_pipeline(flecs::system system, flecs::entity_t phase) {
    // Assign a name to the system if not already named
    if (system.name() == nullptr || String(system.name()).is_empty()) {
        ERR_PRINT("System must have a name before being added to the pipeline.");
        return;
    }

    // Assign the system to the specified phase
    system.add(phase);

    // Debug: Check if the world object is initialized
    if (!world) {
        ERR_PRINT("World object is not initialized.");
        return;
    }

    // Debug: Print the number of entities in the world
    print_line("Number of entities in the world: " + String::num_int64(world->count<flecs::entity>()));

    // Debug: Print the pipeline name if available
    if (pipeline.is_valid()) {
        print_line("Pipeline name: " + String(pipeline.name().c_str()));
    } else {
        ERR_PRINT("Pipeline is not valid.");
    }

    // Add the system to the pipeline
    pipeline.add(system);

    // Store the system in the systems vector for future reference
    systems.push_back({system, phase});

    // Debug: Confirm system added to pipeline
    print_line("System added to pipeline with phase: " + String::num_int64(phase));
}

flecs::entity PipelineManager::create_custom_phase(const String &phase_name, const String &depends_on) {
    // Create a custom phase entity
    flecs::entity custom_phase = world->entity(phase_name.ascii().get_data());

    // If a dependency is provided, set the phase to depend on it
    if (!depends_on.is_empty()) {
        flecs::entity dependency_phase = world->entity(depends_on.ascii().get_data());
        if (dependency_phase.is_valid()) {
            custom_phase.add(flecs::DependsOn, dependency_phase);
            print_line("Custom phase '" + phase_name + "' depends on '" + depends_on + "'.");
        } else {
            ERR_PRINT("Dependency phase not found: " + depends_on);
        }
    }

    print_line("Custom phase created: " + phase_name);
    return custom_phase;
}

void PipelineManager::set_world(flecs::world *p_world) {
    world = p_world;
    pipeline = world->get_pipeline();
}

flecs::world *PipelineManager::get_world() {
    return world;
}
