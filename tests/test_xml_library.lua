local test = require "orca.test"
-- Headless tests for XML libraries: one file holding many named objects.
-- filesystem.loadObject("Dir/Name") falls back to the entry Name="Name" of
-- Dir.xml when Dir/Name.xml does not exist.
--
-- Run with: $(TARGET) -test=tests/test_xml_library.lua

local filesystem = require "orca.filesystem"
require "orca.UIKit"

local LIB = "tests/fixtures/library/Widgets"

local function test_loads_entry_by_name()
  local panel = filesystem.loadObject(LIB .. "/Panel")
  test.expect(panel ~= nil, "library entry should load")
  test.expect_eq(panel.className, "Node2D", "entry keeps its element class")
  test.expect_eq(panel.Name, "Panel", "entry keeps its name")
  test.expect_eq(panel.Width, 40, "entry attributes become properties")
  test.expect_eq(panel.source_file, LIB .. "/Panel", "SourceFile is the path that loads the entry again")

  local label = filesystem.loadObject(LIB .. "/Label")
  test.expect_eq(label.className, "TextBlock", "entries may have different classes")
  print("PASS: test_loads_entry_by_name")
end

local function test_each_load_is_a_fresh_object()
  local a = filesystem.loadObject(LIB .. "/Panel")
  local b = filesystem.loadObject(LIB .. "/Panel")
  test.expect(a ~= b, "an entry loads as a new object each time, like a standalone file")
  print("PASS: test_each_load_is_a_fresh_object")
end

local function test_standalone_file_wins()
  local o = filesystem.loadObject(LIB .. "/Override")
  test.expect_eq(o.Width, 2, "Dir/Name.xml takes precedence over the entry in Dir.xml")
  print("PASS: test_standalone_file_wins")
end

local function test_missing_entry()
  test.expect(filesystem.loadObject(LIB .. "/Missing") == nil, "unknown entry loads as nil")
  test.expect(filesystem.loadObject("tests/fixtures/library/NoSuchLibrary/Panel") == nil,
              "entry of a missing library loads as nil")
  print("PASS: test_missing_entry")
end

test_loads_entry_by_name()
test_each_load_is_a_fresh_object()
test_standalone_file_wins()
test_missing_entry()
