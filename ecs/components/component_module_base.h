#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include <cassert>

template <typename ComponentsStruct>
class MultiComponentModule {
protected:
	static inline ComponentsStruct *components = nullptr;

public:
	static void initialize(flecs::world &world) {
		static ComponentsStruct _components(world);
		components = &_components;
	}

	static ComponentsStruct &get_components() {
		assert(components && "MultiComponentModule not initialized! Call initialize(world) first.");
		return *components;
	}
};
