local test = require "orca.test"
-- Headless tests for the ORCA style system — exercised from Lua using
-- the CSS string API, node.class, and StyleController.ThemeChanged.
--
-- Run with: $(TARGET) -test=tests/test_styles_lua.lua
--
-- Complements the C unit tests in test_styles.c by covering the Lua
-- property-setting path end-to-end: CSS string → StyleSheet.Parse →
-- screen.StyleSheet → class assignment → ThemeChanged → values applied.

local filesystem = require "orca.filesystem"
local ui         = require "orca.UIKit"

-- filesystem.loadObjectFromCssString exposes the pure-C CSS parser.

-- ---------------------------------------------------------------------------
-- Node2D inherits StyleController, so every Node2D has one attached.
-- applyStyles(node) triggers style resolution.
local function applyStyles(node)
  node:send("StyleController.ThemeChanged")
end

-- ---------------------------------------------------------------------------
-- Test 1: CSS opacity rule + class assignment → Opacity applied
-- ---------------------------------------------------------------------------
local function test_style_applies_opacity()
  local screen = ui.Screen { 
    Width = 200,
    Height = 200,
    ResizeMode = "NoResize",
    StyleSheet = filesystem.loadObjectFromCssString ".highlight { opacity: 0.4; }",
  }
  local node = screen + ui.Node2D {
    class = "highlight",
    Opacity = 1.0
  }

  applyStyles(node)

  test.expect_near(node.Opacity, 0.4, 0.001, "StyleController applied Opacity=0.4")

  node:removeFromParent()
  print("PASS: test_style_applies_opacity")
end

-- ---------------------------------------------------------------------------
-- Test 2: Style rule does NOT apply without the matching class
-- ---------------------------------------------------------------------------
local function test_style_not_applied_without_class()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".hidden { opacity: 0.0; }"
  local node = screen + ui.Node2D {}

  node.Opacity = 0.8
  -- Intentionally NOT setting node.class = "hidden"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.8, 0.001, "Opacity unchanged when class not set")

  node:removeFromParent()
  print("PASS: test_style_not_applied_without_class")
end

-- ---------------------------------------------------------------------------
-- Test 3: Multiple properties in a single rule are all applied
-- ---------------------------------------------------------------------------
local function test_style_multiple_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".box { width: 200; height: 80; }"
  local node = screen + ui.Node2D { Width = 100, Height = 50 }

  node.class = "box"
  applyStyles(node)

  test.expect_near(node.Width,  200, 0.5, "StyleController applied Width=200")
  test.expect_near(node.Height,  80, 0.5, "StyleController applied Height=80")

  node:removeFromParent()
  print("PASS: test_style_multiple_properties")
end

-- ---------------------------------------------------------------------------
-- Test 4: Multiple classes — only matching class rules apply
-- ---------------------------------------------------------------------------
local function test_style_multiple_classes()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".alpha50 { opacity: 0.5; } .alpha25 { opacity: 0.25; }"
  local node = screen + ui.Node2D {}

  node.Opacity = 1.0
  node.class = "alpha50"
  applyStyles(node)
  test.expect_near(node.Opacity, 0.5, 0.001, "alpha50 applied: Opacity=0.5")

  node.class = "alpha25"
  applyStyles(node)
  test.expect_near(node.Opacity, 0.25, 0.001, "alpha25 applied: Opacity=0.25")

  node:removeFromParent()
  print("PASS: test_style_multiple_classes")
end

-- ---------------------------------------------------------------------------
-- Test 5: Hover pseudo-state — rule with :hover does NOT apply by default
-- ---------------------------------------------------------------------------
local function test_style_hover_not_applied_by_default()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".btn:hover { opacity: 0.6; }"
  local node = screen + ui.Node2D {}

  node.Opacity = 1.0
  node.class = "btn"
  applyStyles(node)

  -- The :hover rule only fires when the hover state flag is set on the object.
  -- Without programmatic hover, Opacity stays at 1.0.
  test.expect_near(node.Opacity, 1.0, 0.001, "hover rule ignored when not hovered")

  node:removeFromParent()
  print("PASS: test_style_hover_not_applied_by_default")
