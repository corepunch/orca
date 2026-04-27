test = require "orca.test"
-- Headless tests for orca.core.application
-- Covers: route return value stored in ctx.content.inner,
-- class-based layout receives render context, class-level helpers
-- accessible in layout, and dispatch result structure.
--
-- Uses a mock layout Widget that avoids UIKit so tests run fully headless.
--
-- Run via: $(TARGET) -test=tests/application_spec.moon

Widget      = require "orca.core.widget"
Application = require "orca.core.application"
system      = require "orca.system"
filesystem  = require "orca.filesystem"

-- ---------------------------------------------------------------------------
-- A minimal mock layout: captures inner and render context for inspection
-- ---------------------------------------------------------------------------
class MockLayout extends Widget
  content: =>
    @_inner   = @content_for "inner"
    @_title   = if @app_title then @app_title! else nil
    "layout_result"

-- ---------------------------------------------------------------------------
-- Test 1: route return value is stored in ctx.content.inner
-- ---------------------------------------------------------------------------
test_route_result_stored_in_inner = ->
  class App extends Application
    layout: MockLayout
    "/": => "my_body"

  app = App!
  result = app\dispatch "/"

  test.expect result ~= nil, "dispatch should return a result table"
  test.expect result.context ~= nil, "result should have a context"
  test.expect_eq result.context.content.inner, "my_body", "route return should be ctx.content.inner"
  print "PASS: test_route_result_stored_in_inner"

-- ---------------------------------------------------------------------------
-- Test 2: class-based layout's content() is called and its return is view
-- ---------------------------------------------------------------------------
test_layout_content_called = ->
  class App extends Application
    layout: MockLayout
    "/": => "body_value"

  app = App!
  result = app\dispatch "/"

  test.expect_eq result.view, "layout_result", "layout:content() return should be result.view"
  print "PASS: test_layout_content_called"

-- ---------------------------------------------------------------------------
-- Test 3: layout receives render context via set_render_context
-- ---------------------------------------------------------------------------
test_layout_receives_render_context = ->
  captured_layout = nil

  class CapturingLayout extends Widget
    content: =>
      captured_layout = self
      "ok"

  class App extends Application
    layout: CapturingLayout
    "/": => "x"

  app = App!
  result = app\dispatch "/"

  test.expect captured_layout ~= nil, "layout content() should have been called"
  test.expect (captured_layout\get_render_context!) ~= nil, "layout should have render context"
  test.expect_eq (captured_layout\content_for "inner"), "x", "inner should be accessible in layout"
  print "PASS: test_layout_receives_render_context"

-- ---------------------------------------------------------------------------
-- Test 4: class-level helpers are accessible inside layout via @app_title
-- ---------------------------------------------------------------------------
test_class_helpers_accessible_in_layout = ->
  captured_title = nil

  class TitleLayout extends Widget
    content: =>
      captured_title = if @app_title then @app_title! else nil
      "ok"

  class App extends Application
    layout: TitleLayout

    @include_helpers {
      app_title: => "MyApp"
    }

    "/": => "body"

  app = App!
  app\dispatch "/"

  test.expect_eq captured_title, "MyApp", "app_title helper should be visible in layout"
  print "PASS: test_class_helpers_accessible_in_layout"

-- ---------------------------------------------------------------------------
-- Test 5: unknown route returns nil body but still produces a result table
-- ---------------------------------------------------------------------------
test_unknown_route_returns_nil_body = ->
  class App extends Application
    layout: MockLayout
    "/": => "home"

  app = App!
  result = app\dispatch "/missing"

  test.expect result ~= nil, "dispatch should always return a result table"
  test.expect_eq result.context.content.inner, nil, "unknown route should yield nil inner"
  print "PASS: test_unknown_route_returns_nil_body"

