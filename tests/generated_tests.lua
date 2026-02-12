-- Auto-generated test file from XML specifications
-- Do not edit manually - regenerate using 'make modules'

local orca = require "orca"

-- Tests for Object

local function test_Object_AddChild_add_child_to_parent()
	-- Test adding a child to a parent object
	local parent = orca.ui.Screen { Name = "Parent", Width = 800, Height = 600 }
	local child = orca.ui.TextBlock { Name = "Child", Text = "Test" }
	parent:addChild(child)
	local found = parent:findChild("Child", false)
	assert(found == child, "Child should be added to parent and findable")
	parent:clear()
end

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
		{ name = "Object.AddChild.add_child_to_parent", func = test_Object_AddChild_add_child_to_parent },
		{ name = "Object.FindChild.find_immediate_child", func = test_Object_FindChild_find_immediate_child },
		{ name = "Object.FindChild.find_recursive_child", func = test_Object_FindChild_find_recursive_child },
		{ name = "Object.GetName.get_object_name", func = test_Object_GetName_get_object_name },
		{ name = "Object.SetName.set_object_name", func = test_Object_SetName_set_object_name },
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
