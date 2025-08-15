
#ifdef DEFINE_COMPONENT_PROXY
#pragma message("DEFINE_COMPONENT_PROXY is defined here")
#else
/*#error "DEFINE_COMPONENT_PROXY not defined yet!"*/
#endif

#include "flecs_world.h"

#include "../../../../core/object/ref_counted.h"
#include "../../../../core/string/string_name.h"
#include "flecs_script_system.h"
#include "ecs/components/navigation/3d/3d_navigation_components.h"
#include "ecs/components/physics/2d/2d_physics_components.h"
#include "ecs/components/physics/3d/3d_physics_components.h"
#include "ecs/components/rendering/rendering_components.h"
#include "ecs/components/script_component_registry.h"
#include "ecs/components/script_visible_component.h"
#include "ecs/components/transform_2d_component.h"
#include "ecs/components/transform_3d_component.h"
#include "ecs/components/worldcomponents.h"
#include "ecs/components/object_instance_component.h"
#include "systems/rendering/mulitmesh_render_system.h"
#include "ecs/utility/ref_storage.h"
#include "ecs/utility/node_storage.h"


void FlecsWorld::_bind_methods()
{

	ClassDB::bind_method(D_METHOD("init_world"), &FlecsWorld::init_world);
	ClassDB::bind_method(D_METHOD("progress", "delta"),&FlecsWorld::progress);
	ClassDB::bind_method(D_METHOD("create_entity"),&FlecsWorld::create_entity);
	ClassDB::bind_method(D_METHOD("create_entity_n"),&FlecsWorld::create_entity_n);
	ClassDB::bind_method(D_METHOD("create_entity_nc"),&FlecsWorld::create_entity_nc);
	ClassDB::bind_method(D_METHOD("set_component", "comp_ref"),&FlecsWorld::set_component);
	ClassDB::bind_method(D_METHOD("add_script_system", "component_types", "callable"), &FlecsWorld::add_script_system);
	ClassDB::bind_method(D_METHOD("register_component_type", "type_name", "script_visible_component_ref"), &FlecsWorld::register_component_type);
	ClassDB::bind_method(D_METHOD("init_render_system"), &FlecsWorld::init_render_system);
	ClassDB::bind_method(D_METHOD("get_component", "component_type"), &FlecsWorld::get_component);
	ClassDB::bind_method(D_METHOD("has_component", "component_type"), &FlecsWorld::has_component);
	ClassDB::bind_method(D_METHOD("get_component_types"), &FlecsWorld::get_component_types);
	ClassDB::bind_method(D_METHOD("set_log_level", "level"), &FlecsWorld::set_log_level);
	ClassDB::bind_method(D_METHOD("add_relationship", "pair"), &FlecsWorld::add_relationship);
	ClassDB::bind_method(D_METHOD("remove_relationship", "first_entity", "second_entity"), &FlecsWorld::remove_relationship);
	ClassDB::bind_method(D_METHOD("get_relationship", "first_entity", "second_entity"), &FlecsWorld::get_relationship);
	ClassDB::bind_method(D_METHOD("get_relationships"), &FlecsWorld::get_relationships);


}


HashMap<StringName, ComponentTypeInfo> FlecsWorld::component_registry;
HashMap<StringName, SingletonComponentTypeInfo> FlecsWorld::singleton_component_registry;