end

-- ---------------------------------------------------------------------------
-- Test 6: Non-hover rule for the same class still applies
-- ---------------------------------------------------------------------------
local function test_style_non_hover_rule_applies()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".card { opacity: 0.7; } .card:hover { opacity: 1.0; }"
  local node = screen + ui.Node2D {}

  node.Opacity = 1.0
  node.class = "card"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.7, 0.001, "non-hover card rule applied: Opacity=0.7")

  node:removeFromParent()
  print("PASS: test_style_non_hover_rule_applies")
end

-- ---------------------------------------------------------------------------
-- Test 7: ThemeChanged with recursive=true propagates to children
-- ---------------------------------------------------------------------------
local function test_style_recursive_children()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".dim { opacity: 0.3; }"
  local parent = screen + ui.Node2D {}
  local child  = parent + ui.Node2D {}

  child.Opacity = 1.0
  child.class = "dim"

  -- Fire ThemeChanged recursively on the parent
  parent:send("StyleController.ThemeChanged", { recursive = true })

  test.expect_near(child.Opacity, 0.3, 0.001, "recursive ThemeChanged applied to child: Opacity=0.3")

  parent:removeFromParent()
  print("PASS: test_style_recursive_children")
end

-- ---------------------------------------------------------------------------
-- Test 8: Rule applied via dot-prefixed selector ".classname" is equivalent
-- ---------------------------------------------------------------------------
local function test_style_dot_prefix_selector()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".primary { opacity: 0.55; }"
  local node = screen + ui.Node2D {}

  node.Opacity = 1.0
  node.class = "primary"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.55, 0.001, ".primary selector applied: Opacity=0.55")

  node:removeFromParent()
  print("PASS: test_style_dot_prefix_selector")
end

-- ---------------------------------------------------------------------------
-- Test 9: Numeric CSS value is applied correctly
-- ---------------------------------------------------------------------------
local function test_style_numeric_value()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".sized { width: 150; }"
  local node = screen + ui.Node2D {}

  node.class = "sized"
  applyStyles(node)

  test.expect_near(node.Width, 150, 0.5, "CSS width: 150 applied from style rule")

  node:removeFromParent()
  print("PASS: test_style_numeric_value")
end

-- ---------------------------------------------------------------------------
-- Test 10: Applying a style to a fresh object (no prior property set)
-- ---------------------------------------------------------------------------
local function test_style_applies_to_new_node()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".ghost { opacity: 0.1; }"
  local node = screen + ui.Node2D {}

  -- Opacity defaults to 1.0; style overrides it
  node.class = "ghost"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.1, 0.001, "ghost class: Opacity=0.1 on fresh node")

  node:removeFromParent()
  print("PASS: test_style_applies_to_new_node")
end


-- ---------------------------------------------------------------------------
-- Test 11: @apply directive merges another rule's declarations
-- ---------------------------------------------------------------------------
local function test_style_apply_directive()
  local css = [[
    .base  { opacity: 0.4; }
    .child { @apply: .base; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "child"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.4, 0.001, "@apply: child inherited opacity from base")

  node:removeFromParent()
  print("PASS: test_style_apply_directive")
end

-- ---------------------------------------------------------------------------
-- Test 12: Transitive @apply chain  A → B → C propagates C's properties to A
-- ---------------------------------------------------------------------------
local function test_style_apply_transitive()
  -- .c defines the property; .b applies .c; .a applies .b
  local css = [[
    .c { opacity: 0.2; }
    .b { @apply: .c; }
    .a { @apply: .b; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "a"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.2, 0.001, "transitive @apply: A inherited opacity from C via B")

  node:removeFromParent()
  print("PASS: test_style_apply_transitive")
end


test_style_applies_opacity()
test_style_not_applied_without_class()
test_style_multiple_properties()
test_style_multiple_classes()
test_style_hover_not_applied_by_default()
test_style_non_hover_rule_applies()
test_style_recursive_children()
test_style_dot_prefix_selector()
test_style_numeric_value()
test_style_applies_to_new_node()
test_style_apply_directive()
test_style_apply_transitive()

print("All style tests passed.")
