#pragma once
#include "comp_base.h"
#include "thirdparty/flecs/distr/flecs.h"
#include "core/variant/variant.h"
#include "ecs/systems/rendering/occlusion/tile.h"
#include "core/templates/rid.h"
#include "core/math/vector2.h"
#include "core/math/projection.h"
#include "core/templates/vector.h"
#include "core/variant/dictionary.h"
#include "ecs/components/component_registry.h"
#include <memory>

struct MeshComponent : CompBase {
	RID mesh_id;
	Vector<RID> material_ids;
	AABB custom_aabb;

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<MeshComponent>()) {
			const MeshComponent &mesh_component = entity.get<MeshComponent>();
			dict.set("mesh_id", mesh_component.mesh_id);
			dict.set("material_ids", mesh_component.material_ids);
			dict.set("custom_aabb", mesh_component.custom_aabb);
		}else{
			ERR_PRINT("MeshComponent::to_dict: entity does not have MeshComponent");
			dict.set("mesh_id", RID());
			dict.set("material_ids", Vector<RID>());
			dict.set("custom_aabb", AABB());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<MeshComponent>()) {
			MeshComponent &mesh_component = entity.get_mut<MeshComponent>();
			mesh_component.mesh_id = dict["mesh_id"];
			mesh_component.material_ids = dict["material_ids"];
			mesh_component.custom_aabb = dict["custom_aabb"];
		} else {
			ERR_PRINT("MeshComponent::from_dict: entity does not have MeshComponent");
		}
	}

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("mesh_id", mesh_id);
		dict.set("material_ids", material_ids);
		dict.set("custom_aabb", custom_aabb);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		mesh_id = dict["mesh_id"];
		material_ids = dict["material_ids"];
		custom_aabb = dict["custom_aabb"];
	}

	StringName get_type_name() const override {
		return "MeshComponent";
	}

	MeshComponent() = default;
	~MeshComponent() = default;
	MeshComponent(const RID& id, const Vector<RID>& material_ids, const AABB& custom_aabb) : mesh_id(id) , material_ids(material_ids), custom_aabb(custom_aabb) {}
};
REGISTER_COMPONENT(MeshComponent);

struct MultiMeshComponent : CompBase {
	RID multi_mesh_id;
	uint32_t instance_count = 0U;
	
	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("multi_mesh_id", multi_mesh_id);
		dict.set("instance_count", instance_count);
		return dict;
	}
	void from_dict(const Dictionary &dict) override {
		multi_mesh_id = dict["multi_mesh_id"];
		instance_count = dict["instance_count"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<MultiMeshComponent>()) {
			const MultiMeshComponent &multi_mesh_component = entity.get<MultiMeshComponent>();
			dict.set("multi_mesh_id", multi_mesh_component.multi_mesh_id);
			dict.set("instance_count", multi_mesh_component.instance_count);
		} else {
			ERR_PRINT("MultiMeshComponent::to_dict: entity does not have MultiMeshComponent");
			dict.set("multi_mesh_id", RID());
			dict.set("instance_count", 0U);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<MultiMeshComponent>()) {
			MultiMeshComponent &multi_mesh_component = entity.get_mut<MultiMeshComponent>();
			multi_mesh_component.multi_mesh_id = dict["multi_mesh_id"];
			multi_mesh_component.instance_count = dict["instance_count"];
		} else {
			ERR_PRINT("MultiMeshComponent::from_dict: entity does not have MultiMeshComponent");
		}
	}

	StringName get_type_name() const override {
		return "MultiMeshComponent";
	}
};
REGISTER_COMPONENT(MultiMeshComponent);

