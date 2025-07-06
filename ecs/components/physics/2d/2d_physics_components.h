#pragma once
#include "core/templates/rid.h"

struct Area2DComponent
{
	RID area_id;
};

struct Body2DComponent
{
	RID body_id;
};

struct Joint2DComponent
{
	RID joint_id;
};

struct SoftBody2DComponent
{
	RID soft_body_id;
};

struct Space2DComponent
{
	RID space_id;
};
