local test = require "orca.test"
-- Headless tests for orca.core.application
-- Covers: route return value stored in ctx.content.inner,
-- class-based layout receives render context, class-level helpers
-- accessible in layout, and dispatch result structure.
--
-- Uses a mock layout Widget that avoids UIKit so tests run fully headless.

local Widget      = require "orca.core.widget"
local Application = require "orca.core.application"
local system      = require "orca.system"
local filesystem  = require "orca.filesystem"

-- ---------------------------------------------------------------------------
-- A minimal mock layout: captures inner and render context for inspection
-- ---------------------------------------------------------------------------
local MockLayout = Widget:extend {
  content = function(self)
    self._inner = self:content_for("inner")
    self._title = self.app_title and self:app_title() or nil
    return "layout_result"
  end
}

-- ---------------------------------------------------------------------------
-- Test 1: route return value is stored in ctx.content.inner
-- ---------------------------------------------------------------------------
local function test_route_result_stored_in_inner()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return "my_body" end,
  }
  local app = App()
  local result = app:dispatch("/")

  test.expect(result ~= nil, "dispatch should return a result table")
  test.expect(result.context ~= nil, "result should have a context")
  test.expect_eq(result.context.content.inner, "my_body", "route return should be ctx.content.inner")
  print("PASS: test_route_result_stored_in_inner")
end

-- ---------------------------------------------------------------------------
-- Test 2: class-based layout's content() is called and its return is view
-- ---------------------------------------------------------------------------
local function test_layout_content_called()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return "body_value" end,
  }
  local app = App()
  local result = app:dispatch("/")

  test.expect_eq(result.view, "layout_result", "layout:content() return should be result.view")
  print("PASS: test_layout_content_called")
end

-- ---------------------------------------------------------------------------
-- Test 3: layout receives render context via set_render_context
-- ---------------------------------------------------------------------------
local function test_layout_receives_render_context()
  local captured_layout = nil

  local CapturingLayout = Widget:extend {
    content = function(self)
      captured_layout = self
      return "ok"
    end
  }

  local App = Application:extend {
    layout = CapturingLayout,
    ["/"] = function(self) return "x" end,
  }
  local app = App()
  local result = app:dispatch("/")

  test.expect(captured_layout ~= nil, "layout content() should have been called")
  test.expect(captured_layout:get_render_context() ~= nil, "layout should have render context")
  test.expect_eq(captured_layout:content_for("inner"), "x", "inner should be accessible in layout")
  print("PASS: test_layout_receives_render_context")
end

-- ---------------------------------------------------------------------------
-- Test 4: class-level helpers are accessible inside layout via self:app_title()
-- ---------------------------------------------------------------------------
local function test_class_helpers_accessible_in_layout()
  local captured_title = nil

  local TitleLayout = Widget:extend {
    content = function(self)
      captured_title = self.app_title and self:app_title() or nil
      return "ok"
    end
  }

  local App = Application:extend {
    layout = TitleLayout,
    ["/"] = function(self) return "body" end,
  }
  App:include_helpers({
    app_title = function(self) return "MyApp" end
  })

  local app = App()
  app:dispatch("/")

  test.expect_eq(captured_title, "MyApp", "app_title helper should be visible in layout")
  print("PASS: test_class_helpers_accessible_in_layout")
end

-- ---------------------------------------------------------------------------
-- Test 5: unknown route returns nil body but still produces a result table
-- ---------------------------------------------------------------------------
local function test_unknown_route_returns_nil_body()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return "home" end,
  }
  local app = App()
  local result = app:dispatch("/missing")

  test.expect(result ~= nil, "dispatch should always return a result table")
  test.expect_eq(result.context.content.inner, nil, "unknown route should yield nil inner")
  print("PASS: test_unknown_route_returns_nil_body")
end

-- ---------------------------------------------------------------------------
-- Test 6: dispatch result always has view and context keys
-- ---------------------------------------------------------------------------
local function test_dispatch_result_shape()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return "content" end,
  }
  local app = App()
  local result = app:dispatch("/")

  test.expect(result.view ~= nil,    "result.view should be present")
  test.expect(result.context ~= nil, "result.context should be present")
  print("PASS: test_dispatch_result_shape")
