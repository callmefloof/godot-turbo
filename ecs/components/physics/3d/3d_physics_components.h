#pragma once
#include "core/templates/rid.h"

struct Area3DComponent
{
	RID area_id;
};

struct Body3DComponent
{
	RID body_id;
};

struct Joint3DComponent
{
	RID joint_id;
};

struct SoftBody3DComponent
{
	RID soft_body_id;
};

struct Space3DComponent
{
	RID space_id;
};