-- ---------------------------------------------------------------------------
-- Test 6: dispatch result always has view and context keys
-- ---------------------------------------------------------------------------
test_dispatch_result_shape = ->
  class App extends Application
    layout: MockLayout
    "/": => "content"

  app = App!
  result = app\dispatch "/"

  test.expect (result.view ~= nil),    "result.view should be present"
  test.expect (result.context ~= nil), "result.context should be present"
  print "PASS: test_dispatch_result_shape"

-- ---------------------------------------------------------------------------
-- Test 7: resolve_body default passes body through unchanged
-- ---------------------------------------------------------------------------
test_resolve_body_default_passthrough = ->
  class App extends Application
    layout: MockLayout
    "/": => "raw_body"

  app = App!
  route_info = app.router\resolve "/"
  result = app\resolve_body "raw_body", route_info

  test.expect_eq result, "raw_body", "default resolve_body should return body unchanged"
  print "PASS: test_resolve_body_default_passthrough"

-- ---------------------------------------------------------------------------
-- Test 8: resolve_body with render:true loads view via views_prefix
-- ---------------------------------------------------------------------------
test_resolve_body_render_true_loads_view = ->
  -- Stub package.loaded so require "views/Home" returns a known Widget class
  stub_content = "stub_view_content"
  class StubView
    content: => stub_content

  package.loaded["myviews/Home"] = StubView

  class App extends Application
    layout: MockLayout
    views_prefix: "myviews"
    [Home: "/"]: => render: true

  app = App!
  route_info = app.router\resolve "/"
  result = app\resolve_body { render: true }, route_info

  test.expect_eq result, stub_content, "resolve_body should call content() on required view class"
  package.loaded["myviews/Home"] = nil
  print "PASS: test_resolve_body_render_true_loads_view"

-- ---------------------------------------------------------------------------
-- Test 9: render:true propagates through dispatch into ctx.content.inner
-- ---------------------------------------------------------------------------
test_render_true_dispatch_end_to_end = ->
  stub_content = "e2e_view_content"
  class StubView2
    content: => stub_content

  package.loaded["app2views/Home"] = StubView2

  class App extends Application
    layout: MockLayout
    views_prefix: "app2views"
    [Home: "/"]: => render: true

  app = App!
  result = app\dispatch "/"

  test.expect_eq result.context.content.inner, stub_content,
    "render:true should resolve view and store content in ctx.content.inner"
  package.loaded["app2views/Home"] = nil
  print "PASS: test_render_true_dispatch_end_to_end"

-- ---------------------------------------------------------------------------
-- Test 10: render:true with no views_prefix falls back to body passthrough
-- ---------------------------------------------------------------------------
test_render_true_without_prefix_passthrough = ->
  class App extends Application
    layout: MockLayout
    "/": => render: true

  app = App!
  route_info = app.router\resolve "/"
  result = app\resolve_body { render: true }, route_info

  -- No views_prefix → resolve_body returns the table unmodified
  test.expect type(result) == "table", "without views_prefix body should pass through as table"
  test.expect result.render == true, "render flag should be preserved when no views_prefix"
  print "PASS: test_render_true_without_prefix_passthrough"

-- ---------------------------------------------------------------------------
-- Test 11: non-render-true body is never touched by resolve_body
-- ---------------------------------------------------------------------------
test_resolve_body_ignores_non_render_true = ->
  class App extends Application
    layout: MockLayout
    views_prefix: "irrelevant"
    "/": => "literal_string"

  app = App!
  route_info = app.router\resolve "/"
  result = app\resolve_body "literal_string", route_info

  test.expect_eq result, "literal_string", "non-render:true body should pass through unchanged"
  print "PASS: test_resolve_body_ignores_non_render_true"

-- ---------------------------------------------------------------------------
-- Test 12: detached helper method reference keeps first payload argument
-- ---------------------------------------------------------------------------
test_detached_helper_method_keeps_argument = ->
  captured_route = nil

  class NavLayout extends Widget
    content: =>
      navigate = @navigate
      navigate "/alerts"
      "ok"

  class App extends Application
    layout: NavLayout
    "/": => "home"
    navigate: (route, ...) =>
      captured_route = route

  app = App!
  app\dispatch "/"

  test.expect_eq captured_route, "/alerts", "navigate = @navigate should preserve route argument"
  print "PASS: test_detached_helper_method_keeps_argument"

