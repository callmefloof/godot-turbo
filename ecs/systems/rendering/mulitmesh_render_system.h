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
		q.each([&](const flecs::iter& it) {
			//auto mmc = it.
			//auto t3d = it.field<Transform3DComponent>(it);
		});
	});
}
inline MultiMeshRenderSystem::~MultiMeshRenderSystem() {
}