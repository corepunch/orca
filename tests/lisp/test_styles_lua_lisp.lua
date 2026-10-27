local test = require "orca.test"
-- Style classes assigned from Lisp; tests/test_styles_lua.lua covers the
-- format-independent style system.
--
-- Run with: $(TARGET) -test=tests/lisp/test_styles_lua_lisp.lua

local filesystem = require "orca.filesystem"
require "orca.renderer"
local ui         = require "orca.UIKit"


-- ---------------------------------------------------------------------------
-- Node2D inherits StyleController, so every Node2D has one attached.
-- applyStyles(node) triggers style resolution.
local function applyStyles(node)
  node:send("StyleController.ThemeChanged")
end

local function test_styleclass_lisp_alias()
  local root = filesystem.loadObjectFromLispString [[
(Screen Name="styleclass-root" Width=200 Height=200 ResizeMode="NoResize"
  (Node2D Name="pascal-style" StyleClass="pascal-alias" Opacity=1)
  (Node2D Name="html-style" class="html-alias" Opacity=1))]]

  test.expect(root ~= nil, "StyleClass Lisp should load")
  root.StyleSheet = ui.loadObjectFromCssString [[
    .pascal-alias { opacity: 0.42; }
    .html-alias { opacity: 0.24; }
  ]]

  local pascal = root:findChild("pascal-style", true)
  local html = root:findChild("html-style", true)
  test.expect(pascal ~= nil, "StyleClass Lisp child should exist")
  test.expect(html ~= nil, "class Lisp child should exist")

  applyStyles(pascal)
  applyStyles(html)

  test.expect_near(pascal.Opacity, 0.42, 0.001, "StyleClass Lisp alias applies style classes")
  test.expect_near(html.Opacity, 0.24, 0.001, "class Lisp alias still applies style classes")

  root:clear()
  print("PASS: test_styleclass_lisp_alias")
end

test_styleclass_lisp_alias()

print("All Lisp style tests passed.")
