# Release Notes - Godot Turbo ECS v1.3.1-beta.1

**Release Date:** May 6, 2026  
**Status:** Beta Release  
**Godot Version:** 4.6+

---

## Overview

This is a patch release that restores the `debug/` source directory, which was accidentally excluded from the repository by a `.gitignore` rule (`[Dd]ebug/`) since v1.2.1-beta.1 (February 2026). Apologies for the oversight.

### What changed

- Removed `[Dd]ebug/` from `.gitignore` so the `debug/` module directory is tracked correctly.
- Added all files under `debug/` that were missing from the repository.

The `debug/` directory contains:

| File | Description |
|------|-------------|
| `ecs_trace_bridge.h` | ECS trace bridge header |
| `flecs_os_api_traced.h` | Traced Flecs OS API header |
| `turbo_debug_draw.cpp/.h` | Debug draw utilities |
| `turbo_debug_panel.cpp/.h` | Debug panel implementation |
| `turbo_debug_system.cpp/.h` | Debug system |
| `turbo_entity_picker.cpp/.h` | Entity picker tool |
| `SCsub` | SConstruct build file for the debug module |
| `README.md` | Debug module documentation |

No API or behavior changes were made in this release.
