//
// Created by Floof on 13-7-2025.
//

#ifndef FLECS_SYSTEM_H
#define FLECS_SYSTEM_H
#include "../../../../core/object/ref_counted.h"

class FlecsSystem : public RefCounted {

	public:
	FlecsSystem();
	virtual ~FlecsSystem() = default;
	static Ref<FlecsSystem> create();
	void each()
};

#endif //FLECS_SYSTEM_H
