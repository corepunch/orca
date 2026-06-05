local test = require "orca.test"
-- Headless tests for the ORCA style system — exercised from Lua using
-- the CSS string API, node.StyleClass/node.class, and StyleController.ThemeChanged.
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

-- ui.loadObjectFromCssString exposes the pure-C CSS parser.

-- ---------------------------------------------------------------------------
-- Node2D inherits StyleController, so every Node2D has one attached.
-- applyStyles(node) triggers style resolution.
local function applyStyles(node)
  node:send("StyleController.ThemeChanged")
end

local function test_styleclass_lua_alias()
  local screen = ui.Screen {
    Width = 200,
    Height = 200,
    ResizeMode = "NoResize",
    StyleSheet = ui.loadObjectFromCssString ".native-alias { opacity: 0.33; } .runtime-alias { opacity: 0.44; }",
  }
  local node = screen + ui.Node2D {
    StyleClass = "native-alias",
    Opacity = 1.0,
  }
  local runtime = screen + ui.Node2D {
    Opacity = 1.0,
  }
  runtime.StyleClass = "runtime-alias"

  applyStyles(node)
  applyStyles(runtime)

  test.expect_near(node.Opacity, 0.33, 0.001, "StyleClass property table alias applies style classes")
  test.expect_near(runtime.Opacity, 0.44, 0.001, "StyleClass runtime assignment applies style classes")

  runtime:removeFromParent()
  node:removeFromParent()
  print("PASS: test_styleclass_lua_alias")
end

local function test_styleclass_xml_alias()
  local root = filesystem.loadObjectFromXmlString [[
<Screen Name="styleclass-root" Width="200" Height="200" ResizeMode="NoResize">
  <Node2D Name="pascal-style" StyleClass="pascal-alias" Opacity="1" />
  <Node2D Name="html-style" class="html-alias" Opacity="1" />
</Screen>]]

  test.expect(root ~= nil, "StyleClass XML should load")
  root.StyleSheet = ui.loadObjectFromCssString [[
    .pascal-alias { opacity: 0.42; }
    .html-alias { opacity: 0.24; }
  ]]

  local pascal = root:findChild("pascal-style", true)
  local html = root:findChild("html-style", true)
  test.expect(pascal ~= nil, "StyleClass XML child should exist")
  test.expect(html ~= nil, "class XML child should exist")

  applyStyles(pascal)
  applyStyles(html)

  test.expect_near(pascal.Opacity, 0.42, 0.001, "StyleClass XML alias applies style classes")
  test.expect_near(html.Opacity, 0.24, 0.001, "class XML alias still applies style classes")

  root:removeFromParent()
  print("PASS: test_styleclass_xml_alias")
end

