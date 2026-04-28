local test = require "orca.test"
-- Headless tests for orca.core.widget
-- Covers: content_for, set_render_context, instance-level include_helpers,
-- class-level include_helpers, and helper/core-method non-shadowing.

local Widget = require "orca.core.widget"

-- ---------------------------------------------------------------------------
-- Test 1: content_for errors without a render context
-- ---------------------------------------------------------------------------
local function test_content_for_errors_without_context()
  local w = Widget()
  local ok, err = pcall(function() w:content_for("inner") end)
  test.expect(not ok, "content_for without context should error")
  test.expect(err:find("without render context") ~= nil, "error should mention 'without render context'")
  print("PASS: test_content_for_errors_without_context")
end

-- ---------------------------------------------------------------------------
-- Test 2: set_render_context + content_for stores and retrieves a value
-- ---------------------------------------------------------------------------
local function test_content_for_stores_and_retrieves()
  local w = Widget()
  local ctx = { content = {} }
  w:set_render_context(ctx)
  w:content_for("inner", "hello")
  test.expect_eq(w:content_for("inner"), "hello", "content_for store/retrieve")
  print("PASS: test_content_for_stores_and_retrieves")
end

-- ---------------------------------------------------------------------------
-- Test 3: content_for returns nil for missing key (not an error)
-- ---------------------------------------------------------------------------
local function test_content_for_nil_for_missing_key()
  local w = Widget()
  local ctx = { content = {} }
  w:set_render_context(ctx)
  test.expect_eq(w:content_for("missing"), nil, "content_for missing key should be nil")
  print("PASS: test_content_for_nil_for_missing_key")
end

-- ---------------------------------------------------------------------------
-- Test 4: instance-level include_helper exposes a helper method
-- ---------------------------------------------------------------------------
local function test_instance_helper_found()
  local w = Widget()
  w:include_helper({ my_title = function(self) return "Instance Title" end })
  test.expect_eq(type(w.my_title), "function", "helper method should be accessible")
  test.expect_eq(w:my_title(), "Instance Title", "helper should return correct value")
  print("PASS: test_instance_helper_found")
end

-- ---------------------------------------------------------------------------
-- Test 5: class-level @include_helpers is found on instances
-- ---------------------------------------------------------------------------
local function test_class_helper_found_on_instance()
  local MyWidget = Widget:extend {}
  MyWidget:include_helpers({
    app_title = function(self) return "Class Title" end
  })
  local w = MyWidget()
  test.expect_eq(type(w.app_title), "function", "class-level helper should be accessible on instance")
  test.expect_eq(w:app_title(), "Class Title", "class-level helper should return correct value")
  print("PASS: test_class_helper_found_on_instance")
end

-- ---------------------------------------------------------------------------
-- Test 6: class-level helper does not shadow core Widget methods
-- ---------------------------------------------------------------------------
local function test_class_helper_does_not_shadow_core()
  local MyWidget = Widget:extend {}
  MyWidget:include_helpers({
    set_render_context = function(self) return "hijacked" end
  })
  local w = MyWidget()
  local ctx = { content = {} }
  w:set_render_context(ctx)  -- must still be the real core method
  test.expect_eq(rawget(w, "__render_ctx"), ctx, "core method must not be shadowed by helper")
  print("PASS: test_class_helper_does_not_shadow_core")
end

-- ---------------------------------------------------------------------------
-- Test 7: has_content_for returns false before set, true after
-- ---------------------------------------------------------------------------
local function test_has_content_for()
  local w = Widget()
  local ctx = { content = {} }
  w:set_render_context(ctx)
  test.expect(not w:has_content_for("section"), "has_content_for should be false before set")
  w:content_for("section", "value")
  test.expect(w:has_content_for("section"), "has_content_for should be true after set")
  print("PASS: test_has_content_for")
end

-- Run all
test_content_for_errors_without_context()
test_content_for_stores_and_retrieves()
test_content_for_nil_for_missing_key()
test_instance_helper_found()
test_class_helper_found_on_instance()
test_class_helper_does_not_shadow_core()
test_has_content_for()

print("All widget tests passed.")
