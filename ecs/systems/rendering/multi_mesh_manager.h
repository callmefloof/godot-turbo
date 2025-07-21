//
// Created by Floof on 20-7-2025.
//

#ifndef MULTI_MESH_MANAGER_H
#define MULTI_MESH_MANAGER_H
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../../../../../core/object/ref_counted.h"
#include "../../../../../scene/resources/mesh.h"
#include "../../../../../core/math/transform_3d.h"
#include "../../../../../scene/resources/material.h"
#include "../../../../../core/templates/hash_map.h"


class MultiMeshManager {
public:
	// Registers a new entity with given mesh and material
	void add_instance(flecs::world &world, Ref<Mesh> mesh, Ref<Material> material, const Transform3D& transform);

	// Updates the transform of an existing entity
	void update_instance(Entity entity, const Transform3D& transform);

	// Removes the instance (clears from MultiMesh, updates mapping)
	void remove_instance(Entity entity);

	// Hides the entity by moving it off-screen
	void hide_instance(Entity entity);

private:
	HashMap<uint64_t, MultiMeshBatch> batches;

	uint64_t get_batch_key(const Ref<Mesh>& mesh, const Ref<Material>& material) const;
	MultiMeshBatch& get_or_create_batch(const Ref<Mesh>& mesh, const Ref<Material>& material);
};
#endif //MULTI_MESH_MANAGER_H
