#pragma once
#include "../../thirdparty/flecs/distr/flecs.h"
#include "core/string/ustring.h"
#include <cassert>

namespace godot_turbo::systems {
template<typename... Components> class AbstractSystem {
public:
	virtual ~AbstractSystem() = default;

	/// Override this for per-entity logic (if needed)
	virtual void run_each(flecs::entity e, Components &...comps) = 0;

	/// Optional: override for iter-based system
	virtual void run_iter(flecs::iter &it, Components *...comps) {
		for (int i = 0; i < it.count(); ++i) {
			run_each(it.entity(i), comps[i]...);
		}
	}

	/// Registers system logic for a specific phase
	template <typename T>
	void register_each_system(flecs::world &world, T* inst, void (T::*method)( f,String name = nullptr, flecs::entity phase = flecs::OnUpdate) {
		assert(name != nullptr && "System name must not be null");

		// Register main logic system (iter version by default)
		world.system<Components...>(name)
				.kind(phase)
				.each([this]() {})

	}

	template <typename T>
	void register_iter_system(flecs::world &world, T , String name = nullptr, flecs::entity phase = flecs::OnUpdate) {
		assert(name != nullptr && "System name must not be null");

		// Register main logic system (iter version by default)
		world.system<Components...>(name)
				.kind(phase)
				.iter([this](flecs::iter &it, Components *...comps) {
					this->run_iter(it, comps...);
				});
	}
};
}