end

-- ---------------------------------------------------------------------------
-- Test 7: resolve_body default passes body through unchanged
-- ---------------------------------------------------------------------------
local function test_resolve_body_default_passthrough()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return "raw_body" end,
  }
  local app = App()
  local route_info = app.router:resolve("/")
  local result = app:resolve_body("raw_body", route_info)

  test.expect_eq(result, "raw_body", "default resolve_body should return body unchanged")
  print("PASS: test_resolve_body_default_passthrough")
end

-- ---------------------------------------------------------------------------
-- Test 8: resolve_body with render:true loads and returns the view instance
-- ---------------------------------------------------------------------------
local function test_resolve_body_render_true_loads_view()
  local stub_content = "stub_view_content"
  local StubView = Widget:extend {
    content = function(self) return stub_content end
  }
  package.loaded["myviews/Home"] = StubView

  local App = Application:extend {
    layout = MockLayout,
    views_prefix = "myviews",
    [{ Home = "/" }] = function(self) return { render = true } end,
  }
  local app = App()
  local route_info = app.router:resolve("/")
  local result = app:resolve_body({ render = true }, route_info)

  test.expect(result ~= nil, "resolve_body should return a view instance")
  test.expect_eq(result:content(), stub_content, "resolved view content() should match stub")
  package.loaded["myviews/Home"] = nil
  print("PASS: test_resolve_body_render_true_loads_view")
end

-- ---------------------------------------------------------------------------
-- Test 9: render:true propagates through dispatch into ctx.content.inner
-- ---------------------------------------------------------------------------
local function test_render_true_dispatch_end_to_end()
  local stub_content = "e2e_view_content"
  local StubView2 = Widget:extend {
    content = function(self) return stub_content end
  }
  package.loaded["app2views/Home"] = StubView2

  local App = Application:extend {
    layout = MockLayout,
    views_prefix = "app2views",
    [{ Home = "/" }] = function(self) return { render = true } end,
  }
  local app = App()
  local result = app:dispatch("/")

  test.expect_eq(result.context.content.inner, stub_content,
    "render:true should resolve view and store content in ctx.content.inner")
  package.loaded["app2views/Home"] = nil
  print("PASS: test_render_true_dispatch_end_to_end")
end

-- ---------------------------------------------------------------------------
-- Test 10: render:true with no views_prefix falls back to body passthrough
-- ---------------------------------------------------------------------------
local function test_render_true_without_prefix_passthrough()
  local App = Application:extend {
    layout = MockLayout,
    ["/"] = function(self) return { render = true } end,
  }
  local app = App()
  local route_info = app.router:resolve("/")
  local result = app:resolve_body({ render = true }, route_info)

  test.expect(type(result) == "table", "without views_prefix body should pass through as table")
  test.expect(result.render == true, "render flag should be preserved when no views_prefix")
  print("PASS: test_render_true_without_prefix_passthrough")
end

-- ---------------------------------------------------------------------------
-- Test 11: non-render-true body is never touched by resolve_body
-- ---------------------------------------------------------------------------
local function test_resolve_body_ignores_non_render_true()
  local App = Application:extend {
    layout = MockLayout,
    views_prefix = "irrelevant",
    ["/"] = function(self) return "literal_string" end,
  }
  local app = App()
  local route_info = app.router:resolve("/")
  local result = app:resolve_body("literal_string", route_info)

  test.expect_eq(result, "literal_string", "non-render:true body should pass through unchanged")
  print("PASS: test_resolve_body_ignores_non_render_true")
end

-- ---------------------------------------------------------------------------
-- Test 12: detached helper method reference keeps first payload argument
-- ---------------------------------------------------------------------------
local function test_detached_helper_method_keeps_argument()
  local captured_route = nil

  local NavLayout = Widget:extend {
    content = function(self)
      local navigate = self.navigate
      navigate("/alerts")
      return "ok"
    end
  }

  local App = Application:extend {
    layout = NavLayout,
    ["/"] = function(self) return "home" end,
    navigate = function(self, route, ...)
      captured_route = route
    end,
  }
  local app = App()
  app:dispatch("/")

  test.expect_eq(captured_route, "/alerts", "navigate = self.navigate should preserve route argument")
  print("PASS: test_detached_helper_method_keeps_argument")
