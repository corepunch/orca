local test = require "orca.test"
-- MarkdownView: Markdown source rebuilt into TextBlocks with Run children.

require "orca.renderer"
local ui = require "orca.UIKit"

local function kids(node)
  local list = {}
  for c in node.children do list[#list + 1] = c end
  return list
end

-- Each generated class gets a distinct opacity, so a node's opacity names it.
local KINDS = {
  ["md-h1"] = 0.11, ["md-h2"] = 0.12, ["md-p"] = 0.2, ["md-quote"] = 0.3, ["md-ul"] = 0.41,
  ["md-ol"] = 0.42, ["md-li"] = 0.5, ["md-marker"] = 0.6, ["md-li-body"] = 0.7, ["md-pre"] = 0.8,
  ["md-code-block"] = 0.81, ["md-hr"] = 0.9,
}
local css = { ".md-code { font-size: 11; }" }
for name, value in pairs(KINDS) do css[#css + 1] = "." .. name .. " { opacity: " .. value .. "; }" end
local SHEET = table.concat(css, "\n")

local function kind(node)
  for name, value in pairs(KINDS) do
    if math.abs(node.Opacity - value) < 0.001 then return name end
  end
  return "?"
end

-- The runs of a TextBlock as "text|flags": bold, italic, underline, strike, code.
local function runs(block)
  local list = {}
  for _, r in ipairs(kids(block)) do
    list[#list + 1] = r.Text .. "|" .. (r.FontWeight == "Bold" and "b" or "") .. (r.FontStyle == "Italic" and "i" or "")
      .. (r.TextDecorationType == "Underline" and "u" or "") .. (r.TextDecorationType == "Strikethrough" and "s" or "")
      .. (r.FontSize == 11 and "c" or "")
  end
  return table.concat(list, " ~ ")
end

-- Source is set once the view is on screen, as a binding would.
local function render(source)
  local screen = ui.Screen { Width = 400, Height = 2000, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString(SHEET)
  local md = screen + ui.MarkdownView {}
  md.Source = source
  screen:UpdateLayout(screen.Width, screen.Height)
  return md, screen
end

local function test_blocks()
  local md = render(table.concat({
    "# Title",
    "",
    "A *soft*",
    "wrapped **line**.",
    "",
    "Sub",
    "---",
    "",
    "> quoted",
    "lazy",
    "",
    "- one",
    "- two",
    "  - nested",
    "",
    "3. three",
    "4. four",
    "",
    "```lua",
    "local x = 1",
    "  indented",
    "```",
    "",
    "***",
    "#hashtag stays text",
  }, "\n"))
  local b = kids(md)
  local kinds = {}
  for i, n in ipairs(b) do kinds[i] = kind(n) end
  test.expect_eq(table.concat(kinds, ","),
    "md-h1,md-p,md-h2,md-quote,md-ul,md-ol,md-pre,md-hr,md-p", "block kinds in order")
  test.expect_eq(runs(b[1]), "Title|b", "heading runs inherit bold")
  test.expect_eq(b[1].FontWeight, "Bold", "headings are bold")
  test.expect_eq(runs(b[2]), "A | ~ soft|i ~  wrapped | ~ line|b ~ .|", "soft break becomes a space")
  test.expect_eq(runs(kids(b[4])[1]), "quoted lazy|", "lazy line continues the quote")

  local items = kids(b[5])
  test.expect_eq(#items, 2, "two bullet items")
  local marker, body = table.unpack(kids(items[2]))
  test.expect_eq(kind(items[2]) .. "," .. kind(marker) .. "," .. kind(body), "md-li,md-marker,md-li-body", "item parts")
  test.expect_eq(marker.Text, "•", "bullet marker")
  test.expect_eq(kind(kids(body)[2]), "md-ul", "indented item nests a list")
  test.expect_eq(kids(kids(b[6])[1])[1].Text, "3.", "ordered list keeps its start number")
  test.expect_eq(kids(kids(b[6])[2])[1].Text, "4.", "ordered list counts on")
  local code = kids(b[7])[1]
  test.expect_eq(kind(code), "md-code-block", "code text sits in the pre box")
  test.expect_eq(code.Text, "local x = 1\n  indented", "code block keeps lines and indentation")
  test.expect_eq(runs(b[9]), "#hashtag stays text|", "a hash without a space is not a heading")
  print("PASS: test_blocks")
end

local function test_inline()
  local md = render("Use `a*b`, [the *site*](http://x.test) and https://orca.test/p. ~~old~~ ***both***\\\n"
    .. "next  \nlast snake_case_name \\*lit\\*")
  test.expect_eq(runs(kids(md)[1]), table.concat({
    "Use |", "a*b|c", ", |", "the |u", "site|iu", " and |", "https://orca.test/p|u", ". |",
    "old|s", " |", "both|bi", "\nnext\nlast snake_case_name *lit*|",
  }, " ~ "), "inline spans")
  print("PASS: test_inline")
end

local function test_rebuild_and_layout()
  local md, screen = render("short")
  local height = kids(md)[1].ActualHeight
  test.expect(height > 0, "paragraph has height")
  md.Source = "short\n\nsecond paragraph that is long enough to wrap across the four hundred point screen width"
  screen:UpdateLayout(screen.Width, screen.Height)
  local b = kids(md)
  test.expect_eq(#b, 2, "rebuilt on Source change")
  test.expect(b[2].ActualHeight >= height * 2 and b[2].ActualWidth <= screen.Width, "long paragraph wraps")
  md.Source = ""
  test.expect_eq(#kids(md), 0, "empty source clears")
  print("PASS: test_rebuild_and_layout")
end

local function test_css_font_size()
  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  screen.StyleSheet = ui.loadObjectFromCssString ".md-h1 { font-size: 40; } .md-p { font-size: 12; }"
  local md = screen + ui.MarkdownView {}
  md.Source = "# Big\n\nsmall"
  screen:UpdateLayout(screen.Width, screen.Height)
  local h1, p = table.unpack(kids(md))
  test.expect_eq(h1.FontSize, 40, "heading class styled from CSS")
  test.expect(h1.ActualHeight > p.ActualHeight * 2, "heading runs inherit the block's size")
  print("PASS: test_css_font_size")
end

test_blocks()
test_inline()
test_rebuild_and_layout()
test_css_font_size()