FlecsWorld::FlecsWorld(/* args */) : pipeline_manager(&world) {
	
	RenderingComponentModule::initialize(&world);

	RenderingBaseComponents& rendering_components = RenderingComponentModule::get_components();
	ComponentTypeInfo mesh_info;
	mesh_info.creator = []() { return memnew(MeshComponentRef); };
	mesh_info.apply = [](const flecs::entity &e, Ref<MeshComponentRef> comp_ref) {
		Ref<MeshComponentRef> mesh_comp = comp_ref;
		e.set<MeshComponent>(mesh_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.mesh.name()),mesh_info);

	ComponentTypeInfo multi_mesh_info;
	multi_mesh_info.creator = []() { return memnew(MultiMeshComponentRef); };
	multi_mesh_info.apply = [](const flecs::entity &e, Ref<MultiMeshComponentRef> comp_ref) {
		Ref<MultiMeshComponentRef> multi_mesh_comp = comp_ref;
		e.set<MultiMeshComponent>(multi_mesh_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.multi_mesh.name()), multi_mesh_info);
	ComponentTypeInfo multi_mesh_instance_info;
	multi_mesh_instance_info.creator = []() { return memnew(MultiMeshInstanceComponentRef); };
	multi_mesh_instance_info.apply = [](const flecs::entity &e, Ref<MultiMeshInstanceComponentRef> comp_ref) {
		Ref<MultiMeshInstanceComponentRef> multi_mesh_instance_comp = comp_ref;
		e.set<MultiMeshInstanceComponent>(multi_mesh_instance_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.mesh_instance.name()), multi_mesh_instance_info);
	ComponentTypeInfo particles_info;
	particles_info.creator = []() { return memnew(ParticlesComponentRef); };
	particles_info.apply = [](const flecs::entity &e, Ref<ParticlesComponentRef> comp_ref) {
		Ref<ParticlesComponentRef> particles_comp = comp_ref;
		e.set<ParticlesComponent>(particles_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.particles.name()), particles_info);
	ComponentTypeInfo reflection_probe_info;
	reflection_probe_info.creator = []() { return memnew(ReflectionProbeComponentRef); };
	reflection_probe_info.apply = [](const flecs::entity &e, Ref<ReflectionProbeComponentRef> comp_ref) {
		Ref<ReflectionProbeComponentRef> reflection_probe_comp = comp_ref;
		e.set<ReflectionProbeComponent>(reflection_probe_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.probe.name()), reflection_probe_info);
	
	ComponentTypeInfo skeleton_info;
	skeleton_info.creator = []() { return memnew(SkeletonComponentRef); };
	skeleton_info.apply = [](const flecs::entity &e, Ref<SkeletonComponentRef> comp_ref) {
		Ref<SkeletonComponentRef> skeleton_comp = comp_ref;
		e.set<SkeletonComponent>(skeleton_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.skeleton.name()), skeleton_info);


	ComponentTypeInfo point_light_info;
	point_light_info.creator = []() { return memnew(PointLightComponentRef); };
	point_light_info.apply = [](const flecs::entity &e, Ref<PointLightComponentRef> comp_ref) {
		Ref<PointLightComponentRef> point_light_comp = comp_ref;
		e.set<PointLightComponent>(point_light_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.point_light.name()), point_light_info);
	ComponentTypeInfo directional_light_3d_info;
	directional_light_3d_info.creator = []() { return memnew(DirectionalLight3DComponentRef); };
	directional_light_3d_info.apply = [](const flecs::entity &e, Ref<DirectionalLight3DComponentRef> comp_ref) {
		Ref<DirectionalLight3DComponentRef> directional_light_3d_comp = comp_ref;
		e.set<DirectionalLight3DComponent>(directional_light_3d_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.directional_light.name()), directional_light_3d_info);

	ComponentTypeInfo spot_light_info;
	spot_light_info.creator = []() { return memnew(SpotLightComponentRef); };
	spot_light_info.apply = [](const flecs::entity &e, Ref<SpotLightComponentRef> comp_ref) {
		Ref<SpotLightComponentRef> spot_light_comp = comp_ref;
		e.set<SpotLightComponent>(spot_light_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.spot_light.name()), spot_light_info);

	ComponentTypeInfo omni_light_info;
	omni_light_info.creator = []() { return memnew(OmniLightComponentRef); };
	omni_light_info.apply = [](const flecs::entity &e, Ref<OmniLightComponentRef> comp_ref) {
		Ref<OmniLightComponentRef> omni_light_comp = comp_ref;
		e.set<OmniLightComponent>(omni_light_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.omni_light.name()), omni_light_info);



	ComponentTypeInfo camera_info;
	camera_info.creator = []() { return memnew(CameraComponentRef); };
	camera_info.apply = [](const flecs::entity &e, Ref<CameraComponentRef> comp_ref) {
		Ref<CameraComponentRef> camera_comp = comp_ref;
		e.set<CameraComponent>(camera_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.camera.name()), camera_info);
	ComponentTypeInfo main_camera_info;
	main_camera_info.creator = []() { return memnew(MainCameraRef); };
	main_camera_info.apply = [](const flecs::entity &e, Ref<MainCameraRef> comp_ref) {
		Ref<MainCameraRef> main_camera_comp = comp_ref;
		e.add<MainCamera>();
	};
	component_registry.insert(StringName(rendering_components.main_camera.name()), main_camera_info);

	ComponentTypeInfo environment_info;
	environment_info.creator = []() { return memnew(EnvironmentComponentRef); };
	environment_info.apply = [](const flecs::entity &e, Ref<EnvironmentComponentRef> comp_ref) {
		Ref<EnvironmentComponentRef> environment_comp = comp_ref;
		e.set<EnvironmentComponent>(environment_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.environment.name()), environment_info);

	ComponentTypeInfo compositor_info;
	compositor_info.creator = []() { return memnew(CompositorComponentRef); };
	compositor_info.apply = [](const flecs::entity &e, Ref<CompositorComponentRef> comp_ref) {
		Ref<CompositorComponentRef> compositor_comp = comp_ref;
		e.set<CompositorComponent>(compositor_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.compositor.name()), compositor_info);

	
	ComponentTypeInfo viewport_info;
	viewport_info.creator = []() { return memnew(ViewportComponentRef); };
	viewport_info.apply = [](const flecs::entity &e, Ref<ViewportComponentRef> comp_ref) {
		Ref<ViewportComponentRef> viewport_comp = comp_ref;
		e.set<ViewportComponent>(viewport_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.viewport.name()), viewport_info);

	ComponentTypeInfo scenario_info;
	scenario_info.creator = []() { return memnew(ScenarioComponentRef); };
	scenario_info.apply = [](const flecs::entity &e, Ref<ScenarioComponentRef> comp_ref) {
		Ref<ScenarioComponentRef> scenario_comp = comp_ref;
		e.set<ScenarioComponent>(scenario_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.scenario.name()), scenario_info);

	ComponentTypeInfo voxel_gi_info;
	voxel_gi_info.creator = []() { return memnew(VoxelGIComponentRef); };
	voxel_gi_info.apply = [](const flecs::entity &e, Ref<VoxelGIComponentRef> comp_ref) {
		Ref<VoxelGIComponentRef> voxel_gi_comp = comp_ref;
		e.set<VoxelGIComponent>(voxel_gi_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.voxel_gi.name()), voxel_gi_info);

	ComponentTypeInfo render_instance_info;
	render_instance_info.creator = []() { return memnew(RenderInstanceComponentRef); };
	render_instance_info.apply = [](const flecs::entity &e, Ref<RenderInstanceComponentRef> comp_ref) {
		Ref<RenderInstanceComponentRef> instance_comp = comp_ref;
		e.set<RenderInstanceComponent>(instance_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.instance.name()), render_instance_info);

	ComponentTypeInfo canvas_item_info;
	canvas_item_info.creator = []() { return memnew(CanvasItemComponentRef); };
	canvas_item_info.apply = [](const flecs::entity &e, Ref<CanvasItemComponentRef> comp_ref) {
		Ref<CanvasItemComponentRef> canvas_item_comp = comp_ref;
		e.set<CanvasItemComponent>(canvas_item_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.canvas_item.name()), canvas_item_info);

	ComponentTypeInfo occluder_info;
	occluder_info.creator = []() { return memnew(OccluderRef); };
	occluder_info.apply = [](const flecs::entity &e, Ref<OccluderRef> comp_ref) {
		Ref<OccluderRef> occluder_comp = comp_ref;
		e.set<Occluder>(occluder_comp->get_data());
	};

	component_registry.insert(StringName(rendering_components.occluder.name()), occluder_info);
	
	ComponentTypeInfo occludee_info;
	occludee_info.creator = []() { return memnew(OccludeeRef); };
	occludee_info.apply = [](const flecs::entity &e, Ref<OccludeeRef> comp_ref) {
		Ref<OccludeeRef> occludee_comp = comp_ref;
		e.set<Occludee>(occludee_comp->get_data());
	};
	component_registry.insert(StringName(rendering_components.occludee.name()), occludee_info);

	
	ComponentTypeInfo frustum_culled_info;
	frustum_culled_info.creator = []() { return memnew(FrustumCulledRef); };
	frustum_culled_info.apply = [](const flecs::entity &e, Ref<FrustumCulledRef> comp_ref) {
		Ref<FrustumCulledRef> frustum_culled_comp = comp_ref;
		e.add<FrustumCulled>();
	};

	component_registry.insert(StringName(rendering_components.frustum_culled.name()), frustum_culled_info);

	ComponentTypeInfo occluded_info;
	occluded_info.creator = []() { return memnew(OccludedRef); };
	occluded_info.apply = [](const flecs::entity &e, Ref<OccludedRef> comp_ref) {
		Ref<OccludedRef> occluded_comp = comp_ref;
		e.add<Occluded>();
	};
	component_registry.insert(StringName(rendering_components.occluded.name()), occluded_info);


	Physics2DComponentModule::initialize(&world);
	Physics2DBaseComponents& phsyics_components = Physics2DComponentModule::get_components();

	ComponentTypeInfo area_info;
	area_info.creator = []() { return memnew(Area2DComponentRef); };
	area_info.apply = [](const flecs::entity &e, Ref<Area2DComponentRef> comp_ref) {
		Ref<Area2DComponentRef> area_comp = comp_ref;
		e.set<Area2DComponent>(area_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_components.area.name()), area_info);
	ComponentTypeInfo body_info;
	body_info.creator = []() { return memnew(Body2DComponentRef); };
	body_info.apply = [](const flecs::entity &e, Ref<Body2DComponentRef> comp_ref) {
		Ref<Body2DComponentRef> body_comp = comp_ref;
		e.set<Body2DComponent>(body_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_components.body.name()), body_info);
	ComponentTypeInfo joint_info;
	joint_info.creator = []() { return memnew(Joint2DComponentRef); };
	joint_info.apply = [](const flecs::entity &e, Ref<Joint2DComponentRef> comp_ref) {
		Ref<Joint2DComponentRef> joint_comp = comp_ref;
		e.set<Joint2DComponent>(joint_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_components.joint.name()), joint_info);



	Physics3DComponentModule::initialize(&world);

	Physics3DBaseComponents& phsyics_3d_components = Physics3DComponentModule::get_components();
	ComponentTypeInfo area_3d_info;
	area_3d_info.creator = []() { return memnew(Area3DComponentRef); };
	area_3d_info.apply = [](const flecs::entity &e, Ref<Area3DComponentRef> comp_ref) {
		Ref<Area3DComponentRef> area_3d_comp = comp_ref;
		e.set<Area3DComponent>(area_3d_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_3d_components.area.name()), area_3d_info);
	ComponentTypeInfo body_3d_info;
	body_3d_info.creator = []() { return memnew(Body3DComponentRef); };
	body_3d_info.apply = [](const flecs::entity &e, Ref<Body3DComponentRef> comp_ref) {
		Ref<Body3DComponentRef> body_3d_comp = comp_ref;
		e.set<Body3DComponent>(body_3d_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_3d_components.body.name()), body_3d_info);
	ComponentTypeInfo joint_3d_info;
	joint_3d_info.creator = []() { return memnew(Joint3DComponentRef); };
	joint_3d_info.apply = [](const flecs::entity &e, Ref<Joint3DComponentRef> comp_ref) {
		Ref<Joint3DComponentRef> joint_3d_comp = comp_ref;
		e.set<Joint3DComponent>(joint_3d_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_3d_components.joint.name()), joint_3d_info);
	ComponentTypeInfo soft_body_info;
	soft_body_info.creator = []() { return memnew(SoftBody3DComponentRef); };
	soft_body_info.apply = [](const flecs::entity &e, Ref<SoftBody3DComponentRef> comp_ref) {
		Ref<SoftBody3DComponentRef> soft_body_3d_comp = comp_ref;
		e.set<SoftBody3DComponent>(soft_body_3d_comp->get_data());
	};
	component_registry.insert(StringName(phsyics_3d_components.soft_body.name()), soft_body_info);

	Navigation2DComponentModule::initialize(&world);
	
	Navigation2DBaseComponents& navigation_2d_components = Navigation2DComponentModule::get_components();
	ComponentTypeInfo nav_agent_2d_info;
	nav_agent_2d_info.creator = []() { return memnew(NavAgent2DComponentRef); };
	nav_agent_2d_info.apply = [](const flecs::entity &e, Ref<NavAgent2DComponentRef> comp_ref) {
		Ref<NavAgent2DComponentRef> nav_agent_2d_comp = comp_ref;
		e.set<NavAgent2DComponent>(nav_agent_2d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_2d_components.agent.name()), nav_agent_2d_info);
	ComponentTypeInfo nav_link_2d_info;
	nav_link_2d_info.creator = []() { return memnew(NavLink2DComponentRef); };
	nav_link_2d_info.apply = [](const flecs::entity &e, Ref<NavLink2DComponentRef> comp_ref) {
		Ref<NavLink2DComponentRef> nav_link_2d_comp = comp_ref;
		e.set<NavLink2DComponent>(nav_link_2d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_2d_components.link.name()), nav_link_2d_info);
	ComponentTypeInfo nav_obstacle_2d_info;
	nav_obstacle_2d_info.creator = []() { return memnew(NavObstacle2DComponentRef); };
	nav_obstacle_2d_info.apply = [](const flecs::entity &e, Ref<NavObstacle2DComponentRef> comp_ref) {
		Ref<NavObstacle2DComponentRef> nav_obstacle_2d_comp = comp_ref;
		e.set<NavObstacle2DComponent>(nav_obstacle_2d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_2d_components.obstacle.name()), nav_obstacle_2d_info);
	ComponentTypeInfo nav_region_2d_info;
	nav_region_2d_info.creator = []() { return memnew(NavRegion2DComponentRef); };
	nav_region_2d_info.apply = [](const flecs::entity &e, Ref<NavRegion2DComponentRef> comp_ref) {
		Ref<NavRegion2DComponentRef> nav_region_2d_comp = comp_ref;
		e.set<NavRegion2DComponent>(nav_region_2d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_2d_components.region.name()), nav_region_2d_info);
	ComponentTypeInfo source_geometry_parser_2d_info;
	source_geometry_parser_2d_info.creator = []() { return memnew(SourceGeometryParser2DComponentRef); };
	source_geometry_parser_2d_info.apply = [](const flecs::entity &e, Ref<SourceGeometryParser2DComponentRef> comp_ref) {
		Ref<SourceGeometryParser2DComponentRef> source_geometry_parser_2d_comp = comp_ref;
		e.set<SourceGeometryParser2DComponent>(source_geometry_parser_2d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_2d_components.source_geometry_parser.name()), source_geometry_parser_2d_info);

	Navigation3DComponentModule::initialize(&world);
	
	Navigation3DBaseComponents& navigation_3d_components = Navigation3DComponentModule::get_components();
	ComponentTypeInfo nav_agent_3d_info;
	nav_agent_3d_info.creator = []() { return memnew(NavAgent3DComponentRef); };
	nav_agent_3d_info.apply = [](const flecs::entity &e, Ref<NavAgent3DComponentRef> comp_ref) {
		Ref<NavAgent3DComponentRef> nav_agent_3d_comp = comp_ref;
		e.set<NavAgent3DComponent>(nav_agent_3d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_3d_components.agent.name()), nav_agent_3d_info);
	ComponentTypeInfo nav_link_3d_info;
	nav_link_3d_info.creator = []() { return memnew(NavLink3DComponentRef); };
	nav_link_3d_info.apply = [](const flecs::entity &e, Ref<NavLink3DComponentRef> comp_ref) {
		Ref<NavLink3DComponentRef> nav_link_3d_comp = comp_ref;
		e.set<NavLink3DComponent>(nav_link_3d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_3d_components.link.name()), nav_link_3d_info);
	ComponentTypeInfo nav_obstacle_3d_info;
	nav_obstacle_3d_info.creator = []() { return memnew(NavObstacle3DComponentRef); };
	nav_obstacle_3d_info.apply = [](const flecs::entity &e, Ref<NavObstacle3DComponentRef> comp_ref) {
		Ref<NavObstacle3DComponentRef> nav_obstacle_3d_comp = comp_ref;
		e.set<NavObstacle3DComponent>(nav_obstacle_3d_comp->get_data());
	};
		component_registry.insert(StringName(navigation_3d_components.obstacle.name()), nav_obstacle_3d_info);
	ComponentTypeInfo nav_region_3d_info;
	nav_region_3d_info.creator = []() { return memnew(NavRegion3DComponentRef); };
	nav_region_3d_info.apply = [](const flecs::entity &e, Ref<NavRegion3DComponentRef> comp_ref) {
		Ref<NavRegion3DComponentRef> nav_region_3d_comp = comp_ref;
		e.set<NavRegion3DComponent>(nav_region_3d_comp->get_data());
	};
		component_registry.insert(StringName(navigation_3d_components.region.name()), nav_region_3d_info);
	ComponentTypeInfo source_geometry_parser_3d_info;
	source_geometry_parser_3d_info.creator = []() { return memnew(SourceGeometryParser3DComponentRef); };
	source_geometry_parser_3d_info.apply = [](const flecs::entity &e, Ref<SourceGeometryParser3DComponentRef> comp_ref) {
		Ref<SourceGeometryParser3DComponentRef> source_geometry_parser_3d_comp = comp_ref;
		e.set<SourceGeometryParser3DComponent>(source_geometry_parser_3d_comp->get_data());
	};
	component_registry.insert(StringName(navigation_3d_components.source_geometry_parser.name()), source_geometry_parser_3d_info);

	Transform2DComponentModule::initialize(&world);
	
	const flecs::component<Transform2DComponent>& transform_2d_component = Transform2DComponentModule::get();
	ComponentTypeInfo transform_2d_info;
	transform_2d_info.creator = []() { return memnew(Transform2DComponentRef); };
	transform_2d_info.apply = [](const flecs::entity &e, Ref<Transform2DComponentRef> comp_ref) {
		Ref<Transform2DComponentRef> transform_2d_comp = comp_ref;
		e.set<Transform2DComponent>(transform_2d_comp->get_data());
	};
	component_registry.insert(StringName(transform_2d_component.name()), transform_2d_info);
	
	Transform3DComponentModule::initialize(&world);

	const flecs::component<Transform3DComponent>& transform_3d_components = Transform3DComponentModule::get();
	ComponentTypeInfo transform_3d_info;
	transform_3d_info.creator = []() { return memnew(Transform3DComponentRef); };
	transform_3d_info.apply = [](const flecs::entity &e, Ref<Transform3DComponentRef> comp_ref) {
		Ref<Transform3DComponentRef> transform_3d_comp = comp_ref;
		e.set<Transform3DComponent>(transform_3d_comp->get_data());
	};
	component_registry.insert(StringName(transform_3d_components.name()), transform_3d_info);

	World3DComponentModule::initialize(&world);
	
	const flecs::component<World3DComponent>& world_3d_component = World3DComponentModule::get();
	ComponentTypeInfo world_3d_info;
	world_3d_info.creator = []() { return memnew(World3DComponentRef); };
	world_3d_info.apply = [](const flecs::entity &e, Ref<World3DComponentRef> comp_ref) {
		Ref<World3DComponentRef> world_3d_comp = comp_ref;
		e.set<World3DComponent>(world_3d_comp->get_data());
	};

	component_registry.insert(StringName(world_3d_component.name()), world_3d_info);

	World2DComponentModule::initialize(&world);

	const flecs::component<World2DComponent>& world_2d_component = World2DComponentModule::get();
	ComponentTypeInfo world_2d_info;
	world_2d_info.creator = []() { return memnew(World2DComponentRef); };
	world_2d_info.apply = [](const flecs::entity &e, Ref<World2DComponentRef> comp_ref) {
		Ref<World2DComponentRef> world_2d_comp = comp_ref;
		e.set<World2DComponent>(world_2d_comp->get_data());
	};
	component_registry.insert(StringName(world_2d_component.name()), world_2d_info);

	ScriptVisibleComponentModule::initialize(&world);

	ObjectInstanceComponentModule::initialize(&world);
	const flecs::component<ObjectInstanceComponent>& object_instance_component = ObjectInstanceComponentModule::get();
	ComponentTypeInfo object_instance_info;
	object_instance_info.creator = []() { return memnew(ObjectInstanceComponentRef); };
	object_instance_info.apply = [](const flecs::entity &e, Ref<ObjectInstanceComponentRef> comp_ref) {
		Ref<ObjectInstanceComponentRef> object_instance_comp = comp_ref;
		e.set<ObjectInstanceComponent>(object_instance_comp->get_data());
	};
	component_registry.insert(StringName(object_instance_component.name()), object_instance_info);

	const flecs::component<ScriptVisibleComponent>& script_visible_component = ScriptVisibleComponentModule::get();
	ComponentTypeInfo script_visible_info;
	script_visible_info.creator = []() { return memnew(ScriptVisibleComponentRef); };
	script_visible_info.apply = [](const flecs::entity &e, Ref<ScriptVisibleComponentRef> comp_ref) {
		Ref<ScriptVisibleComponentRef> script_visible_comp = comp_ref;
		e.set<ScriptVisibleComponent>(script_visible_comp->get_data());
	};
	component_registry.insert(StringName(script_visible_component.name()), script_visible_info);
}

