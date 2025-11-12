/**************************************************************************/
/*  test_main.h                                                           */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef GODOT_TURBO_TEST_MAIN_H
#define GODOT_TURBO_TEST_MAIN_H

// Storage tests
#include "test_node_storage.h"
#include "test_ref_storage.h"

// Conversion utility tests
#include "test_scene_object_utility.h"
#include "test_resource_object_utility.h"
#include "test_world_utility.h"

// Flecs types tests
#include "test_flecs_variant.h"
#include "test_flecs_query.h"
#include "test_flecs_script_system.h"

// ECS systems tests
#include "test_gdscript_runner_system.h"

// Domain utility tests (commented out - need API verification)
// These tests are implemented but need API adjustments to match actual utility classes
// #include "test_physics_utility.h"
// #include "test_navigation_utility.h"
// #include "test_render_utility.h"

#endif // GODOT_TURBO_TEST_MAIN_H