struct MultiMeshInstanceComponent : CompBase {
	uint32_t index = 0; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("index", index);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		index = dict["index"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<MultiMeshInstanceComponent>()) {
			const MultiMeshInstanceComponent &instance_component = entity.get<MultiMeshInstanceComponent>();
			dict.set("index", instance_component.index);
		} else {
			ERR_PRINT("MultiMeshInstanceComponent::to_dict: entity does not have MultiMeshInstanceComponent");
			dict.set("index", 0U);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<MultiMeshInstanceComponent>()) {
			MultiMeshInstanceComponent &instance_component = entity.get_mut<MultiMeshInstanceComponent>();
			instance_component.index = dict["index"];
		} else {
			ERR_PRINT("MultiMeshInstanceComponent::from_dict: entity does not have MultiMeshInstanceComponent");
		}
	}

	StringName get_type_name() const override {
		return "MultiMeshInstanceComponent";
	}
};
REGISTER_COMPONENT(MultiMeshInstanceComponent);

struct ParticlesComponent : CompBase {
	RID particles_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("particles_id", particles_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		particles_id = dict["particles_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ParticlesComponent>()) {
			const ParticlesComponent &particles_component = entity.get<ParticlesComponent>();
			dict.set("particles_id", particles_component.particles_id);
		} else {
			ERR_PRINT("ParticlesComponent::to_dict: entity does not have ParticlesComponent");
			dict.set("particles_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ParticlesComponent>()) {
			ParticlesComponent &particles_component = entity.get_mut<ParticlesComponent>();
			particles_component.particles_id = dict["particles_id"];
		} else {
			ERR_PRINT("ParticlesComponent::from_dict: entity does not have ParticlesComponent");
		}
	}

	StringName get_type_name() const override {
		return "ParticlesComponent";
	}
};
REGISTER_COMPONENT(ParticlesComponent);

struct ReflectionProbeComponent : CompBase {
	RID probe_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("probe_id", probe_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		probe_id = dict["probe_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ReflectionProbeComponent>()) {
			const ReflectionProbeComponent &probe_component = entity.get<ReflectionProbeComponent>();
			dict.set("probe_id", probe_component.probe_id);
		} else {
			ERR_PRINT("ReflectionProbeComponent::to_dict: entity does not have ReflectionProbeComponent");
			dict.set("probe_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ReflectionProbeComponent>()) {
			ReflectionProbeComponent &probe_component = entity.get_mut<ReflectionProbeComponent>();
			probe_component.probe_id = dict["probe_id"];
		} else {
			ERR_PRINT("ReflectionProbeComponent::from_dict: entity does not have ReflectionProbeComponent");
		}
	}

	StringName get_type_name() const override {
		return "ReflectionProbeComponent";
	}
};
REGISTER_COMPONENT(ReflectionProbeComponent);

struct SkeletonComponent : CompBase {
	uint32_t bone_count = 0; // Default initialization
	RID skeleton_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("bone_count", bone_count);
		dict.set("skeleton_id", skeleton_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		bone_count = dict["bone_count"];
		skeleton_id = dict["skeleton_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<SkeletonComponent>()) {
			const SkeletonComponent &skeleton_component = entity.get<SkeletonComponent>();
			dict.set("bone_count", skeleton_component.bone_count);
			dict.set("skeleton_id", skeleton_component.skeleton_id);
		} else {
			ERR_PRINT("SkeletonComponent::to_dict: entity does not have SkeletonComponent");
			dict.set("bone_count", 0U);
			dict.set("skeleton_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SkeletonComponent>()) {
			SkeletonComponent &skeleton_component = entity.get_mut<SkeletonComponent>();
			skeleton_component.bone_count = dict["bone_count"];
			skeleton_component.skeleton_id = dict["skeleton_id"];
		} else {
			ERR_PRINT("SkeletonComponent::from_dict: entity does not have SkeletonComponent");
		}
	}

	StringName get_type_name() const override {
		return "SkeletonComponent";
	}
};
REGISTER_COMPONENT(SkeletonComponent);

struct EnvironmentComponent : CompBase {
	RID environment_id;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("environment_id", environment_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		environment_id = dict["environment_id"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<EnvironmentComponent>()) {
			const EnvironmentComponent &environment_component = entity.get<EnvironmentComponent>();
			dict.set("environment_id", environment_component.environment_id);
		} else {
			ERR_PRINT("EnvironmentComponent::to_dict: entity does not have EnvironmentComponent");
			dict.set("environment_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<EnvironmentComponent>()) {
			EnvironmentComponent &environment_component = entity.get_mut<EnvironmentComponent>();
			environment_component.environment_id = dict["environment_id"];
		} else {
			ERR_PRINT("EnvironmentComponent::from_dict: entity does not have EnvironmentComponent");
		}
	}

	StringName get_type_name() const override {
		return "EnvironmentComponent";
	}
};
REGISTER_COMPONENT(EnvironmentComponent);

struct CameraComponent : CompBase {
	RID camera_id;
	Vector<Plane> frustum;
	Vector3 position;
	float far = 0.0F;
	float near = 0.0F;
	Projection projection;
	Vector2 camera_offset;
	CameraComponent() = default;
	~CameraComponent() = default;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("camera_id", camera_id);
		dict.set("frustum", frustum);
		dict.set("position", position);
		dict.set("far", far);
		dict.set("near", near);
		dict.set("projection", projection);
		dict["camera_offset"] = camera_offset;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		camera_id = dict["camera_id"];
		frustum = dict["frustum"];
		position = dict["position"];
		far = dict["far"];
		near = dict["near"];
		projection = dict["projection"];
		camera_offset = dict["camera_offset"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<CameraComponent>()) {
			const CameraComponent &camera_component = entity.get<CameraComponent>();
			dict.set("camera_id", camera_component.camera_id);
			dict.set("frustum", camera_component.frustum);
			dict.set("position", camera_component.position);
			dict.set("far", camera_component.far);
			dict.set("near", camera_component.near);
			dict.set("projection", camera_component.projection);
			dict["camera_offset"] = camera_component.camera_offset;
		} else {
			ERR_PRINT("CameraComponent::to_dict: entity does not have CameraComponent");
			dict.set("camera_id", RID());
			dict.set("frustum", Vector<Plane>());
			dict.set("position", Vector3());
			dict.set("far", 0.0F);
			dict.set("near", 0.0F);
			dict.set("projection", Projection());
			dict["camera_offset"] = Vector2();
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<CameraComponent>()) {
			CameraComponent &camera_component = entity.get_mut<CameraComponent>();
			camera_component.camera_id = dict["camera_id"];
			camera_component.frustum = dict["frustum"];
			camera_component.position = dict["position"];
			camera_component.far = dict["far"];
			camera_component.near = dict["near"];
			camera_component.projection = dict["projection"];
			camera_component.camera_offset = dict["camera_offset"];
		} else {
			ERR_PRINT("CameraComponent::from_dict: entity does not have CameraComponent");
		}
	}

