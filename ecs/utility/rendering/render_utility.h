#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../components/rendering/rendering_components.h"
#include "scene/resources/mesh.h"
#include "core/templates/vector.h"
using namespace godot_turbo::components::rendering;

namespace godot_turbo::utility::rendering
{
	class RenderUtility {
	public:
		static inline flecs::entity_t CreateMesh(flecs::world &world, Ref<Mesh> &mesh) {
			mesh->get_rid();
			Vector<RID> material_ids;
			for (int i = 0; i < mesh->get_surface_count(); ++i) {
				Ref<Material> material = mesh->surface_get_material(i);
				if (material.is_valid()) {
					material_ids.push_back(material->get_rid());
				}
			}

			flecs::entity_t mesh_entity = world.entity(mesh->get_name()).set<MeshComponent>({ mesh->get_rid(), std::move(material_ids) });
			return mesh_entity;
		}

		static inline flecs::entity_t CreateMultiMesh(flecs::world &world, const uint32_t instance_count) {
			return world.entity("MultiMesh")
					.set<MultiMeshComponent>({ instance_count });
		}

		static inline flecs::entity_t CreateMultiMeshInstance(flecs::world &world, const uint32_t index) {
			return world.entity("MultiMeshInstance")
					.set<MultiMeshInstanceComponent>({ index });
		}

		static inline flecs::entity_t CreateParticles(flecs::world &world, const RID &particles_id) {
			return world.entity("Particles")
					.set<ParticlesComponent>({ particles_id });
		}
	};
	
};
	
