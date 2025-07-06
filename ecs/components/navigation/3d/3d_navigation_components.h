#pragma once
#include "core/templates/rid.h"

struct NavAgent3DComponent {
	RID agent_id;
};

struct NavLink3DComponent {
	RID link_id;
};

struct NavMap3DComponent {
	RID map_id;
};

struct NavObstacle3DComponent {
	RID obstacle_id;
};

struct NavRegion3Dcomponent {
	RID region_id;
};

struct SourceGeometryParserComponent {
	RID source_geometry_parser_id;
};
