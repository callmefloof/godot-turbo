#pragma once
#include "../../modules/register_module_types.h"


class FlecsServer;

void initialize_godot_turbo_module(ModuleInitializationLevel p_level);
void uninitialize_godot_turbo_module(ModuleInitializationLevel p_level);
static FlecsServer* p_fs = nullptr;
#define MODULE_GODOT_TURBO_HAS_PREREGISTER
void preregister_godot_turbo_types();

void register_godot_turbo_types();
void unregister_godot_turbo_types();

template <class T>
void component_register();
