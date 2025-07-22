#include "register_types.h"

#include "../../core/object/class_db.h"
#include "ecs/flecs_types/flecs_world.h"
#include "ecs/flecs_types/flecs_entity.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/flecs_types/flecs_component_base.h"
#include "ecs/flecs_types/flecs_script_system.h"
#include "ecs/flecs_types/flecs_world.h"
#include "ecs/components/physics/2d/2d_physics_components.h"
#include "ecs/components/physics/3d/3d_physics_components.h"
#include "ecs/components/navigation/2d/2d_navigation_components.h"
#include "ecs/components/navigation/3d/3d_navigation_components.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/scene_node_component.h"
#include "ecs/components/worldcomponents.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/resource_component.h"
#include "ecs/components/queryable_component.h"
#include "ecs/utility/world_utility.h"



#ifndef COMPONENT_REF_CLASS
#define COMPONENT_REF_CLASS\

#endif

#ifndef REGISTER_FLECS_COMPONENT_CLASS
#define REGISTER_FLECS_COMPONENT_CLASS(ClassName)\
component_register<ClassName>();\
ClassDB::register_class<FlecsComponent<ClassName>>();\
ClassDB::register_class<##ClassName##Ref>();
#endif



void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<FlecsEntity>();
	ClassDB::register_abstract_class<FlecsComponentBase>();
    ClassDB::register_class<FlecsWorld>();
	ClassDB::register_class<FlecsScriptSystem>();

	// Rendering Components: 20 - 21-jul-2025
	REGISTER_FLECS_COMPONENT_CLASS(MeshComponent)
	REGISTER_FLECS_COMPONENT_CLASS(MultiMeshComponent)
	REGISTER_FLECS_COMPONENT_CLASS(MultiMeshInstanceComponent)
	REGISTER_FLECS_COMPONENT_CLASS(ParticlesComponent)
	REGISTER_FLECS_COMPONENT_CLASS(ReflectionProbeComponent)
	REGISTER_FLECS_COMPONENT_CLASS(SkeletonComponent)
	REGISTER_FLECS_COMPONENT_CLASS(EnvironmentComponent)
	REGISTER_FLECS_COMPONENT_CLASS(CameraComponent)
	REGISTER_FLECS_COMPONENT_CLASS(CompositorComponent)
	REGISTER_FLECS_COMPONENT_CLASS(DirectionalLight3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(PointLightComponent)
	REGISTER_FLECS_COMPONENT_CLASS(LightOccluderComponent)
	REGISTER_FLECS_COMPONENT_CLASS(OmniLightComponent)
	REGISTER_FLECS_COMPONENT_CLASS(SpotLightComponent)
	REGISTER_FLECS_COMPONENT_CLASS(ViewportComponent)
	REGISTER_FLECS_COMPONENT_CLASS(VoxelGIComponent)
	REGISTER_FLECS_COMPONENT_CLASS(ScenarioComponent)
	REGISTER_FLECS_COMPONENT_CLASS(RenderInstanceComponent)
	REGISTER_FLECS_COMPONENT_CLASS(CanvasItemComponent)
	REGISTER_FLECS_COMPONENT_CLASS(OccluderComponent)

	// Physics Components
	REGISTER_FLECS_COMPONENT_CLASS(Area2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Body2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Joint2DComponent)


	REGISTER_FLECS_COMPONENT_CLASS(Area3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Body3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Joint3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(SoftBody3DComponent)


	// Navigation Components
	REGISTER_FLECS_COMPONENT_CLASS(NavAgent3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavLink3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavObstacle3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavRegion3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(SourceGeometryParser3DComponent)


	REGISTER_FLECS_COMPONENT_CLASS(NavAgent2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavLink2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavObstacle2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(NavRegion2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(SourceGeometryParser2DComponent)

	// misc
	REGISTER_FLECS_COMPONENT_CLASS(SceneNodeComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Transform2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(Transform3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(QueryableComponent)
	REGISTER_FLECS_COMPONENT_CLASS(ResourceComponent)
	REGISTER_FLECS_COMPONENT_CLASS(World3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(World2DComponent)
}

void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
   // Nothing to do here in this example.
}
template <class T>
void component_register() {
	ClassDB::register_class<FlecsComponent<T>>();
}


