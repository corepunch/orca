local test = require "orca.test"
-- Headless tests for orca.core.widget
-- Covers: content_for, set_render_context, instance-level include_helpers,
-- class-level include_helpers, helper/core-method non-shadowing, and modal
-- coroutine handoff.

local Widget = require "orca.core.widget"
local filesystem = require "orca.filesystem"
local ui = require "orca.UIKit"
local system = require "orca.system"

local function pump_messages(root)
  while true do
    local msg = system.peekMessage()
    if not msg then
      return
    end
    system.dispatchMessage(root, msg)
  end
end

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

-- ---------------------------------------------------------------------------
-- Test 8: content_for rejects a non-string name
-- ---------------------------------------------------------------------------
local function test_content_for_rejects_non_string_name()
  local w = Widget()
  w:set_render_context({ slots = {} })
  local ok, err = pcall(function() w:content_for(nil, "value") end)
  test.expect(not ok, "content_for with nil name should error")
  test.expect(err:find("name must be a string") ~= nil, "error should mention 'name must be a string'")

  local ok2, err2 = pcall(function() w:content_for(42, "value") end)
  test.expect(not ok2, "content_for with number name should error")
  test.expect(err2:find("name must be a string") ~= nil, "error should mention 'name must be a string'")
  print("PASS: test_content_for_rejects_non_string_name")
end

test_content_for_rejects_non_string_name()

-- ---------------------------------------------------------------------------
-- Test 9: showModal attaches the modal popup and Popup.ClosePopup stores the result
-- ---------------------------------------------------------------------------
local function test_show_modal_attaches_and_closes_popup()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local modal = filesystem.loadObjectFromXmlString([[
<Popup Name="DialogPopup">
  <StackView Name="DialogPopupOverlay" Direction="Vertical" AlignItems="Center" JustifyContent="Center" Padding="24" BackgroundColor="#00000088">
    <StackView Name="DialogPopupCard" Direction="Vertical" Spacing="12" Width="240" BackgroundColor="#223344" Padding="16">
      <TextBlock Name="DialogPopupClose" Text="Close" Height="32" BackgroundColor="#4466AA" TextHorizontalAlignment="Center" TextVerticalAlignment="Center"
        LeftButtonUp="{Popup.ClosePopup ReturnValue=2.25}"/>
    </StackView>
  </StackView>
</Popup>]])
  test.expect(modal ~= nil, "popup XML should load")
  local close = modal:findChild("DialogPopupClose", true)
  test.expect(close ~= nil, "close label should exist")

  local w = Widget()
  rawset(w, "screen", screen)
  w:showModal(modal)
  test.expect_eq(screen:getNext(), modal, "screen:showModal should attach the popup as the modal child")

  close:send("Node.LeftButtonUp")
  pump_messages(screen)
  test.expect_eq(modal.DialogResult, 2.25, "Popup.ClosePopup should store the modal DialogResult")
  test.expect_eq(screen:getNext(), nil, "Popup should detach from the screen after closing")

  print("PASS: test_show_modal_attaches_and_closes_popup")
end

test_show_modal_attaches_and_closes_popup()

-- ---------------------------------------------------------------------------
-- Test 10: showModal wraps plain widget content in a UIKit Popup
-- ---------------------------------------------------------------------------
local function test_show_modal_wraps_widget_content()
  local last_prompt = nil
  local Prompt = Widget:extend {
    content = function(self)
      last_prompt = self
      local root = ui.StackView { Name = "WidgetPromptContent" }
      root:addChild(ui.TextBlock {
        Name = "WidgetPromptYes",
        Text = "Yes",
        LeftButtonUp = function()
          self:on_result({ ReturnValue = 1 })
        end
      })
      return root
    end
  }

  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local host = Widget()
  local result = nil
  rawset(host, "screen", screen)

  local co = coroutine.create(function()
    result = host:showModal(Prompt)
  end)
  local ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should start")
  test.expect_eq(coroutine.status(co), "suspended", "showModal should wait for widget modal result")

  local modal = screen:getNext()
  test.expect(modal ~= nil, "widget showModal should attach a UIKit Popup")
  test.expect_eq(modal:getClassName(), "Popup", "widget showModal wrapper should be a Popup")
  test.expect(modal:findChild("WidgetPromptContent", true) ~= nil, "widget content should be inside the Popup")

  test.expect(modal:findChild("WidgetPromptYes", true) ~= nil, "widget modal button should exist")
  test.expect(last_prompt ~= nil, "widget class should be instantiated by showModal")
  ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should tolerate scheduler resumes before close")
  test.expect_eq(coroutine.status(co), "suspended", "showModal should stay suspended until widget modal result")
  test.expect_eq(result, nil, "widget showModal should not return nil before the modal closes")

  last_prompt:on_result({ ReturnValue = 1 })
  pump_messages(screen)
  ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should resume after modal result")

  test.expect_eq(result, 1, "widget showModal should return the widget result")
  test.expect_eq(modal.DialogResult, 1, "widget showModal should close the Popup with the result")
  test.expect_eq(screen:getNext(), nil, "widget modal should detach after closing")

  print("PASS: test_show_modal_wraps_widget_content")
end

test_show_modal_wraps_widget_content()

-- ---------------------------------------------------------------------------
-- Test 11: showModal waits for raw Popup close results in a coroutine
-- ---------------------------------------------------------------------------
local function test_show_modal_waits_for_popup_close_result()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local modal = filesystem.loadObjectFromXmlString([[
<Popup Name="CoroutineDialogPopup">
  <StackView Name="CoroutineDialogOverlay" Direction="Vertical" AlignItems="Center" JustifyContent="Center" Padding="24" BackgroundColor="#00000088">
    <TextBlock Name="CoroutineDialogClose" Text="Close" Height="32" BackgroundColor="#4466AA" TextHorizontalAlignment="Center" TextVerticalAlignment="Center"
      LeftButtonUp="{Popup.ClosePopup ReturnValue=2}"/>
  </StackView>
</Popup>]])
  test.expect(modal ~= nil, "popup XML should load")
  local close = modal:findChild("CoroutineDialogClose", true)
  test.expect(close ~= nil, "close label should exist")

  local host = Widget()
  local ok_result = nil
  local accepted = nil
  rawset(host, "screen", screen)

  local co = coroutine.create(function()
    ok_result, accepted = host:showModal(modal)
  end)
  local ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should start")
  test.expect_eq(coroutine.status(co), "suspended", "showModal should wait for raw popup close")
  test.expect_eq(screen:getNext(), modal, "showModal should attach the raw popup")

  ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should tolerate scheduler resumes before raw popup close")
  test.expect_eq(coroutine.status(co), "suspended", "showModal should keep waiting while raw popup remains attached")
  test.expect_eq(ok_result, nil, "raw popup showModal should not return before close")

  close:send("Node.LeftButtonUp")
  pump_messages(screen)
  ok, err = coroutine.resume(co)
  test.expect(ok, err or "showModal coroutine should resume after raw popup closes")

  test.expect_eq(ok_result, true, "raw popup showModal should return a successful modal status")
  test.expect_eq(accepted, false, "raw popup showModal should preserve the C accepted-result convention")
  test.expect_eq(screen:getNext(), nil, "raw popup should detach after closing")

  print("PASS: test_show_modal_waits_for_popup_close_result")
end

test_show_modal_waits_for_popup_close_result()

print("All widget tests passed.")
