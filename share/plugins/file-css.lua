-- Mapping from CSS property names to ORCA component property names.
-- Keys are lowercase CSS names; values are fully-qualified ORCA names
-- (Class.Property) to disambiguate across component namespaces.
local css_property_map = {
  -- Visual / background
  ["background-color"]  = "Node2D.BackgroundColor",
  ["color"]             = "Node2D.ForegroundColor",
  ["opacity"]           = "Node.Opacity",
  -- Box model
  ["width"]             = "Node.Width",
  ["height"]            = "Node.Height",
  ["min-width"]         = "Node.MinWidth",
  ["min-height"]        = "Node.MinHeight",
  ["margin"]            = "Node.Margin",
  ["margin-top"]        = "Node.MarginTop",
  ["margin-right"]      = "Node.MarginRight",
  ["margin-bottom"]     = "Node.MarginBottom",
  ["margin-left"]       = "Node.MarginLeft",
  ["padding"]           = "Node.Padding",
  ["padding-top"]       = "Node.PaddingTop",
  ["padding-right"]     = "Node.PaddingRight",
  ["padding-bottom"]    = "Node.PaddingBottom",
  ["padding-left"]      = "Node.PaddingLeft",
  -- Border
  ["border"]            = "Node.Border",
  ["border-color"]      = "Node.BorderColor",
  ["border-width"]      = "Node.BorderWidth",
  -- Typography (TextRun / TextBlockConcept)
  ["font-size"]         = "TextRun.FontSize",
  ["font-family"]       = "TextRun.FontFamily",
  ["line-height"]       = "TextRun.LineHeight",
  ["letter-spacing"]    = "TextRun.LetterSpacing",
  ["word-wrap"]         = "TextBlockConcept.WordWrap",
  ["text-overflow"]     = "TextBlockConcept.TextOverflow",
  -- Visibility
  ["visibility"]        = "Node.Visible",
}

-- example CSS input:
-- .button, .link { color: red; font-size: 14px; }
-- .card { background-color: white; padding: 8px;
--   &:hover { background-color: grey; }
--   & .title { font-weight: bold; }
-- }
-- would parse to:
-- out = {
--   [".button"]      = { color="red", ["font-size"]="14px" },
--   [".link"]        = { color="red", ["font-size"]="14px" },  -- same table
--   [".card"]        = { ["background-color"]="white", padding="8px" },
--   [".card:hover"]  = { ["background-color"]="grey" },
--   [".card .title"] = { ["font-weight"]="bold" },
-- }

local function flatten_css(css)
  -- expand .parent { & .child { ... } } into .parent .child { ... }
  local changed = true
  while changed do
    changed = false
    css = css:gsub("([^{}-][^{]-)%s*{([^{}]-)&([^{}]-)(%b{})}", function(parent, before, sel_suffix, block)
      changed = true
      local inner = block:sub(2, -2)  -- strip { }
      return before .. "\n" .. parent:match("^%s*(.-)%s*$") .. sel_suffix:match("^%s*(.-)%s*$") .. " " .. "{" .. inner .. "}"
    end)
  end
  return css
end

local function css_parse(css)
  local result = {}

  css = css:gsub("/%*.-%*/", "")  -- strip comments
  css = flatten_css(css)

  for sel_raw, block in css:gmatch("([^{]+){([^}]*)}") do
    local props = {}

    for decl in block:gmatch("[^;]+") do
      local key, val = decl:match("^%s*([%-%w]+)%s*:%s*(.-)%s*$")
      if key and key ~= "" then
        props[key] = val
      end
    end

    for sel in (sel_raw .. ","):gmatch("%s*(.-)%s*,") do
      if sel ~= "" then
        result[sel] = props
      end
    end
  end

  return result
end

-- Build and return a core.StyleSheet from a parsed CSS table.
-- Each top-level selector (e.g. ".button", ".card:hover") becomes a StyleRule
-- child on the sheet, with property overrides attached so that ThemeChanged
-- can apply them via a direct binary copy — no string re-parsing.
local function css_to_stylesheet(parsed)
  local core = require "orca.core"
  local sheet = core.StyleSheet()
  for selector, props in pairs(parsed) do
    local rule = sheet + core.StyleRule { selector = selector }
    for k, v in pairs(props) do
      local prop_name = css_property_map[k:lower()]
      assert(prop_name, "Unsupported CSS property: " .. k)
      rule[prop_name] = v
    end
  end
  return sheet
end

table.insert(package.searchers, function(path)
	local filesystem = require "orca.filesystem"
	local contents = filesystem.readTextFile(path..'.css')
	return contents and function() return css_to_stylesheet(css_parse(contents)) end or nil
end)
