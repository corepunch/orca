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
local geometry = require "orca.geometry"
require "orca.renderer"
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
-- Test 10: Enum CSS values are matched case-insensitively
-- ---------------------------------------------------------------------------
local function test_style_enum_value_ignorecase()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".overflowing { text-overflow: ellipsis; }"
  local text = screen + ui.TextBlock {
    Text = "long text",
    TextOverflow = "Clip",
  }

  text.class = "overflowing"
  applyStyles(text)

  test.expect_eq(text.TextOverflow, "Ellipsis", "CSS enum value 'ellipsis' applied as TextOverflow=Ellipsis")

  text:removeFromParent()
  print("PASS: test_style_enum_value_ignorecase")
end

-- ---------------------------------------------------------------------------
-- Test 11: Applying a style to a fresh object (no prior property set)
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
-- Test 12: @apply directive merges another rule's declarations
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
-- Test 13: Transitive @apply chain  A → B → C propagates C's properties to A
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

-- ---------------------------------------------------------------------------
-- Test 14: CSS block comments are ignored
-- ---------------------------------------------------------------------------
local function test_css_comments_are_ignored()
  local css = [[
    /* .commented { opacity: 0.0; } */
    .clean { opacity: 0.42; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "clean"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.42, 0.001, "CSS comments should not create rules")

  node:removeFromParent()
  print("PASS: test_css_comments_are_ignored")
end

-- ---------------------------------------------------------------------------
-- Test 15: Comma-separated selector lists apply to every selector
-- ---------------------------------------------------------------------------
local function test_css_comma_selectors()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".alpha, .beta { width: 123; height: 45; }"
  local a = screen + ui.Node2D {}
  local b = screen + ui.Node2D {}

  a.class = "alpha"
  b.class = "beta"
  applyStyles(a)
  applyStyles(b)

  test.expect_near(a.Width, 123, 0.5, "comma selector .alpha applied Width=123")
  test.expect_near(b.Width, 123, 0.5, "comma selector .beta applied Width=123")
  test.expect_near(a.Height, 45, 0.5, "comma selector .alpha applied Height=45")
  test.expect_near(b.Height, 45, 0.5, "comma selector .beta applied Height=45")

  a:removeFromParent()
  b:removeFromParent()
  print("PASS: test_css_comma_selectors")
end

-- ---------------------------------------------------------------------------
-- Test 16: Repeated selectors merge declarations
-- ---------------------------------------------------------------------------
local function test_css_repeated_selector_merges()
  local css = [[
    .repeat { width: 40; }
    .repeat { height: 50; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D {}

  node.class = "repeat"
  applyStyles(node)

  test.expect_near(node.Width, 40, 0.5, "repeated selector keeps first declaration")
  test.expect_near(node.Height, 50, 0.5, "repeated selector adds later declaration")

  node:removeFromParent()
  print("PASS: test_css_repeated_selector_merges")
end

-- ---------------------------------------------------------------------------
-- Test 17: Property names are case-insensitive and duplicate keys last-win
-- ---------------------------------------------------------------------------
local function test_css_property_name_ignorecase_and_duplicate_overwrite()
  local css = ".casey { OpAcItY: 0.2; opacity: 0.65; WIDTH: 111; }"
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "casey"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.65, 0.001, "duplicate opacity declaration should use the last value")
  test.expect_near(node.Width, 111, 0.5, "uppercase WIDTH should map to Width")

  node:removeFromParent()
  print("PASS: test_css_property_name_ignorecase_and_duplicate_overwrite")
end

-- ---------------------------------------------------------------------------
-- Test 18: Type selectors match the ORCA class name
-- ---------------------------------------------------------------------------
local function test_css_type_selector()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString "Label { height: 33; }"
  local label = screen + ui.Label { Text = "caption" }

  applyStyles(label)

  test.expect_near(label.Height, 33, 0.5, "type selector Label should match ui.Label")

  label:removeFromParent()
  print("PASS: test_css_type_selector")
end

-- ---------------------------------------------------------------------------
-- Test 19: Selectors are case-sensitive
-- ---------------------------------------------------------------------------
local function test_css_selectors_are_case_sensitive()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".Token { opacity: 0.25; }"
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "token"
  applyStyles(node)

  test.expect_near(node.Opacity, 1.0, 0.001, "selector class names should be case-sensitive")

  node:removeFromParent()
  print("PASS: test_css_selectors_are_case_sensitive")
end

-- ---------------------------------------------------------------------------
-- Test 20: Unsupported declarations are ignored while supported ones apply
-- ---------------------------------------------------------------------------
local function test_css_unsupported_declarations_are_ignored()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".plain { made-up-property: 123; opacity: 0.66; }"
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "plain"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.66, 0.001, "supported declarations should still apply")

  node:removeFromParent()
  print("PASS: test_css_unsupported_declarations_are_ignored")
end

-- ---------------------------------------------------------------------------
-- Test 21: Boolean CSS values are parsed case-insensitively
-- ---------------------------------------------------------------------------
local function test_css_boolean_value_ignorecase()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".wrapped { word-wrap: TRUE; }"
  local text = screen + ui.TextBlock {
    Text = "wrap me",
    WordWrap = false,
  }

  text.class = "wrapped"
  applyStyles(text)

  test.expect(text.WordWrap == true, "CSS bool value TRUE should apply as true")

  text:removeFromParent()
  print("PASS: test_css_boolean_value_ignorecase")
end

-- ---------------------------------------------------------------------------
-- Test 22: Mapped color properties are parsed and applied
-- ---------------------------------------------------------------------------
local function test_css_color_properties()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".painted { background-color: #336699; }"
  local node = screen + ui.Node2D {}

  node.class = "painted"
  applyStyles(node)

  test.expect_near(node.BackgroundColor.R, 0x33 / 255, 0.01, "background-color R")
  test.expect_near(node.BackgroundColor.G, 0x66 / 255, 0.01, "background-color G")
  test.expect_near(node.BackgroundColor.B, 0x99 / 255, 0.01, "background-color B")

  node:removeFromParent()
  print("PASS: test_css_color_properties")
end

-- ---------------------------------------------------------------------------
-- Test 23: CSS theme variables resolve through the active theme
-- ---------------------------------------------------------------------------
local function test_css_theme_variables()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString [[
    .themed {
      background-color: var(--accent);
      color: var($accent-foreground);
      border-color: $control-border;
    }
  ]]
  local node = screen + ui.Node2D {}

  node.class = "themed"
  applyStyles(node)

  local bg = geometry.Color.parse(filesystem.getThemeValue("$accent"))
  local fg = geometry.Color.parse(filesystem.getThemeValue("$accent-foreground"))
  local border = geometry.Color.parse(filesystem.getThemeValue("$control-border"))

  test.expect_near(node.BackgroundColor.R, bg.R, 0.01, "var(--accent) BackgroundColor.R")
  test.expect_near(node.BackgroundColor.G, bg.G, 0.01, "var(--accent) BackgroundColor.G")
  test.expect_near(node.BackgroundColor.B, bg.B, 0.01, "var(--accent) BackgroundColor.B")
  test.expect_near(node.ForegroundColor.R, fg.R, 0.01, "var($accent-foreground) ForegroundColor.R")
  test.expect_near(node.ForegroundColor.G, fg.G, 0.01, "var($accent-foreground) ForegroundColor.G")
  test.expect_near(node.ForegroundColor.B, fg.B, 0.01, "var($accent-foreground) ForegroundColor.B")
  test.expect_near(node.BorderColor.R, border.R, 0.01, "$control-border BorderColor.R")
  test.expect_near(node.BorderColor.G, border.G, 0.01, "$control-border BorderColor.G")
  test.expect_near(node.BorderColor.B, border.B, 0.01, "$control-border BorderColor.B")

  node:removeFromParent()
  print("PASS: test_css_theme_variables")
end

-- ---------------------------------------------------------------------------
-- Test 23: Text CSS properties map onto TextRun/TextBlockConcept properties
-- ---------------------------------------------------------------------------
local function test_css_text_property_map()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString ".copy { font-size: 18; line-height: 1.5; letter-spacing: 2; text-overflow: clip; }"
  local text = screen + ui.TextBlock {
    Text = "copy",
    FontSize = 10,
    LineHeight = 1,
    LetterSpacing = 0,
    TextOverflow = "Ellipsis",
  }

  text.class = "copy"
  applyStyles(text)

  test.expect_near(text.FontSize, 18, 0.5, "font-size maps to FontSize")
  test.expect_near(text.LineHeight, 1.5, 0.01, "line-height maps to LineHeight")
  test.expect_near(text.LetterSpacing, 2, 0.01, "letter-spacing maps to LetterSpacing")
  test.expect_eq(text.TextOverflow, "Clip", "text-overflow maps to TextOverflow")

  text:removeFromParent()
  print("PASS: test_css_text_property_map")
end

-- ---------------------------------------------------------------------------
-- Test 24: @apply can reference selectors without a leading dot
-- ---------------------------------------------------------------------------
local function test_css_apply_reference_without_dot()
  local css = [[
    .base { opacity: 0.37; }
    .child { @apply: base; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "child"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.37, 0.001, "@apply should resolve references without dots")

  node:removeFromParent()
  print("PASS: test_css_apply_reference_without_dot")
end

-- ---------------------------------------------------------------------------
-- Test 25: @apply merges multiple sources
-- ---------------------------------------------------------------------------
local function test_css_apply_multiple_sources()
  local css = [[
    .size-x { width: 71; }
    .size-y { height: 72; }
    .both { @apply: .size-x .size-y; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D {}

  node.class = "both"
  applyStyles(node)

  test.expect_near(node.Width, 71, 0.5, "@apply copied Width from first source")
  test.expect_near(node.Height, 72, 0.5, "@apply copied Height from second source")

  node:removeFromParent()
  print("PASS: test_css_apply_multiple_sources")
end

-- ---------------------------------------------------------------------------
-- Test 26: Local declarations override @apply sources
-- ---------------------------------------------------------------------------
local function test_css_apply_preserves_local_declarations()
  local css = [[
    .base { opacity: 0.2; width: 12; }
    .child { opacity: 0.8; @apply: .base; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "child"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.8, 0.001, "local opacity should beat @apply opacity")
  test.expect_near(node.Width, 12, 0.5, "@apply should still copy missing properties")

  node:removeFromParent()
  print("PASS: test_css_apply_preserves_local_declarations")
end

-- ---------------------------------------------------------------------------
-- Test 27: Body selector applies to the root object that owns the stylesheet
-- ---------------------------------------------------------------------------
local function test_css_body_selector_applies_to_root()
  local screen = ui.Screen {
    Width = 200,
    Height = 200,
    ResizeMode = "NoResize",
    Opacity = 1.0,
    StyleSheet = filesystem.loadObjectFromCssString "body { opacity: 0.44; }",
  }

  applyStyles(screen)

  test.expect_near(screen.Opacity, 0.44, 0.001, "body selector should apply to stylesheet root")

  print("PASS: test_css_body_selector_applies_to_root")
end

-- ---------------------------------------------------------------------------
-- Test 28: ID selectors match object names
-- ---------------------------------------------------------------------------
local function test_css_id_selector()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString "#HeroImage { opacity: 0.73; }"
  local image = screen + ui.ImageView {
    Name = "HeroImage",
    Opacity = 1.0,
  }
  local other = screen + ui.ImageView {
    Name = "OtherImage",
    Opacity = 1.0,
  }

  applyStyles(image)
  applyStyles(other)

  test.expect_near(image.Opacity, 0.73, 0.001, "#HeroImage should match object Name=HeroImage")
  test.expect_near(other.Opacity, 1.0, 0.001, "#HeroImage should not match a different object name")

  image:removeFromParent()
  other:removeFromParent()
  print("PASS: test_css_id_selector")
end

-- ---------------------------------------------------------------------------
-- Test 29: Direct parent selectors match only immediate children
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_selector()
  local css = "StackView > Label { opacity: 0.31; }"
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local stack = screen + ui.StackView {}
  local direct = stack + ui.Label { Text = "direct", Opacity = 1.0 }
  local wrapper = stack + ui.Node2D {}
  local nested = wrapper + ui.Label { Text = "nested", Opacity = 1.0 }

  applyStyles(direct)
  applyStyles(nested)

  test.expect_near(direct.Opacity, 0.31, 0.001, "StackView > Label should match a direct child Label")
  test.expect_near(nested.Opacity, 1.0, 0.001, "StackView > Label should not match nested descendants")

  stack:removeFromParent()
  print("PASS: test_css_direct_parent_selector")
end

-- ---------------------------------------------------------------------------
-- Test 30: Direct parent selectors also support class and ID selectors
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_class_and_id_selectors()
  local css = [[
    .toolbar > #SaveLabel { width: 88; }
    #Footer > .status { height: 22; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local toolbar = screen + ui.StackView { class = "toolbar" }
  local save = toolbar + ui.Label { Name = "SaveLabel", Text = "Save" }
  local footer = screen + ui.StackView { Name = "Footer" }
  local status = footer + ui.Label { class = "status", Text = "Ready" }

  applyStyles(save)
  applyStyles(status)

  test.expect_near(save.Width, 88, 0.5, ".toolbar > #SaveLabel should match")
  test.expect_near(status.Height, 22, 0.5, "#Footer > .status should match")

  toolbar:removeFromParent()
  footer:removeFromParent()
  print("PASS: test_css_direct_parent_class_and_id_selectors")
end

-- ---------------------------------------------------------------------------
-- Test 31: Pseudo-classes apply to class, ID, and type selectors
-- ---------------------------------------------------------------------------
local function test_css_pseudo_classes_on_selector_types()
  local css = [[
    .choice:active { opacity: 0.4; }
    #NamedChoice:active { width: 91; }
    Label:active { height: 29; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString(css)
  local inactive = screen + ui.Label {
    Name = "NamedChoice",
    class = "choice",
    Text = "inactive",
    Opacity = 1.0,
  }
  local active = screen + ui.Label {
    Name = "NamedChoice",
    class = "choice",
    Text = "active",
    Opacity = 1.0,
  }

  active.selected = true
  applyStyles(inactive)
  applyStyles(active)

  test.expect_near(inactive.Opacity, 1.0, 0.001, ":active should not apply before selected=true")
  test.expect_near(active.Opacity, 0.4, 0.001, ".choice:active should match selected object")
  test.expect_near(active.Width, 91, 0.5, "#NamedChoice:active should match selected object")
  test.expect_near(active.Height, 29, 0.5, "Label:active should match selected object")

  inactive:removeFromParent()
  active:removeFromParent()
  print("PASS: test_css_pseudo_classes_on_selector_types")
end

-- ---------------------------------------------------------------------------
-- Test 32: Pseudo-classes work on direct parent selectors
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_selector_with_pseudo_class()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString "StackView > Label:active { opacity: 0.21; }"
  local stack = screen + ui.StackView {}
  local label = stack + ui.Label {
    Text = "selected",
    Opacity = 1.0,
  }

  label.selected = true
  applyStyles(label)

  test.expect_near(label.Opacity, 0.21, 0.001, "StackView > Label:active should match selected direct child")

  stack:removeFromParent()
  print("PASS: test_css_direct_parent_selector_with_pseudo_class")
end

-- ---------------------------------------------------------------------------
-- Test 33: CSS font-family accepts quoted names and generic fallbacks
-- ---------------------------------------------------------------------------
local function test_css_font_family_list_uses_registered_fallback()
  local screen = ui.Screen { Width = 300, Height = 120, ResizeMode = "NoResize" }
  screen.StyleSheet = filesystem.loadObjectFromCssString [[
    .novel { font-family: "Times New Roman", serif; }
  ]]
  local text = screen + ui.TextBlock {
    Text = "chapter",
  }

  text.class = "novel"
  applyStyles(text)

  test.expect(text.FontFamily ~= nil, "font-family list should resolve through registered generic fallback")
  test.expect_eq(text.FontFamily.Regular, "NotoSerif-Regular.ttf", "serif should resolve to shared Noto Serif")

  text:removeFromParent()
  print("PASS: test_css_font_family_list_uses_registered_fallback")
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
test_style_enum_value_ignorecase()
test_style_applies_to_new_node()
test_style_apply_directive()
test_style_apply_transitive()
test_css_comments_are_ignored()
test_css_comma_selectors()
test_css_repeated_selector_merges()
test_css_property_name_ignorecase_and_duplicate_overwrite()
test_css_type_selector()
test_css_selectors_are_case_sensitive()
test_css_unsupported_declarations_are_ignored()
test_css_boolean_value_ignorecase()
test_css_color_properties()
test_css_theme_variables()
test_css_text_property_map()
test_css_apply_reference_without_dot()
test_css_apply_multiple_sources()
test_css_apply_preserves_local_declarations()
test_css_body_selector_applies_to_root()
test_css_id_selector()
test_css_direct_parent_selector()
test_css_direct_parent_class_and_id_selectors()
test_css_pseudo_classes_on_selector_types()
test_css_direct_parent_selector_with_pseudo_class()
test_css_font_family_list_uses_registered_fallback()

print("All style tests passed.")
