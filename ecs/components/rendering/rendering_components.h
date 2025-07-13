#pragma once
#include "../../../thirdparty/flecs/distr/flecs.h"
#include "../component_module_base.h"
#include "core/string/ustring.h"
#include "core/templates/rid.h"
#include "core/templates/vector.h"
#include "modules/godot_turbo/ecs/components/script_visible_component.h"

#include <servers/rendering_server.h>

struct MeshComponent : ScriptVisibleComponent{
	RID mesh_id;
	Vector<RID> material_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["mesh_id"] = mesh_id;
		dict["material_id"] = material_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "mesh_id", mesh_id, Variant::RID);
		SET_SCRIPT_COMPONENT_VALUE(dict, "material_id", material_id, Variant::RID);
	}
	virtual ~MeshComponent() {
		for (const RID &mat_id : material_id) {
			if (mat_id.is_valid()) {
				RenderingServer::get_singleton()->free(mat_id);
			}
		}
		if (mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(mesh_id);
		}
	}
};

struct MultiMeshComponent : ScriptVisibleComponent {
	RID multi_mesh_id;
	uint32_t instance_count = 0U;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["instance_count"] = instance_count;
		dict["multi_mesh_id"] = multi_mesh_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "instance_count", instance_count, Variant::INT);
		SET_SCRIPT_COMPONENT_VALUE(dict, "multi_mesh_id", multi_mesh_id, Variant::RID);
	}
	virtual ~MultiMeshComponent() {
		// Ensure that the RID is released when the component is destroyed
		if (multi_mesh_id.is_valid()) {
			RenderingServer::get_singleton()->free(multi_mesh_id);
		}
	}
};

struct MultiMeshInstanceComponent : ScriptVisibleComponent {
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["index"] = index;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "index", index, Variant::INT);
	}
	uint32_t index;
};

struct ParticlesComponent : ScriptVisibleComponent{
	RID particles_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["particles_id"] = particles_id;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "particles_id", particles_id, Variant::RID);
	}
	virtual ~ParticlesComponent() {
		if (particles_id.is_valid()) {
			RenderingServer::get_singleton()->free(particles_id);
		}
	}
};
struct ReflectionProbeComponent : ScriptVisibleComponent{
	RID probe_id;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["probe_id"] = probe_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "probe_id", probe_id, Variant::RID);
	}
	virtual ~ReflectionProbeComponent() {
		if (probe_id.is_valid()) {
			RenderingServer::get_singleton()->free(probe_id);
		}
	}
};
struct SkeletonComponent : ScriptVisibleComponent {
	RID skeleton_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["skeleton_id"] = skeleton_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "skeleton_id", skeleton_id, Variant::RID);
	}
	virtual ~SkeletonComponent() {
		if (skeleton_id.is_valid()) {
			RenderingServer::get_singleton()->free(skeleton_id);
		}
	}
};
struct EnvironmentComponent : ScriptVisibleComponent{
	RID environment_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["environment_id"] = environment_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "environment_id", environment_id, Variant::RID);
	}
	virtual ~EnvironmentComponent() {
		if (environment_id.is_valid()) {
			RenderingServer::get_singleton()->free(environment_id);
		}
	}
};
struct CameraComponent : ScriptVisibleComponent{
	RID camera_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["camera_id"] = camera_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "camera_id", camera_id, Variant::RID);
	}
	virtual ~CameraComponent() {
		if (camera_id.is_valid()) {
			RenderingServer::get_singleton()->free(camera_id);
		}
	}
};
struct CompositorComponent : ScriptVisibleComponent{
	RID compositor_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["compositor_id"] = compositor_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "compositor_id", compositor_id, Variant::RID);
	}
	virtual ~CompositorComponent() {
		if (compositor_id.is_valid()) {
			RenderingServer::get_singleton()->free(compositor_id);
		}
	}
};
struct DirectionalLight3DComponent : ScriptVisibleComponent {
	RID directional_light_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["directional_light_id"] = directional_light_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "directional_light_id", directional_light_id, Variant::RID);
	}
	virtual ~DirectionalLight3DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

struct DirectionalLight2DComponent : ScriptVisibleComponent {
	RID directional_light_id;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict["directional_light_id"] = directional_light_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "directional_light_id", directional_light_id, Variant::RID);
	}
	virtual ~DirectionalLight2DComponent() {
		if (directional_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(directional_light_id);
		}
	}
};

struct PointLightComponent : ScriptVisibleComponent{
	RID point_light_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["point_light_id"] = point_light_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "point_light_id", point_light_id, Variant::RID);
	}
	virtual ~PointLightComponent() {
		if (point_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(point_light_id);
		}
	}
};

struct LightOccluderComponent : ScriptVisibleComponent {
	RID light_occluder_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["light_occluder_id"] = light_occluder_id;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "light_occluder_id", light_occluder_id, Variant::RID);
	}
	virtual ~LightOccluderComponent() {
		if (light_occluder_id.is_valid()) {
			RenderingServer::get_singleton()->free(light_occluder_id);
		}
	}
};