	StringName get_type_name() const override {
		return "CameraComponent";
	}
};
REGISTER_COMPONENT(CameraComponent);

struct CompositorComponent : CompBase {

	RID compositor_id;
	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("compositor_id", compositor_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		compositor_id = dict["compositor_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<CompositorComponent>()) {
			const CompositorComponent &compositor_component = entity.get<CompositorComponent>();
			dict.set("compositor_id", compositor_component.compositor_id);
		} else {
			ERR_PRINT("CompositorComponent::to_dict: entity does not have CompositorComponent");
			dict.set("compositor_name", "");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<CompositorComponent>()) {
			CompositorComponent &compositor_component = entity.get_mut<CompositorComponent>();
			compositor_component.compositor_id = dict["compositor_id"].operator RID();
		} else {
			ERR_PRINT("CompositorComponent::from_dict: entity does not have CompositorComponent");
		}
	}

	StringName get_type_name() const override {
		return "CompositorComponent";
	}
};
REGISTER_COMPONENT(CompositorComponent);

struct DirectionalLight3DComponent : CompBase {
	RID light_id; // RID for the light resource
	Color light_color;
	float intensity = 0.0f; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_id", light_id);
		dict.set("light_color", light_color);
		dict.set("intensity", intensity);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_id = dict["light_id"].operator RID();
		light_color = dict["light_color"].operator Color();
		intensity = dict["intensity"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<DirectionalLight3DComponent>()) {
			const DirectionalLight3DComponent &light_component = entity.get<DirectionalLight3DComponent>();
			dict.set("light_color", light_component.light_color);
			dict.set("intensity", light_component.intensity);
		} else {
			ERR_PRINT("DirectionalLight3DComponent::to_dict: entity does not have DirectionalLight3DComponent");
			dict.set("light_color", Color());
			dict.set("intensity", 0.0f);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<DirectionalLight3DComponent>()) {
			DirectionalLight3DComponent &light_component = entity.get_mut<DirectionalLight3DComponent>();
			light_component.light_color = dict["light_color"].operator Color();
			light_component.intensity = dict["intensity"];
		} else {
			ERR_PRINT("DirectionalLight3DComponent::from_dict: entity does not have DirectionalLight3DComponent");
		}
	}