end

-- ---------------------------------------------------------------------------
-- Test 13: activate_controller updates active screen, app singleton, and editor host
-- ---------------------------------------------------------------------------
local function test_activate_controller_updates_screen()
  local called_editor_screen = nil
  local called_post = nil
  local original_load_editor = Application.load_editor

  Application.load_editor = function(screen)
    called_editor_screen = screen
  end

  local FakeScreen = {}
  FakeScreen.__index = FakeScreen
  function FakeScreen:post(message, size)
    called_post = { message, size }
  end
  local screen = setmetatable({}, FakeScreen)

  local ctrl = { view = screen }
  local app = Application()
  local result = app:activate_controller(ctrl)

  Application.load_editor = original_load_editor

  test.expect_eq(result, ctrl, "activate_controller should return the controller")
  test.expect_eq(app.controller, ctrl, "activate_controller should store controller on app")
  test.expect_eq(app.screen, screen, "activate_controller should store screen on app")
  test.expect_eq(Application.app, app, "activate_controller should update Application.app")
  test.expect_eq(called_editor_screen, screen, "activate_controller should update editor host screen")
  test.expect(called_post ~= nil, "activate_controller should request a paint")
  test.expect_eq(called_post[1], "Window.Paint", "activate_controller should post Window.Paint")
  print("PASS: test_activate_controller_updates_screen")
end

-- ---------------------------------------------------------------------------
-- Test 14: Application.current returns singleton app instance
-- ---------------------------------------------------------------------------
local function test_application_current_returns_app()
  local previous = Application.app
  local app = Application()
  Application.app = app

  local current = Application.current()

  Application.app = previous

  test.expect_eq(current, app, "Application.current should return Application.app")
  print("PASS: test_application_current_returns_app")
end

-- ---------------------------------------------------------------------------
-- Test 15: run dispatches using dispatchMessage(screen, msg)
-- ---------------------------------------------------------------------------
local function test_run_dispatches_with_screen()
  local original_get_message = system.getMessage
  local original_dispatch    = system.dispatchMessage
  local original_translate   = system.translateMessage
  local original_has_changed = filesystem.hasChangedFiles

  local dispatched_screen = nil
  local dispatched_msg    = nil
  local event = {
    is = function(self, name) return name == "Node.MouseMoved" end
  }
  local call_count = 0
  local event_list = {
    event,
    { is = function(self, name) return name == "Window.Closed" end }
  }

  system.getMessage     = function() call_count = call_count + 1; return event_list[call_count] end
  system.translateMessage = function(msg) end
  system.dispatchMessage  = function(screen, msg)
    dispatched_screen = screen
    dispatched_msg    = msg
    return false
  end
  filesystem.hasChangedFiles = function() return false end

  local FakeScreen = {}
  FakeScreen.__index = FakeScreen
  function FakeScreen:post(message, size) end
  local screen = setmetatable({}, FakeScreen)

  local app = Application()
  app.screen = screen
  app:run()

  system.getMessage          = original_get_message
  system.dispatchMessage     = original_dispatch
  system.translateMessage    = original_translate
  filesystem.hasChangedFiles = original_has_changed

  test.expect_eq(dispatched_screen, app.screen, "run should pass current screen as dispatchMessage first arg")
  test.expect_eq(dispatched_msg, event, "run should pass message as dispatchMessage second arg")
  print("PASS: test_run_dispatches_with_screen")
end

-- Run all
test_route_result_stored_in_inner()
test_layout_content_called()
test_layout_receives_render_context()
test_class_helpers_accessible_in_layout()
test_unknown_route_returns_nil_body()
test_dispatch_result_shape()
test_resolve_body_default_passthrough()
test_resolve_body_render_true_loads_view()
test_render_true_dispatch_end_to_end()
test_render_true_without_prefix_passthrough()
test_resolve_body_ignores_non_render_true()
test_detached_helper_method_keeps_argument()
test_activate_controller_updates_screen()
test_application_current_returns_app()
test_run_dispatches_with_screen()

print("All application tests passed.")
