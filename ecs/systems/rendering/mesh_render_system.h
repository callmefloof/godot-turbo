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
	MeshRenderSystem(flecs::world& world) :
			rendering_server(RenderingServer::get_singleton()),
			world(world){
		assert(rendering_server != nullptr && "RenderingServer must not be null");
	}
	void register_system()
	{
		world.system<const RenderInstanceComponent, const MeshComponent, const Transform3DComponent>("MeshRenderSystem")
				.kind(flecs::OnUpdate)
				.run([this](flecs::iter it) {
					auto render_instances = it.field<RenderInstanceComponent>(1);
					auto meshes = it.field<MeshComponent>(2);
					auto transforms = it.field<Transform3DComponent>(3);

					for (int i = 0; i < it.count(); ++i) {
						const auto &render_instance = render_instances[i];
						const auto &mesh = meshes[i];
						const auto &transform = transforms[i];

						rendering_server->instance_set_transform(render_instance.instance_id, transform.transform);
					}
				});
	}
};


