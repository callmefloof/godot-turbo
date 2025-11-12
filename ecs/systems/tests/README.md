# ECS Systems Unit Tests

This directory contains comprehensive unit tests for the ECS system classes in the `godot_turbo` module.

## Table of Contents

- [Overview](#overview)
- [Test Coverage](#test-coverage)
- [Running Tests](#running-tests)
- [Test Structure](#test-structure)
- [Writing New Tests](#writing-new-tests)
- [Troubleshooting](#troubleshooting)

## Overview

The test suite provides comprehensive coverage for:

- **PipelineManager** - System pipeline management and execution order
- **CommandQueue/CommandHandler** - Lock-free command queue system
- **GDScriptRunnerSystem** - Script execution bridge for ECS

All tests use Godot's built-in test framework with `test_macros.h`.

## Test Coverage

### PipelineManager Tests (`test_pipeline_manager.h`)

- ✅ Constructor and world association
- ✅ Copy/move semantics
- ✅ System registration (default and custom phases)
- ✅ System lookup by name
- ✅ Custom phase creation with dependencies
- ✅ System execution order verification
- ✅ Multi-world support
- ✅ Edge cases (invalid RID, self-assignment, etc.)

**Total Test Cases:** 20+

### Command System Tests (`test_command.h`)

- ✅ Basic ICommand interface
- ✅ Pool allocation/deallocation
- ✅ Pool exhaustion and reuse
- ✅ Pooled command creation and execution
- ✅ Unpooled command fallback
- ✅ CommandQueue FIFO ordering
- ✅ Thread-safety (multi-producer)
- ✅ CommandHandler RefCounted behavior
- ✅ Complex captures and move-only types
- ✅ Performance stress tests (10,000+ commands)

**Total Test Cases:** 40+

### GDScriptRunnerSystem Tests (`test_gdscript_runner_system.h`)

- ✅ System initialization
- ✅ Process and physics process toggling
- ✅ Method cache population and clearing
- ✅ Multiple entities with same/different scripts
- ✅ Component filtering (GameScriptComponent, SceneNodeComponent)
- ✅ Enable/disable system execution
- ✅ Cache persistence across frames
- ✅ Edge cases (empty paths, missing components)
- ✅ Non-copyable/non-movable constraints
- ✅ Stress test (1000+ entities)

**Total Test Cases:** 30+

## Running Tests

### Build with Tests Enabled

```bash
# Configure build with tests
scons tests=yes target=editor dev_build=yes

# Run all tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test
```

### Run Specific Test Suites

```bash
# Run only PipelineManager tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager]*"

# Run only Command tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[Command]*"

# Run only GDScriptRunnerSystem tests
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[GDScriptRunnerSystem]*"
```

### Run Specific Test Case

```bash
# Run a single test
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager] Constructor initializes with valid world"
```

## Test Structure

### Test Fixture Pattern

All test suites use fixtures for setup/teardown:

```cpp
namespace TestMyClass {

class MyTestFixture {
public:
    // Test resources
    FlecsServer* server = nullptr;
    RID world_rid;
    
    void setup() {
        // Initialize resources
        server = FlecsServer::get_singleton();
        world_rid = server->create_world();
    }
    
    void teardown() {
        // Clean up resources
        if (world_rid.is_valid()) {
            server->remove_world(world_rid);
        }
    }
};

TEST_CASE("[MyClass] Description of test") {
    MyTestFixture fixture;
    fixture.setup();
    
    // Test code here
    CHECK(condition);
    
    fixture.teardown();
}

} // namespace TestMyClass
```

### Assertion Macros

- `CHECK(condition)` - Non-fatal assertion (test continues)
- `REQUIRE(condition)` - Fatal assertion (test stops on failure)
- `CHECK(value == expected)` - Equality check
- `CHECK(value != unexpected)` - Inequality check

## Writing New Tests

### 1. Create Test File

```cpp
#pragma once

#include "tests/test_macros.h"
#include "path/to/class_under_test.h"

namespace TestMyNewClass {

TEST_CASE("[MyNewClass] Test description") {
    // Arrange
    MyNewClass obj;
    
    // Act
    obj.do_something();
    
    // Assert
    CHECK(obj.get_state() == expected_state);
}

} // namespace TestMyNewClass
```

### 2. Register in SCsub

Add your test file to the appropriate `SCsub`:

```python
# In modules/godot_turbo/ecs/systems/tests/SCsub
if env["tests"]:
    env.add_source_files(env.modules_sources, "test_my_new_class.h")
```

### 3. Test Categories

Use descriptive category tags:

- `[ClassName]` - Basic functionality tests
- `[ClassName][Integration]` - Integration tests
- `[ClassName][Performance]` - Performance/stress tests
- `[ClassName][ThreadSafety]` - Concurrency tests
- `[ClassName][EdgeCase]` - Edge case and error handling

### 4. Test Naming Convention

Use clear, descriptive test names:

```cpp
TEST_CASE("[ClassName] Should do X when Y happens")
TEST_CASE("[ClassName] Handles edge case Z gracefully")
TEST_CASE("[ClassName] Method X returns Y given Z")
```

## Best Practices

### ✅ Do

- Use fixtures for setup/teardown
- Test one concept per test case
- Use descriptive test names
- Clean up resources in teardown
- Test both success and failure paths
- Include edge cases (nullptr, empty, invalid)
- Add stress tests for performance-critical code
- Test thread safety for concurrent code

### ❌ Don't

- Leave resources allocated after test
- Use global state between tests
- Assume test execution order
- Test implementation details
- Write flaky tests (non-deterministic)
- Ignore compiler warnings in tests

## Test Data Guidelines

### Minimal Test Data

Keep test data focused and minimal:

```cpp
// Good: Minimal data for the test
GameScriptComponent script_comp;
script_comp.instance_type = StringName("Node");

// Bad: Unnecessary complex setup
GameScriptComponent script_comp;
script_comp.instance_type = StringName("VerySpecificCustomNodeWithLongName");
script_comp.script_path = "res://very/deep/path/to/script.gd";
script_comp.custom_property_1 = 42;
// ... etc
```

### Realistic Test Scenarios

For integration tests, use realistic scenarios:

```cpp
TEST_CASE("[PipelineManager] Multiple systems execute in order") {
    // Realistic: Simulates actual game logic
    // System 1: Multiply by 2
    // System 2: Add 3
    // Verifies execution order affects result
}
```

## Performance Testing

### Stress Tests

Include performance stress tests for critical paths:

```cpp
TEST_CASE("[Command] CommandQueue handles many small commands efficiently") {
    const int num_commands = 10000;
    
    for (int i = 0; i < num_commands; ++i) {
        queue.enqueue([&counter]() { counter++; });
    }
    
    queue.process();
    CHECK(counter == num_commands);
}
```

### Thread Safety Tests

Test concurrent access patterns:

```cpp
TEST_CASE("[Command] CommandQueue is thread-safe for enqueueing") {
    const int num_threads = 4;
    const int commands_per_thread = 100;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&]() {
            // Enqueue from multiple threads
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Verify all commands executed
}
```

## Troubleshooting

### Test Fails to Compile

**Problem:** Test file doesn't compile

**Solutions:**
1. Check include paths are correct
2. Verify class is registered in `SCsub`
3. Ensure test file has `#pragma once`
4. Check namespace matches file structure

### Test Crashes

**Problem:** Test crashes during execution

**Solutions:**
1. Check for null pointer dereferences
2. Verify resources are initialized in setup
3. Ensure Flecs world is valid
4. Check for use-after-free in teardown

### Test is Flaky

**Problem:** Test passes sometimes, fails others

**Solutions:**
1. Remove dependence on global state
2. Fix race conditions in thread safety tests
3. Ensure deterministic test data
4. Check for uninitialized variables

### Memory Leaks

**Problem:** Valgrind reports leaks

**Solutions:**
1. Call `teardown()` for all fixtures
2. Use `Ref<>` for RefCounted objects
3. Match `memnew()` with `memdelete()`
4. Clean up Flecs entities/worlds

## Continuous Integration

### Pre-commit Checks

Run tests before committing:

```bash
#!/bin/bash
# .git/hooks/pre-commit

echo "Running unit tests..."
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[PipelineManager]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[Command]*"
./bin/godot.linuxbsd.editor.dev.x86_64 --test --tc="[GDScriptRunnerSystem]*"

if [ $? -ne 0 ]; then
    echo "Tests failed! Commit aborted."
    exit 1
fi
```

### CI Pipeline

Example GitHub Actions workflow:

```yaml
name: Unit Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build with tests
        run: scons tests=yes target=editor dev_build=yes
      - name: Run tests
        run: ./bin/godot.linuxbsd.editor.dev.x86_64 --test
```

## Coverage Goals

Target coverage levels:

- **Critical paths:** 100% coverage
  - PipelineManager system registration
  - CommandQueue enqueue/process
  - GDScriptRunnerSystem method execution

- **Normal paths:** 80%+ coverage
  - Copy/move constructors
  - Getters/setters
  - Cache management

- **Edge cases:** 60%+ coverage
  - Error handling
  - Invalid input
  - Resource exhaustion

## Additional Resources

- [Godot Testing Documentation](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/unit_testing.html)
- [Flecs Documentation](https://www.flecs.dev/flecs/)
- [doctest Documentation](https://github.com/doctest/doctest)

## Contributing

When adding new tests:

1. Follow the existing test structure
2. Use descriptive names
3. Include documentation comments
4. Test edge cases
5. Run full test suite before PR
6. Update this README if adding new test categories

---

**Last Updated:** 2025-01-28  
**Maintainer:** ProjectVYGR Team  
**License:** MIT