#pragma once

#include "thirdparty/flecs/distr/flecs.h"
#include "ecs/components/component_proxy.h"
#include "ecs/components/component_module_base.h"
#include "ecs/flecs_types/flecs_component.h"
#include "ecs/components/single_component_module.h"

struct DirtyTransform {};

class DirtyTransformRef : public FlecsComponent<DirtyTransform> {
    DEFINE_TAG_PROXY(DirtyTransform)
};

using DirtyTransformModule = SingleComponentModule<DirtyTransform>;