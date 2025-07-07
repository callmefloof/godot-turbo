#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/transform_3d_component.h"
#include "../../components/rendering/rendering_components.h"
#include "servers/rendering_server.h"
#include "../abstract_system.h"

namespace godot_turbo::systems::rendering
{
	using namespace godot_turbo::systems;
	using namespace godot_turbo::components::rendering;
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
			world.system<const RenderInstanceComponent, const MeshComponent, const MeshInstanceComponent,
						 const Transform3DComponent>("MeshRenderSystem")
					.kind(flecs::OnUpdate)
					.iter([this](flecs::iter &it, const RenderInstanceComponent *render_instances,
								  const MeshComponent *meshes, const MeshInstanceComponent *mesh_instances,
								  const Transform3DComponent *transforms) {
						for (int i = 0; i < it.count(); ++i) {
							auto &render_instance = render_instances[i];
							auto &mesh = meshes[i];
							auto &mesh_instance = mesh_instances[i];
							auto &transform = transforms[i];
							// Call the rendering server to draw the mesh
							rendering_server->instance_set_transform(render_instance.instance_id, transform.transform);
							rendering_server->instance(render_instance.instance_id, mesh.mesh_id);
							rendering_server->instance_set_material(render_instance.instance_id, mesh.material_id[0]);
						}
					});
		}
	};
}

