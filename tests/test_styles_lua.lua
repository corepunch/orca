-- Headless tests for the ORCA style system — exercised from Lua using
-- the native StyleRule/StyleSheet API, node.class, and StyleController.ThemeChanged.
--
-- Run with: $(TARGET) -test=tests/test_styles_lua.lua
--
-- Complements the C unit tests in test_styles.c by covering the Lua
-- property-setting path end-to-end: StyleRule → class assignment →
-- ThemeChanged dispatch → property values applied.

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------------
local function fail(msg)
  io.stderr:write("FAIL: " .. msg .. "\n")
  os.exit(1)
end

local function expect(cond, label)
  if not cond then fail(label) end
end

local function expect_near(actual, expected, eps, label)
  if math.abs(actual - expected) > (eps or 0.01) then
    fail(string.format("%s: expected ~%s, got %s", label, tostring(expected), tostring(actual)))
  end
end

-- Node2D inherits StyleController, so every Node2D has one attached.
-- applyStyles(node) triggers style resolution.
local function applyStyles(node)
  node:send("StyleController.ThemeChanged")
end

-- Replacement for the removed addStyleRule Lua method.
-- Creates a native StyleRule, parses the selector into ClassName/PseudoClass,
-- sets property overrides, and adds the rule to the node's per-object StyleSheet.
local function addStyleRule(node, selector, props)
  local rule = core.StyleRule()
  -- Strip optional leading '.' and split off pseudo-state at ':'
  local base = selector:match('^%.?([^:]+)') or selector
  local pseudo = selector:match(':(.+)$')
  rule.ClassName = base
  if pseudo then rule.PseudoClass = pseudo end
  for k, v in pairs(props) do rule[k] = v end
  core.getObjectStyleSheet(node):addChild(rule)
end

-- ---------------------------------------------------------------------------
-- Test 1: addStyleRule + class assignment → Opacity applied
-- ---------------------------------------------------------------------------
local function test_style_applies_opacity()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  addStyleRule(node, "highlight", { Opacity = "0.4" })
  node.class = "highlight"
  applyStyles(node)

  expect_near(node.Opacity, 0.4, 0.001, "StyleController applied Opacity=0.4")

  node:removeFromParent()
  print("PASS: test_style_applies_opacity")
end

-- ---------------------------------------------------------------------------
-- Test 2: Style rule does NOT apply without the matching class
-- ---------------------------------------------------------------------------
local function test_style_not_applied_without_class()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 0.8
  addStyleRule(node, "hidden", { Opacity = "0.0" })
  -- Intentionally NOT setting node.class = "hidden"
  applyStyles(node)

  expect_near(node.Opacity, 0.8, 0.001, "Opacity unchanged when class not set")

  node:removeFromParent()
  print("PASS: test_style_not_applied_without_class")
end

-- ---------------------------------------------------------------------------
-- Test 3: Multiple properties in a single rule are all applied
-- ---------------------------------------------------------------------------
local function test_style_multiple_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Width = 100, Height = 50 }

  addStyleRule(node, "box", { Width = "200", Height = "80" })
  node.class = "box"
  applyStyles(node)

  expect_near(node.Width,  200, 0.5, "StyleController applied Width=200")
  expect_near(node.Height,  80, 0.5, "StyleController applied Height=80")

  node:removeFromParent()
  print("PASS: test_style_multiple_properties")
end

-- ---------------------------------------------------------------------------
-- Test 4: Multiple classes — only matching class rules apply
-- ---------------------------------------------------------------------------
local function test_style_multiple_classes()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  addStyleRule(node, "alpha50",   { Opacity = "0.5" })
  addStyleRule(node, "alpha25",   { Opacity = "0.25" })

  node.class = "alpha50"
  applyStyles(node)
  expect_near(node.Opacity, 0.5, 0.001, "alpha50 applied: Opacity=0.5")

  -- Reassign to another class
  -- Note: addStyleRule adds to the existing stylesheet; class= replaces.
  node.class = "alpha25"
  applyStyles(node)
  expect_near(node.Opacity, 0.25, 0.001, "alpha25 applied: Opacity=0.25")

  node:removeFromParent()
  print("PASS: test_style_multiple_classes")
end

-- ---------------------------------------------------------------------------
-- Test 5: Hover pseudo-state — rule with :hover does NOT apply by default
-- ---------------------------------------------------------------------------
local function test_style_hover_not_applied_by_default()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  addStyleRule(node, "btn:hover", { Opacity = "0.6" })
  node.class = "btn"
  applyStyles(node)

  -- The :hover rule only fires when the hover state flag is set on the object.
  -- Without programmatic hover, Opacity stays at 1.0.
  expect_near(node.Opacity, 1.0, 0.001, "hover rule ignored when not hovered")

  node:removeFromParent()
  print("PASS: test_style_hover_not_applied_by_default")
end