-- ---------------------------------------------------------------------------
-- Test 1: CSS opacity rule + class assignment → Opacity applied
-- ---------------------------------------------------------------------------
local function test_style_applies_opacity()
  local screen = ui.Screen { 
    Width = 200,
    Height = 200,
    ResizeMode = "NoResize",
    StyleSheet = ui.loadObjectFromCssString ".highlight { opacity: 0.4; }",
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
  screen.StyleSheet = ui.loadObjectFromCssString ".hidden { opacity: 0.0; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".box { width: 200; height: 80; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".alpha50 { opacity: 0.5; } .alpha25 { opacity: 0.25; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".btn:hover { opacity: 0.6; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".card { opacity: 0.7; } .card:hover { opacity: 1.0; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".dim { opacity: 0.3; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".primary { opacity: 0.55; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".sized { width: 150; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".overflowing { text-overflow: ellipsis; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".ghost { opacity: 0.1; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
  screen.StyleSheet = ui.loadObjectFromCssString ".alpha, .beta { width: 123; height: 45; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
  screen.StyleSheet = ui.loadObjectFromCssString "Label { height: 33; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".Token { opacity: 0.25; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".plain { made-up-property: 123; opacity: 0.66; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".wrapped { word-wrap: TRUE; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString ".painted { background-color: #336699; }"
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
  screen.StyleSheet = ui.loadObjectFromCssString [[
    .themed {
      background-color: var(--accent);
      color: var(--accent-foreground);
      border-color: var(--control-border);
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
  test.expect_near(node.ForegroundColor.R, fg.R, 0.01, "var(--accent-foreground) ForegroundColor.R")
  test.expect_near(node.ForegroundColor.G, fg.G, 0.01, "var(--accent-foreground) ForegroundColor.G")
  test.expect_near(node.ForegroundColor.B, fg.B, 0.01, "var(--accent-foreground) ForegroundColor.B")
  test.expect_near(node.BorderColor.R, border.R, 0.01, "var(--control-border) BorderColor.R")
  test.expect_near(node.BorderColor.G, border.G, 0.01, "var(--control-border) BorderColor.G")
  test.expect_near(node.BorderColor.B, border.B, 0.01, "var(--control-border) BorderColor.B")

  node:removeFromParent()
  print("PASS: test_css_theme_variables")
end

-- ---------------------------------------------------------------------------
-- Test 23: Text CSS properties map onto TextRun/TextBlockConcept properties
-- ---------------------------------------------------------------------------
local function test_css_text_property_map()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString ".copy { font-size: 18; line-height: 1.5; letter-spacing: 2; text-overflow: clip; }"
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
-- Test 24: Expanded CSS property aliases map onto UIKit/Core properties
-- ---------------------------------------------------------------------------
local function test_css_expanded_property_aliases()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString [[
    .layout-css {
      horizontal-align: center;
      vertical-align: bottom;
      margin-inline: 4;
      padding-block: 6;
      border-color: #aabbcc;
      border-width: 7;
      border-style: solid;
      border-radius: 13;
      border-left-width: 3;
      border-top-left-radius: 5;
      background: #112233;
      foreground: #445566;
      overflow-x: hidden;
      ring-width: 2;
      ring-color: #778899;
      box-shadow-color: #010203;
      pointer-events: none;
      flex-direction: horizontal;
      gap: 11;
      align-items: end;
      justify-content: space-between;
    }

    .copy-css {
      font-weight: bold;
      font-style: italic;
      character-spacing: 3;
      fixed-character-width: 9;
      text-decoration-thickness: 2;
      text-align: right;
      text-vertical-align: bottom;
      text-wrap: no-wrap;
    }
  ]]

  local stack = screen + ui.StackView {
    class = "layout-css",
  }
  local grid = screen + ui.Grid {
    class = "layout-css",
  }
  local text = screen + ui.TextBlock {
    class = "copy-css",
    Text = "copy",
  }

  applyStyles(stack)
  applyStyles(grid)
  applyStyles(text)

  test.expect_eq(stack.HorizontalAlignment, "Center", "horizontal-align maps to Node.HorizontalAlignment")
  test.expect_eq(stack.VerticalAlignment, "Bottom", "vertical-align maps to Node.VerticalAlignment")
  test.expect_near(stack.MarginLeft, 4, 0.001, "margin-inline maps to horizontal margin")
  test.expect_near(stack.MarginRight, 4, 0.001, "margin-inline maps to horizontal margin")
  test.expect_near(stack.PaddingTop, 6, 0.001, "padding-block maps to vertical padding")
  test.expect_near(stack.PaddingBottom, 6, 0.001, "padding-block maps to vertical padding")
  test.expect_near(stack.BorderColor.R, 0xaa / 255, 0.01, "border-color maps to Node.BorderColor")
  test.expect_near(stack.BorderWidthRight, 7, 0.001, "border-width maps to Node.BorderWidth")
  test.expect_eq(stack.BorderStyle, "Solid", "border-style maps to Node.BorderStyle")
  test.expect_near(stack.BorderRadius.TopRightRadius, 13, 0.001, "border-radius maps to Node.BorderRadius")
  test.expect_near(stack.BorderWidthLeft, 3, 0.001, "border-left-width maps to Node.BorderWidthLeft")
  test.expect_near(stack.BorderRadius.TopLeftRadius, 5, 0.001, "border-top-left-radius maps to Node.BorderTopLeftRadius")
  test.expect_near(stack.BackgroundColor.R, 0x11 / 255, 0.01, "background maps to BackgroundColor")
  test.expect_near(stack.ForegroundColor.G, 0x55 / 255, 0.01, "foreground maps to ForegroundColor")
  test.expect_eq(stack.OverflowX, "Hidden", "overflow-x maps to Node2D.OverflowX")
  test.expect_near(stack.RingWidth, 2, 0.001, "ring-width maps to Node2D.RingWidth")
  test.expect_near(stack.RingColor.B, 0x99 / 255, 0.01, "ring-color maps to Node2D.RingColor")
  test.expect_near(stack.BoxShadowColor.B, 0x03 / 255, 0.01, "box-shadow-color maps to Node2D.BoxShadowColor")
  test.expect(stack.IgnoreHitTest == true, "pointer-events: none maps to Node2D.IgnoreHitTest")
  test.expect_eq(stack.Direction, "Horizontal", "flex-direction maps to StackView.Direction")
  test.expect_near(stack.Spacing, 11, 0.001, "gap maps to StackView.Spacing")
  test.expect_near(grid.Spacing, 11, 0.001, "gap maps to Grid.Spacing")
  test.expect_eq(stack.AlignItems, "End", "align-items maps to StackView.AlignItems")
  test.expect_eq(stack.JustifyContent, "SpaceBetween", "hyphenated justify-content value maps to SpaceBetween")

  test.expect_eq(text.FontWeight, "Bold", "font-weight maps to TextRun.FontWeight")
  test.expect_eq(text.FontStyle, "Italic", "font-style maps to TextRun.FontStyle")
  test.expect_near(text.CharacterSpacing, 3, 0.001, "character-spacing maps to TextRun.CharacterSpacing")
  test.expect_near(text.FixedCharacterWidth, 9, 0.001, "fixed-character-width maps to TextRun.FixedCharacterWidth")
  test.expect_near(text.UnderlineWidth, 2, 0.001, "text-decoration-thickness maps to TextRun.UnderlineWidth")
  test.expect_eq(text.TextHorizontalAlignment, "Right", "text-align maps to TextHorizontalAlignment")
  test.expect_eq(text.TextVerticalAlignment, "Bottom", "text-vertical-align maps to TextVerticalAlignment")
  test.expect_eq(text.TextWrapping, "NoWrap", "hyphenated text-wrap value maps to NoWrap")

  stack:removeFromParent()
  grid:removeFromParent()
  text:removeFromParent()
  print("PASS: test_css_expanded_property_aliases")
end

-- ---------------------------------------------------------------------------
-- Test 25: CSS edge shorthands use CSS order before ORCA Thickness parsing
-- ---------------------------------------------------------------------------
local function test_css_edge_shorthand_order()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString [[
    .two-values {
      padding: 10 32;
      margin: 4 8;
    }

    .three-values {
      padding: 10 20 30;
    }

    .four-values {
      margin: 1 2 3 4;
      border-width: 5 6 7 8;
    }
  ]]
  local two = screen + ui.Node2D { class = "two-values" }
  local three = screen + ui.Node2D { class = "three-values" }
  local four = screen + ui.Node2D { class = "four-values" }

  applyStyles(two)
  applyStyles(three)
  applyStyles(four)

  test.expect_near(two.PaddingTop, 10, 0.001, "padding: 10 32 sets vertical padding to 10")
  test.expect_near(two.PaddingBottom, 10, 0.001, "padding: 10 32 sets bottom padding to 10")
  test.expect_near(two.PaddingLeft, 32, 0.001, "padding: 10 32 sets horizontal padding to 32")
  test.expect_near(two.PaddingRight, 32, 0.001, "padding: 10 32 sets right padding to 32")
  test.expect_near(two.MarginTop, 4, 0.001, "margin: 4 8 sets vertical margin to 4")
  test.expect_near(two.MarginLeft, 8, 0.001, "margin: 4 8 sets horizontal margin to 8")

  test.expect_near(three.PaddingTop, 10, 0.001, "padding: 10 20 30 sets top padding to 10")
  test.expect_near(three.PaddingLeft, 20, 0.001, "padding: 10 20 30 sets horizontal padding to 20")
  test.expect_near(three.PaddingRight, 20, 0.001, "padding: 10 20 30 sets right padding to 20")
  test.expect_near(three.PaddingBottom, 30, 0.001, "padding: 10 20 30 sets bottom padding to 30")

  test.expect_near(four.MarginTop, 1, 0.001, "margin: 1 2 3 4 sets top margin to 1")
  test.expect_near(four.MarginRight, 2, 0.001, "margin: 1 2 3 4 sets right margin to 2")
  test.expect_near(four.MarginBottom, 3, 0.001, "margin: 1 2 3 4 sets bottom margin to 3")
  test.expect_near(four.MarginLeft, 4, 0.001, "margin: 1 2 3 4 sets left margin to 4")
  test.expect_near(four.BorderWidthTop, 5, 0.001, "border-width: 5 6 7 8 sets top border to 5")
  test.expect_near(four.BorderWidthRight, 6, 0.001, "border-width: 5 6 7 8 sets right border to 6")
  test.expect_near(four.BorderWidthBottom, 7, 0.001, "border-width: 5 6 7 8 sets bottom border to 7")
  test.expect_near(four.BorderWidthLeft, 8, 0.001, "border-width: 5 6 7 8 sets left border to 8")

  two:removeFromParent()
  three:removeFromParent()
  four:removeFromParent()
  print("PASS: test_css_edge_shorthand_order")
end

-- ---------------------------------------------------------------------------
-- Test 26: @apply can reference selectors without a leading dot
-- ---------------------------------------------------------------------------
local function test_css_apply_reference_without_dot()
  local css = [[
    .base { opacity: 0.37; }
    .child { @apply: base; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "child"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.37, 0.001, "@apply should resolve references without dots")

  node:removeFromParent()
  print("PASS: test_css_apply_reference_without_dot")
end

-- ---------------------------------------------------------------------------
-- Test 27: @apply merges multiple sources
-- ---------------------------------------------------------------------------
local function test_css_apply_multiple_sources()
  local css = [[
    .size-x { width: 71; }
    .size-y { height: 72; }
    .both { @apply: .size-x .size-y; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local node = screen + ui.Node2D {}

  node.class = "both"
  applyStyles(node)

  test.expect_near(node.Width, 71, 0.5, "@apply copied Width from first source")
  test.expect_near(node.Height, 72, 0.5, "@apply copied Height from second source")

  node:removeFromParent()
  print("PASS: test_css_apply_multiple_sources")
end

-- ---------------------------------------------------------------------------
-- Test 28: Local declarations override @apply sources
-- ---------------------------------------------------------------------------
local function test_css_apply_preserves_local_declarations()
  local css = [[
    .base { opacity: 0.2; width: 12; }
    .child { opacity: 0.8; @apply: .base; }
  ]]
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local node = screen + ui.Node2D { Opacity = 1.0 }

  node.class = "child"
  applyStyles(node)

  test.expect_near(node.Opacity, 0.8, 0.001, "local opacity should beat @apply opacity")
  test.expect_near(node.Width, 12, 0.5, "@apply should still copy missing properties")

  node:removeFromParent()
  print("PASS: test_css_apply_preserves_local_declarations")
end

-- ---------------------------------------------------------------------------
-- Test 29: Body selector applies to the root object that owns the stylesheet
-- ---------------------------------------------------------------------------
local function test_css_body_selector_applies_to_root()
  local screen = ui.Screen {
    Width = 200,
    Height = 200,
    ResizeMode = "NoResize",
    StyleSheet = ui.loadObjectFromCssString "body { width: 240; }",
  }

  applyStyles(screen)

  test.expect_near(screen.Width, 240, 0.001, "body selector should apply to screen root properties")

  print("PASS: test_css_body_selector_applies_to_root")
end

-- ---------------------------------------------------------------------------
-- Test 30: ID selectors match object names
-- ---------------------------------------------------------------------------
local function test_css_id_selector()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString "#HeroImage { opacity: 0.73; }"
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
-- Test 31: Direct parent selectors match only immediate children
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_selector()
  local css = "StackView > Label { opacity: 0.31; }"
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
-- Test 32: Direct parent selectors also support class and ID selectors
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_class_and_id_selectors()
  local css = [[
    .toolbar > #SaveLabel { width: 88; }
    #Footer > .status { height: 22; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
-- Test 33: Descendant selectors match any ancestor, not just direct parents
-- ---------------------------------------------------------------------------
local function test_css_descendant_selector()
  local css = [[
    .popup .panel { opacity: 0.27; }
    .popup Label { width: 77; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local popup = screen + ui.Node2D { class = "popup" }
  local wrapper = popup + ui.Node2D {}
  local panel = wrapper + ui.Node2D { class = "panel", Opacity = 1.0 }
  local label = wrapper + ui.Label { Text = "nested" }
  local outside = screen + ui.Node2D { class = "panel", Opacity = 1.0 }

  applyStyles(panel)
  applyStyles(label)
  applyStyles(outside)

  test.expect_near(panel.Opacity, 0.27, 0.001, ".popup .panel should match nested descendants")
  test.expect_near(label.Width, 77, 0.5, ".popup Label should match nested type selectors")
  test.expect_near(outside.Opacity, 1.0, 0.001, ".popup .panel should not match without popup ancestor")

  popup:removeFromParent()
  outside:removeFromParent()
  print("PASS: test_css_descendant_selector")
end

-- ---------------------------------------------------------------------------
-- Test 34: Descendant selectors compose with direct child selectors
-- ---------------------------------------------------------------------------
local function test_css_mixed_descendant_and_direct_child_selectors()
  local css = [[
    .popup > .chrome .panel { height: 64; }
    .popup .chrome > .panel { width: 65; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local popup = screen + ui.Node2D { class = "popup" }
  local chrome = popup + ui.Node2D { class = "chrome" }
  local direct = chrome + ui.Node2D { class = "panel" }
  local wrapper = chrome + ui.Node2D {}
  local nested = wrapper + ui.Node2D { class = "panel" }

  applyStyles(direct)
  applyStyles(nested)

  test.expect_near(direct.Height, 64, 0.5, ".popup > .chrome .panel should match direct panel")
  test.expect_near(direct.Width, 65, 0.5, ".popup .chrome > .panel should match direct panel")
  test.expect_near(nested.Height, 64, 0.5, ".popup > .chrome .panel should match nested panel")
  test.expect_near(nested.Width, 0, 0.5, ".popup .chrome > .panel should not match nested panel")

  popup:removeFromParent()
  print("PASS: test_css_mixed_descendant_and_direct_child_selectors")
end

-- ---------------------------------------------------------------------------
-- Test 35: Compound selectors match type, class, and ID on one object
-- ---------------------------------------------------------------------------
local function test_css_compound_type_class_selectors()
  local css = [[
    Button.primary {
      background-color: #112233;
      color: #ddeeff;
      width: 101;
    }

    Button.secondary {
      background-color: #445566;
      height: 33;
    }

    .popup Button.primary {
      padding-left: 32;
    }

    #PrimaryButton.primary {
      padding-right: 24;
    }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
  local popup = screen + ui.Node2D { class = "popup" }
  local primary = popup + ui.Button {
    Name = "PrimaryButton",
    class = "primary",
    Text = "Primary",
  }
  local secondary = popup + ui.Button {
    class = "secondary",
    Text = "Secondary",
  }
  local plain = popup + ui.Button {
    Text = "Plain",
  }
  local label = popup + ui.Label {
    class = "primary",
    Text = "Not a button",
  }

  applyStyles(primary)
  applyStyles(secondary)
  applyStyles(plain)
  applyStyles(label)

  test.expect_near(primary.BackgroundColor.R, 0x11 / 255, 0.01, "Button.primary should match button class primary")
  test.expect_near(primary.ForegroundColor.G, 0xee / 255, 0.01, "Button.primary color should map to ForegroundColor")
  test.expect_near(primary.Width, 101, 0.5, "Button.primary should apply Width")
  test.expect_near(primary.PaddingLeft, 32, 0.5, ".popup Button.primary should match nested primary button")
  test.expect_near(primary.PaddingRight, 24, 0.5, "#PrimaryButton.primary should match id and class")

  test.expect_near(secondary.BackgroundColor.G, 0x55 / 255, 0.01, "Button.secondary should match button class secondary")
  test.expect_near(secondary.Height, 33, 0.5, "Button.secondary should apply Height")
  test.expect_near(plain.Width, 0, 0.5, "Button.primary should not match a plain button")
  test.expect_near(label.Width, 0, 0.5, "Button.primary should not match a Label with class primary")

  popup:removeFromParent()
  print("PASS: test_css_compound_type_class_selectors")
end

-- ---------------------------------------------------------------------------
-- Test 36: Pseudo-classes apply to class, ID, and type selectors
-- ---------------------------------------------------------------------------
local function test_css_pseudo_classes_on_selector_types()
  local css = [[
    .choice:active { opacity: 0.4; }
    #NamedChoice:active { width: 91; }
    Label:active { height: 29; }
  ]]
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(css)
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
-- Test 37: Pseudo-classes work on direct parent selectors
-- ---------------------------------------------------------------------------
local function test_css_direct_parent_selector_with_pseudo_class()
  local screen = ui.Screen { Width = 300, Height = 300, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString "StackView > Label:active { opacity: 0.21; }"
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
-- Test 38: CSS font-family accepts quoted names and generic fallbacks
-- ---------------------------------------------------------------------------
local function test_css_font_family_list_uses_registered_fallback()
  local screen = ui.Screen { Width = 300, Height = 120, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString [[
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


test_styleclass_lua_alias()
test_styleclass_xml_alias()
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
test_css_expanded_property_aliases()
test_css_edge_shorthand_order()
test_css_apply_reference_without_dot()
test_css_apply_multiple_sources()
test_css_apply_preserves_local_declarations()
test_css_body_selector_applies_to_root()
test_css_id_selector()
test_css_direct_parent_selector()
test_css_direct_parent_class_and_id_selectors()
test_css_descendant_selector()
test_css_mixed_descendant_and_direct_child_selectors()
test_css_compound_type_class_selectors()
test_css_pseudo_classes_on_selector_types()
test_css_direct_parent_selector_with_pseudo_class()
test_css_font_family_list_uses_registered_fallback()

print("All style tests passed.")
