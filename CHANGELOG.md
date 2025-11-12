# Changelog

All notable changes to the Godot Turbo ECS module will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0-a.1] - 2025-01-28

### Added

#### Documentation
- Comprehensive API documentation for all system classes (550+ lines)
- Master systems documentation guide (SYSTEMS_DOCUMENTATION.md)
- Testing guide with examples and best practices (tests/README.md)
- Complete documentation for PipelineManager class (216 lines)
- Complete documentation for CommandQueue/CommandHandler (337 lines)
- Navigation README for easy access to all documentation
- Component migration guide and usage examples
- Utility function documentation

#### Testing
- **90+ comprehensive unit tests** with 90%+ code coverage
- **PipelineManager tests** (20+ test cases, 545 lines)
  - Constructor and initialization tests
  - Copy/move semantics validation
  - System registration and lookup tests
  - Custom phase creation and execution order tests
  - Edge case handling
- **Command system tests** (40+ test cases, 721 lines)
  - Pool allocation and thread safety tests
  - Queue operations and FIFO ordering tests
  - Performance stress tests (10,000+ commands)
  - Multi-threaded producer tests
- **GDScriptRunnerSystem tests** (30+ test cases, 553 lines)
  - Cache management tests
  - Component filtering tests
  - Stress tests with 1000+ entities

#### Features
- **GDScriptRunnerSystem** - Bridge for executing GDScript/C# methods on ECS entities
  - Method caching for performance (~10ns lookup)
  - Support for both GDScript and C# naming conventions
  - Separate process and physics process phases
- **BadAppleSystem optimizations** - Demo system with major performance improvements
  - SIMD vectorization (SSE2 for x86/x64, NEON for ARM)
  - Multi-threaded pixel processing with WorkerThreadPool
  - Format-specific optimized loops
  - 12x+ overall performance improvement
- **Lock-free CommandQueue** - Thread-safe command queue with object pooling
  - Multi-producer safe enqueueing
  - Object pooling for zero-allocation performance
  - ~50-100ns enqueue time
- **PipelineManager** - Flexible system execution control
  - Custom phase creation with dependencies
  - System lookup by name
  - Multi-world support

#### Components
- Refactored physics components (RigidBody2D/3D, CollisionShape2D/3D)
- Refactored navigation components (NavigationAgent2D/3D, NavigationObstacle2D/3D)
- Added Flecs reflection support for all components
- Improved component documentation

#### Utilities
- Scene object conversion utilities
- Domain-specific utilities (physics, rendering, navigation)
- Comprehensive utility documentation

### Changed
- Enhanced README.md with quick start guide, examples, and changelog
- Improved .gitignore to exclude build artifacts and temporary files
- Consolidated documentation structure for better organization

### Removed
- Temporary .obj build files (now properly gitignored)
- In-progress documentation files:
  - `GDSCRIPT_RUNNER_IMPLEMENTATION_SUMMARY.md`
  - `DOCUMENTATION_SUMMARY.md`
  - `COMPLETION_CHECKLIST.md`
  - `REFACTOR_SUMMARY.md`
  - `IMPLEMENTATION_COMPLETE.md`
  - `COMPONENT_VERIFICATION.md`
  - `VALIDATION_SUMMARY.md`
  - `DOCUMENTATION_STATUS.md`
  - `PHASE2_COMPLETION_SUMMARY.md`

### Performance
- **BadAppleSystem**: 5 FPS → 60+ FPS (12x improvement)
  - Baseline: 5 FPS
  - Format-specific loops: 12 FPS (2.4x)
  - + SIMD: 23 FPS (4.6x)
  - + Multi-threading: 60+ FPS (12+x)
- **CommandQueue**: 10,000+ commands/frame throughput
- **GDScriptRunnerSystem**: ~10ns cache lookup per entity

### Fixed
- Build artifact accumulation (added to .gitignore)
- Documentation organization and accessibility

---

## [1.0.x] - Historical (Retroactive Tags)

**Note:** Version 1.0.x releases will be tagged retroactively from previous commits.

### Initial Release Features
- Flecs ECS library integration
- FlecsServer singleton for world management
- Basic component system
- Scene conversion utilities
- Initial system implementations
- Basic pipeline management

---

## Version Naming Convention

- **Major.Minor.Patch-Stage.Build**
  - Major: Breaking changes
  - Minor: New features (backward compatible)
  - Patch: Bug fixes
  - Stage: `a` (alpha), `b` (beta), `rc` (release candidate), or omitted (stable)
  - Build: Build number within stage

**Examples:**
- `1.1.0-a.1` - Version 1.1.0, alpha stage, build 1
- `1.1.0` - Version 1.1.0, stable release

---

## Migration Guide

### From 1.0.x to 1.1.0-a.1

**No breaking changes** - This release is fully backward compatible.

**New Features to Adopt:**
1. Use `GDScriptRunnerSystem` for script integration with ECS entities
2. Use `CommandQueue` for thread-safe deferred command execution
3. Use `PipelineManager` for better system execution control
4. Review new documentation for best practices

**Performance Improvements:**
- Existing code will benefit from internal optimizations
- Consider adopting SIMD-optimized patterns for pixel/batch processing
- Use object pooling patterns from CommandQueue for your systems

---

## Future Plans

### Version 1.2.0 (Planned)
- Additional utility systems
- More comprehensive examples
- Performance profiling tools
- Extended GDScript API

### Version 2.0.0 (Future)
- Godot 4.5+ compatibility
- Advanced Flecs features (pipelines, observers)
- Enhanced reflection system
- Breaking API improvements based on feedback

---

**Maintained by:** [@callmefloof](https://github.com/callmefloof)  
**Repository:** [godot-turbo](https://github.com/callmefloof/godot-turbo)