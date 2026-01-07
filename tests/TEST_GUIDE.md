# Godot Turbo ECS Utility Test Suite

**Phase:** 3 - Testing & Validation  
**Status:** ✅ Active  
**Module:** `modules/godot_turbo/tests/`

---

## Overview

This directory contains comprehensive unit tests, integration tests, and thread-safety tests for the Godot Turbo ECS utility layer. The test suite validates correctness, performance, and thread-safety of all 12 core utilities.

---

## Test Structure

### Test Categories

1. **Unit Tests** - Test individual utility classes in isolation
2. **Integration Tests** - Test utilities working together
3. **Thread-Safety Tests** - Validate concurrent access patterns
4. **Stress Tests** - Large-scale operations and edge cases
5. **Performance Tests** - Benchmarks and profiling

### Test Files

#### ✅ Implemented

- `test_ref_storage.h` - RefStorage unit tests (454 lines, 13 test cases)
- `test_node_storage.h` - NodeStorage unit tests (552 lines, 14 test cases)
- `test_main.h` - Test registration and runner

#### 🔄 To Be Implemented

- `test_scene_object_utility.h` - SceneObjectUtility tests
- `test_resource_object_utility.h` - ResourceObjectUtility tests
- `test_world_utility.h` - World2DUtility/World3DUtility tests
- `test_physics_utility.h` - Physics2D/3D utility tests
- `test_navigation_utility.h` - Navigation2D/3D utility tests
- `test_render_utility.h` - Render2D/3D utility tests
- `test_integration.h` - Cross-utility integration tests
- `test_performance.h` - Performance benchmarks

---

## Running Tests

### Command Line

```bash
# Build Godot with tests enabled
scons tests=yes

# Run all tests
./bin/godot.linuxbsd.editor.x86_64 --test --source-file=modules/godot_turbo/tests/test_main.h

# Run specific test suite
./bin/godot.linuxbsd.editor.x86_64 --test --test-case="*RefStorage*"
./bin/godot.linuxbsd.editor.x86_64 --test --test-case="*NodeStorage*"
```

### From Editor

Tests can also be run from the Godot editor's test runner (if available in your build).

---

## Test Framework

### Godot Test Macros

The test suite uses Godot's built-in test framework based on doctest:

```cpp
// Test case definition
TEST_CASE("[Category] Test name") {
    // Test setup
    
    CHECK_MESSAGE(
        condition,
        "Descriptive failure message");
}

// Subcase for variants
SUBCASE("Variant name") {
    // Variant-specific tests
}
```

### Common Patterns

#### Basic Functionality Test
```cpp
TEST_CASE("[RefStorage] Add and retrieve resource") {
    RefStorage storage;
    Ref<Resource> resource = memnew(Resource());
    
    RID rid = storage.add(resource);
    
    CHECK_MESSAGE(rid.is_valid(), "RID should be valid");
    CHECK_MESSAGE(storage.has(rid), "Storage should contain resource");
}
```

#### Thread-Safety Test
```cpp
#ifndef DISABLE_THREADED_TESTS
TEST_CASE("[RefStorage] Thread-safety - concurrent adds") {
    RefStorage storage;
    const int THREAD_COUNT = 4;
    
    // Create and start threads
    Thread threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i].start(thread_function, &data);
    }
    
    // Wait for completion
    for (int i = 0; i < THREAD_COUNT; i++) {
        threads[i].wait_to_finish();
    }
    
    // Validate results
    CHECK_MESSAGE(storage.size() == expected, "All operations completed");
}
#endif
```

#### Stress Test
```cpp
TEST_CASE("[RefStorage] Stress test - many resources") {
    RefStorage storage;
    const int COUNT = 10000;
    
    for (int i = 0; i < COUNT; i++) {
        storage.add(memnew(Resource()));
    }
    
    CHECK_MESSAGE(storage.size() == COUNT, "All resources added");
}
```

---

## Test Coverage

### Current Coverage

| Utility | Unit Tests | Thread Tests | Stress Tests | Integration | Status |
|---------|------------|--------------|--------------|-------------|--------|
| RefStorage | ✅ 10 | ✅ 2 | ✅ 1 | 🔄 | Complete |
| NodeStorage | ✅ 11 | ✅ 2 | ✅ 1 | 🔄 | Complete |
| SceneObjectUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| ResourceObjectUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| World2DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| World3DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| Physics2DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| Physics3DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| Navigation2DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| Navigation3DUtility | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| RenderUtility2D | 🔄 | 🔄 | 🔄 | 🔄 | Pending |
| RenderUtility3D | 🔄 | 🔄 | 🔄 | 🔄 | Pending |