	StringName get_type_name() const override {
		return "DirectionalLight3DComponent";
	}
};
REGISTER_COMPONENT(DirectionalLight3DComponent);

struct DirectionalLight2DComponent : CompBase {
	RID light_id; // RID for the light resource
	Color light_color;
	float intensity = 0.0f; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_color", light_color);
		dict.set("intensity", intensity);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_color = dict["light_color"].operator Color();
		intensity = dict["intensity"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<DirectionalLight2DComponent>()) {
			const DirectionalLight2DComponent &light_component = entity.get<DirectionalLight2DComponent>();
			dict.set("light_color", light_component.light_color);
			dict.set("intensity", light_component.intensity);
		} else {
			ERR_PRINT("DirectionalLight2DComponent::to_dict: entity does not have DirectionalLight2DComponent");
			dict.set("light_color", Color());
			dict.set("intensity", 0.0f);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<DirectionalLight2DComponent>()) {
			DirectionalLight2DComponent &light_component = entity.get_mut<DirectionalLight2DComponent>();
			light_component.light_color = dict["light_color"].operator Color();
			light_component.intensity = dict["intensity"];
		} else {
			ERR_PRINT("DirectionalLight2DComponent::from_dict: entity does not have DirectionalLight2DComponent");
		}
	}

	StringName get_type_name() const override {
		return "DirectionalLight2DComponent";
	}
};
REGISTER_COMPONENT(DirectionalLight2DComponent);

struct PointLightComponent : CompBase {
	RID light_id; // RID for the light resource
	Color light_color;
	float intensity = 0.0f; // Default initialization
	float range = 0.0f; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_color", light_color);
		dict.set("intensity", intensity);
		dict.set("range", range);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_color = dict["light_color"].operator Color();
		intensity = dict["intensity"];
		range = dict["range"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<PointLightComponent>()) {
			const PointLightComponent &light_component = entity.get<PointLightComponent>();
			dict.set("light_color", light_component.light_color);
			dict.set("intensity", light_component.intensity);
			dict.set("range", light_component.range);
		} else {
			ERR_PRINT("PointLightComponent::to_dict: entity does not have PointLightComponent");
			dict.set("light_color", Color());
			dict.set("intensity", 0.0f);
			dict.set("range", 0.0f);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<PointLightComponent>()) {
			PointLightComponent &light_component = entity.get_mut<PointLightComponent>();
			light_component.light_color = dict["light_color"].operator Color();
			light_component.intensity = dict["intensity"];
			light_component.range = dict["range"];
		} else {
			ERR_PRINT("PointLightComponent::from_dict: entity does not have PointLightComponent");
		}
	}

	StringName get_type_name() const override {
		return "PointLightComponent";
	}
};
REGISTER_COMPONENT(PointLightComponent);

