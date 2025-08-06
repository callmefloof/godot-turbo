#include "register_types.h"

#include "../../core/object/class_db.h"
#include "ecs/components/navigation/2d/2d_navigation_components.h"
#include "ecs/components/navigation/3d/3d_navigation_components.h"
#include "ecs/components/physics/2d/2d_physics_components.h"
#include "ecs/components/physics/3d/3d_physics_components.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/resource_component.h"
#include "ecs/components/scene_node_component.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/worldcomponents.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/flecs_types/flecs_component_base.h"
#include "ecs/flecs_types/flecs_entity.h"
#include "ecs/flecs_types/flecs_script_system.h"
#include "ecs/flecs_types/flecs_world.h"
#include "ecs/utility/world_utility.h"
#include "systems/rendering/mulitmesh_render_system.h"
#include "visibility_component.h"
#include "ecs/utility/scene_object_utility.h"
#include "ecs/utility/resource_object_utility.h"

#include "ecs/utility/navigation/2d/navigation2d_utility.h"
#include "ecs/utility/navigation/3d/navigation3d_utility.h"
#include "ecs/utility/physics/2d/physics2d_utility.h"
#include "ecs/utility/physics/3d/physics3d_utility.h"

#include "ecs/utility/rendering/2d/render_utility_2d.h"
#include "ecs/utility/rendering/3d/render_utility_3d.h"

#ifndef COMPONENT_REF_CLASS
#define COMPONENT_REF_CLASS\

#endif

#ifndef REGISTER_FLECS_COMPONENT_CLASS
#define REGISTER_FLECS_COMPONENT_CLASS(ClassName)\
ClassDB::register_class<ClassName##Ref>();
#endif


//Macro for
#define INSTANTIATE_TYPED_ACCESSOR(Component) \
template Component& FlecsComponentBase::get_typed_data<Component>() const;


// We need to do this to initalize getting the typed data of a component
// Rendering Components
INSTANTIATE_TYPED_ACCESSOR(MeshComponent)
INSTANTIATE_TYPED_ACCESSOR(MultiMeshComponent)
INSTANTIATE_TYPED_ACCESSOR(MultiMeshInstanceComponent)
INSTANTIATE_TYPED_ACCESSOR(ParticlesComponent)
INSTANTIATE_TYPED_ACCESSOR(ReflectionProbeComponent)
INSTANTIATE_TYPED_ACCESSOR(SkeletonComponent)
INSTANTIATE_TYPED_ACCESSOR(EnvironmentComponent)
INSTANTIATE_TYPED_ACCESSOR(CameraComponent)
INSTANTIATE_TYPED_ACCESSOR(CompositorComponent)
INSTANTIATE_TYPED_ACCESSOR(DirectionalLight3DComponent)
INSTANTIATE_TYPED_ACCESSOR(PointLightComponent)
INSTANTIATE_TYPED_ACCESSOR(LightOccluderComponent)
INSTANTIATE_TYPED_ACCESSOR(OmniLightComponent)
INSTANTIATE_TYPED_ACCESSOR(SpotLightComponent)
INSTANTIATE_TYPED_ACCESSOR(ViewportComponent)
INSTANTIATE_TYPED_ACCESSOR(VoxelGIComponent)
INSTANTIATE_TYPED_ACCESSOR(ScenarioComponent)
INSTANTIATE_TYPED_ACCESSOR(RenderInstanceComponent)
INSTANTIATE_TYPED_ACCESSOR(CanvasItemComponent)

// Physics Components
INSTANTIATE_TYPED_ACCESSOR(Area2DComponent)
INSTANTIATE_TYPED_ACCESSOR(Body2DComponent)
INSTANTIATE_TYPED_ACCESSOR(Joint2DComponent)


INSTANTIATE_TYPED_ACCESSOR(Area3DComponent)
INSTANTIATE_TYPED_ACCESSOR(Body3DComponent)
INSTANTIATE_TYPED_ACCESSOR(Joint3DComponent)
INSTANTIATE_TYPED_ACCESSOR(SoftBody3DComponent)

// Navigation Components
INSTANTIATE_TYPED_ACCESSOR(NavAgent3DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavLink3DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavObstacle3DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavRegion3DComponent)
INSTANTIATE_TYPED_ACCESSOR(SourceGeometryParser3DComponent)


INSTANTIATE_TYPED_ACCESSOR(NavAgent2DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavLink2DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavObstacle2DComponent)
INSTANTIATE_TYPED_ACCESSOR(NavRegion2DComponent)
INSTANTIATE_TYPED_ACCESSOR(SourceGeometryParser2DComponent)

// misc
INSTANTIATE_TYPED_ACCESSOR(SceneNodeComponent)
INSTANTIATE_TYPED_ACCESSOR(Transform2DComponent)
INSTANTIATE_TYPED_ACCESSOR(Transform3DComponent)
INSTANTIATE_TYPED_ACCESSOR(ResourceComponent)
INSTANTIATE_TYPED_ACCESSOR(World3DComponent)
INSTANTIATE_TYPED_ACCESSOR(World2DComponent)
INSTANTIATE_TYPED_ACCESSOR(VisibilityComponent)



void initialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	ClassDB::register_class<FlecsEntity>();
	ClassDB::register_abstract_class<FlecsComponentBase>();
    ClassDB::register_class<FlecsWorld>();
	ClassDB::register_class<FlecsScriptSystem>();
	ClassDB::register_runtime_class<FlecsSystem>();
	ClassDB::register_runtime_class<RenderSystem>();
	ClassDB::register_runtime_class<MultiMeshRenderSystem>();
	ClassDB::register_runtime_class<OcclusionSystem>();
	ClassDB::register_class<RenderUtility2D>();
	ClassDB::register_class<RenderUtility3D>();
	ClassDB::register_class<Physics3DUtility>();
	ClassDB::register_class<Physics2DUtility>();
	ClassDB::register_class<Navigation2DUtility>();
	ClassDB::register_class<Navigation3DUtility>();
	ClassDB::register_class<World3DUtility>();
	ClassDB::register_class<World2DUtility>();
	ClassDB::register_class<SceneObjectUtility>();
	ClassDB::register_class<ResourceObjectUtility>();

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
	REGISTER_FLECS_COMPONENT_CLASS(ResourceComponent)
	REGISTER_FLECS_COMPONENT_CLASS(World3DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(World2DComponent)
	REGISTER_FLECS_COMPONENT_CLASS(VisibilityComponent)

}

void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
   // Nothing to do here in this example.
}


