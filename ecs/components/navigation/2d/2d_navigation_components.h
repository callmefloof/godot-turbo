#pragma once
#include "core/templates/rid.h"

struct NavAgent2DComponent
{
	RID agent_id;
};

struct NavLink2DComponent
{
	RID link_id;
};

struct NavMap2DComponent
{
	RID map_id;
};

struct NavObstacle2DComponent
{
	RID obstacle_id;
};

struct NavRegion2Dcomponent
{
	RID region_id;
};

struct SourceGeometryParserComponent
{
	RID source_geometry_parser_id;
};

