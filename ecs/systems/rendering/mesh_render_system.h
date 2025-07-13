#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/transform_3d_component.h"
#include "../../components/rendering/rendering_components.h"
#include "servers/rendering_server.h"

class MeshRenderSystem
{

protected:
	RenderingServer *rendering_server;
	flecs::world &world;
public:
	explicit MeshRenderSystem(flecs::world& world) :
			rendering_server(RenderingServer::get_singleton()),
			world(world){
		assert(rendering_server != nullptr && "RenderingServer must not be null");
	}
	void register_system() const {
		world.system<const RenderInstanceComponent, const MeshComponent, const Transform3DComponent>("MeshRenderSystem")
				.kind(flecs::OnUpdate)
				.run([this](const flecs::iter it) {
					const flecs::field<RenderInstanceComponent> render_instances = it.field<RenderInstanceComponent>(1);
					const flecs::field<MeshComponent> meshes = it.field<MeshComponent>(2);
					const flecs::field<Transform3DComponent> transforms = it.field<Transform3DComponent>(3);

					for (int i = 0; i < it.count(); ++i) {
						const auto &[instance_id] = render_instances[i];
						const auto &mesh = meshes[i];
						const auto &[transform] = transforms[i];

						rendering_server->instance_set_transform(instance_id, transform);
					}
				});
	}
};