FlecsWorld::~FlecsWorld()
{
	system_command_queue.clear();
	world.quit();
	NodeStorage::release_all();
	RefStorage::release_all();
	script_systems.clear();
	for (auto &comp : components) {
		if (comp.is_valid()) {
			comp->set_internal_world(nullptr); // Clear the internal world reference
		}
	}
	components.clear();
	print_line("FlecsWorld destroyed: " + itos((uint64_t)world.c_ptr()));
	
	
}

void FlecsWorld::init_world() {
	world.import<flecs::stats>();
	world.set<flecs::Rest>({});
    print_line("World initialized: " + itos((uint64_t)world.c_ptr()));

}

bool FlecsWorld::progress(const double delta) {
	for (auto &sys : script_systems) {
		if (sys.is_null()) {
			ERR_PRINT("FlecsWorld::progress: null system");
			continue;
		};
		if (!sys.is_valid()) {
			ERR_PRINT("FlecsWorld::progress: invalid system");
			continue;
		}
		sys->run();
	}
	const bool progress = world.progress(delta);
	system_command_queue.process();

	return progress;
}



Ref<FlecsEntity> FlecsWorld::create_entity() {
	ERR_FAIL_COND_V(!world.c_ptr(), Ref<FlecsEntity>()); // flecs::world::is_alive() returns false if uninitialized
	const flecs::entity raw_entity = world.entity();
	Ref<FlecsEntity> flecs_entity = memnew(FlecsEntity);
	flecs_entity->set_entity(raw_entity); // <— make sure entity is alive here
	flecs_entity->set_internal_world(&world);
	return flecs_entity;
}
	 