struct LightOccluderComponent : CompBase {
	RID light_occluder_id; // RID for the light resource

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_occluder_id", light_occluder_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_occluder_id = dict["light_occluder_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<LightOccluderComponent>()) {
			const LightOccluderComponent &occluder_component = entity.get<LightOccluderComponent>();
			dict.set("light_occluder_id", occluder_component.light_occluder_id);
		} else {
			ERR_PRINT("LightOccluderComponent::to_dict: entity does not have LightOccluderComponent");
			dict.set("light_occluder_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<LightOccluderComponent>()) {
			LightOccluderComponent &occluder_component = entity.get_mut<LightOccluderComponent>();
			occluder_component.light_occluder_id = dict["light_occluder_id"].operator RID();
		} else {
			ERR_PRINT("LightOccluderComponent::from_dict: entity does not have LightOccluderComponent");
		}
	}

	StringName get_type_name() const override {
		return "LightOccluderComponent";
	}
};
REGISTER_COMPONENT(LightOccluderComponent);

struct OmniLightComponent : CompBase {
	RID light_id; // RID for the light resource
	Color light_color;
	float intensity = 0.0f; // Default initialization
	float range = 0.0f; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_id", light_id);
		dict.set("light_color", light_color);
		dict.set("intensity", intensity);
		dict.set("range", range);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_id = dict["light_id"].operator RID();
		light_color = dict["light_color"].operator Color();
		intensity = dict["intensity"];
		range = dict["range"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;

		if (entity.has<OmniLightComponent>()) {
			const OmniLightComponent &light_component = entity.get<OmniLightComponent>();
			dict.set("light_id", light_component.light_id);
			dict.set("light_color", light_component.light_color);
			dict.set("intensity", light_component.intensity);
			dict.set("range", light_component.range);
		} else {
			ERR_PRINT("OmniLightComponent::to_dict: entity does not have OmniLightComponent");
			dict.set("light_id", RID());
			dict.set("light_color", Color());
			dict.set("intensity", 0.0f);
			dict.set("range", 0.0f);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<OmniLightComponent>()) {
			OmniLightComponent &light_component = entity.get_mut<OmniLightComponent>();
			light_component.light_id = dict["light_id"].operator RID();
			light_component.light_color = dict["light_color"].operator Color();
			light_component.intensity = dict["intensity"];
			light_component.range = dict["range"];
		} else {
			ERR_PRINT("OmniLightComponent::from_dict: entity does not have OmniLightComponent");
		}
	}

	StringName get_type_name() const override {
		return "OmniLightComponent";
	}
};
REGISTER_COMPONENT(OmniLightComponent);

struct SpotLightComponent : CompBase {
	RID light_id; // RID for the light resource
	Color light_color;
	float intensity = 0.0f; // Default initialization
	float range = 0.0f; // Default initialization
	float spot_angle = 0.0f; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("light_id", light_id);
		dict.set("light_color", light_color);
		dict.set("intensity", intensity);
		dict.set("range", range);
		dict.set("spot_angle", spot_angle);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		light_id = dict["light_id"].operator RID();
		light_color = dict["light_color"].operator Color();
		intensity = dict["intensity"];
		range = dict["range"];
		spot_angle = dict["spot_angle"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<SpotLightComponent>()) {
			const SpotLightComponent &light_component = entity.get<SpotLightComponent>();
			dict.set("light_id", light_component.light_id);
			dict.set("light_color", light_component.light_color);
			dict.set("intensity", light_component.intensity);
			dict.set("range", light_component.range);
			dict.set("spot_angle", light_component.spot_angle);
		} else {
			ERR_PRINT("SpotLightComponent::to_dict: entity does not have SpotLightComponent");
			dict.set("light_id", RID());
			dict.set("light_color", Color());
			dict.set("intensity", 0.0f);
			dict.set("range", 0.0f);
			dict.set("spot_angle", 0.0f);
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<SpotLightComponent>()) {
			SpotLightComponent &light_component = entity.get_mut<SpotLightComponent>();
			light_component.light_id = dict["light_id"].operator RID();
			light_component.light_color = dict["light_color"].operator Color();
			light_component.intensity = dict["intensity"];
			light_component.range = dict["range"];
			light_component.spot_angle = dict["spot_angle"];
		} else {
			ERR_PRINT("SpotLightComponent::from_dict: entity does not have SpotLightComponent");
		}
	}

	StringName get_type_name() const override {
		return "SpotLightComponent";
	}
};
REGISTER_COMPONENT(SpotLightComponent);

