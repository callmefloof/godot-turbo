#pragma once
#include "../../modules/register_module_types.h"
#include "thirdparty/flecs/distr/flecs.h"

void initialize_godot_turbo_module(ModuleInitializationLevel p_level);
void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level);

#define MODULE_GODOT_TURBO_HAS_PREREGISTER
void preregister_godot_turbo_types();

void register_godot_turbo_types();
void unregister_godot_turbo_types();

template <class T>
void component_register();