void FlecsWorld::set_component(const Ref<FlecsComponentBase> &comp_ref) {
	if (!comp_ref.is_valid()) {
		ERR_PRINT("add_component(): Component is null or invalid.");
		return;
	}

	// Handle dynamic script-visible components
	if (comp_ref->is_dynamic()) {
		const Ref<ScriptVisibleComponentRef> dyn = comp_ref;
		ScriptVisibleComponent& data = dyn->get_internal_owner().get_mut<ScriptVisibleComponent>();

		const StringName type_name = data.name;
		const auto* schema = ScriptComponentRegistry::get_singleton()->get_schema(type_name);

		if (!schema) {
			ERR_PRINT("add_component(): Unknown script component type: " + type_name);
			return;
		}


		// Fill in missing defaults
		for (auto it = schema->begin(); it != schema->end(); ++it ) {
			StringName field_name = it->key;
			ScriptComponentRegistry::FieldDef def = it->value;

			if (!data.fields.has(field_name)) {
				data.fields.insert(field_name, def.default_value);
			} else {
				// Optional: Validate type
				if (data.fields.getptr(field_name)->get_type() != def.type) {
					WARN_PRINT("Field '" + String(field_name) + "' has wrong type — expected " + Variant::get_type_name(def.type));
				}
			}
		}

		// Set in ECS
		// ReSharper disable once CppExpressionWithoutSideEffects
		world.set<ScriptVisibleComponent>(data);
		dyn->set_data(data); // ensures pointer is synced
		return;
	}

	// Static typed component path
	components.append(comp_ref);
}

