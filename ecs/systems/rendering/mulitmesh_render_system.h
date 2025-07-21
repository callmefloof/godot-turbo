#pragma once
#include "../../../../../core/string/string_name.h"
#include "../../../../../core/string/ustring.h"

#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/rendering/rendering_components.h"
#include "../../components/transform_3d_component.h"

class MultiMeshRenderSystem {

protected:
	flecs::world &world;
public:
	MultiMeshRenderSystem(flecs::world &w) : world(w) {};
	void initialize(const String& system_name);
	virtual ~MultiMeshRenderSystem();


};
inline void MultiMeshRenderSystem::initialize(const String& system_name) {
	world.system<const MultiMeshComponent>()
	.name(system_name.ascii().get_data())
	.multi_threaded()
	.each([&](const MultiMeshComponent& mmc) {
		auto q = world.query_builder<const MultiMeshInstanceComponent, const Transform3DComponent>().cache_kind(flecs::QueryCacheDefault).build();
		q.each([&](const MultiMeshInstanceComponent& mmc, const Transform3DComponent& t) {
			mmc.
		});
	});
}
inline MultiMeshRenderSystem::~MultiMeshRenderSystem() {
}