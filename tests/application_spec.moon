test = require "orca.test"
-- Headless tests for orca.core2.application
-- Covers: route return value stored in ctx.content.inner,
-- class-based layout receives render context, class-level helpers
-- accessible in layout, and dispatch result structure.
--
-- Uses a mock layout Widget that avoids UIKit so tests run fully headless.
--
-- Run via: $(TARGET) -test=tests/run_core2_specs.lua

Widget      = require "orca.core2.widget"
Application = require "orca.core2.application"

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

-- Run all
test_route_result_stored_in_inner!
test_layout_content_called!
test_layout_receives_render_context!
test_class_helpers_accessible_in_layout!
test_unknown_route_returns_nil_body!
test_dispatch_result_shape!