Ref<FlecsComponentBase> FlecsWorld::get_component(const StringName &component_type) const {
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		if (components[i]->get_type_name() == component_type) {
			return components[i];
		}
	}
	ERR_PRINT("component type not found. returning nullptr");
	return Ref<FlecsComponentBase>();
}
bool FlecsWorld::has_component(const StringName &component_type) const {
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			continue;
		}
		if (components[i].is_null()) {
			continue;
		}
		if (components[i]->get_type_name() == component_type) {
			return true;
		}
	}
	return false;
}
PackedStringArray FlecsWorld::get_component_types() const {
	PackedStringArray ret;
	for (int i = 0; i < components.size(); i++) {
		if (!components[i].is_valid()) {
			ERR_PRINT("component reference has become invalid, skipping index.");
			continue;
		}
		if (components[i].is_null()) {
			ERR_PRINT("component is null, skipping index.");
			continue;
		}
		ret.push_back(components[i]->get_type_name());
	}
	return ret;
}

Ref<FlecsEntity> FlecsWorld::create_entity_n(const StringName &p_name) {
	Ref<FlecsEntity> flecs_entity = create_entity();
	flecs_entity->set_entity_name(p_name);
	return flecs_entity;
}

Ref<FlecsEntity> FlecsWorld::create_entity_nc(const StringName &p_name, const Ref<FlecsComponentBase> &p_comp) {
	Ref<FlecsEntity> flecs_entity = create_entity();
	flecs_entity->set_name(p_name);
	flecs_entity->set_entity_name(p_name);
	flecs_entity->set_component(p_comp);
	return flecs_entity;
}