struct ViewportComponent : CompBase {
	RID viewport_id; // RID for the viewport resource

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("viewport_id", viewport_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		viewport_id = dict["viewport_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ViewportComponent>()) {
			const ViewportComponent &viewport_component = entity.get<ViewportComponent>();
			dict.set("viewport_id", viewport_component.viewport_id);
		} else {
			ERR_PRINT("ViewportComponent::to_dict: entity does not have ViewportComponent");
			dict.set("viewport_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ViewportComponent>()) {
			ViewportComponent &viewport_component = entity.get_mut<ViewportComponent>();
			viewport_component.viewport_id = dict["viewport_id"].operator RID();
		} else {
			ERR_PRINT("ViewportComponent::from_dict: entity does not have ViewportComponent");
		}
	}

	StringName get_type_name() const override {
		return "ViewportComponent";
	}
};
REGISTER_COMPONENT(ViewportComponent);

struct VoxelGIComponent : CompBase {
	RID voxel_gi_id;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("voxel_gi_id", voxel_gi_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		voxel_gi_id = dict["voxel_gi_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<VoxelGIComponent>()) {
			const VoxelGIComponent &voxel_component = entity.get<VoxelGIComponent>();
			dict.set("voxel_gi_id", voxel_component.voxel_gi_id);
		} else {
			ERR_PRINT("VoxelGIComponent::to_dict: entity does not have VoxelGIComponent");
			dict.set("voxel_gi_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<VoxelGIComponent>()) {
			VoxelGIComponent &voxel_component = entity.get_mut<VoxelGIComponent>();
			voxel_component.voxel_gi_id = dict["voxel_gi_id"].operator RID();
		} else {
			ERR_PRINT("VoxelGIComponent::from_dict: entity does not have VoxelGIComponent");
		}
	}

	StringName get_type_name() const override {
		return "VoxelGIComponent";
	}
};
REGISTER_COMPONENT(VoxelGIComponent);

struct ScenarioComponent : CompBase {
	RID scenario_id; // RID for the scenario resource

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("scenario_id", scenario_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		scenario_id = dict["scenario_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<ScenarioComponent>()) {
			const ScenarioComponent &scenario_component = entity.get<ScenarioComponent>();
			dict.set("scenario_id", scenario_component.scenario_id);
		} else {
			ERR_PRINT("ScenarioComponent::to_dict: entity does not have ScenarioComponent");
			dict.set("scenario_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<ScenarioComponent>()) {
			ScenarioComponent &scenario_component = entity.get_mut<ScenarioComponent>();
			scenario_component.scenario_id = dict["scenario_id"].operator RID();
		} else {
			ERR_PRINT("ScenarioComponent::from_dict: entity does not have ScenarioComponent");
		}
	}

	StringName get_type_name() const override {
		return "ScenarioComponent";
	}
};
REGISTER_COMPONENT(ScenarioComponent);

struct RenderInstanceComponent : CompBase {
	RID instance_id; // Default initialization

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("instance_id", instance_id);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		instance_id = dict["instance_id"].operator RID();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<RenderInstanceComponent>()) {
			const RenderInstanceComponent &render_instance = entity.get<RenderInstanceComponent>();
			dict.set("instance_id", render_instance.instance_id);
		} else {
			ERR_PRINT("RenderInstanceComponent::to_dict: entity does not have RenderInstanceComponent");
			dict.set("instance_id", RID());
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<RenderInstanceComponent>()) {
			RenderInstanceComponent &render_instance = entity.get_mut<RenderInstanceComponent>();
			render_instance.instance_id = dict["instance_id"];
		} else {
			ERR_PRINT("RenderInstanceComponent::from_dict: entity does not have RenderInstanceComponent");
		}
	}

