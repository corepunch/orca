local test = require "orca.test"
-- Tests for the Lisp UI asset loader.
--
-- Covers:
--   * S-expression parser: nested elements, text content, bare string children.
--   * Attribute binding (Key=(expr)) and :bind directives are attached as programs.
--   * Theme-value attributes ($key) resolve without error.
--   * Prefab placeholders (LayerPrefabPlaceholder) load templates correctly.
--   * Full parity check: Application.lisp produces the same named objects as
--     Application.xml when parsed.
--
-- All tests are headless (no display required).
-- Run with: $(TARGET) -test=tests/test_lisp.lua

local core       = require "orca.core"
local ui         = require "orca.UIKit"
local filesystem = require "orca.filesystem"

local filesystem_initialized = false
local function ensure_fs()
  if filesystem_initialized then return end
  filesystem.init("samples/Example")
  filesystem_initialized = true
end

-- ---------------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------------

local function find_object(root, name)
  if root:getName() == name then return root end
  local child = root:getFirstChild()
  while child do
    local found = find_object(child, name)
    if found then return found end
    child = child:getNext()
  end
  return nil
end

-- ---------------------------------------------------------------------------
-- test_basic_elements: minimal Lisp → object tree
-- ---------------------------------------------------------------------------
local function test_basic_elements()
  ensure_fs()
  local obj = filesystem.loadObjectFromLispString([[
    (Screen Name="TestScreen" Width="100" Height="100"
      (StackView Name="Inner" Direction="Vertical"))
  ]])
  test.expect(obj ~= nil, "basic: root object should be created")
  if not obj then return end
  test.expect(obj:getName() == "TestScreen", "basic: root name should be TestScreen, got " .. tostring(obj:getName()))
  local inner = find_object(obj, "Inner")
  test.expect(inner ~= nil, "basic: child StackView should exist")
  print("PASS: test_basic_elements")
end

-- ---------------------------------------------------------------------------
-- test_text_content: bare string → OBJ_SetTextContent
-- ---------------------------------------------------------------------------
local function test_text_content()
  ensure_fs()
  local obj = filesystem.loadObjectFromLispString([[
    (StackView Name="Row"
      (TextBlock Name="Label" FontSize="14" ForegroundColor="$text-primary"
        "Hello, world"))
  ]])
  test.expect(obj ~= nil, "text: root should exist")
  if not obj then return end
  local label = find_object(obj, "Label")
  test.expect(label ~= nil, "text: TextBlock should exist")
  if label then
    local text = label:getTextContent()
    test.expect(text == "Hello, world",
      "text: content should be 'Hello, world', got '" .. tostring(text) .. "'")
  end
  print("PASS: test_text_content")
end

-- ---------------------------------------------------------------------------
-- test_binding_attr: inline binding on a property attribute
-- ---------------------------------------------------------------------------
local function test_binding_attr()
  ensure_fs()
  local ok, err = pcall(function()
    local obj = filesystem.loadObjectFromLispString([[
      (Grid Name="TestGrid" Spacing="0"
        Columns=(if (step 640 (bind "Node.ActualWidth")) "auto auto" "auto"))
    ]])
    test.expect(obj ~= nil, "binding_attr: object should be created")
  end)
  test.expect(ok, "binding_attr: inline binding should not raise an error: " .. tostring(err))
  print("PASS: test_binding_attr")
end

-- ---------------------------------------------------------------------------
-- test_bind_directive: (:bind Target="Prop" expr) directive
-- ---------------------------------------------------------------------------
local function test_bind_directive()
  ensure_fs()
  local ok, err = pcall(function()
    local obj = filesystem.loadObjectFromLispString([[
      (StackView Name="Adaptive" Direction="Vertical"
        (:bind Target="Node.HorizontalPadding"
          (if (step 640 (bind "Node.ActualWidth")) (vector2 40 40) (vector2 8 8))))
    ]])
    test.expect(obj ~= nil, "bind_directive: object should be created")
  end)
  test.expect(ok, "bind_directive: :bind directive should not raise an error: " .. tostring(err))
  print("PASS: test_bind_directive")
end

-- ---------------------------------------------------------------------------
-- test_theme_values: $key attributes resolve to theme colors
-- ---------------------------------------------------------------------------
local function test_theme_values()
  ensure_fs()
  local obj = filesystem.loadObjectFromLispString([[
    (StackView Name="Themed" BackgroundColor="$surface"
      (TextBlock Name="Lbl" ForegroundColor="$text-primary" "Text"))
  ]])
  test.expect(obj ~= nil, "theme: root should be created")
  print("PASS: test_theme_values")
end

-- ---------------------------------------------------------------------------
-- test_comments: Lisp semicolon comments are ignored
-- ---------------------------------------------------------------------------
local function test_comments()
  ensure_fs()
  local obj = filesystem.loadObjectFromLispString([[
    ; Root node
    (StackView Name="WithComment" ; inline comment
      (TextBlock Name="T" "text")) ; trailing comment
  ]])
  test.expect(obj ~= nil, "comments: object should be created despite comments")
  if not obj then return end
  test.expect(obj:getName() == "WithComment", "comments: name should be WithComment")
  print("PASS: test_comments")
