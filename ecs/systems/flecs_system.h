#pragma once

#include "core/io/resource.h"
#include "core/string/ustring.h"
#include "core/object/ref_counted.h"
#include "core/object/class_db.h"
#include "thirdparty/flecs/distr/flecs.h"

class FlecsWorld;

class FlecsSystem : public Resource {
    GDCLASS(FlecsSystem, Resource);
    protected:
        flecs::world* world = nullptr;
        FlecsWorld* flecs_world_ref = nullptr;
    public:
    FlecsSystem() = default;
    virtual ~FlecsSystem() = default;
    flecs::world* _get_world() const;
    FlecsWorld *get_world() const;
	void _set_world(flecs::world *p_world);
    void set_world(FlecsWorld* p_world);
    static void _bind_methods();

};