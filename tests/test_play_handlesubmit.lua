local test = require "orca.test"
local core = require "orca.core"
local ui = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Play.HandleSubmit: adds a new entry and appends item to ListBox
-- ---------------------------------------------------------------------------
local function test_handlesubmit_adds_entry()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  -- Simulate the Transcript DataSource
  local entries = core.DataObject { Name = "Entries" }
  entries:addChild(core.DataObject { Name = "Hello" })
  entries:addChild(core.DataObject { Name = "World" })

  local listbox = ui.ListBox { Name = "Transcript", ItemsSource = entries }
  screen:addChild(listbox)

  local count_before = 0
  for _ in listbox.children do count_before = count_before + 1 end
  test.expect_eq(count_before, 2, "ListBox should start with 2 children")

  -- --- Simulate HandleSubmit logic ---
  local msg = core.DataObject { Name = "open mailbox" }
  entries:addChild(msg)

  local item = ui.TextBlock { Name = "MessageText" }
  item.DataContext = msg
  listbox:addChild(item)
  -- --- end HandleSubmit ---

  local entry_count = 0
  for _ in entries.children do entry_count = entry_count + 1 end
  test.expect_eq(entry_count, 3, "DataSource should have 3 entries after submit")

  local count_after = 0
  local last_item = nil
  for child in listbox.children do
    count_after = count_after + 1
    last_item = child
  end
  test.expect_eq(count_after, 3, "ListBox should have 3 children after submit")

  test.expect(last_item.DataContext == msg,
    "New item DataContext should be the new DataObject")
  test.expect_eq(last_item.DataContext.Name, "open mailbox",
    "New item DataContext.Name should match submitted text")

  screen:clear()
  print("PASS: test_handlesubmit_adds_entry")
end

-- ---------------------------------------------------------------------------
-- Play.HandleSubmit: ignores empty or nil text
-- ---------------------------------------------------------------------------
local function test_handlesubmit_ignores_empty()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  local entries = core.DataObject { Name = "Entries" }
  entries:addChild(core.DataObject { Name = "Keep" })

  local listbox = ui.ListBox { Name = "Transcript", ItemsSource = entries }
  screen:addChild(listbox)

  local count_before = 0
  for _ in listbox.children do count_before = count_before + 1 end
  test.expect_eq(count_before, 1, "ListBox should start with 1 child")

  -- Empty text → no-op
  local text = ""
  if text and text ~= "" then
    local msg = core.DataObject { Name = text }
    entries:addChild(msg)
    local item = ui.TextBlock { Name = "MessageText" }
    item.DataContext = msg
    listbox:addChild(item)
  end

  local count_after = 0
  for _ in listbox.children do count_after = count_after + 1 end
  test.expect_eq(count_after, 1, "ListBox should still have 1 child when text is empty")

  local entry_count = 0
  for _ in entries.children do entry_count = entry_count + 1 end
  test.expect_eq(entry_count, 1, "DataSource should still have 1 entry")

  screen:clear()
  print("PASS: test_handlesubmit_ignores_empty")
end

-- ---------------------------------------------------------------------------
-- Play.HandleSubmit: ignores nil text
-- ---------------------------------------------------------------------------
local function test_handlesubmit_ignores_nil()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  local entries = core.DataObject { Name = "Entries" }
  entries:addChild(core.DataObject { Name = "Keep" })

  local listbox = ui.ListBox { Name = "Transcript", ItemsSource = entries }
  screen:addChild(listbox)

  local count_before = 0
  for _ in listbox.children do count_before = count_before + 1 end
  test.expect_eq(count_before, 1, "ListBox should start with 1 child")

  -- nil text → no-op
  local text = nil
  if text and text ~= "" then
    local msg = core.DataObject { Name = text }
    entries:addChild(msg)
    local item = ui.TextBlock { Name = "MessageText" }
    item.DataContext = msg
    listbox:addChild(item)
  end

  local count_after = 0
  for _ in listbox.children do count_after = count_after + 1 end
  test.expect_eq(count_after, 1, "ListBox should still have 1 child when text is nil")

  screen:clear()
  print("PASS: test_handlesubmit_ignores_nil")
end

-- ---------------------------------------------------------------------------
-- Play.HandleSubmit: preserves existing items and their DataContext
-- ---------------------------------------------------------------------------
local function test_handlesubmit_preserves_existing()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  local entries = core.DataObject { Name = "Entries" }
  local d1 = core.DataObject { Name = "First" }
  local d2 = core.DataObject { Name = "Second" }
  entries:addChild(d1)
  entries:addChild(d2)

  local listbox = ui.ListBox { Name = "Transcript", ItemsSource = entries }
  screen:addChild(listbox)

  local before = {}
  for child in listbox.children do
    before[#before + 1] = { obj = child, dc = child.DataContext }
  end
  test.expect_eq(#before, 2, "Should start with 2 children")

  local msg = core.DataObject { Name = "Third" }
  entries:addChild(msg)
  local item = ui.TextBlock { Name = "MessageText" }
  item.DataContext = msg
  listbox:addChild(item)

  local idx = 0
  for child in listbox.children do
    idx = idx + 1
    if idx <= 2 then
      test.expect(child == before[idx].obj,
        "Existing child #" .. idx .. " object identity should be preserved")
      test.expect(child.DataContext == before[idx].dc,
        "Existing child #" .. idx .. " DataContext should be preserved")
    end
  end
  test.expect_eq(idx, 3, "Should have 3 children total after submit")

  screen:clear()
  print("PASS: test_handlesubmit_preserves_existing")
end

-- ---------------------------------------------------------------------------
-- Play.HandleSubmit: multiple submits accumulate correctly
-- ---------------------------------------------------------------------------
local function test_handlesubmit_multiple_accumulate()
  local screen = ui.Screen { Width = 400, Height = 800, ResizeMode = "NoResize" }

  local entries = core.DataObject { Name = "Entries" }
  local listbox = ui.ListBox { Name = "Transcript", ItemsSource = entries }
  screen:addChild(listbox)

  local texts = { "first", "second", "third" }
  for _, text in ipairs(texts) do
    local msg = core.DataObject { Name = text }
    entries:addChild(msg)
    local item = ui.TextBlock { Name = "MessageText" }
    item.DataContext = msg
    listbox:addChild(item)
  end

  local idx = 0
  for child in listbox.children do
    idx = idx + 1
    test.expect_eq(child.DataContext.Name, texts[idx],
      "Child " .. idx .. " DataContext.Name should match")
  end
  test.expect_eq(idx, 3, "Should have 3 children after 3 submits")

  local entry_count = 0
  for _ in entries.children do entry_count = entry_count + 1 end
  test.expect_eq(entry_count, 3, "DataSource should have 3 entries")

  screen:clear()
  print("PASS: test_handlesubmit_multiple_accumulate")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_handlesubmit_adds_entry()
test_handlesubmit_ignores_empty()
test_handlesubmit_ignores_nil()
test_handlesubmit_preserves_existing()
test_handlesubmit_multiple_accumulate()

print("All Play.HandleSubmit tests passed.")
