//
// Created by Floof on 13-7-2025.
//

#ifndef FLECS_SYSTEM_H
#define FLECS_SYSTEM_H
#include "../../../../core/object/object.h"
#include "../../../../core/object/ref_counted.h"
#include "../../../../core/io/resource.h"

class FlecsSystem : public Resource {
	GDCLASS(FlecsSystem,Resource)
	public:
	FlecsSystem();
	static Ref<FlecsSystem> create();
	void each();
};

#endif //FLECS_SYSTEM_H