flecs::world* FlecsWorld::get_world_ref() {
	return &world;
}

Ref<FlecsEntity> FlecsWorld::add_entity(const flecs::entity &e) {
	if (!e.is_valid()) {
		ERR_PRINT("FlecsWorld::get_entity: entity is not valid. Returning nullptr.");
		return Ref<FlecsEntity>();
	}
	if(world != e.world()) {
		ERR_PRINT("FlecsWorld::get_entity: entity does not belong to this world. Returning nullptr.");
		return Ref<FlecsEntity>();
	}


	Ref<FlecsEntity> new_entity = memnew(FlecsEntity);
	auto entity_iter = entities.insert(e, new_entity);
	new_entity->set_entity(e);
	new_entity->set_name(e.name().c_str());
	
	e.each([&](flecs::id id) {
		// Get the type id (component or tag)
		if (id.is_pair()) {
			// Extract the relation and object
			flecs::entity relation = id.first();
			FlecsEntity * gd_relation = nullptr;
			if(entities.has(relation)) {
				gd_relation = entities[relation].ptr();
			} else {
				gd_relation = memnew(FlecsEntity);
				gd_relation->set_entity(relation);
				entities.insert(relation, gd_relation);
			}

			flecs::entity object = id.second();
			FlecsEntity * gd_object = nullptr;
			if(entities.has(object)) {
				gd_object = entities[object].ptr();
			} else {
				gd_object = memnew(FlecsEntity);
				gd_object->set_entity(object);
				entities.insert(object, gd_object);
			}

			// Log or handle the pair
			String relation_name = relation.name().c_str();
			String object_name = object.name().c_str();
			print_line("Pair detected: (" + relation_name + ", " + object_name + ")");
			FlecsPair *pair = memnew(FlecsPair);
			pair->set_first(gd_relation);
			pair->set_second(gd_object);
			new_entity->add_relationship(pair);
        return;
    }
		StringName comp_name = StringName(id.entity().name());
		if(component_registry.has(comp_name)) {
			Ref<FlecsComponentBase> comp = component_registry[comp_name].creator();
			new_entity->set_component(comp);
		}
	});
	return new_entity;
}