### Target Coverage

- **100% of public API methods** tested
- **All thread-safety claims** validated with concurrent tests
- **Common edge cases** covered (null inputs, invalid RIDs, etc.)
- **Performance characteristics** measured and documented

---

## Test Implementation Details

### RefStorage Tests (`test_ref_storage.h`)

**Test Cases (13 total):**

1. ✅ Constructor and basic properties
2. ✅ Add and retrieve single resource
3. ✅ Add multiple resources
4. ✅ Release single resource
5. ✅ Release nonexistent resource
6. ✅ Release all resources
7. ✅ Get nonexistent resource
8. ✅ Has with invalid RID
9. ✅ Resource reference counting
10. ✅ Add null resource
11. ✅ Move semantics
12. ✅ Stress test - many resources (1000 resources)
13. ✅ Thread-safety - concurrent adds (4 threads × 100 ops)
14. ✅ Thread-safety - concurrent reads and writes (4 threads × 50 ops)

**Coverage:**
- All public methods: `add()`, `release()`, `release_all()`, `has()`, `get()`, `size()`, `is_empty()`
- Move constructor and move assignment
- Thread-safety with mutex protection
- Edge cases: null resources, invalid RIDs, nonexistent entries

### NodeStorage Tests (`test_node_storage.h`)

**Test Cases (14 total):**

1. ✅ Constructor and basic properties
2. ✅ Add and retrieve single node
3. ✅ Add multiple nodes
4. ✅ Release single node
5. ✅ Release nonexistent node
6. ✅ Release all nodes
7. ✅ Try get nonexistent node
8. ✅ Has with invalid RID
9. ✅ Add null node
10. ✅ Get all IDs
11. ✅ Make inert (nullify node reference)
12. ✅ Move semantics
13. ✅ Stress test - many nodes (1000 nodes)
14. ✅ Thread-safety - concurrent adds (4 threads × 100 ops)
15. ✅ Thread-safety - concurrent reads and writes (4 threads × 50 ops)

**Coverage:**
- All public methods: `add()`, `release()`, `release_all()`, `has()`, `try_get()`, `make_inert()`, `get_all_ids()`, `size()`, `is_empty()`
- Move constructor and move assignment
- Thread-safety with mutex protection
- Node lifecycle management
- Edge cases: null nodes, invalid RIDs, inert nodes

---

## Thread-Safety Testing Strategy

### Concurrent Operations Tested

1. **Concurrent Adds**
   - Multiple threads adding resources/nodes simultaneously
   - Validates mutex protection prevents corruption
   - Ensures all operations complete successfully

2. **Concurrent Reads and Writes**
   - Mixed reader and writer threads
   - Validates no race conditions
   - Ensures data consistency

3. **High Contention Scenarios**
   - Many threads accessing same storage
   - Small delays to increase lock contention
   - Validates deadlock-free operation

### Thread Test Parameters

```cpp
const int THREAD_COUNT = 4;          // Number of concurrent threads
const int OPS_PER_THREAD = 100;      // Operations per thread
const int CONTENTION_DELAY_US = 1;   // Microsecond delay between ops
```

### Thread-Safety Guarantees Validated

✅ **RefStorage:**
- Concurrent `add()` operations are safe
- Concurrent `get()` operations are safe
- Concurrent `has()` operations are safe
- Mixed reads and writes are safe
- No memory corruption under concurrent access

✅ **NodeStorage:**
- Concurrent `add()` operations are safe
- Concurrent `try_get()` operations are safe
- Concurrent `has()` operations are safe
- Mixed reads and writes are safe
- No memory corruption under concurrent access

---

## Edge Cases Tested

### Common Edge Cases

- ✅ Null/nullptr inputs
- ✅ Invalid RIDs
- ✅ Nonexistent entries
- ✅ Empty storage operations
- ✅ Duplicate operations (release same RID twice)
- ✅ Move semantics edge cases

### Resource-Specific Edge Cases

- ✅ Reference counting during storage lifecycle
- ✅ Resource destruction coordination
- ✅ Ref<T> validity after release

### Node-Specific Edge Cases

- ✅ Node destruction coordination
- ✅ Inert node behavior (nullified references)
- ✅ Node pointer validity after release

---

## Performance Characteristics

### Stress Test Results

**RefStorage (1000 resources):**
- Add operations: O(1) average
- Has operations: O(1) average
- Get operations: O(1) average
- Release operations: O(1) average