	StringName get_type_name() const override {
		return "RenderInstanceComponent";
	}
};
REGISTER_COMPONENT(RenderInstanceComponent);

struct CanvasItemComponent : CompBase {
	String item_name;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("item_name", item_name);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		item_name = dict["item_name"].operator String();
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<CanvasItemComponent>()) {
			const CanvasItemComponent &canvas_item = entity.get<CanvasItemComponent>();
			dict.set("item_name", canvas_item.item_name);
		} else {
			ERR_PRINT("CanvasItemComponent::to_dict: entity does not have CanvasItemComponent");
			dict.set("item_name", "");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<CanvasItemComponent>()) {
			CanvasItemComponent &canvas_item = entity.get_mut<CanvasItemComponent>();
			canvas_item.item_name = dict["item_name"].operator String();
		} else {
			ERR_PRINT("CanvasItemComponent::from_dict: entity does not have CanvasItemComponent");
		}
	}

	StringName get_type_name() const override {
		return "CanvasItemComponent";
	}
};
REGISTER_COMPONENT(CanvasItemComponent);

struct FrustumCulled : CompBase {

	Dictionary to_dict() const override {
		Dictionary dict;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		
	}

	StringName get_type_name() const override {
		return "FrustumCulled";
	}
};
REGISTER_COMPONENT(FrustumCulled);

struct Occluded : CompBase {

	Dictionary to_dict() const override {
		Dictionary dict;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		
	}

	StringName get_type_name() const override {
		return "Occluded";
	}
};
REGISTER_COMPONENT(Occluded);

struct MainCamera : CompBase {

	Dictionary to_dict() const override {
		Dictionary dict;
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {}

	StringName get_type_name() const override {
		return "MainCamera";
	}
};
REGISTER_COMPONENT(MainCamera);

struct Occluder : CompBase {
	RID occluder_id;
	Vector<ScreenTriangle> screen_triangles;
	PackedVector3Array vertices;
	PackedInt32Array indices;
	Occluder() = default;
	~Occluder() = default;

	Dictionary to_dict() const {
		Dictionary dict;
		dict.set("occluder_id", occluder_id);
		Array screen_triangles_array;
		for (const auto &triangle : screen_triangles) {
			screen_triangles_array.append(triangle.to_dict());
		}
		dict.set("screen_triangles", screen_triangles_array);
		dict.set("vertices", vertices);
		dict.set("indices", indices);
		return dict;
	}

	void from_dict(const Dictionary &dict) {
		occluder_id = dict["occluder_id"];
		Array screen_triangles_array = dict["screen_triangles"];
		screen_triangles.clear();
		for (int i = 0; i < screen_triangles_array.size(); ++i) {
			ScreenTriangle triangle;
			triangle.from_dict(screen_triangles_array[i]);
			screen_triangles.push_back(triangle);
		}
		vertices = dict["vertices"];
		indices = dict["indices"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Occluder>()) {
			const Occluder &occluder_component = entity.get<Occluder>();
			dict.set("occluder_id", occluder_component.occluder_id);
			Array screen_triangles_array;
			for (const auto &triangle : occluder_component.screen_triangles) {
				screen_triangles_array.append(triangle.to_dict());
			}
			dict.set("screen_triangles", screen_triangles_array);
			dict.set("vertices", occluder_component.vertices);
			dict.set("indices", occluder_component.indices);
		} else {
			ERR_PRINT("Occluder::to_dict: entity does not have Occluder");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Occluder>()) {
			Occluder &occluder_component = entity.get_mut<Occluder>();
			occluder_component.occluder_id = dict["occluder_id"];
			Array screen_triangles_array = dict["screen_triangles"];
			occluder_component.screen_triangles.clear();
			for (int i = 0; i < screen_triangles_array.size(); ++i) {
				ScreenTriangle triangle;
				triangle.from_dict(screen_triangles_array[i]);
				occluder_component.screen_triangles.push_back(triangle);
			}
			occluder_component.vertices = dict["vertices"];
			occluder_component.indices = dict["indices"];
		} else {
			ERR_PRINT("Occluder::from_dict: entity does not have Occluder");
		}
	}
};

struct Occludee : CompBase {
	AABB worldAABB;
	AABB aabb;
	Occludee() = default;
	~Occludee() = default;