void FlecsWorld::init_render_system() {
	flecs::entity main_camera;

	world.each<CameraComponent>([&](flecs::entity e, const CameraComponent& cam) {
		if (e.has<MainCamera>()) {
			main_camera = e;
			return; // early out
		}
	});
	
	if(!main_camera.is_alive()){
		ERR_PRINT("Main camera not found! Cancelling init.");
		return;
	}
	print_line("World initialized: " + itos((uint64_t)world.c_ptr()));
	multi_mesh_render_system.set_world(this);
	occlusion_system.set_world(this);
	mesh_render_system.set_world(this);

	multi_mesh_render_system.set_main_camera(main_camera);
	occlusion_system.set_main_camera(main_camera);
	mesh_render_system.set_main_camera(main_camera);

	multi_mesh_render_system.create_frustum_culling(system_command_queue, pipeline_manager);
	occlusion_system.create_occlusion_culling(system_command_queue, pipeline_manager);
	multi_mesh_render_system.create_rendering(system_command_queue, pipeline_manager);
	mesh_render_system.create_mesh_render_system(system_command_queue, pipeline_manager);
}

void FlecsWorld::set_log_level(const int level) {
	flecs::log::set_level(level);
}

void FlecsWorld::add_relationship(FlecsPair *pair) {
	if (!pair) {
		ERR_PRINT("FlecsWorld::add_relationship called with null pair");
		return;
	}
	relationships.append(pair);
}