-- ---------------------------------------------------------------------------
-- Test 13: activate_controller updates active screen and editor host
-- ---------------------------------------------------------------------------
test_activate_controller_updates_screen = ->
  called_editor_screen = nil
  called_post = nil
  original_load_editor = Application.load_editor

  Application.load_editor = (screen) ->
    called_editor_screen = screen

  class FakeScreen
    post: (message, size) =>
      called_post = { message, size }

  screen = FakeScreen!
  ctrl = { view: screen }
  app = Application!

  result = app\activate_controller ctrl

  Application.load_editor = original_load_editor

  test.expect_eq result, ctrl, "activate_controller should return the controller"
  test.expect_eq app.controller, ctrl, "activate_controller should store controller on app"
  test.expect_eq app.screen, screen, "activate_controller should store screen on app"
  test.expect_eq Application.app, app, "activate_controller should update Application.app"
  test.expect_eq called_editor_screen, screen, "activate_controller should update editor host screen"
  test.expect called_post ~= nil, "activate_controller should request a paint"
  test.expect_eq called_post[1], "Window.Paint", "activate_controller should post Window.Paint"
  print "PASS: test_activate_controller_updates_screen"

-- ---------------------------------------------------------------------------
-- Test 14: Application.current returns singleton app instance
-- ---------------------------------------------------------------------------
test_application_current_returns_app = ->
  previous = Application.app
  app = Application!
  Application.app = app

  current = Application.current!

  Application.app = previous

  test.expect_eq current, app, "Application.current should return Application.app"
  print "PASS: test_application_current_returns_app"

-- ---------------------------------------------------------------------------
-- Test 15: run dispatches using dispatchMessage(screen, msg)
-- ---------------------------------------------------------------------------
test_run_dispatches_with_screen = ->
  original_get_message = system.getMessage
  original_dispatch = system.dispatchMessage
  original_translate = system.translateMessage
  original_has_changed = filesystem.hasChangedFiles

  dispatched_screen = nil
  dispatched_msg = nil
  event = {
    is: (name) -> name == "Node.MouseMoved"
  }
  call_count = 0
  -- event_list[call_count] returns nil after the last entry, signalling iterator exhaustion
  event_list = { event, { is: (name) -> name == "Window.Closed" } }

  system.getMessage = ->
    call_count += 1
    return event_list[call_count]

  system.translateMessage = (msg) -> nil
  system.dispatchMessage = (screen, msg) ->
    dispatched_screen = screen
    dispatched_msg = msg
    false

  filesystem.hasChangedFiles = -> false

  class FakeScreen
    post: (message, size) =>
      nil

  app = Application!
  app.screen = FakeScreen!
  app\run!

  system.getMessage = original_get_message
  system.dispatchMessage = original_dispatch
  system.translateMessage = original_translate
  filesystem.hasChangedFiles = original_has_changed

  test.expect_eq dispatched_screen, app.screen, "run should pass current screen as dispatchMessage first arg"
  test.expect_eq dispatched_msg, event, "run should pass message as dispatchMessage second arg"
  print "PASS: test_run_dispatches_with_screen"

-- Run all
test_route_result_stored_in_inner!
test_layout_content_called!
test_layout_receives_render_context!
test_class_helpers_accessible_in_layout!
test_unknown_route_returns_nil_body!
test_dispatch_result_shape!
test_resolve_body_default_passthrough!
test_resolve_body_render_true_loads_view!
test_render_true_dispatch_end_to_end!
test_render_true_without_prefix_passthrough!
test_resolve_body_ignores_non_render_true!
test_detached_helper_method_keeps_argument!
test_activate_controller_updates_screen!
test_application_current_returns_app!
test_run_dispatches_with_screen!