	Dictionary to_dict() const override {
		Dictionary dict;
		dict.set("worldAABB", worldAABB);
		dict.set("aabb", aabb);
		return dict;
	}

	void from_dict(const Dictionary &dict) override {
		worldAABB = dict["worldAABB"];
		aabb = dict["aabb"];
	}

	Dictionary to_dict_with_entity(flecs::entity &entity) const override {
		Dictionary dict;
		if (entity.has<Occludee>()) {
			const Occludee &occludee_component = entity.get<Occludee>();
			dict.set("worldAABB", occludee_component.worldAABB);
			dict.set("aabb", occludee_component.aabb);
		} else {
			ERR_PRINT("Occludee::to_dict: entity does not have Occludee");
		}
		return dict;
	}

	void from_dict_with_entity(const Dictionary &dict, flecs::entity &entity) override {
		if (entity.has<Occludee>()) {
			Occludee &occludee_component = entity.get_mut<Occludee>();
			occludee_component.worldAABB = dict["worldAABB"];
			occludee_component.aabb = dict["aabb"];
		} else {
			ERR_PRINT("Occludee::from_dict: entity does not have Occludee");
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
	flecs::component<MainCamera> main_camera;
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
	flecs::component<Occluder> occluder;
	flecs::component<Occludee> occludee;
	flecs::component<FrustumCulled> frustum_culled;
	flecs::component<Occluded> occluded;

	explicit RenderingBaseComponents(const flecs::world *world) :
			mesh(world->component<MeshComponent>("MeshComponent")),
			multi_mesh(world->component<MultiMeshComponent>("MultiMeshComponent")),
			mesh_instance(world->component<MultiMeshInstanceComponent>("MultiMeshInstanceComponent")),
			particles(world->component<ParticlesComponent>("ParticlesComponent")),
			probe(world->component<ReflectionProbeComponent>("ReflectionProbeComponent")),
			skeleton(world->component<SkeletonComponent>("SkeletonComponent")),
			environment(world->component<EnvironmentComponent>("EnvironmentComponent")),
			camera(world->component<CameraComponent>("CameraComponent")),
			main_camera(world->component<MainCamera>("MainCamera")),
			compositor(world->component<CompositorComponent>("CompositorComponent")),
			directional_light(world->component<DirectionalLight3DComponent>("DirectionalLightComponent")),
			directional_light_2d(world->component<DirectionalLight2DComponent>("DirectionalLight2DComponent")),
			point_light(world->component<PointLightComponent>("PointLightComponent")),
			omni_light(world->component<OmniLightComponent>("OmniLightComponent")),
			spot_light(world->component<SpotLightComponent>("SpotLightComponent")),
			viewport(world->component<ViewportComponent>("ViewportComponent")),
			scenario(world->component<ScenarioComponent>("ScenarioComponent")),
			voxel_gi(world->component<VoxelGIComponent>("VoxelGIComponent")),
			instance(world->component<RenderInstanceComponent>("RenderInstanceComponent")),
			canvas_item(world->component<CanvasItemComponent>("CanvasItemComponent")),
			occluder(world->component<Occluder>("Occluder")),
			occludee(world->component<Occludee>("Occludee")),
			frustum_culled(world->component<FrustumCulled>("FrustumCulled")),
			occluded(world->component<Occluded>("Occluded"))
			{}
};