void FlecsWorld::remove_relationship(const StringName &first_entity, const StringName &second_entity) {
	int8_t index = -1;
	FlecsPair *pair = nullptr;
	for (int i = 0; i < relationships.size(); i++) {
		pair = relationships[i].ptr();
		if (pair->get_first()->get_name() == first_entity && pair->get_second()->get_name() == second_entity) {
			index = i;
			break;
		}
	}
	if(!pair){
		ERR_PRINT("FlecsWorld::remove_relationship: pair not found for " + first_entity + " and " + second_entity);
		return;
	}

	world.remove(pair->get_first()->get_entity(),pair->get_second()->get_entity());
	memdelete(pair->get_first());
	memdelete(pair->get_second());
	pair->set_first(nullptr);
	pair->set_second(nullptr);
	relationships.erase(pair);
	memdelete(pair);
	return;
}

Ref<FlecsPair> FlecsWorld::get_relationship(const StringName &first_entity, const StringName &second_entity) const {
	for (const auto &pair : relationships) {
		if (pair.is_null()) {
			ERR_PRINT("FlecsWorld::get_relationship: pair is null, skipping.");
			continue;
		}
		if (pair->get_first()->get_name() == first_entity && pair->get_second()->get_name() == second_entity) {
			return pair;
		}
	}
	ERR_PRINT("FlecsWorld::get_relationship: relationship not found for " + first_entity + " and " + second_entity);
	return Ref<FlecsPair>();
}

TypedArray<FlecsPair> FlecsWorld::get_relationships() const {
	TypedArray<FlecsPair> result;
	for (const auto &pair : relationships) {
		if (pair.is_null()) {
			ERR_PRINT("FlecsWorld::get_relationships: pair is null, skipping.");
			continue;
		}
		result.append(pair);
	}
	return result;
}

void FlecsWorld::register_component_type(const StringName &type_name, const Ref<ScriptVisibleComponentRef> &script_visible_component_ref) const {
	if (!script_visible_component_ref.is_valid() || script_visible_component_ref.is_null()) {
		ERR_PRINT("component is not valid.");
		return;
	}
	const char *ctype_name = String(type_name).ascii().get_data();
	ecs_component_desc_t desc = { 0 };
	desc.entity = world.entity(ctype_name);
	desc.type.size = sizeof(ScriptVisibleComponent);
	desc.type.alignment = alignof(ScriptVisibleComponent);
	flecs::entity_t comp = ecs_component_init(world, &desc);

	component_registry[type_name] = {
		[]() -> Ref<FlecsComponentBase> {
			return memnew(ScriptVisibleComponentRef); // or component_ref->clone()
		},
		[type_name](const flecs::entity &e, const Ref<FlecsComponentBase> &comp) {
			if (!comp.is_valid() || comp.is_null()) {
				ERR_PRINT("Invalid component passed to set callback.");
				return;
			}
			if (comp->get_type_name() != type_name) {
				ERR_PRINT("Component type mismatch.");
				return;
			}
			// Cast and set data
			ScriptVisibleComponent* data =  comp->try_get_typed_data<ScriptVisibleComponent>();
			if(!data){
				if(!Engine::get_singleton()->is_editor_hint()){
					WARN_PRINT("Data is null. Are you trying to instantiate a tag type as a component?");
					WARN_PRINT("Returing base type T");
				}
				static ScriptVisibleComponent empty = ScriptVisibleComponent{};
				e.add<ScriptVisibleComponent>();
			}
			e.set<ScriptVisibleComponent>(*data); // Actually sets component on entity
		}
	};
}
 void FlecsWorld::add_script_system(const Array &component_types, const Callable &callable) {
	FlecsScriptSystem* sys = memnew(FlecsScriptSystem);
	sys->set_world(this);
	Vector<String> component_names;
	component_names.resize(component_types.size());
	int count = 0;
	for (auto it = component_types.begin(); it != component_types.end(); ++it) {
		component_names.set(count, *it);
		count++;
	}
	sys->init(this,component_names,callable);
	script_systems.append(sys);
}

