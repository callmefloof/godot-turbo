#pragma once
#include "thirdparty/flecs/distr/flecs.h"
#include "core/templates/vector.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include <optional>

class PipelineManager {
    private:
        struct FlecsSystemContainer{
            flecs::system system;
            flecs::entity_t relationship = 0ULL;
        };
        flecs::entity pipeline;
        RID world_rid;
        Vector<FlecsSystemContainer> systems;
    public:
        PipelineManager() = default;
        PipelineManager(const RID& p_world_rid);
        virtual ~PipelineManager() = default;
        PipelineManager(const PipelineManager& rhs);
        PipelineManager& operator=(const PipelineManager& rhs);
        PipelineManager(PipelineManager&& rhs) noexcept;
        PipelineManager& operator=(PipelineManager&& rhs) noexcept;
        flecs::system* try_get_system(const String &name);
        void add_to_pipeline(flecs::system system);
        void add_to_pipeline(flecs::system system, flecs::entity_t phase);
        flecs::entity create_custom_phase(const String &phase_name, const String &depends_on = "");
        void set_world(const RID& p_world_rid);
        RID get_world();
};