struct OmniLightComponent : ScriptVisibleComponent {
	RID omni_light_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["omni_light_id"] = omni_light_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "omni_light_id", omni_light_id, Variant::RID);
	}
	virtual ~OmniLightComponent() {
		if (omni_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(omni_light_id);
		}
	}
};
struct SpotLightComponent : ScriptVisibleComponent{
	RID spot_light_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["spot_light_id"] = spot_light_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "spot_light_id", spot_light_id, Variant::RID);
	}
	virtual ~SpotLightComponent() {
		if (spot_light_id.is_valid()) {
			RenderingServer::get_singleton()->free(spot_light_id);
		}
	}
};
struct ViewportComponent : ScriptVisibleComponent{
	RID viewport_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["viewport_id"] = viewport_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "viewport_id", viewport_id, Variant::RID);
	}
};
struct VoxelGIComponent : ScriptVisibleComponent {
	RID voxel_gi_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["voxel_gi_id"] = voxel_gi_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "voxel_gi_id", voxel_gi_id, Variant::RID);
	}
	virtual ~VoxelGIComponent() {
		if (voxel_gi_id.is_valid()) {
			RenderingServer::get_singleton()->free(voxel_gi_id);
		}
	}
};
struct ScenarioComponent : ScriptVisibleComponent{
	RID id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["id"] = id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "id", id, Variant::RID);
	}
};

struct RenderInstanceComponent : ScriptVisibleComponent{
	RID instance_id;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["instance_id"] = instance_id;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "instance_id", instance_id, Variant::RID);
	}
	virtual ~RenderInstanceComponent() {
		if (instance_id.is_valid()) {
			RenderingServer::get_singleton()->free(instance_id);
		}
	}
};

struct CanvasItemComponent : ScriptVisibleComponent {
	RID canvas_item_id;
	String class_name;
	Dictionary to_dict() const override{
		Dictionary dict;
		dict["canvas_item_id"] = canvas_item_id;
		dict["class_name"] = class_name;
		return dict;
	}
	void from_dict(Dictionary dict) override {
		SET_SCRIPT_COMPONENT_VALUE(dict, "canvas_item_id", canvas_item_id, Variant::RID);
		SET_SCRIPT_COMPONENT_VALUE(dict, "class_name", class_name, Variant::STRING);
	}
	virtual ~CanvasItemComponent() {
		if (canvas_item_id.is_valid()) {
			RenderingServer::get_singleton()->canvas_item_clear(canvas_item_id);
			RenderingServer::get_singleton()->free(canvas_item_id);
		}
	}
};

struct RenderingBaseComponents{
	flecs::component<MeshComponent> mesh;
	flecs::component<MultiMeshComponent> multi_mesh;
	flecs::component<MultiMeshInstanceComponent> mesh_instance;
	flecs::component<ParticlesComponent> particles;
	flecs::component<ReflectionProbeComponent> probe;
	flecs::component<SkeletonComponent> skeleton;
	flecs::component<EnvironmentComponent> environment;
	flecs::component<CameraComponent> camera;
	flecs::component<CompositorComponent> compositor;
	flecs::component<DirectionalLight3DComponent> directional_light;
	flecs::component<DirectionalLight2DComponent> directional_light_2d;
	flecs::component<PointLightComponent> point_light;
	flecs::component<OmniLightComponent> omni_light;
	flecs::component<SpotLightComponent> spot_light;
	flecs::component<ViewportComponent> viewport;
	flecs::component<ScenarioComponent> scenario;

	flecs::component<VoxelGIComponent> voxel_gi;
	flecs::component<RenderInstanceComponent> instance;
	flecs::component<CanvasItemComponent> canvas_item;

	explicit RenderingBaseComponents(const flecs::world &world) :
			mesh(world.component<MeshComponent>("MeshComponent")),
			multi_mesh(world.component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world.component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world.component<ParticlesComponent>("ParticlesComponent")),
			probe(world.component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world.component<SkeletonComponent>("SkeletonComponent")),
			environment(world.component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world.component<CameraComponent>("CameraComponent")),
			compositor(world.component<CompositorComponent>("CompositorComponent")),
			directional_light(world.component<DirectionalLight3DComponent>("DirectionalLightComponent")),
			directional_light_2d(world.component<DirectionalLight2DComponent>("DirectionalLight2DComponent")),
			point_light(world.component<PointLightComponent>("PointLightComponent")),
			omni_light(world.component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world.component<SpotLightComponent>("SpotLightComponent")),
			viewport(world.component<ViewportComponent>("ViewportComponent")),
			scenario(world.component<ScenarioComponent>("ScenarioComponent")),
			voxel_gi(world.component<VoxelGIComponent>("VoxelGIComponent")),
			instance(world.component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas_item(world.component<CanvasItemComponent>("CanvasItemComponent")) {}
};

using RenderingComponentModule = MultiComponentModule<RenderingBaseComponents>;