**NodeStorage (1000 nodes):**
- Add operations: O(1) average
- Has operations: O(1) average
- Try_get operations: O(1) average
- Release operations: O(1) average

### Thread Contention

With 4 concurrent threads and 100 operations each:
- No deadlocks observed
- All operations complete successfully
- No data corruption detected

---

## Next Steps

### Immediate Priorities

1. **Conversion Utility Tests** (High Priority)
   - [ ] SceneObjectUtility tests
     - Scene traversal
     - Node type detection
     - Entity creation
     - Hierarchy handling
     - Max depth limits
   - [ ] ResourceObjectUtility tests
     - Resource entity creation
     - Script attachment
   - [ ] World utility tests
     - Auto-create vs use-existing
     - Update-in-place behavior

2. **Domain Utility Tests** (High Priority)
   - [ ] Physics utility tests (2D & 3D)
   - [ ] Navigation utility tests (2D & 3D)
   - [ ] Render utility tests (2D & 3D)

3. **Integration Tests** (Medium Priority)
   - [ ] Full scene conversion
   - [ ] Complex hierarchies
   - [ ] Mixed node types
   - [ ] Resource dependencies

### Future Enhancements

1. **Performance Benchmarks**
   - [ ] Conversion speed measurements
   - [ ] Memory usage profiling
   - [ ] Lock contention analysis

2. **Memory Safety Tests**
   - [ ] Leak detection
   - [ ] Use-after-free detection
   - [ ] Reference counting validation

3. **Stress Tests**
   - [ ] Very large scenes (10,000+ nodes)
   - [ ] Deep hierarchies (100+ levels)
   - [ ] Rapid add/remove cycles

---

## Best Practices

### Writing Tests

1. **Descriptive Names**
   ```cpp
   TEST_CASE("[Category] What is being tested") {
       // Clear, specific test name
   }
   ```

2. **Clear Assertions**
   ```cpp
   CHECK_MESSAGE(
       condition,
       "Explain what should happen and what went wrong");
   ```

3. **Proper Cleanup**
   ```cpp
   // Always clean up allocated resources
   storage.release_all();
   memdelete(node);
   ```

4. **Thread Test Guards**
   ```cpp
   #ifndef DISABLE_THREADED_TESTS
   // Thread tests here
   #endif
   ```

### Test Organization

- Group related tests in namespaces
- Use helper classes for test data
- Keep tests independent (no shared state)
- Test one thing per test case

### Performance Testing

- Use realistic data sizes
- Measure multiple runs for average
- Document hardware used
- Note any environment-specific results

---

## Troubleshooting

### Common Issues

**Tests not found:**
- Ensure `#include "test_main.h"` in test runner
- Check that test files are in correct directory
- Verify test registration macros are correct

**Thread tests failing:**
- May be disabled with `DISABLE_THREADED_TESTS`
- Check for platform-specific threading issues
- Verify OS threading primitives work correctly

**Memory leaks:**
- Ensure all `memnew()` calls have matching `memdelete()`
- Check that storage `release()` is called
- Use proper Ref<T> for Resources

**Flaky tests:**
- Thread tests may occasionally fail due to timing
- Use proper synchronization primitives
- Avoid relying on precise timing

---

## Contributing Tests

When adding new tests:

1. Follow existing test structure and style
2. Add comprehensive documentation
3. Test both success and failure cases
4. Include thread-safety tests where applicable
5. Add stress tests for performance validation
6. Update this guide with new test information

---

## Test Metrics

### Current Status

| Metric | Value |
|--------|-------|
| Test Files | 3 (2 implemented, 1 registration) |
| Total Test Cases | 27 |
| Lines of Test Code | ~1,006 |
| Utilities Covered | 2/12 (17%) |
| Thread Tests | 4 |
| Stress Tests | 2 |

### Target Goals

| Metric | Target |
|--------|--------|
| Utilities Covered | 12/12 (100%) |
| Total Test Cases | 150+ |
| Code Coverage | 90%+ |
| Thread Tests | 24+ |
| Stress Tests | 12+ |

---

## References

- [Godot Test Framework Documentation](https://docs.godotengine.org/en/stable/contributing/development/core_and_modules/unit_testing.html)
- [doctest Documentation](https://github.com/doctest/doctest)
- [Module README](../ecs/utility/README.md)
- [Phase 2 Documentation](../ecs/utility/DOMAIN_UTILITIES_DOC.md)

---

**Version:** 1.1  
**Last Updated:** 2025-01-29  
**Status:** Active  
**Next Review:** After network tests implemented