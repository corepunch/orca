-- Headless tests for orca.core2.widget
-- Covers: content_for, set_render_context, instance-level include_helpers,
-- class-level include_helpers, and helper/core-method non-shadowing.
--
-- Run via: $(TARGET) -test=tests/run_core2_specs.lua

Widget = require "orca.core2.widget"

-- ---------------------------------------------------------------------------
-- Test 1: content_for errors without a render context
-- ---------------------------------------------------------------------------
test_content_for_errors_without_context = ->
  w = Widget!
  ok, err = pcall -> w\content_for "inner"
  expect (not ok), "content_for without context should error"
  expect (err\find "without render context") ~= nil, "error should mention 'without render context'"
  print "PASS: test_content_for_errors_without_context"

-- ---------------------------------------------------------------------------
-- Test 2: set_render_context + content_for stores and retrieves a value
-- ---------------------------------------------------------------------------
test_content_for_stores_and_retrieves = ->
  w = Widget!
  ctx = { content: {} }
  w\set_render_context ctx
  w\content_for "inner", "hello"
  expect_eq (w\content_for "inner"), "hello", "content_for store/retrieve"
  print "PASS: test_content_for_stores_and_retrieves"

-- ---------------------------------------------------------------------------
-- Test 3: content_for returns nil for missing key (not an error)
-- ---------------------------------------------------------------------------
test_content_for_nil_for_missing_key = ->
  w = Widget!
  ctx = { content: {} }
  w\set_render_context ctx
  expect_eq (w\content_for "missing"), nil, "content_for missing key should be nil"
  print "PASS: test_content_for_nil_for_missing_key"

-- ---------------------------------------------------------------------------
-- Test 4: instance-level include_helper exposes a helper method
-- ---------------------------------------------------------------------------
test_instance_helper_found = ->
  w = Widget!
  w\include_helper { my_title: (self) -> "Instance Title" }
  expect_eq (type w.my_title), "function", "helper method should be accessible"
  expect_eq (w\my_title!), "Instance Title", "helper should return correct value"
  print "PASS: test_instance_helper_found"

-- ---------------------------------------------------------------------------
-- Test 5: class-level @include_helpers is found on instances (the bug we fixed)
-- ---------------------------------------------------------------------------
test_class_helper_found_on_instance = ->
  class MyWidget extends Widget
    @include_helpers {
      app_title: => "Class Title"
    }
  w = MyWidget!
  expect_eq (type w.app_title), "function", "class-level helper should be accessible on instance"
  expect_eq (w\app_title!), "Class Title", "class-level helper should return correct value"
  print "PASS: test_class_helper_found_on_instance"

-- ---------------------------------------------------------------------------
-- Test 6: class-level helper does not shadow core Widget methods
-- ---------------------------------------------------------------------------
test_class_helper_does_not_shadow_core = ->
  class MyWidget extends Widget
    @include_helpers {
      set_render_context: => "hijacked"
    }
  w = MyWidget!
  ctx = { content: {} }
  w\set_render_context ctx  -- must still be the real core method
  expect_eq (rawget w, "__render_ctx"), ctx, "core method must not be shadowed by helper"
  print "PASS: test_class_helper_does_not_shadow_core"

-- ---------------------------------------------------------------------------
-- Test 7: has_content_for returns false before set, true after
-- ---------------------------------------------------------------------------
test_has_content_for = ->
  w = Widget!
  ctx = { content: {} }
  w\set_render_context ctx
  expect (not w\has_content_for "section"), "has_content_for should be false before set"
  w\content_for "section", "value"
  expect (w\has_content_for "section"), "has_content_for should be true after set"
  print "PASS: test_has_content_for"

-- Run all
test_content_for_errors_without_context!
test_content_for_stores_and_retrieves!
test_content_for_nil_for_missing_key!
test_instance_helper_found!
test_class_helper_found_on_instance!
test_class_helper_does_not_shadow_core!
test_has_content_for!
