# Unit Test Feature - Complete Example

This document shows a complete example of adding unit tests to the core.xml file and the resulting generated test code.

## Example: Testing the SetName and GetName Methods

### 1. XML Source (source/core/core.xml)

Here's how the methods are defined with embedded test specifications:

```xml
<method name="GetName" const="true">
  <summary>Retrieves the object's name identifier</summary>
  <details>Returns the string name assigned to this object instance for identification and lookup purposes.</details>
  <returns type="string">Current name of the object</returns>
  
  <test name="get_object_name">
    <description>Test retrieving object name</description>
    <code><![CDATA[
local obj = orca.ui.TextBlock { Name = "TestObject" }
assert(obj:getName() == "TestObject", "getName should return the object's name")
obj:removeFromParent()
    ]]></code>
  </test>
</method>

<method name="SetName">
  <summary>Sets the object's name identifier</summary>
  <details>Assigns a new name to this object for identification. Names are typically used for hierarchical lookups and debugging.</details>
  <arg name="name" type="string">New name to assign to the object</arg>
  
  <test name="set_object_name">
    <description>Test setting object name</description>
    <code><![CDATA[
local obj = orca.ui.TextBlock { Name = "OriginalName" }
assert(obj:getName() == "OriginalName", "Initial name should be OriginalName")
obj:setName("NewName")
assert(obj:getName() == "NewName", "Name should be updated to NewName")
obj:removeFromParent()
    ]]></code>
  </test>
</method>
```

### 2. Build Process

Run the module generation:

```bash
$ make modules
```

Output:
```
cd tools && \
python3 conv-module.py \
../source/geometry/geom.xml \
../source/core/core.xml \
../source/renderer/api/renderer.xml \
../source/UIKit/UIKit.xml \
../source/SceneKit/SceneKit.xml \
../source/filesystem/filesystem.xml && cd ..
Generated 5 tests in ../tests/generated_tests.lua
```

### 3. Generated Test File (tests/generated_tests.lua)

The conv-module.py script extracts the tests and generates:

```lua
-- Auto-generated test file from XML specifications
-- Do not edit manually - regenerate using 'make modules'

local orca = require "orca"

-- Tests for Object

local function test_Object_GetName_get_object_name()
	-- Test retrieving object name
	local obj = orca.ui.TextBlock { Name = "TestObject" }
	assert(obj:getName() == "TestObject", "getName should return the object's name")
	obj:removeFromParent()
end

local function test_Object_SetName_set_object_name()
	-- Test setting object name
	local obj = orca.ui.TextBlock { Name = "OriginalName" }
	assert(obj:getName() == "OriginalName", "Initial name should be OriginalName")
	obj:setName("NewName")
	assert(obj:getName() == "NewName", "Name should be updated to NewName")
	obj:removeFromParent()
end

-- Run all tests
local function run_all_tests()
	local passed = 0
	local failed = 0
	local tests = {
		{ name = "Object.GetName.get_object_name", func = test_Object_GetName_get_object_name },
		{ name = "Object.SetName.set_object_name", func = test_Object_SetName_set_object_name },
		-- ... more tests ...
	}

	for _, test in ipairs(tests) do
		local success, err = pcall(test.func)
		if success then
			print("✓ PASSED: " .. test.name)
			passed = passed + 1
		else
			print("✗ FAILED: " .. test.name)
			print("  Error: " .. tostring(err))
			failed = failed + 1
		end
	end

	print("\n" .. string.rep("=", 50))
	print(string.format("Tests: %d passed, %d failed, %d total", passed, failed, passed + failed))
	print(string.rep("=", 50))
	return failed == 0
end

-- Execute tests
local success = run_all_tests()
os.exit(success and 0 or 1)
```

### 4. Running the Tests

```bash
$ ./build/bin/orca tests/generated_tests.lua
```

Output:
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

## Example: Testing FindChild with Multiple Test Cases

Here's a more complex example with multiple test cases for the same method:

### XML Definition

```xml
<method name="FindChild">
  <summary>Find a child object by name.</summary>
  <details>Searches for a child object with the specified name. When recursive is false, only searches immediate children. When recursive is true, performs a depth-first search through the entire subtree to find the first object with a matching name.</details>
  <arg name="name" type="string">The exact name of the child object to locate</arg>
  <arg name="recursive" type="bool">If true, searches recursively through all descendants; if false, searches only immediate children</arg>
  <returns type="Object" pointer="true">Pointer to the first child object found with the matching name, or null if no match is found</returns>
  
  <!-- Test Case 1: Immediate children -->
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
  
  <!-- Test Case 2: Recursive search -->
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

### Generated Test Functions

```lua
local function test_Object_FindChild_find_immediate_child()
	-- Test finding an immediate child by name
	local parent = orca.ui.Screen { Name = "Parent", Width = 800, Height = 600 }
	local child = orca.ui.TextBlock { Name = "Child1", Text = "Hello" }
	parent:addChild(child)
	local found = parent:findChild("Child1", false)
	assert(found == child, "Should find immediate child by name")
	assert(parent:findChild("NonExistent", false) == nil, "Should return nil for non-existent child")
	parent:clear()
end

local function test_Object_FindChild_find_recursive_child()
	-- Test finding a nested child recursively
	local grandparent = orca.ui.Screen { Name = "GrandParent", Width = 800, Height = 600 }
	local parent = orca.ui.StackView { Name = "Parent" }
	local child = orca.ui.TextBlock { Name = "DeepChild", Text = "Nested" }
	parent:addChild(child)
	grandparent:addChild(parent)
	local found = grandparent:findChild("DeepChild", true)
	assert(found == child, "Should find deeply nested child with recursive search")
	assert(grandparent:findChild("DeepChild", false) == nil, "Should not find nested child without recursive search")
	grandparent:clear()
end
```

## Key Advantages

1. **Documentation as Code**: Tests serve as executable examples of how to use each function
2. **Automatic Generation**: No need to manually maintain test boilerplate
3. **Co-location**: Tests are defined right next to the functions they test
4. **Easy Discovery**: All tests are collected and run automatically
5. **Regression Prevention**: Changes that break tests are immediately visible

## Workflow Summary

```
┌─────────────────────┐
│  Edit core.xml      │
│  Add <test> blocks  │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────┐
│  make modules       │
│  (conv-module.py)   │
└──────────┬──────────┘
           │
           ▼
┌─────────────────────────────┐
│  generated_tests.lua        │
│  - Test functions           │
│  - Test runner              │
│  - Pass/fail reporting      │
└──────────┬──────────────────┘
           │
           ▼
┌─────────────────────┐
│  make               │
│  Build project      │
└──────────┬──────────┘
           │
           ▼
┌──────────────────────────┐
│  ./build/bin/orca        │
│  tests/generated_tests   │
└──────────┬───────────────┘
           │
           ▼
┌─────────────────────┐
│  Test Results       │
│  ✓ PASSED / ✗ FAILED│
└─────────────────────┘
```
