# Release Notes - Godot Turbo ECS v1.3.2-beta.1

**Release Date:** May 6, 2026
**Status:** Beta Release
**Godot Version:** 4.6+

---

## Overview

This is a patch release for Flecs master compatibility and runtime stability around Flecs world progression. It updates the bundled Flecs revision and hardens component writes, profiler metrics, and system pause operations against stale entities and re-entrant world access.

## What changed

- Updated the bundled Flecs submodule from `a53b471` to `b50bcc5`.
- Replaced removed `ecs_world_info_t::min_id` / `max_id` usage with the current Flecs entity range API.
- Added `FlecsServer::is_entity_alive()` for checking RID wrappers against the live Flecs entity.
- Added liveness checks before `set_component()` and cursor-based component writes call Flecs `ensure()`.
- Guarded live system metrics and pause/resume operations while a world is inside `progress_world()`.

## AxiomScript Integration

The paired AxiomScript update prevents `LightingInfluenceSystem` from re-entering `FlecsServer` from suppressed observers and removes stale light/entity cache entries before they can write components to deleted Flecs entities.

## Upgrade Notes

This release is intended as a drop-in patch over `v1.3.1-beta.1`. No public API removal is included. Code that caches ECS entity RIDs should treat `get_world_of_entity()` as an ownership lookup only and use `is_entity_alive()` when it needs to write back into Flecs.