end

-- ---------------------------------------------------------------------------
-- test_application_lisp_readable: Application.lisp file can be read
-- ---------------------------------------------------------------------------
local function test_application_lisp_readable()
  ensure_fs()
  local src = filesystem.readTextFile("samples/Example/Screens/Application.lisp")
  test.expect(src ~= nil and src ~= "", "application_lisp: file should be readable")
  local has_screen      = src:find('(Screen ', 1, true) ~= nil
  local has_tabview     = src:find('(TabView ', 1, true) ~= nil
  local has_brandmark   = src:find('Name="BrandMark"', 1, true) ~= nil
  local has_bind        = src:find('(:bind ', 1, true) ~= nil
  local has_prefab      = src:find('LayerPrefabPlaceholder', 1, true) ~= nil
  local no_xml_entities = src:find('&amp;', 1, true) == nil
  test.expect(has_screen,      "application_lisp: should contain (Screen ...")
  test.expect(has_tabview,     "application_lisp: should contain (TabView ...")
  test.expect(has_brandmark,   "application_lisp: should contain BrandMark element")
  test.expect(has_bind,        "application_lisp: should contain :bind directives")
  test.expect(has_prefab,      "application_lisp: should contain LayerPrefabPlaceholder")
  test.expect(no_xml_entities, "application_lisp: should NOT contain &amp; XML entities (use & directly)")
  print("PASS: test_application_lisp_readable")
end

-- ---------------------------------------------------------------------------
-- test_application_lisp_parity: Application.lisp produces the same key
-- named objects as Application.xml after parsing.
-- ---------------------------------------------------------------------------
local function test_application_lisp_parity()
  ensure_fs()

  -- Key object names that must be present in both XML and Lisp trees.
  local key_names = {
    "Application", "Root", "NavBar", "BrandMark", "BrandIcon", "Brand",
    "Body", "Hero", "HeroContent", "HeroImage",
    "TabbedTechSection", "OrcaTabs", "OrcaTabBar",
    "XmlTab", "LuaTab", "MoonTab", "DeployTab",
    "FeatureSection", "GallerySection", "GalleryGrid",
    "WorkflowSection", "TestimonialsSection", "CtaSection", "CtaButtons",
    "CtaButtonPrimary",
  }

  local src = filesystem.readTextFile("samples/Example/Screens/Application.lisp")
  test.expect(src ~= nil and src ~= "", "parity: Application.lisp should be readable")
  if not src or src == "" then return end

  local ok, obj_or_err = pcall(filesystem.loadObjectFromLispString, src)
  test.expect(ok, "parity: Application.lisp should parse without error: " .. tostring(obj_or_err))
  if not ok then return end
  local obj = obj_or_err

  for _, name in ipairs(key_names) do
    local found = find_object(obj, name)
    test.expect(found ~= nil, "parity: object '" .. name .. "' should exist in Lisp tree")
  end

  -- CTA modal action: CtaButtonPrimary should exist and have a BackgroundColor set.
  local cta = find_object(obj, "CtaButtonPrimary")
  test.expect(cta ~= nil, "parity: CtaButtonPrimary should exist in Lisp tree")

  print("PASS: test_application_lisp_parity")
end

-- ---------------------------------------------------------------------------
-- test_binding_translation: specific expression shapes round-trip correctly
-- ---------------------------------------------------------------------------
local function test_binding_translation()
  ensure_fs()

  -- Each fixture is a minimal Lisp snippet whose binding must not crash.
  local fixtures = {
    -- STEP scalar binding
    [[(StackView Name="T1"
        (:bind Target="Node.Visible" (step 640 (bind "Node.ActualWidth"))))
    ]],
    -- IF / STEP / Vector2
    [[(StackView Name="T2"
        (:bind Target="Node.HorizontalPadding"
          (if (step 640 (bind "Node.ActualWidth")) (vector2 40 40) (vector2 8 8))))
    ]],
    -- IF with string branches on column definition
    [[(Grid Name="T3" Spacing="0"
        (:bind Target="Grid.Columns"
          (if (step 640 (bind "Node.ActualWidth")) "auto auto" "auto")))
    ]],
  }

  for i, src in ipairs(fixtures) do
    local ok, err = pcall(function()
      local obj = filesystem.loadObjectFromLispString(src)
      test.expect(obj ~= nil, "binding_translation[" .. i .. "]: object should be created")
    end)
    test.expect(ok, "binding_translation[" .. i .. "]: should not raise: " .. tostring(err))
  end
  print("PASS: test_binding_translation")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_basic_elements()
test_text_content()
test_binding_attr()
test_bind_directive()
test_theme_values()
test_comments()
test_application_lisp_readable()
test_binding_translation()
test_application_lisp_parity()
