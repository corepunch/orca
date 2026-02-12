# Unit Test Specifications in XML

This document explains how to add unit test specifications to the core.xml (and other module XML files) to automatically generate test cases.

## Overview

The ORCA build system now supports embedding unit test specifications directly in the XML files that define the API (such as `core.xml`, `UIKit.xml`, etc.). These test specifications are automatically extracted during the build process and compiled into a Lua test file.

## Benefits

- **Documentation**: Tests serve as executable documentation showing how to use each function
- **Validation**: Automatically verify that the API works as documented
- **Regression Testing**: Catch breaking changes when modifying the codebase
- **Co-location**: Keep tests close to the function definitions they test

## XML Test Format

Tests are defined as `<test>` elements within `<method>` or `<function>` elements:

```xml
<method name="MethodName">
  <summary>Brief description of the method</summary>
  <details>Detailed description...</details>
  <arg name="param1" type="string">Parameter description</arg>
  <returns type="bool">Return value description</returns>
  
  <test name="test_identifier">
    <description>Human-readable test description</description>
    <code><![CDATA[
-- Lua test code goes here
local obj = orca.ui.TextBlock { Name = "Test" }
assert(obj:getName() == "Test", "Name should match")
obj:removeFromParent()
    ]]></code>
  </test>
  
  <test name="another_test">
    <description>Another test case for the same method</description>
    <code><![CDATA[
-- More test code...
    ]]></code>
  </test>
</method>
```

### Test Element Structure

- `<test name="...">`: Container for a single test case
  - `name` attribute: Unique identifier for the test (used in generated function names)
  
- `<description>`: Brief explanation of what the test validates (optional but recommended)

- `<code>`: Lua code to execute for the test
  - Should be wrapped in `<![CDATA[...]]>` to preserve special characters
  - Should include assertions to validate behavior
  - Should clean up any created objects

## Example: Testing Object Methods

Here's a complete example from `core.xml`:

```xml
<method name="FindChild">
  <summary>Find a child object by name.</summary>
  <details>Searches for a child object with the specified name...</details>
  <arg name="name" type="string">The exact name of the child object to locate</arg>
  <arg name="recursive" type="bool">If true, searches recursively through all descendants</arg>
  <returns type="Object" pointer="true">Pointer to the child object or null if not found</returns>
  
  <test name="find_immediate_child">
    <description>Test finding an immediate child by name</description>
    <code><![CDATA[
local parent = orca.ui.Screen { Name = "Parent", Width = 800, Height = 600 }
local child = orca.ui.TextBlock { Name = "Child1", Text = "Hello" }
parent:addChild(child)
local found = parent:findChild("Child1", false)
assert(found == child, "Should find immediate child by name")
assert(parent:findChild("NonExistent", false) == nil, "Should return nil for non-existent child")
parent:clear()
    ]]></code>
  </test>
  
  <test name="find_recursive_child">
    <description>Test finding a nested child recursively</description>
    <code><![CDATA[
local grandparent = orca.ui.Screen { Name = "GrandParent", Width = 800, Height = 600 }
local parent = orca.ui.StackView { Name = "Parent" }
local child = orca.ui.TextBlock { Name = "DeepChild", Text = "Nested" }
parent:addChild(child)
grandparent:addChild(parent)
local found = grandparent:findChild("DeepChild", true)
assert(found == child, "Should find deeply nested child with recursive search")
assert(grandparent:findChild("DeepChild", false) == nil, "Should not find nested child without recursive search")
grandparent:clear()
    ]]></code>
  </test>
</method>
```

## Build Process

When you run `make modules`, the `conv-module.py` script:

1. Parses all XML module files
2. Extracts `<test>` elements from methods and functions
3. Generates `tests/generated_tests.lua` containing:
   - Individual test functions for each test case
   - A test runner that executes all tests
   - Pass/fail reporting

## Running Tests

After building the project with `make`, run the generated tests:

```bash
# Run all generated tests
./build/bin/orca tests/generated_tests.lua

# Or use the test target
make test
```

The test runner will:
- Execute each test function
- Print pass/fail status for each test
- Display a summary at the end
- Exit with status 0 if all tests pass, 1 if any fail

## Test Output Example

```
✓ PASSED: Object.AddChild.add_child_to_parent
✓ PASSED: Object.FindChild.find_immediate_child
✓ PASSED: Object.FindChild.find_recursive_child
✓ PASSED: Object.GetName.get_object_name
✓ PASSED: Object.SetName.set_object_name

==================================================
Tests: 5 passed, 0 failed, 5 total
==================================================
```

## Best Practices

1. **Keep tests focused**: Each test should validate one specific behavior
2. **Use descriptive names**: Test names should clearly indicate what is being tested
3. **Clean up resources**: Always clean up objects created during tests (e.g., `removeFromParent()`, `clear()`)
4. **Include edge cases**: Test both normal and boundary conditions
5. **Add assertions**: Every test should have at least one assertion
6. **Use clear messages**: Assertion messages should explain what went wrong

## Test Naming Convention

Generated test function names follow this pattern:
```
test_<Interface>_<Method>_<test_name>
```

For example:
- `test_Object_FindChild_find_immediate_child`
- `test_Object_SetName_set_object_name`

## Limitations

- Tests are written in Lua and test the Lua API bindings
- Tests run in sequence (not parallel)
- Each test should be independent (no shared state between tests)
- Generated test file is overwritten on each build (do not edit manually)

## Future Enhancements

Potential improvements to consider:

- Support for setup/teardown functions
- Test fixtures and shared test data
- Performance/benchmark tests
- Coverage reporting
- Integration with CI/CD pipelines