-- ---------------------------------------------------------------------------
-- Test 6: Non-hover rule for the same class still applies
-- ---------------------------------------------------------------------------
local function test_style_non_hover_rule_applies()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  -- Add both a normal and a hover rule for the same selector
  addStyleRule(node, "card",       { Opacity = "0.7" })
  addStyleRule(node, "card:hover", { Opacity = "1.0" })
  node.class = "card"
  applyStyles(node)

  expect_near(node.Opacity, 0.7, 0.001, "non-hover card rule applied: Opacity=0.7")

  node:removeFromParent()
  print("PASS: test_style_non_hover_rule_applies")
end

-- ---------------------------------------------------------------------------
-- Test 7: ThemeChanged with recursive=true propagates to children
-- ---------------------------------------------------------------------------
local function test_style_recursive_children()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  local parent = screen + ui.Node2D {}
  local child  = parent + ui.Node2D {}

  child.Opacity = 1.0
  addStyleRule(child, "dim", { Opacity = "0.3" })
  child.class = "dim"

  -- Fire ThemeChanged recursively on the parent
  parent:send("StyleController.ThemeChanged", { recursive = true })

  expect_near(child.Opacity, 0.3, 0.001, "recursive ThemeChanged applied to child: Opacity=0.3")

  parent:removeFromParent()
  print("PASS: test_style_recursive_children")
end

-- ---------------------------------------------------------------------------
-- Test 8: Rule applied via dot-prefixed selector ".classname" is equivalent
-- ---------------------------------------------------------------------------
local function test_style_dot_prefix_selector()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  -- Both "primary" and ".primary" should hash to the same class ID.
  addStyleRule(node, ".primary", { Opacity = "0.55" })
  node.class = "primary"
  applyStyles(node)

  expect_near(node.Opacity, 0.55, 0.001, ".primary selector applied: Opacity=0.55")

  node:removeFromParent()
  print("PASS: test_style_dot_prefix_selector")
end

-- ---------------------------------------------------------------------------
-- Test 9: Numeric value in style rule is applied correctly
-- ---------------------------------------------------------------------------
local function test_style_numeric_value()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  -- Pass numeric value directly (not as a string)
  addStyleRule(node, "sized", { Width = 150 })
  node.class = "sized"
  applyStyles(node)

  expect_near(node.Width, 150, 0.5, "numeric Width=150 applied from style rule")

  node:removeFromParent()
  print("PASS: test_style_numeric_value")
end

-- ---------------------------------------------------------------------------
-- Test 10: Applying a style to a fresh object (no prior property set)
-- ---------------------------------------------------------------------------
local function test_style_applies_to_new_node()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  -- Opacity defaults to 1.0; style overrides it
  addStyleRule(node, "ghost", { Opacity = "0.1" })
  node.class = "ghost"
  applyStyles(node)

  expect_near(node.Opacity, 0.1, 0.001, "ghost class: Opacity=0.1 on fresh node")

  node:removeFromParent()
  print("PASS: test_style_applies_to_new_node")
end

-- ---------------------------------------------------------------------------
-- Test 11: @apply directive only inherits default rules (no pseudo-state inheritance)
-- ---------------------------------------------------------------------------
local function test_style_apply_inherits_default_only()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  -- Define a base class "base" with a default rule and a :hover rule
  addStyleRule(node, "base",       { Opacity = "0.5", Width = "120" })
  addStyleRule(node, "base:hover", { Opacity = "0.9" })

  -- Define a derived class "derived" that @apply's "base" (no pseudo-state)
  -- → should inherit Opacity=0.5 and Width=120 but NOT the :hover Opacity=0.9
  addStyleRule(node, "derived", { ["@apply"] = "base", Height = "80" })

  node.class = "derived"
  applyStyles(node)

  -- Default base rule inherited: Opacity=0.5, Width=120; Height from derived=80
  expect_near(node.Opacity, 0.5, 0.001, "@apply: inherited Opacity=0.5 (default rule)")
  expect_near(node.Width,  120,  0.5,   "@apply: inherited Width=120 (default rule)")
  expect_near(node.Height,  80,  0.5,   "@apply: own Height=80 applied")

  node:removeFromParent()
  print("PASS: test_style_apply_inherits_default_only")
end

-- ---------------------------------------------------------------------------
-- Test 12: @apply with pseudo-state selector inherits only that pseudo-state rule
-- ---------------------------------------------------------------------------
local function test_style_apply_inherits_pseudo_state()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  addStyleRule(node, "src",       { Width = "100" })  -- default rule for "src"
  addStyleRule(node, "src:hover", { Width = "200" })  -- :hover rule for "src"

  -- "@apply src:hover" should copy only the :hover Width=200, not the default Width=100
  addStyleRule(node, "dst", { ["@apply"] = "src:hover" })

  node.class = "dst"
  applyStyles(node)

  -- "dst" has no default Width rule of its own — only the @apply-ed :hover Width=200
  expect_near(node.Width, 200, 0.5, "@apply src:hover: inherited Width=200")

  node:removeFromParent()
  print("PASS: test_style_apply_inherits_pseudo_state")
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
test_style_apply_inherits_default_only()
test_style_apply_inherits_pseudo_state()

print("All style tests passed.")
