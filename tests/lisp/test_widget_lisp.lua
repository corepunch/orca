local test = require "orca.test"
-- Headless tests for orca.core.widget modals with popups declared in Lisp;
-- tests/widget_spec.lua covers the format-independent widget behavior.

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
-- Test 9: showModal attaches the modal popup and Popup.ClosePopup stores the result
-- ---------------------------------------------------------------------------
local function test_show_modal_attaches_and_closes_popup()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local modal = filesystem.loadObjectFromLispString([[
(Popup Name="DialogPopup"
  (StackView Name="DialogPopupOverlay" Direction="Vertical" AlignItems="Center" JustifyContent="Center" Padding=24 BackgroundColor="#00000088"
    (StackView Name="DialogPopupCard" Direction="Vertical" Spacing=12 Width=240 BackgroundColor="#223344" Padding=16
      (TextBlock Name="DialogPopupClose" Text="Close" Height=32 BackgroundColor="#4466AA" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" LeftButtonUp=(Popup.ClosePopup ReturnValue=2.25)))))]])
  test.expect(modal ~= nil, "popup Lisp should load")
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
-- Test 11: showModal waits for raw Popup close results in a coroutine
-- ---------------------------------------------------------------------------
local function test_show_modal_waits_for_popup_close_result()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local modal = filesystem.loadObjectFromLispString([[
(Popup Name="CoroutineDialogPopup"
  (StackView Name="CoroutineDialogOverlay" Direction="Vertical" AlignItems="Center" JustifyContent="Center" Padding=24 BackgroundColor="#00000088"
    (TextBlock Name="CoroutineDialogClose" Text="Close" Height=32 BackgroundColor="#4466AA" TextHorizontalAlignment="Center" TextVerticalAlignment="Center" LeftButtonUp=(Popup.ClosePopup ReturnValue=2))))]])
  test.expect(modal ~= nil, "popup Lisp should load")
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

print("All Lisp widget tests passed.")
