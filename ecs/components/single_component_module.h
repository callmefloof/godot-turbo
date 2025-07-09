#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include <cassert>



template <typename T>
class SingleComponentModule {
private:
	static inline const flecs::component<T> *component = nullptr;

public:
	static void initialize(flecs::world &world, const char *name = nullptr) {
		static flecs::component<T> _component = name ? world.component<T>(name) : world.component<T>();

		component = &_component;
	}

	static const flecs::component<T> &get() {
		assert(component != nullptr && "Component not initialized! Call initialize(world) first.");
		return *component;
	}
};
