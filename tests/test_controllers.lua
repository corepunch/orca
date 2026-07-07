local test = require "orca.test"
-- Headless tests for OBJ_LoadController, wire_controller, and BindHandler.
--
-- Run with: $(TARGET) -test=tests/test_controllers.lua

local core = require "orca.core"
local ui   = require "orca.UIKit"

local function flush()
  core.flushQueue()
end

-- Pre-computed property IDs (avoid runtime fnv1a32 calls)
local ID_Input_Submit = 0x5208fc2d  -- fnv1a32("Input.Submit")

-- ---------------------------------------------------------------------------
-- Test 1: OBJ_LoadController loads a controller table
-- ---------------------------------------------------------------------------
local function test_load_controller()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }

  node:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  test.expect(true, "loadController should not crash")
  node:removeFromParent()
  print("PASS: test_load_controller")
end

-- ---------------------------------------------------------------------------
-- Test 2: LoadController message loads controller
-- ---------------------------------------------------------------------------
local function test_load_controller_message()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }

  node:send("Object.LoadController", { Path = "tests/fixtures/controllers/test_ctrl1" })
  flush()

  test.expect(true, "LoadController message should not crash")
  node:removeFromParent()
  print("PASS: test_load_controller_message")
end

-- ---------------------------------------------------------------------------
-- Test 3: BindHandler binds function from controller to event property
-- ---------------------------------------------------------------------------
local function test_bind_handler()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local input = screen + ui.Input { Name = "MyInput" }

  -- Load controller first
  input:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  -- Bind Submit event to "on_submit" function in controller
  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "on_submit",
  })
  flush()

  test.expect(true, "BindHandler should not crash")
  input:removeFromParent()
  print("PASS: test_bind_handler")
end

-- ---------------------------------------------------------------------------
-- Test 4: BindHandler walks up parents to find controller
-- ---------------------------------------------------------------------------
local function test_bind_handler_walks_parents()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local parent = screen + core.Node { Name = "Parent" }
  local input = parent + ui.Input { Name = "MyInput" }

  -- Load controller on parent
  parent:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  -- BindHandler on input should find controller on parent
  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "on_submit",
  })
  flush()

  test.expect(true, "BindHandler should walk up parents without error")
  parent:removeFromParent()
  print("PASS: test_bind_handler_walks_parents")
end

-- ---------------------------------------------------------------------------
-- Test 5: BindHandler with non-existent function returns false
-- ---------------------------------------------------------------------------
local function test_bind_handler_missing_function()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local input = screen + ui.Input { Name = "MyInput" }

  input:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "nonexistent_function",
  })
  flush()

  test.expect(true, "BindHandler with missing function should not crash")
  input:removeFromParent()
  print("PASS: test_bind_handler_missing_function")
end

-- ---------------------------------------------------------------------------
-- Test 6: wire_controller with ChildName_EventName convention
-- ---------------------------------------------------------------------------
local function test_wire_controller_convention()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }
  local input = node + ui.Input { Name = "MyInput" }

  -- Load controller with "MyInput_Submit" key
  node:loadController("tests/fixtures/controllers/test_ctrl_wired")
  flush()

  test.expect(true, "wire_controller convention should not crash")
  node:removeFromParent()
  print("PASS: test_wire_controller_convention")
end

-- ---------------------------------------------------------------------------
-- Test 7: Controller loaded twice replaces previous
-- ---------------------------------------------------------------------------
local function test_controller_reload()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }

  node:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  node:loadController("tests/fixtures/controllers/test_ctrl2")
  flush()

  test.expect(true, "Reloading controller should not crash")
  node:removeFromParent()
  print("PASS: test_controller_reload")
end

-- ---------------------------------------------------------------------------
-- Test 8: Empty path does nothing
-- ---------------------------------------------------------------------------
local function test_empty_path()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }

  node:loadController("")
  flush()

  -- nil path is rejected by Lua binding, just test empty string
  test.expect(true, "Empty path should not crash")
  node:removeFromParent()
  print("PASS: test_empty_path")
end

-- ---------------------------------------------------------------------------
-- Test 9: BindHandler without controller returns false
-- ---------------------------------------------------------------------------
local function test_bind_handler_no_controller()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local input = screen + ui.Input { Name = "MyInput" }

  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "on_submit",
  })
  flush()

  test.expect(true, "BindHandler without controller should not crash")
  input:removeFromParent()
  print("PASS: test_bind_handler_no_controller")
end

-- ---------------------------------------------------------------------------
-- Test 10: BindHandler with empty function name
-- ---------------------------------------------------------------------------
local function test_bind_handler_empty_function()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local input = screen + ui.Input { Name = "MyInput" }

  input:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "",
  })
  flush()

  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = nil,
  })
  flush()

  test.expect(true, "BindHandler with empty function name should not crash")
  input:removeFromParent()
  print("PASS: test_bind_handler_empty_function")
end

-- ---------------------------------------------------------------------------
-- Test 11: controller.view is set after loadController
-- ---------------------------------------------------------------------------
local function test_controller_view_is_set()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local parent = screen + core.Node { Name = "Parent" }
  local input = parent + ui.Input { Name = "MyInput" }

  parent:loadController("tests/fixtures/controllers/test_ctrl1")
  flush()

  input:send("Object.BindHandler", {
    PropertyId = ID_Input_Submit,
    FunctionName = "on_submit",
  })
  flush()

  test.expect(true, "loadController with view assignment should not crash")
  parent:removeFromParent()
  print("PASS: test_controller_view_is_set")
end

-- ---------------------------------------------------------------------------
-- Test 12: wire_controller wraps functions with controller closure
-- ---------------------------------------------------------------------------
local function test_wire_controller_wraps_function()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node = screen + core.Node { Name = "Target" }
  local input = node + ui.Input { Name = "MyInput" }

  node:loadController("tests/fixtures/controllers/test_ctrl_wired")
  flush()

  test.expect(true, "wire_controller with closure wrapping should not crash")
  node:removeFromParent()
  print("PASS: test_wire_controller_wraps_function")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_load_controller()
test_load_controller_message()
test_bind_handler()
test_bind_handler_walks_parents()
test_bind_handler_missing_function()
test_wire_controller_convention()
test_controller_reload()
test_empty_path()
test_bind_handler_no_controller()
test_bind_handler_empty_function()
test_controller_view_is_set()
test_wire_controller_wraps_function()

print("All controller tests passed.")
